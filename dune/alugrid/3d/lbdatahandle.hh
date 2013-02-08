#ifndef DUNE_ALUGRID_LBDATAHANDLE_HH
#define DUNE_ALUGRID_LBDATAHANDLE_HH

#include <dune/alugrid/3d/datahandle.hh>

#include <dune/grid/common/datahandleif.hh>
#include <dune/common/bartonnackmanifcheck.hh>

namespace Dune
{

  template <class LoadBalanceHandleImpl>
  class LoadBalanceHandleIF 
  {
  protected:  
    // one should not create an explicit instance of this inteface object
    LoadBalanceHandleIF() {} 

  public:
    bool userDefinedPartitioning () const 
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().userDefinedPartitioning()));
      return asImp().userDefinedPartitioning();
    }
    // return true if user defined load balancing weights are provided
    bool userDefinedLoadWeights () const 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().userDefinedPartitioning()));
      return asImp().userDefinedLoadWeights();
    }
    // returns true if user defined partitioning needs to be readjusted 
    bool repartition () const 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().repartition()));
      return asImp().repartition();
    }
    // return load weight of given element 
    template <class Entity>
    int loadWeight( const Entity &element ) const 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().loadWeight(element)));
      return asImp().loadWeight( element );
    }
    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    template <class Entity>
    int destination( const Entity &element ) const 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().destination(element)));
      return asImp().destination( element );
    }

  private:
    //!  Barton-Nackman trick 
    LoadBalanceHandleImpl& asImp () {return static_cast<LoadBalanceHandleImpl &> (*this);}
    //!  Barton-Nackman trick 
    const LoadBalanceHandleImpl& asImp () const 
    {
      return static_cast<const LoadBalanceHandleImpl &>(*this);
    }
  }; // end class LoadBalanceDataHandleIF 


  template< class Grid, class DataHandleImpl, class Data >
  class ALUGridLoadBalanceDataHandle1
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

  private:
    const Grid &grid_;
    DataHandle &dataHandle_;

  public:
    ALUGridLoadBalanceDataHandle1 ( const Grid &grid, DataHandle &dataHandle )
    : grid_( grid ),
      dataHandle_( dataHandle )
    {}

    void inlineData ( ObjectStream &stream, const Element &element ) const
    {
      inlineElementData( stream, element );

      const int maxLevel = grid_.maxLevel();
      const HierarchicIterator end = element.hend( maxLevel );
      for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
        inlineElementData( stream, *it );
    }

    void xtractData ( ObjectStream &stream, const Element &element, size_t newElements )
    {
      xtractElementData( stream, element );

      const int maxLevel = grid_.maxLevel();
      const HierarchicIterator end = element.hend( maxLevel );
      for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
        xtractElementData( stream, *it );
    }

    // return true if user defined partitioning methods should be used 
    bool userDefinedPartitioning () const 
    {
      return false;
    }
    // return true if user defined load balancing weights are provided
    bool userDefinedLoadWeights () const 
    { 
      return false;
    }
    // returns true if user defined partitioning needs to be readjusted 
    bool repartition () const 
    { 
      return false;
    }
    // return load weight of given element 
    int loadWeight( const Element &element ) const 
    { 
      return 1;
    }
    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    int destination( const Element &element ) const 
    { 
      return -1;
    }

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
  class ALUGridLoadBalanceHandle2
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
  private:
    const Grid &grid_;
    LoadBalanceHandle &ldbHandle_;

  public:
    ALUGridLoadBalanceHandle2 ( const Grid &grid, LoadBalanceHandle &loadBalanceHandle )
    : grid_( grid ),
      ldbHandle_( loadBalanceHandle )
    {}

    void inlineData ( ObjectStream &stream, const Element &element ) const
    {
    }
    void xtractData ( ObjectStream &stream, const Element &element, size_t newElements )
    {
    }

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
    void compress () 
    {
    } 
  };

  template< class Grid, class LoadBalanceHandleImpl, class DataHandleImpl >
  class ALUGridLoadBalanceDataHandle 
  {
    typedef typename Grid :: Traits :: HierarchicIterator HierarchicIterator;
  public:
    typedef typename Grid :: ObjectStreamType ObjectStream;

    template< int codim >
    struct Codim
    {
      typedef typename Grid :: Traits :: template Codim< codim > :: Entity Entity;
      typedef typename Grid :: Traits :: template Codim< codim > :: EntityPointer
        EntityPointer;
    };
    static const int dimension = Grid :: dimension;
    typedef typename DataHandleImpl::Entity Element;

  private:
    const Grid &grid_;
    LoadBalanceHandleImpl &ldbHandle_;
    DataHandleImpl &dataHandle_;

  public:
    ALUGridLoadBalanceDataHandle ( const Grid &grid, LoadBalanceHandleImpl &ldbHandle, DataHandleImpl &dataHandle )
    : grid_(grid),
      ldbHandle_(ldbHandle),
      dataHandle_(dataHandle)
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
      return ldbHandle_.loadWeight( element );
    }
    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    int destination( const Element &element ) const 
    { 
      return ldbHandle_.destination( element );
    }

    void inlineData ( ObjectStream &stream, const Element &element ) const
    {
      inlineElementData( stream, element );

      const int maxLevel = grid_.maxLevel();
      const HierarchicIterator end = element.hend( maxLevel );
      for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
        inlineElementData( stream, *it );
    }

    void xtractData ( ObjectStream &stream, const Element &element, size_t newElements )
    {
      xtractElementData( stream, element );

      const int maxLevel = grid_.maxLevel();
      const HierarchicIterator end = element.hend( maxLevel );
      for( HierarchicIterator it = element.hbegin( maxLevel ); it != end; ++it )
        xtractElementData( stream, *it );
    }

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

#undef CHECK_INTERFACE_IMPLEMENTATION
#undef CHECK_AND_CALL_INTERFACE_IMPLEMENTATION
}

#endif
