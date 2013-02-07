#ifndef DUNE_ALUGRID_LBDATAHANDLE_HH
#define DUNE_ALUGRID_LBDATAHANDLE_HH

#include <dune/alugrid/3d/datahandle.hh>

#include <dune/grid/common/datahandleif.hh>
#include <dune/common/bartonnackmanifcheck.hh>

namespace Dune
{

  template< class Grid, class DataHandleImpl, class Data >
  class ALUGridLoadBalanceOldDataHandle
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
    ALUGridLoadBalanceOldDataHandle ( const Grid &grid, DataHandle &dataHandle )
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

  template <class DataHandleImpl, class DataTypeImpl>
  class LoadBalanceDataHandleIF : public CommDataHandleIF<DataHandleImpl, DataTypeImpl>
  {
  protected:  
    // one should not create an explicit instance of this inteface object
    LoadBalanceDataHandleIF() {} 

  public:
    //! data type of data to communicate 
    typedef DataTypeImpl DataType; 

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

    // from CommDataHandleIF
  public:
    bool contains (int dim, int codim) const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().contains(dim,codim)));
      return asImp().contains(dim,codim);
    }
    bool fixedsize (int dim, int codim) const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().fixedsize(dim,codim)));
      return asImp().fixedsize(dim,codim); 
    }
    template<class EntityType>
    size_t size (const EntityType& e) const
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().size(e)));
      return asImp().size(e);
    }
    template<class MessageBufferImp, class EntityType>
    void gather (MessageBufferImp& buff, const EntityType& e) const
    { 
      MessageBufferIF<MessageBufferImp> buffIF(buff);
    }
    template<class MessageBufferImp, class EntityType>
    void scatter (MessageBufferImp& buff, const EntityType& e, size_t n)
    { 
      MessageBufferIF<MessageBufferImp> buffIF(buff);
      CHECK_AND_CALL_INTERFACE_IMPLEMENTATION((asImp().scatter(buffIF,e,n)));
    }

  private:
    //!  Barton-Nackman trick 
    DataHandleImpl& asImp () {return static_cast<DataHandleImpl &> (*this);}
    //!  Barton-Nackman trick 
    const DataHandleImpl& asImp () const 
    {
      return static_cast<const DataHandleImpl &>(*this);
    }
  }; // end class LoadBalanceDataHandleIF 

  template< class Grid, class DataHandleImpl, class Data >
  class ALUGridLoadBalanceDataHandle : public ALUGridLoadBalanceOldDataHandle<Grid,DataHandleImpl,Data>
  {
  public:
    typedef LoadBalanceDataHandleIF< DataHandleImpl, Data > DataHandle;
    typedef CommDataHandleIF< DataHandleImpl, Data > CommDataHandle;
    typedef typename DataHandleImpl::Entity Entity;
    typedef ALUGridLoadBalanceOldDataHandle<Grid,DataHandleImpl,Data> Base;

  private:
    DataHandle &dataHandle_;

  public:
    ALUGridLoadBalanceDataHandle ( const Grid &grid, DataHandle &dataHandle )
    : Base(grid, (CommDataHandle&)(dataHandle) ),
      dataHandle_( dataHandle )
    {}

    // return true if user defined partitioning methods should be used 
    bool userDefinedPartitioning () const 
    {
      return dataHandle_.userDefinedPartitioning();
    }
    // return true if user defined load balancing weights are provided
    bool userDefinedLoadWeights () const 
    { 
      return dataHandle_.userDefinedLoadWeights();
    }
    // returns true if user defined partitioning needs to be readjusted 
    bool repartition () const 
    { 
      return dataHandle_.repartition();
    }
    // return load weight of given element 
    int loadWeight( const Entity &element ) const 
    { 
      return dataHandle_.loadWeight( element );
    }
    // return destination (i.e. rank) where the given element should be moved to 
    // this needs the methods userDefinedPartitioning to return true
    int destination( const Entity &element ) const 
    { 
      return dataHandle_.destination( element );
    }
  };

#undef CHECK_INTERFACE_IMPLEMENTATION
#undef CHECK_AND_CALL_INTERFACE_IMPLEMENTATION
}

#endif
