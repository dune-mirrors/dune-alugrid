#ifndef DUNE_ALUGRID_LBDATAHANDLE_HH
#define DUNE_ALUGRID_LBDATAHANDLE_HH

#include <dune/alugrid/3d/datahandle.hh>

#include <dune/common/bartonnackmanifcheck.hh>
#include <dune/grid/common/datahandleif.hh>

namespace Dune
{
  struct LoadBalanceHandleWithReserveAndCompress {};

  template< class Grid, class DataHandleImpl, class Data >
  class ALUGridDataHandleWrapper
  {
    typedef typename Grid :: Traits :: HierarchicIterator HierarchicIterator;

  public:
    typedef typename Grid :: ObjectStreamType ObjectStream;

    typedef CommDataHandleIF< DataHandleImpl, Data > DataHandle;

    static const int dimension = Grid :: dimension;

    template< int codim >
    struct Codim
    {
      typedef typename Grid :: Traits :: template Codim< codim > :: Entity Entity;
      typedef typename Grid :: Traits :: template Codim< codim > :: EntityPointer
        EntityPointer;
    };

    typedef typename Codim< 0 > :: Entity Element;
  protected:
    template <class DH, bool>
    struct CompressAndReserve
    {
      static DataHandleImpl& asImp( DH& dh ) { return static_cast<DataHandleImpl &> (dh); }

      static void reserveMemory( DH& dataHandle, const size_t newElements )
      {
        asImp( dataHandle ).reserveMemory( newElements );
      }
      static void compress( DH& dataHandle )
      {
        asImp( dataHandle ).compress();
      }
    };

    template <class DH>
    struct CompressAndReserve< DH, false >
    {
      static void reserveMemory( DH& dataHandle, const size_t newElements ) {}
      static void compress( DH& dataHandle ) {}
    };

    // check whether DataHandleImpl is derived from LoadBalanceHandleWithReserveAndCompress
    static const bool hasCompressAndReserve =  Conversion< DataHandleImpl,
                      LoadBalanceHandleWithReserveAndCompress >::exists ;
    // don't transmit size in case we have special DataHandleImpl
    static const bool transmitSize = ! hasCompressAndReserve ;

    typedef CompressAndReserve< DataHandle, hasCompressAndReserve >  CompressAndReserveType;

  protected:
    const Grid &grid_;
    DataHandle &dataHandle_;

  public:
    ALUGridDataHandleWrapper( const Grid &grid, DataHandle &dataHandle )
    : grid_( grid ),
      dataHandle_( dataHandle )
    {
    }

    //! write data to object stream 
    void inlineData ( ObjectStream &stream, const Element &element ) const
    {
      inlineElementData( stream, element );

      const int maxLevel = grid_.maxLevel();
      const HierarchicIterator end = element.hend( maxLevel );
      for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
        inlineElementData( stream, *it );
    }

    //! read data from object stream 
    void xtractData ( ObjectStream &stream, const Element &element, size_t newElements )
    {
      // if data handle provides reserve feature, reserve memory
      // the data handle has to be derived from LoadBalanceHandleWithReserveAndCompress 
      CompressAndReserveType :: reserveMemory( dataHandle_, newElements );

      xtractElementData( stream, element );

      const int maxLevel = grid_.maxLevel();
      const HierarchicIterator end = element.hend( maxLevel );
      for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
        xtractElementData( stream, *it );
    }

    //! \brief data compress
    //! \note nothing is done since the DataHandleIF does not provide this feature
    void compress ()
    {
      // if data handle provides compress, do compress here
      // the data handle has to be derived from LoadBalanceHandleWithReserveAndCompress 
      CompressAndReserveType :: compress( dataHandle_ );
    }

  private:
    void inlineElementData ( ObjectStream &stream, const Element &element ) const
    {
      // call element data direct without creating entity pointer
      if( dataHandle_.contains( dimension, 0 ) )
      {
        inlineEntityData<0>( stream, element ); 
      }

      // now call all higher codims 
      inlineCodimData< 1 >( stream, element );
      inlineCodimData< 2 >( stream, element );
      inlineCodimData< 3 >( stream, element );
    }

    void xtractElementData ( ObjectStream &stream, const Element &element )
    {
      // call element data direct without creating entity pointer
      if( dataHandle_.contains( dimension, 0 ) )
      {
        xtractEntityData<0>( stream, element ); 
      }

      // now call all higher codims 
      xtractCodimData< 1 >( stream, element );
      xtractCodimData< 2 >( stream, element );
      xtractCodimData< 3 >( stream, element );
    }

    template< int codim >
    void inlineCodimData ( ObjectStream &stream, const Element &element ) const
    {
      typedef typename Codim< codim > :: EntityPointer EntityPointer;

      if( dataHandle_.contains( dimension, codim ) )
      {
        const int numSubEntities = element.template count< codim >();
        for( int i = 0; i < numSubEntities; ++i )
        {
          const  EntityPointer pEntity = element.template subEntity< codim >( i );
          inlineEntityData< codim >( stream, *pEntity );
        }
      }
    }

    template< int codim >
    void xtractCodimData ( ObjectStream &stream, const Element &element )
    {
      typedef typename Codim< codim > :: EntityPointer EntityPointer;

      if( dataHandle_.contains( dimension, codim ) )
      {
        const int numSubEntities = element.template count< codim >();
        for( int i = 0; i < numSubEntities; ++i )
        {
          const  EntityPointer pEntity = element.template subEntity< codim >( i );
          xtractEntityData< codim >( stream, *pEntity );
        }
      }
    }

    template< int codim >
    void inlineEntityData ( ObjectStream &stream,
                            const typename Codim< codim > :: Entity &entity ) const
    {
      if( transmitSize ) 
      {
        const size_t size = dataHandle_.size( entity );
        stream.write( size );
      }
      dataHandle_.gather( stream, entity );
    }

    template< int codim >
    void xtractEntityData ( ObjectStream &stream,
                            const typename Codim< codim > :: Entity &entity )
    {
      size_t size = 0;
      if( transmitSize ) 
      {
        stream.read( size );
      }
      dataHandle_.scatter( stream, entity, size );
    }

  public:
    bool userDefinedPartitioning () const 
    {
      return false;
    }
    bool repartition () const 
    { 
      return false;
    }
    bool userDefinedLoadWeights () const
    {
      return false;
    }
    double loadWeight( const Element &element ) const 
    { 
      return -1;
    }
    int destination( const Element &element ) const 
    { 
      return -1;
    }
  };


  template< class Grid, class LoadBalanceHandle, bool internal >
  class ALUGridLoadBalanceHandleWrapper
  {
  public:
    template< int codim >
    struct Codim
    {
      typedef typename Grid :: Traits :: template Codim< codim > :: Entity Entity;
      typedef typename Grid :: Traits :: template Codim< codim > :: EntityPointer
        EntityPointer;
    };

    typedef typename Codim< 0 > :: Entity Element;
  protected:
    const Grid &grid_;
    LoadBalanceHandle &ldbHandle_;

  public:
    ALUGridLoadBalanceHandleWrapper ( const Grid &grid, LoadBalanceHandle &loadBalanceHandle )
    : grid_( grid ),
      ldbHandle_( loadBalanceHandle )
    {}

    // return true if user defined partitioning methods should be used 
    bool userDefinedPartitioning () const 
    {
      std::cout << "lbdatahandle.hh:userDefinedPartitioning " << !internal << std::endl;
      return !internal;
    }
    // returns true if user defined partitioning needs to be readjusted 
    bool repartition () const 
    { 
      return true;
    }

    // return true if user defined weights have been provided
    bool userDefinedLoadWeights () const
    {
      return internal;
    }
    // return load weight of given element 
    double loadWeight( const Element &element ) const 
    { 
      assert( internal );
      return ldbHandle_(element);
    }
    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    int destination( const Element &element ) const 
    { 
      std::cout << "lbdatahandle.hh:destination" << std::endl;
      assert( !internal );
      return ldbHandle_(element);
    }
  };

  template< class Grid, class LoadBalanceHandle, class DataHandleImpl, class Data, bool internal >
  class ALUGridLoadBalanceDataHandleWrapper 
    : public ALUGridLoadBalanceHandleWrapper< Grid, LoadBalanceHandle, internal >,
      public ALUGridDataHandleWrapper< Grid, DataHandleImpl, Data >
  {
    typedef ALUGridLoadBalanceHandleWrapper< Grid, LoadBalanceHandle, internal >  LDBHandleBase ;
    typedef ALUGridDataHandleWrapper< Grid, DataHandleImpl, Data >                DataHandleBase ;

  public:
    typedef typename DataHandleBase :: DataHandle        DataHandle ;
    typedef typename LDBHandleBase::Element Element;

    ALUGridLoadBalanceDataHandleWrapper( const Grid &grid, 
                                         LoadBalanceHandle &ldbHandle, 
                                         DataHandle &dataHandle )
    : LDBHandleBase( grid, ldbHandle ),
      DataHandleBase( grid, dataHandle )
    {}

#if 0
    // methods inherited from ALUGridLoadBalanceHandleWrapper 
    using LDBHandleBase :: loadWeight ;
    using LDBHandleBase :: destination ; 
#endif
    int loadWeight( const Element &element ) const 
    { return LDBHandleBase::loadWeight(element); }
    int destination( const Element &element ) const 
    { return LDBHandleBase::destination(element); }

    // methods inherited from ALUGridDataHandleWrapper
    using DataHandleBase :: inlineData ;
    using DataHandleBase :: xtractData ;
    using DataHandleBase :: compress   ;
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_LBDATAHANDLE_HH
