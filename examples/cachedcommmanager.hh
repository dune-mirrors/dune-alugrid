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

// only if ALUGrid found and was build for parallel runs
// if HAVE_DUNE_ALUGRID is not defined, ALU3DGRID_PARALLEL shouldn't be either
#if ALU3DGRID_PARALLEL

    /** \brief DependencyCache is a convenience class to build up a map
     * of all dofs of entities to be exchanged during a communication procedure.
     * This speeds up the communication procedure, because no grid traversal is
     * necessary anymore to exchange data. This class is singleton for different
     * discrete function spaces, depending on the BlockMapper.
     */
    template< class BlockMapper >
    class DependencyCache
    {
    public:
      //! type of block mapper of discrete function space (may be the same for
      //! different space (i.e. various DG spaces)
      typedef BlockMapper BlockMapperType;

      typedef std::size_t IndexType;

    protected:
      typedef std::vector< IndexType > IndexMapType;

      // type of IndexMapVector
      typedef std::vector< std::vector< IndexType > >  IndexMapVectorType;

      // type of set of links
      typedef std :: set< int > LinkStorageType;

      // ALUGrid send/recv buffers
      typedef ALU3DSPACE ObjectStream ObjectStreamType;

      // type of communicator
      typedef ALU3DSPACE MpAccessLocal MPAccessInterfaceType;
      // type of communication implementation
      typedef ALU3DSPACE MpAccessMPI MPAccessImplType;

      //! type of communication buffer vector
      typedef std :: vector< ObjectStreamType > ObjectStreamVectorType;

    protected:
      const InterfaceType interface_;
      const CommunicationDirection dir_;

      LinkStorageType linkStorage_;

      IndexMapVectorType  recvIndexMap_;
      IndexMapVectorType  sendIndexMap_;

      // ALUGrid communicator Class
      std::unique_ptr< MPAccessInterfaceType > mpAccess_;

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
      class LinkBuilder;

      /////////////////////////////////////////////////////////////////
      //  begin NonBlockingCommunication
      /////////////////////////////////////////////////////////////////

      class NonBlockingCommunication
      {
        typedef DependencyCache < BlockMapper > DependencyCacheType;

#if HAVE_DUNE_ALUGRID
        typedef MPAccessInterfaceType :: NonBlockingExchange NonBlockingExchange;

        template <class DiscreteFunction>
        class Pack : public NonBlockingExchange :: DataHandleIF
        {
        protected:
          NonBlockingCommunication& commObj_;
          const DiscreteFunction& discreteFunction_;

        public:
          Pack( NonBlockingCommunication& commObj, const DiscreteFunction& df )
          : commObj_( commObj ), discreteFunction_( df )
          {}

          void pack( const int link, ObjectStreamType& buffer )
          {
            commObj_.pack( link, buffer, discreteFunction_ );
          }

          void unpack( const int link, ObjectStreamType& buffer )
          {
            DUNE_THROW(InvalidStateException,"Pack::unpack should not be called!");
          }
        };

        template <class Vector, class Operation>
        class Unpack : public NonBlockingExchange :: DataHandleIF
        {
        protected:
          NonBlockingCommunication& commObj_;
          Vector& vector_;

          // communication operation (usually ADD or COPY)
          const Operation operation_;

        public:
          Unpack( NonBlockingCommunication& commObj, Vector& vec )
          : commObj_( commObj ), vector_( vec ), operation_()
          {}

          void pack( const int link, ObjectStreamType& buffer )
          {
            DUNE_THROW(InvalidStateException,"Unpack::pack should not be called!");
          }

          void unpack( const int link, ObjectStreamType& buffer )
          {
            commObj_.unpack( link, buffer, vector_, operation_ );
          }
        };
#else   // ALUGRID_HAS_NONBLOCKING_COMM is false
        typedef int NonBlockingExchange;
#endif

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
                                  DependencyCacheType& dependencyCache )
          : dependencyCache_( dependencyCache ),
            nonBlockingExchange_(),
            buffer_(),
            exchangeTime_( 0.0 ),
            mySize_( gridView.comm().size() )
        {
          // make sure cache is up2date
          dependencyCache_.rebuild( gridView );

          // notify dependency cache of open communication
          dependencyCache_.attachComm();
        }

        // copy constructor
        NonBlockingCommunication( const NonBlockingCommunication& other )
          : dependencyCache_( other.dependencyCache_ ),
            nonBlockingExchange_(),
            buffer_(),
            exchangeTime_( 0.0 ),
            mySize_( other.mySize_ )
        {
          // notify dependency cache of open communication
          dependencyCache_.attachComm();
        }

        ~NonBlockingCommunication()
        {
          // if this assertion fails some communication has not been finished
          assert( ! nonBlockingExchange_ );
          // notify dependency cache that comm is finished
          dependencyCache_.detachComm() ;
        }

        template < class DiscreteFunction >
        void send( const DiscreteFunction& discreteFunction )
        {
          // check that object is in non-sent state
          assert( ! nonBlockingExchange_ );

          // on serial runs: do nothing
          if( mySize_ <= 1 ) return;

          // take time
          Dune::Timer sendTimer ;

          // this variable can change during rebuild
          const int nLinks = dependencyCache_.nlinks();

          // resize buffer vector
          buffer_.resize( nLinks );

#if HAVE_DUNE_ALUGRID
          // get non-blocking exchange object from mpAccess including message tag
          nonBlockingExchange_.reset( dependencyCache_.mpAccess().nonBlockingExchange( getMessageTag() ) );

          // pack data object
          Pack< DiscreteFunction > packData( *this, discreteFunction );

          // perform send operation including packing of data
          nonBlockingExchange_->send( buffer_, packData );
#else
          // write buffers
          for( int link = 0; link < nLinks; ++link )
            pack( link, buffer_[ link ], discreteFunction );
#endif

          // store time needed for sending
          exchangeTime_ = sendTimer.elapsed();
        }

        //! receive data for discrete function and given operation
        template < class DiscreteFunction, class Operation >
        double receive( DiscreteFunction& discreteFunction, const Operation& operation )
        {
          // on serial runs: do nothing
          if( mySize_ <= 1 ) return 0.0;

          // take time
          Dune::Timer recvTimer ;

#if HAVE_DUNE_ALUGRID
          // unpack data object
          Unpack< DiscreteFunction, Operation > unpackData( *this, discreteFunction );

          // receive data and unpack
          nonBlockingExchange_->receive( unpackData );
#else
          // use exchange for older ALUGrid versions (send and receive)
          buffer_ = dependencyCache_.mpAccess().exchange( buffer_ );

          // this variable can change during rebuild
          const int nLinks = buffer_.size();

          // read buffers and store to discrete function
          for( int link = 0; link < nLinks; ++link )
            unpack( link, buffer_[ link ], discreteFunction, operation );
#endif

          // store time needed for sending
          exchangeTime_ += recvTimer.elapsed();

#if HAVE_DUNE_ALUGRID
          // clear nonBlockingExchange object
          nonBlockingExchange_.reset();
#endif
          return exchangeTime_;
        }

        //! receive method with default operation
        template < class DiscreteFunction >
        double receive( DiscreteFunction& discreteFunction )
        {
          // get type of default operation
          typedef typename DiscreteFunction :: DiscreteFunctionSpaceType
            :: template CommDataHandle< DiscreteFunction > :: OperationType  DefaultOperationType;
          DefaultOperationType operation;
          return receive( discreteFunction, operation );
        }

      protected:
        template <class DiscreteFunction>
        void pack( const int link, ObjectStreamType& buffer, const DiscreteFunction& discreteFunction )
        {
          // reset buffer counters
          buffer.clear();
          // write data of discrete function to message buffer
          dependencyCache_.writeBuffer( link, buffer, discreteFunction );
        }

        template <class DiscreteFunction, class Operation>
        void unpack( const int link, ObjectStreamType& buffer,
                     DiscreteFunction& discreteFunction, const Operation& operation )
        {
          // read data of discrete function from message buffer
          dependencyCache_.readBuffer( link, buffer, discreteFunction, operation );
        }

      protected:
        DependencyCacheType& dependencyCache_;
        std::unique_ptr< NonBlockingExchange > nonBlockingExchange_ ;
        ObjectStreamVectorType buffer_;
        double exchangeTime_ ;
        const int mySize_;
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
      DependencyCache( const int nProcs, const InterfaceType interface, const CommunicationDirection dir )
      : interface_( interface ),
        dir_( dir ),
        linkStorage_(),
        recvIndexMap_( nProcs ),
        sendIndexMap_( nProcs ),
        mpAccess_(),
        exchangeTime_( 0.0 ),
        buildTime_( 0.0 ),
        sequence_( -1 ),
        nonBlockingObjects_( 0 )
      {
      }

      template <class Communication>
      void init( const Communication& comm )
      {
        if( ! mpAccess_ )
        {
          mpAccess_.reset( new MPAccessImplType( comm ) );
        }
      }

      // no copying
      DependencyCache( const DependencyCache & ) = delete;

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

    protected:
      // build linkage and index maps
      template < class GridView >
      inline void buildMaps( const GridView& gv );

      // check consistency of maps
      inline void checkConsistency();

      template< class GridView, class Comm, class LS, class IMV, InterfaceType CI >
      inline void buildMaps( const GridView& gv, LinkBuilder< Comm, LS, IMV, CI > &handle );

    public:
      //! return MPI rank of link
      inline int dest( const int link ) const
      {
        return mpAccess().dest()[ link ];
      }

      //! return number of links
      inline int nlinks() const
      {
        return mpAccess().nlinks();
      }

      /** \brief Rebuild underlying exchange dof mapping.
       *  \note: Different spaces may have the same exchange dof mapping!
       */
      template <class GridView>
      inline void rebuild( const GridView& gridView )
      {
        const auto& comm = gridView.comm();

        // only in parallel we have to do something
        if( comm.size() <= 1 ) return;

        // make sure all non-blocking communications have been finished by now
        assert( noOpenCommunications() );

        // check whether grid has changed.
        //if( sequence_ != spcSequence )
        {
          // take timer needed for rebuild
          Dune::Timer buildTime;

          // rebuild maps holding exchange dof information
          buildMaps( gridView );
          // update sequence number
          //sequence_ = spcSequence;

          // store time needed
          buildTime_ = buildTime.elapsed();
        }
      }

      //! exchange data of discrete function
      template< class Space, class DiscreteFunction, class Operation >
      inline void exchange( const Space& space, DiscreteFunction &discreteFunction, const Operation& operation );

      //! write data of discrete function to buffer
      template< class DiscreteFunction >
      inline void writeBuffer( ObjectStreamVectorType &osv, const DiscreteFunction &discreteFunction ) const;

      //! read data of discrete function from buffer
      template< class DiscreteFunctionType, class Operation >
      inline void readBuffer( ObjectStreamVectorType &osv,
                              DiscreteFunctionType &discreteFunction,
                              const Operation& operation ) const;

      //! return reference to mpAccess object
      inline MPAccessInterfaceType &mpAccess()
      {
        assert( mpAccess_ );
        return *mpAccess_;
      }

      //! return reference to mpAccess object
      inline const MPAccessInterfaceType &mpAccess() const
      {
        assert( mpAccess_ );
        return *mpAccess_;
      }

    protected:
      // write data of DataImp& vector to object stream
      // --writeBuffer
      template< class Data >
      inline void writeBuffer( const int link,
                               ObjectStreamType &str,
                               const Data &data ) const
      {
        const auto &indexMap = sendIndexMap_[ dest( link ) ];
        const int size = indexMap.size();
        //typedef typename Data :: DofType DofType;
        typedef typename Data::value_type value_type;
        typedef typename value_type :: DofType DofType;


        // Dune::Fem::BlockVectorInterface and derived
        // if constexpr ( std::is_base_of< IsBlockVector, Data > :: value )
        {
          //static const int blockSize = Data::blockSize;
          static const int blockSize = value_type::blockSize;
          str.reserve( size * blockSize * sizeof( DofType ) );
          for( int i = 0; i < size; ++i )
          {
            const auto &block = data[ indexMap[ i ] ];
            for( int k=0; k<blockSize; ++k )
              str.writeUnchecked( block[ k ] );
          }
        }
      }

      // read data from object stream to DataImp& data vector
      // --readBuffer
      template< class Data, class Operation >
      inline void readBuffer( const int link,
                              ObjectStreamType &str,
                              Data &data,
                              const Operation& operation ) const
      {
        static_assert( ! std::is_pointer< Operation > :: value,
                       "DependencyCache::readBuffer: Operation needs to be a reference!");

        // get index map of rank belonging to link
        const auto &indexMap = recvIndexMap_[ dest( link ) ];
        const int size = indexMap.size();

        typedef typename Data::value_type value_type;
        typedef typename value_type :: DofType DofType;

        // Dune::Fem::BlockVectorInterface and derived
        // if constexpr ( std::is_base_of< IsBlockVector, Data > :: value )
        {
          //static const int blockSize = value_type::dimension;
          static const int blockSize = value_type::blockSize;
          assert( static_cast< std::size_t >( size * blockSize * sizeof( DofType ) ) <= static_cast< std::size_t >( str.size() ) );
          for( int i = 0; i < size; ++i )
          {
            auto &&block = data[ indexMap[ i ] ];
            for( int k=0; k<blockSize; ++k )
            {
              DofType value;
#if HAVE_DUNE_ALUGRID
              str.readUnchecked( value );
#else // #if HAVE_DUNE_ALUGRID
              str.read( value );
#endif // #else // #if HAVE_DUNE_ALUGRID
              // apply operation, i.e. COPY, ADD, etc.
              operation( value, block[ k ] );
            }
          }
        }
      }
    };

    // --LinkBuilder
    template< class BlockMapper >
    template< class Communication, class LinkStorage, class IndexMapVector, InterfaceType CommInterface >
    class DependencyCache< BlockMapper > :: LinkBuilder
    : public CommDataHandleIF
      < LinkBuilder< Communication, LinkStorage, IndexMapVector, CommInterface >,
                     std::size_t >
    {
    public:
      typedef Communication  CommunicationType;
      typedef BlockMapper    BlockMapperType;

      typedef std::size_t GlobalKeyType;

      typedef LinkStorage LinkStorageType;
      typedef IndexMapVector IndexMapVectorType;

      typedef GlobalKeyType DataType;

    protected:
      const CommunicationType& comm_;
      const BlockMapperType &blockMapper_;

      const GlobalKeyType myRank_;
      const GlobalKeyType mySize_;

      LinkStorageType &linkStorage_;

      IndexMapVectorType &sendIndexMap_;
      IndexMapVectorType &recvIndexMap_;


    public:
      LinkBuilder( const CommunicationType& comm,
                   const BlockMapperType& blockMapper,
                   LinkStorageType &linkStorage,
                   IndexMapVectorType &sendIdxMap,
                   IndexMapVectorType &recvIdxMap )
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
        // create ALU communicator
        MPAccessImplType mpAccess( comm_ );

        // build linkage
        mpAccess.removeLinkage();
        // insert new linkage
        mpAccess.insertRequestSymetric( linkStorage_ );
        // get destination ranks
        std::vector<int> dest = mpAccess.dest();
        // get number of links
        const int nlinks = mpAccess.nlinks();

        // create buffers
        ObjectStreamVectorType osv( nlinks );

        //////////////////////////////////////////////////////////////
        //
        //  at this point complete send maps exsist on receiving side,
        //  so send them back to sending side
        //
        //////////////////////////////////////////////////////////////

        // write all send maps to buffer
        for(int link=0; link<nlinks; ++link)
        {
          auto& indices = sendIndexMap_[ dest[link] ];
          const size_t idxSize = indices.size();
          auto& buffer = osv[link];
          buffer.write( idxSize );
          for(size_t i=0; i<idxSize; ++i)
          {
            buffer.write( indices[i] );
          }
        }

        // exchange data
        osv = mpAccess.exchange( osv );

        // read all send maps from buffer
        for(int link=0; link<nlinks; ++link)
        {
          auto& indices = sendIndexMap_[ dest[link] ];
          size_t idxSize;
          auto& buffer = osv[link];
          buffer.read( idxSize );
          indices.resize( idxSize );
          for(size_t i=0; i<idxSize; ++i)
          {
            buffer.read( indices[i] );
          }
        }
      }

    public:
      //! destructor
      ~LinkBuilder()
      {
        sendBackSendMaps();
      }

      //! returns true if combination is contained
      bool contains( int dim, int codim ) const
      {
        //return blockMapper_.contains( codim );
        return codim == 0 ;//blockMapper_.contains( codim );
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

          //const int numDofs = blockMapper_.numEntityDofs( entity );
          const int numDofs = 1;

          typedef std::vector< GlobalKeyType >  IndicesType ;
          IndicesType indices( numDofs );

          // copy all global keys
          //blockMapper_.mapEachEntityDof( entity, AssignFunctor< IndicesType >( indices ) );
          indices[ 0 ] = blockMapper_.index( entity );

          // write global keys to message buffer
          for( int i = 0; i < numDofs; ++i )
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
          typedef std::vector< GlobalKeyType >  IndicesType ;
          IndicesType indices( dataSize - 1 );
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
            //sendIndexMap_[ rank ].insert( indices );
            insert( sendIndexMap_[rank], indices );

            // build local mapping for receiving of dofs
            const int numDofs = 1 ;//blockMapper_.numEntityDofs( entity );
            indices.resize( numDofs );

            // map each entity dof and store in indices
            indices[ 0 ] = blockMapper_.index( entity );
            // blockMapper_.mapEachEntityDof( entity, AssignFunctor< IndicesType >( indices ) );
            //
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
        return (send) ? (2) : 0;
      }
    };



    template< class BlockMapper >
    template< class GridView >
    inline void DependencyCache< BlockMapper > :: buildMaps( const GridView& gv )
    {
      typedef typename GridView::CollectiveCommunication CommunicationType;
      if( interface_ == InteriorBorder_All_Interface )
      {
        LinkBuilder< CommunicationType, LinkStorageType, IndexMapVectorType,
                     InteriorBorder_All_Interface >
          handle( gv.comm(),
                  gv.indexSet(),
                  // TODO
                  //space.blockMapper(),
                  linkStorage_, sendIndexMap_, recvIndexMap_ );
        buildMaps( gv, handle );
      }
      else if( interface_ == InteriorBorder_InteriorBorder_Interface )
      {
        LinkBuilder< CommunicationType, LinkStorageType, IndexMapVectorType,
                     InteriorBorder_InteriorBorder_Interface >
          handle( gv.comm(),
                  // TODO
                  gv.indexSet(),
                  //space.blockMapper(),
                  linkStorage_, sendIndexMap_, recvIndexMap_ );
        buildMaps( gv, handle );
      }
      else if( interface_ == All_All_Interface )
      {
        LinkBuilder< CommunicationType, LinkStorageType, IndexMapVectorType, All_All_Interface >
          handle( gv.comm(),
                  // TDOD
                  //space.blockMapper(),
                  gv.indexSet(),
                  linkStorage_, sendIndexMap_, recvIndexMap_ );
        buildMaps( gv, handle );
      }
      else
        DUNE_THROW( NotImplemented, "DependencyCache for the given interface has not been implemented, yet." );
#ifndef NDEBUG
      // checks that sizes of index maps are equal on sending and receiving proc
      checkConsistency();
#endif
    }


    template< class BlockMapper >
    template< class GridView, class Comm, class LS, class IMV, InterfaceType CI >
    inline void DependencyCache< BlockMapper >
    :: buildMaps( const GridView& gv, LinkBuilder< Comm, LS, IMV, CI > &handle )
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

      // remove old linkage
      mpAccess().removeLinkage();
      // create new linkage
      mpAccess().insertRequestSymetric( linkStorage_ );
    }

    template< class BlockMapper >
    inline void DependencyCache< BlockMapper > :: checkConsistency()
    {
      const int nLinks = nlinks();

      ObjectStreamVectorType buffer( nLinks );

      // check that order and size are consistent
      for(int l=0; l<nLinks; ++l)
      {
        buffer[l].clear();
        const int sendSize = sendIndexMap_[ dest( l ) ].size();
        buffer[l].write( sendSize );
        for(int i=0; i<sendSize; ++i)
          buffer[l].write( i );
      }

      // exchange data to other procs
      buffer = mpAccess().exchange( buffer );

      // check that order and size are consistent
      for(int l=0; l<nLinks; ++l)
      {
        const int recvSize = recvIndexMap_[ dest( l ) ].size();
        int sendedSize;
        buffer[l].read( sendedSize );

        // compare sizes, must be the same
        if( recvSize != sendedSize )
        {
          DUNE_THROW(InvalidStateException,"Sizes do not match!" << sendedSize << " o|r " << recvSize);
        }

        for(int i=0; i<recvSize; ++i)
        {
          int idx;
          buffer[l].read( idx );

          // ordering should be the same on both sides
          if( i != idx )
          {
            DUNE_THROW(InvalidStateException,"Wrong ordering of send and recv maps!");
          }
        }
      }
    }

    template< class BlockMapper >
    template< class GridView, class Vector, class Operation >
    inline void DependencyCache< BlockMapper >
    :: exchange( const GridView& gv, Vector &vector, const Operation& operation )
    {
      // on serial runs: do nothing
      if( gv.comm().size() <= 1 ) return;

      // create non-blocking communication object
      NonBlockingCommunicationType nbc( gv, *this );

      // perform send operation
      nbc.send( vector );

      // store time for send and receive of data
      exchangeTime_ = nbc.receive( vector, operation );
    }

    template< class BlockMapper >
    template< class DiscreteFunction >
    inline void DependencyCache< BlockMapper >
    :: writeBuffer( ObjectStreamVectorType &osv,
                    const DiscreteFunction &discreteFunction ) const
    {
      const int numLinks = nlinks();
      for( int link = 0; link < numLinks; ++link )
        writeBuffer( link, osv[ link ], discreteFunction );
    }

    template< class BlockMapper >
    template< class DiscreteFunction, class Operation >
    inline void DependencyCache< BlockMapper >
    :: readBuffer( ObjectStreamVectorType &osv,
                   DiscreteFunction &discreteFunction,
                   const Operation& operation ) const
    {
      const int numLinks = nlinks();
      for( int link = 0; link < numLinks; ++link )
        readBuffer( link, osv[ link ], discreteFunction, operation );
    }

#endif  // #if ALU3DGRID_PARALLEL
    //@}

} // namespace Dune
#endif // #ifndef DUNE_FEM_CACHED_COMMUNICATION_MANAGER_HH
