#ifndef DUNE_ALU_PERSISTENTCONTAINER_HH
#define DUNE_ALU_PERSISTENTCONTAINER_HH

#include <dune/grid/utility/persistentcontainer.hh>
#include <dune/alugrid/grid.hh>

namespace Dune
{
  template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm,
            class Data, class Allocator >
  class PersistentContainer< ALUGrid< dim, dimworld, eltype, refinementtype, Comm >, Data, Allocator >
  : public PersistentContainerVector< ALUGrid< dim, dimworld, eltype, refinementtype, Comm >, 
                                      typename ALUGrid< dim, dimworld, eltype, refinementtype, Comm >::HierarchicIndexSet,
                                      std::vector<Data,Allocator> >
  {
  public:
    typedef ALUGrid< dim, dimworld, eltype, refinementtype, Comm > GridType;
  private:
    typedef PersistentContainerVector< GridType, typename GridType::HierarchicIndexSet, std::vector<Data,Allocator> > BaseType;

  public:
    //! Constructor filling the container with values using the default constructor 
    //! Depending on the implementation this could be achieved without allocating memory
    PersistentContainer ( const GridType &grid, const int codim, const Allocator &allocator = Allocator() )
    : BaseType( grid, codim, grid.hierarchicIndexSet(), 1.1, allocator )
    {}
  };

  template< int dim, int dimworld, ALU2DSPACE ElementType elType, class Data, class Allocator >
  class PersistentContainer< ALU2dGrid< dim, dimworld, elType >, Data, Allocator >
  : public PersistentContainerVector< ALU2dGrid< dim, dimworld, elType >, 
                                      typename ALU2dGrid< dim, dimworld, elType >::HierarchicIndexSet,
                                      std::vector<Data,Allocator> >
  {
  public:
    typedef ALU2dGrid< dim, dimworld, elType >  GridType;
  private:
    typedef PersistentContainerVector< GridType, typename GridType::HierarchicIndexSet, std::vector<Data,Allocator> > BaseType;

  public:
    //! Constructor filling the container with values using the default constructor 
    //! Depending on the implementation this could be achieved without allocating memory
    PersistentContainer ( const GridType &grid, const int codim, const Allocator &allocator = Allocator() )
    : BaseType( grid, codim, grid.hierarchicIndexSet(), 1.1, allocator )
    {}
  };

  template< ALU3dGridElementType elType, class Comm, class Data, class Allocator >
  class PersistentContainer< ALU3dGrid< elType, Comm >, Data, Allocator >
  : public PersistentContainerVector< ALU3dGrid< elType, Comm >, 
                                      typename ALU3dGrid< elType, Comm >::HierarchicIndexSet,
                                      std::vector<Data,Allocator> >
  {
  public:
    typedef ALU3dGrid< elType, Comm >  GridType;
  private:
    typedef PersistentContainerVector< GridType, typename GridType::HierarchicIndexSet, std::vector<Data,Allocator> > BaseType;

  protected:
    using BaseType :: index_; 
    using BaseType :: data_; 

  public:
    //! Constructor filling the container with values using the default constructor 
    //! Depending on the implementation this could be achieved without allocating memory
    PersistentContainer ( const GridType &grid, const int codim, const Allocator &allocator = Allocator() )
    : BaseType( grid, codim, grid.hierarchicIndexSet(), 1.1, allocator )
    {}

    //! this method is needed for the level communication 
    //! of ALU3dGrid, see datahandle.hh 
    const Data& getData ( const size_t idx ) const 
    {
      assert( idx < data_.size() );
      return data_[ idx ];
    }
  };

} // end namespace Dune

#endif // end DUNE_ALU_PERSISTENTCONTAINER_HH
