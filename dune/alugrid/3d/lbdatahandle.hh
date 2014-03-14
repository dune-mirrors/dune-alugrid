#ifndef DUNE_ALUGRID_LBDATAHANDLE_HH
#define DUNE_ALUGRID_LBDATAHANDLE_HH

#include <dune/alugrid/3d/datahandle.hh>

#include <dune/common/bartonnackmanifcheck.hh>
#include <dune/grid/common/datahandleif.hh>
#include <dune/alugrid/common/ldbhandleif.hh>

namespace Dune
{

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
      xtractElementData( stream, element );

      const int maxLevel = grid_.maxLevel();
      const HierarchicIterator end = element.hend( maxLevel );
      for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
        xtractElementData( stream, *it );
    }

    //! \brief data compress
    //! \Note nothing is done since the DataHandleIF does not provide this feature
    void compress ()
    {}

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
      const size_t size = dataHandle_.size( entity );
      stream.write( size );
      dataHandle_.gather( stream, entity );
    }

    template< int codim >
    void xtractEntityData ( ObjectStream &stream,
                            const typename Codim< codim > :: Entity &entity )
    {
      size_t size = 0;
      stream.read( size );
      dataHandle_.scatter( stream, entity, size );
    }
  };


  template< class Grid, class LoadBalanceHandleImpl >
  class ALUGridLoadBalanceHandleWrapper
  {
  public:
    typedef LoadBalanceHandleIF< LoadBalanceHandleImpl > LoadBalanceHandle;
    typedef typename Grid :: ObjectStreamType ObjectStream;
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
      return ldbHandle_.userDefinedPartitioning();
    }
    // return true if user defined load balancing weights are provided
    bool userDefinedLoadWeights () const 
    { 
      return ldbHandle_.userDefinedLoadWeights();
    }
    // returns true if user defined partitioning needs to be readjusted 
    bool repartition () const 
    { 
      return ldbHandle_.repartition();
    }
    // return load weight of given element 
    int loadWeight( const Element &element ) const 
    { 
      return ldbHandle_.loadWeight(element);
    }

    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    int destination( const Element &element ) const 
    { 
      return ldbHandle_.destination(element);
    }

    ///////////////////////////////////////////////////////////////
    //  dummy methods to fullfil the internal ALUGrid interface 
    ///////////////////////////////////////////////////////////////
    //! write data to object stream (no data written here)
    void inlineData ( ObjectStream &stream, const Element &element ) const {}
    //! read data from object stream (no data read here)
    void xtractData ( ObjectStream &stream, const Element &element, size_t newElements ) {}
    // compress (no compress here) 
    void compress () {} 
  };

  template< class Grid, class LoadBalanceHandleImpl, class DataHandleImpl, class Data >
  class ALUGridLoadBalanceDataHandleWrapper 
    : public ALUGridLoadBalanceHandleWrapper< Grid, LoadBalanceHandleImpl >,
      public ALUGridDataHandleWrapper< Grid, DataHandleImpl, Data >
  {
    typedef ALUGridLoadBalanceHandleWrapper< Grid, LoadBalanceHandleImpl >  LDBHandleBase ;
    typedef ALUGridDataHandleWrapper< Grid, DataHandleImpl, Data >          DataHandleBase ;

  public:
    typedef typename LDBHandleBase  :: LoadBalanceHandle LoadBalanceHandle;
    typedef typename DataHandleBase :: DataHandle        DataHandle ;

    ALUGridLoadBalanceDataHandleWrapper( const Grid &grid, 
                                         LoadBalanceHandleImpl &ldbHandle, 
                                         DataHandle &dataHandle )
    : LDBHandleBase( grid, ldbHandle ),
      DataHandleBase( grid, dataHandle )
    {}

    // methods inherited from ALUGridLoadBalanceHandleWrapper 
    using LDBHandleBase :: userDefinedPartitioning ;
    using LDBHandleBase :: userDefinedLoadWeights ;
    using LDBHandleBase :: repartition ;
    using LDBHandleBase :: loadWeight ;
    using LDBHandleBase :: destination ; 

    // methods inherited from ALUGridDataHandleWrapper
    using DataHandleBase :: inlineData ;
    using DataHandleBase :: xtractData ;
    using DataHandleBase :: compress   ;
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_LBDATAHANDLE_HH
