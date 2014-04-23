#ifndef DUNE_ALU2DGRIDINDEXSETS_HH
#define DUNE_ALU2DGRIDINDEXSETS_HH

#include <vector>

#include <dune/common/stdstreams.hh>
#include <dune/common/bigunsignedint.hh>

#include <dune/grid/common/grid.hh>
#include <dune/grid/common/indexidset.hh>

#include <dune/alugrid/2d/alu2dinclude.hh>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< int dim, int dimworld, ALU2DSPACE ElementType eltype >
  class ALU2dGrid;

  template<int cd, int dim, class GridImp>
  class ALU2dGridEntity;



  // ALU2dGridHierarchicIndexSet
  // ---------------------------

  //! hierarchic index set of ALU2dGrid 
  template <int dim, int dimworld, ALU2DSPACE ElementType eltype> 
  class ALU2dGridHierarchicIndexSet : 
    public IndexSet< ALU2dGrid< dim, dimworld, eltype >, 
                     ALU2dGridHierarchicIndexSet< dim, dimworld, eltype >, int >
  {
    typedef ALU2dGridHierarchicIndexSet< dim, dimworld, eltype > This;

    typedef ALU2dGrid< dim, dimworld, eltype > GridType;
    enum { numCodim = dim+1 }; // i.e. 3 

    friend class ALU2dGrid< dim, dimworld, eltype >;

    ALU2dGridHierarchicIndexSet( const GridType &grid )
    : grid_( grid )
    {}
        
  public:
    //! return hierarchic index of given entity
    template< int codim >
    int index ( const typename GridType::Traits::template Codim< codim >::Entity &entity ) const
    {
      return GridType::getRealImplementation( entity ).getIndex();
    }

    //! return hierarchic index of given entity
    template< class Entity >
    int index ( const Entity &entity ) const
    {
      return GridType::getRealImplementation( entity ).getIndex();
    }

    //! return subIndex of given entity
    template< class Entity >
    int subIndex ( const Entity &entity, int i, unsigned int codim ) const
    {
      return subIndex< Entity::codimension >( entity, i, codim );
    }

    //! return subIndex of given entity
    template< int cd >
    int subIndex ( const typename GridType::Traits::template Codim< cd >::Entity &entity, int i, unsigned int codim ) const
    {
      return GridType::getRealImplementation( entity ).subIndex( i, codim );
    }

    //! return size of indexset, i.e. maxindex+1
    //! for given type, if type is not exisiting within grid 0 is returned 
    int size ( GeometryType type ) const
    {
      const int codim = dim-type.dim();      
      alugrid_assert ( grid_.geomTypes(codim).size() == 1 );
      if( type != grid_.geomTypes(codim)[0] ) return 0;
      // return size of hierarchic index set
      return grid_.hierSetSize(codim);
    }

    //! return size of indexset, i.e. maxindex+1
    int size ( int codim ) const
    {
      // return size of hierarchic index set
      return grid_.hierSetSize(codim);
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return grid_.geomTypes(codim);
    }

    //! return true because all entities are contained in this set 
    template <class EntityType>
    bool contains (const EntityType &) const { return true; }

  private:
    // our Grid 
    const GridType & grid_;
  };

  //***********************************************************
  //
  //  --LocalIdSet 
  //
  //***********************************************************

  //! hierarchic index set of ALU3dGrid 
  template <int dim, int dimworld, ALU2DSPACE ElementType eltype> 
  class ALU2dGridLocalIdSet : 
    public IdSet < ALU2dGrid< dim, dimworld, eltype >,
                   ALU2dGridLocalIdSet< dim, dimworld, eltype >, int > 
  {
    typedef ALU2dGrid< dim, dimworld, eltype > GridType;
    typedef typename GridType :: HierarchicIndexSet HierarchicIndexSetType;

    friend class ALU2dGrid< dim, dimworld, eltype >;

    // this means that only up to 300000000 entities are allowed 
    enum { codimMultiplier = 300000000 };
 
    // create local id set , only for the grid allowed 
    ALU2dGridLocalIdSet(const GridType & grid) : hset_(grid.hierarchicIndexSet()) 
    {
      for(int i=0; i<dim+1; i++)
        codimStart_[i] = i*codimMultiplier; 
    }

    // fake method to have the same method like GlobalIdSet 
    void updateIdSet() {}

  public:
    //! export type of id 
    typedef int IdType;

    //! return global id of given entity
    template <class EntityType>
    int id (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      alugrid_assert ( hset_.size(cd) < codimMultiplier );
      return codimStart_[cd] + hset_.index(ep);
    }

    //! return global id of given entity
    template <int codim>
    int id (const typename GridType:: template Codim<codim> :: Entity & ep) const
    {
      //enum { cd = EntityType :: codimension };
      alugrid_assert ( hset_.size(codim) < codimMultiplier );
      return codimStart_[codim] + hset_.index(ep);
    }

    //! return subId of given entity
    template< class Entity >
    IdType subId ( const Entity &e, int i, unsigned int codim ) const
    {
      return subId< Entity::codimension >( e, i, codim );
    }

    //! return subId of given entity
    template< int cd >
    IdType subId ( const typename GridType::template Codim< cd >::Entity &e, int i, unsigned int codim ) const
    {
      assert( hset_.size( codim ) < codimMultiplier );
      return codim*codimMultiplier + hset_.subIndex( e, i, codim );
    }

  private:
    // our HierarchicIndexSet 
    const HierarchicIndexSetType & hset_;

    // store start of each codim numbers 
    int codimStart_[dim+1]; 
  };

} // end namespace Dune 

#endif
