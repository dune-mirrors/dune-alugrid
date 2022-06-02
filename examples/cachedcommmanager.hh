#ifndef DUNE_CACHED_COMMUNICATION_MANAGER_HH
#define DUNE_CACHED_COMMUNICATION_MANAGER_HH

#include <cassert>
#include <cstddef>

//- system includes
#include <iostream>
#include <map>
#include <queue>
#include <memory>
#include <vector>

//- dune-common includes
#include <dune/common/math.hh>
#include <dune/common/timer.hh>
#include <dune/common/visibility.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpicommunication.hh>
#include <dune/common/parallel/mpipack.hh>
#include <dune/common/parallel/mpifuture.hh>

//- dune-grid includes
#include <dune/grid/common/grid.hh>
#include <dune/grid/common/datahandleif.hh>
#include <dune/grid/utility/entitycommhelper.hh>

// include alugrid headers to have to communicator class from ALUGrid
#if HAVE_DUNE_ALUGRID
#include <dune/alugrid/3d/alu3dinclude.hh>
#endif

namespace Dune
{

    /** @addtogroup Communication Communication
        @{
     **/

    /** \brief CommunicationPattern is a convenience class to build up a map
     * of all dofs of entities to be exchanged during a communication procedure.
     * This speeds up the communication procedure, because no grid traversal is
     * necessary anymore to exchange data. This class is singleton for different
     * discrete function spaces, depending on the BlockMapper.
     */
    template< class BlockMapper >
    class CommunicationPattern
    {
    public:
      //! type of block mapper of discrete function space (may be the same for
      //! different space (i.e. various DG spaces)
      typedef BlockMapper BlockMapperType;
      typedef typename BlockMapperType :: GlobalKeyType GlobalKeyType;

    protected:
      typedef std::vector< GlobalKeyType > IndexMapType;

      // type of IndexMapVector
      typedef std::map< int, std::vector< GlobalKeyType > >  IndexVectorMapType;

      // type of set of links
      typedef std :: set< int > LinkStorageType;

      typedef Dune::MPIHelper::MPICommunicator MPICommunicatorType;
      typedef Dune::Communication< MPICommunicatorType > CommunicationType;

    protected:
      const InterfaceType interface_;
      const CommunicationDirection dir_;

      LinkStorageType linkStorage_;

      IndexVectorMapType  recvIndexMap_;
      IndexVectorMapType  sendIndexMap_;

      // ALUGrid communicator Class
      std::unique_ptr< CommunicationType > comm_;

      // exchange time
      double exchangeTime_;
      // setup time
      double buildTime_;

      //! know grid sequence number
      int sequence_;

      int nonBlockingObjects_ ;

    protected:
      template< class Communication, class LinkStorage,
                class IndexMapVector, InterfaceType CommInterface >
      class PatternBuilder;

      /////////////////////////////////////////////////////////////////
      //  begin NonBlockingCommunication
      /////////////////////////////////////////////////////////////////

      class NonBlockingCommunication
      {
        typedef CommunicationPattern < BlockMapper > CommunicationPatternType;

        // create an unique tag for the communication
        DUNE_EXPORT static int getMessageTag()
        {
          enum { initial = 665 };
          static int tagCounter = initial ;
          ++ tagCounter;
          int messageTag = tagCounter ;

          // avoid overflow
          if( messageTag < 0 )
          {
            messageTag = initial ;
            tagCounter = initial ;
          }
          return messageTag;
        }

      public:
        template <class GV>
        NonBlockingCommunication( const GV& gridView,
                                  CommunicationPatternType& dependencyCache )
          : dependencyCache_( dependencyCache ),
            exchangeTime_( 0.0 ),
            mySize_( gridView.comm().size() ),
            tag_( getMessageTag() )
        {
          // notify dependency cache of open communication
          dependencyCache_.attachComm();
        }

        // copy constructor
        NonBlockingCommunication( const NonBlockingCommunication& other )
          : dependencyCache_( other.dependencyCache_ ),
            exchangeTime_( 0.0 ),
            mySize_( other.mySize_ ),
            tag_( other.tag_ )
        {
          // notify dependency cache of open communication
          dependencyCache_.attachComm();
        }

        ~NonBlockingCommunication()
        {
          // notify dependency cache that comm is finished
          dependencyCache_.detachComm() ;
        }

        template < class Data >
        void send( const Data& data )
        {
          // on serial runs: do nothing
          if( mySize_ <= 1 ) return;

          sendFutures_.clear();
          recvFutures_.clear();

          // take time
          Dune::Timer sendTimer ;

          const auto& comm = dependencyCache_.comm();

          auto& linkStorage = dependencyCache_.linkStorage();
          for( const auto& dest : linkStorage )
          {
            BufferType buffer( comm );
            pack( dest, buffer, data);
            sendFutures_.insert( std::make_pair( dest, comm.isend(std::move(buffer), dest, tag_) ) );
            //comm.send(buffer, dest, tag_);
          }

          // store time needed for sending
          exchangeTime_ = sendTimer.elapsed();
        }

        //! receive data for discrete function and given operation
        template < class Data, class Operation >
        double receive( Data& data, const Operation& operation )
        {
          // on serial runs: do nothing
          if( mySize_ <= 1 ) return 0.0;

          // take time
          Dune::Timer recvTimer ;

          typedef typename Data::value_type value_type;
          typedef typename value_type :: DofType DofType;

          static const int blockSize = value_type::blockSize;
          const size_t dataSize = static_cast< std::size_t >( blockSize * sizeof( DofType ) );

          const auto& comm = dependencyCache_.comm();
          for( const auto& source : dependencyCache_.linkStorage() )
          {
            const size_t bufSize = dependencyCache_.recvBufferSize( source ) * dataSize;
            recvFutures_.insert( std::make_pair( source, comm.irecv( BufferType(comm, bufSize), source, tag_ ) ) );
            //auto buffer = comm.rrecv( BufferType(comm), source, tag_ );
            //unpack( source, buffer, data, operation );
          }

          for( auto& [rank, future] : recvFutures_ )
          {
            auto buffer = future.get();
            unpack( rank, buffer, data, operation );
          }

          // wait for all sends to finish
          for( auto& [rank, future] : sendFutures_ )
          {
            if( !future.ready() )
              future.wait();
          }

          // store time needed for sending
          exchangeTime_ += recvTimer.elapsed();
          return exchangeTime_;
        }

        //! receive method with default operation
        template < class Data >
        double receive( Data& data )
        {
          // TODO
          // get type of default operation
          auto op = [](const double& a, double& b) { b = a; };
          return receive( data, op );
        }

      protected:
        template <class Buffer, class Data>
        void pack( const int rank, Buffer& buffer, const Data& data )
        {
          // write data of discrete function to message buffer
          dependencyCache_.writeBuffer( rank, buffer, data );
        }

        template <class Buffer, class Data, class Operation>
        void unpack( const int rank, Buffer& buffer,
                     Data& data, const Operation& operation )
        {
          // read data of discrete function from message buffer
          dependencyCache_.readBuffer( rank, buffer, data, operation );
        }

      protected:
        CommunicationPatternType& dependencyCache_;
        typedef MPIPack BufferType;
        typedef MPIFuture< BufferType > FutureType;

        std::map< int, FutureType > recvFutures_;
        std::map< int, FutureType > sendFutures_;

        double exchangeTime_ ;
        const int mySize_;
        const int tag_;
      };

    public:
      typedef NonBlockingCommunication NonBlockingCommunicationType;

      //! return object for non-blocking communication
      template <class Space>
      NonBlockingCommunicationType nonBlockingCommunication( const Space& space )
      {
        // create non-blocking communication object
        return NonBlockingCommunicationType( space, *this );
      }
      /////////////////////////////////////////////////////////////////
      //  end NonBlockingCommunication
      /////////////////////////////////////////////////////////////////

      //! constructor taking communicator object
      CommunicationPattern( const InterfaceType interface, const CommunicationDirection dir )
      : interface_( interface ),
        dir_( dir ),
        linkStorage_(),
        recvIndexMap_(),
        sendIndexMap_(),
        comm_(),
        exchangeTime_( 0.0 ),
        buildTime_( 0.0 ),
        sequence_( -1 ),
        nonBlockingObjects_( 0 )
      {
      }

      template <class Communication>
      void init( const Communication& comm )
      {
        if( ! comm_ )
        {
          comm_.reset( new CommunicationType( comm ) );
        }
      }

      const std::set< int >& linkStorage() const { return linkStorage_; }

      // no copying
      CommunicationPattern( const CommunicationPattern & ) = delete;

      //! return communication interface
      InterfaceType communicationInterface() const
      {
        return interface_;
      }

      //! return communication direction
      CommunicationDirection communicationDirection() const
      {
        return dir_;
      }

      //! return time needed for last build
      double buildTime() const
      {
        return buildTime_;
      }

      //! return time needed for last exchange
      double exchangeTime() const
      {
        return exchangeTime_;
      }

      // notify for open non-blocking communications
      void attachComm()
      {
        ++nonBlockingObjects_;
      }

      // notify for finished non-blocking communication
      void detachComm()
      {
        --nonBlockingObjects_;
        assert( nonBlockingObjects_ >= 0 );
      }

      bool noOpenCommunications() const
      {
        return true ;
      }

      size_t recvBufferSize( const int rank ) const
      {
        auto it = recvIndexMap_.find( rank );
        const auto &indexMap = it->second;
        return indexMap.size();
      }

    protected:
      // build linkage and index maps
      template < class GridView >
      inline void buildMaps( const GridView& gv, const BlockMapper& blockMapper );

      // check consistency of maps
      inline void checkConsistency();

      template< class GridView, class Comm, class LS, class IMV, InterfaceType CI >
      inline void buildMaps( const GridView& gv, PatternBuilder< Comm, LS, IMV, CI > &handle );

    public:
      /** \brief Rebuild underlying exchange dof mapping.
       *  \note: Different spaces may have the same exchange dof mapping!
       */
      template <class GridView>
      inline void rebuild( const GridView& gridView,
                           const BlockMapperType& blockMapper,
                           const bool force = true )
      {
        const auto& comm = gridView.comm();

        // only in parallel we have to do something
        if( comm.size() <= 1 ) return;

        // make sure all non-blocking communications have been finished by now
        assert( noOpenCommunications() );

        // check whether grid has changed.
        if( force )
        {
          // create communicator
          init( comm );

          // take timer needed for rebuild
          Dune::Timer buildTime;

          // rebuild maps holding exchange dof information
          buildMaps( gridView, blockMapper );
          // update sequence number
          //sequence_ = spcSequence;

          // store time needed
          buildTime_ = buildTime.elapsed();
        }
      }

      //! exchange data of discrete function
      template< class Space, class Data, class Operation >
      inline void exchange( const Space& space, Data &data, const Operation& operation );

      //! return reference to communication object
      inline CommunicationType &comm()
      {
        assert( comm_ );
        return *comm_;
      }

      //! return reference to communication object
      inline const CommunicationType &comm() const
      {
        assert( comm_ );
        return *comm_;
      }

    protected:
      // write data of DataImp& vector to object stream
      // --writeBuffer
      template< class Buffer, class Data >
      inline void writeBuffer( const int dest,
                               Buffer &buffer,
                               const Data &data ) const
      {
        auto it = sendIndexMap_.find( dest );
        const auto &indexMap = it->second;
        const int size = indexMap.size();

        //typedef typename Data :: DofType DofType;
        typedef typename Data::value_type value_type;
        typedef typename value_type :: DofType DofType;

        {
          //static const int blockSize = Data::blockSize;
          static const int blockSize = value_type::blockSize;
          buffer.enlarge( size * blockSize * sizeof( DofType ) );
          for( int i = 0; i < size; ++i )
          {
            const auto &block = data[ indexMap[ i ] ];
            for( int k=0; k<blockSize; ++k )
              buffer << block[ k ];
          }
        }
      }

      // read data from object stream to DataImp& data vector
      // --readBuffer
      template< class Buffer, class Data, class Operation >
      inline void readBuffer( const int source,
                              Buffer& buffer,
                              Data &data,
                              const Operation& operation ) const
      {
        static_assert( ! std::is_pointer< Operation > :: value,
                       "CommunicationPattern::readBuffer: Operation needs to be a reference!");

        // get index map of rank belonging to source
        auto it = recvIndexMap_.find( source );
        const auto &indexMap = it->second;

        const int size = indexMap.size();

        typedef typename Data::value_type value_type;
        typedef typename value_type :: DofType DofType;

        {
          //static const int blockSize = value_type::dimension;
          static const int blockSize = value_type::blockSize;
          assert( static_cast< std::size_t >( size * blockSize * sizeof( DofType ) ) <= static_cast< std::size_t >( (buffer.size()-buffer.tell()) ) );
          for( int i = 0; i < size; ++i )
          {
            auto &&block = data[ indexMap[ i ] ];
            for( int k=0; k<blockSize; ++k )
            {
              DofType value;
              buffer >> value;
              // apply operation, i.e. COPY, ADD, etc.
              operation( value, block[ k ] );
            }
          }
        }
      }
    };

    /** --PatternBuilder
     *
     *  BlockMapperInterface:
     *    bool contains( codim ) returns true if dofs are attached to entities with this codim
     *
     *    size_t numEntityDofs( entity ) -> return number of dofs located at this entity
     *
     *    void mapEntityDofs( entity, std::vector< GlobalKey >& indices ) which fills a vector with global keys (vector indices) of the dofs
     *
     */
    template< class BlockMapper >
    template< class Communication, class LinkStorage, class IndexVectorMap, InterfaceType CommInterface >
    class CommunicationPattern< BlockMapper > :: PatternBuilder
    : public CommDataHandleIF
      < PatternBuilder< Communication, LinkStorage, IndexVectorMap, CommInterface >,
                     std::size_t >
    {
    public:
      typedef Communication  CommunicationType;
      typedef BlockMapper    BlockMapperType;

      typedef typename BlockMapperType::GlobalKeyType GlobalKeyType;

      typedef LinkStorage LinkStorageType;
      typedef IndexVectorMap IndexVectorMapType;

      typedef GlobalKeyType DataType;

    protected:
      const CommunicationType& comm_;
      const BlockMapperType &blockMapper_;

      const GlobalKeyType myRank_;
      const GlobalKeyType mySize_;

      LinkStorageType &linkStorage_;

      IndexVectorMapType &sendIndexMap_;
      IndexVectorMapType &recvIndexMap_;


    public:
      PatternBuilder( const CommunicationType& comm,
                   const BlockMapperType& blockMapper,
                   LinkStorageType &linkStorage,
                   IndexVectorMapType &sendIdxMap,
                   IndexVectorMapType &recvIdxMap )
      : comm_( comm ),
        blockMapper_( blockMapper ),
        myRank_( comm.rank() ),
        mySize_( comm.size() ),
        linkStorage_( linkStorage ),
        sendIndexMap_( sendIdxMap ),
        recvIndexMap_( recvIdxMap )
      {}

    protected:
      void sendBackSendMaps()
      {
        typedef MPIPack BufferType;
        typedef MPIFuture< BufferType > FutureType;

        std::map< int, FutureType > recvFutures, sendFutures;

        for( const auto& dest : linkStorage_ )
        {
          BufferType buffer( comm_ );
          buffer << sendIndexMap_[ dest ];
          //sendFutures.insert( std::make_pair( dest, comm_.isend(std::move(buffer), dest, 123) ) );
          comm_.send(buffer, dest, 124);
        }

        for( const auto& source : linkStorage_ )
        {
          //recvFutures.insert( std::make_pair( source, comm_.irecv( BufferType(comm_), source, 123 ) ) );
          auto buffer = comm_.rrecv( BufferType(comm_), source, 124 );
          auto& indices = sendIndexMap_[ source ];
          buffer >> indices;
        }


        /*
        for( auto& [rank, future] : recvFutures )
        {
          auto buffer = future.get();
          auto& indices = recvIndexMap_[ rank ];
          buffer >> indices;
        }
        */

        /*
        // wait for all sends to finish
        for( auto& [rank, future] : sendFutures )
        {
          if( future.valid() )
          {
            if( !future.ready() )
              future.wait();
          }
        }
        */

      }

    public:
      //! destructor
      ~PatternBuilder()
      {
        sendBackSendMaps();
      }

      //! returns true if combination is contained
      bool contains( int dim, int codim ) const
      {
        return blockMapper_.contains( codim );
      }

      //! return whether we have a fixed size
      bool fixedSize( int dim, int codim ) const
      {
        return false;
      }

      //! read buffer and apply operation
      template< class MessageBuffer, class Entity >
      void gather( MessageBuffer &buffer, const Entity &entity ) const
      {
        // check whether we are a sending entity
        const auto myPartitionType = entity.partitionType();
        const bool send = EntityCommHelper< CommInterface > :: send( myPartitionType );

        // if we send data then send rank and dofs
        if( send )
        {
          // send rank for linkage
          buffer.write( myRank_ );

          const unsigned int numDofs = blockMapper_.numEntityDofs( entity );

          std::vector< GlobalKeyType > indices( numDofs );

          // copy all global keys
          blockMapper_.obtainEntityDofs( entity, indices );

          // write global keys to message buffer
          for( unsigned int i = 0; i < numDofs; ++i )
            buffer.write( indices[ i ] );
        }
      }

      //! read buffer and apply operation
      template< class MessageBuffer, class Entity >
      void scatter( MessageBuffer &buffer, const Entity &entity, const size_t dataSize )
      {
        // if data size > 0 then other side is sender
        if( dataSize > 0 )
        {
          // read rank of other side
          GlobalKeyType rank;
          buffer.read( rank );
          assert( (rank >= 0) && (rank < mySize_) );

          // check whether we are a sending entity
          const auto myPartitionType = entity.partitionType();
          const bool receive = EntityCommHelper< CommInterface > :: receive( myPartitionType );

          // insert rank of link into set of links
          linkStorage_.insert( rank );

          // read indices from stream
          std::vector< GlobalKeyType > indices( dataSize - 1 );
          for(size_t i=0; i<dataSize-1; ++i)
            buffer.read( indices[i] );

          // if we are a receiving entity
          if( receive )
          {
            //////////////////////////////////////////////////////////
            //
            // Problem here: sending and receiving order might differ
            // Solution: sort all dofs after receiving order and send
            // senders dofs back at the end
            //
            //////////////////////////////////////////////////////////

            // if data has been send and we are receive entity
            // then insert indices into send map of rank
            insert( sendIndexMap_[rank], indices );

            // resize vector
            indices.resize( blockMapper_.numEntityDofs( entity ) );

            // build local mapping for receiving of dofs
            // copy all global keys
            blockMapper_.obtainEntityDofs( entity, indices );

            insert( recvIndexMap_[rank], indices );
          }
        }
      }

      template <class Vector>
      void insert(Vector& idxMap, const Vector& indices)
      {
        {
          const size_t size = indices.size();
          size_t count = idxMap.size();

          // reserve memory
          idxMap.resize( count + size );
          assert( idxMap.size() == (count + size) );

          // copy indices to index vector
          for( size_t i = 0; i < size; ++i, ++count )
          {
            assert( indices[ i ] >= 0 );
            idxMap[ count ] = indices[ i ];
          }
        }
      }
      //! return local dof size to be communicated
      template< class Entity >
      size_t size( const Entity &entity ) const
      {
        const PartitionType myPartitionType = entity.partitionType();
        const bool send = EntityCommHelper< CommInterface > :: send( myPartitionType );
        return (send) ? (blockMapper_.numEntityDofs( entity ) + 1) : 0;
      }
    };



    template< class BlockMapper >
    template< class GridView >
    inline void CommunicationPattern< BlockMapper > :: buildMaps( const GridView& gv, const BlockMapper& blockMapper )
    {
      typedef typename GridView::CollectiveCommunication CommunicationType;
      if( interface_ == InteriorBorder_All_Interface )
      {
        PatternBuilder< CommunicationType, LinkStorageType, IndexVectorMapType,
                     InteriorBorder_All_Interface >
          handle( gv.comm(),
                  blockMapper,
                  linkStorage_, sendIndexMap_, recvIndexMap_ );
        buildMaps( gv, handle );
      }
      else if( interface_ == InteriorBorder_InteriorBorder_Interface )
      {
        PatternBuilder< CommunicationType, LinkStorageType, IndexVectorMapType,
                     InteriorBorder_InteriorBorder_Interface >
          handle( gv.comm(),
                  blockMapper,
                  linkStorage_, sendIndexMap_, recvIndexMap_ );
        buildMaps( gv, handle );
      }
      else if( interface_ == All_All_Interface )
      {
        PatternBuilder< CommunicationType, LinkStorageType, IndexVectorMapType, All_All_Interface >
          handle( gv.comm(),
                  blockMapper,
                  linkStorage_, sendIndexMap_, recvIndexMap_ );
        buildMaps( gv, handle );
      }
      else
        DUNE_THROW( NotImplemented, "CommunicationPattern for the given interface has not been implemented, yet." );
#ifndef NDEBUG
      // checks that sizes of index maps are equal on sending and receiving proc
      checkConsistency();
#endif
    }


    template< class BlockMapper >
    template< class GridView, class Comm, class LS, class IMV, InterfaceType CI >
    inline void CommunicationPattern< BlockMapper >
    :: buildMaps( const GridView& gv, PatternBuilder< Comm, LS, IMV, CI > &handle )
    {
      linkStorage_.clear();
      const size_t size = recvIndexMap_.size();
      for( size_t i = 0; i < size; ++i )
      {
        recvIndexMap_[ i ].clear();
        sendIndexMap_[ i ].clear();
      }

      // make one all to all communication to build up communication pattern
      gv.communicate( handle, All_All_Interface , ForwardCommunication );
    }

    template< class BlockMapper >
    inline void CommunicationPattern< BlockMapper > :: checkConsistency()
    {
    }

    template< class BlockMapper >
    template< class GridView, class Vector, class Operation >
    inline void CommunicationPattern< BlockMapper >
    :: exchange( const GridView& gv, Vector &vector, const Operation& operation )
    {
      // on serial runs: do nothing
      if( gv.comm().size() <= 1 ) return;

      // create non-blocking communication object
      NonBlockingCommunicationType nbc( gv, *this );

      // perform send operation
      nbc.send( vector );

      // store time for send and receive of data
      exchangeTime_ = nbc.receive( vector, operation);
    }
    //@}

} // namespace Dune
#endif // #ifndef DUNE_FEM_CACHED_COMMUNICATION_MANAGER_HH
