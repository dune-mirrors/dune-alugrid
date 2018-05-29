#define DISABLE_DEPRECATED_METHOD_CHECK 1
//#define DUNE_GRID_CHECK_USE_DEPRECATED_ENTITY_AND_INTERSECTION_INTERFACE 1

#include <config.h>

#ifndef NDEBUG
#ifndef DUNE_DEVEL_MODE
#define DUNE_DEVEL_MODE
#endif
#define DUNE_INTERFACECHECK
#endif

// #define NO_2D
// #define NO_3D

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/version.hh>
#include <dune/common/tupleutility.hh>
//#include <dune/common/tuples.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/geometry/referenceelements.hh>
#include <dune/geometry/type.hh>

#include <dune/grid/io/file/dgfparser/dgfwriter.hh>

//#include "checktwists.cc"

#include <dune/alugrid/dgf.hh>

#if ALU3DGRID_PARALLEL && HAVE_MPI
#define USE_PARALLEL_TEST 1
#endif


int main (int argc , char **argv) {

  // this method calls MPI_Init, if MPI is enabled
  //Dune::MPIHelper &mpihelper =
  Dune::MPIHelper::instance( argc, argv );
  //int myrank = mpihelper.rank();
  //int mysize = mpihelper.size();

  try {
    using Grid = Dune::ALUGrid<2, 2, Dune::ALUGridElementType::simplex, Dune::ALUGridRefinementType::nonconforming>;

    Dune::GridFactory<Grid> factory;
    Dune::GeometryType dSimplex = Dune::GeometryTypes::simplex(2);

    factory.insertVertex({0, 0});
    factory.insertVertex({1, 0});
    factory.insertVertex({1, 1});
    factory.insertVertex({0, 1});
    factory.insertElement(dSimplex, {0, 1, 2});
    factory.insertElement(dSimplex, {0, 2, 3});

    std::unique_ptr< Grid > grid ( factory.createGrid() );
    grid->globalRefine(1);

    using HierarchicIterator = typename Grid::HierarchicIterator;
    auto end = grid->levelView(0).template begin<0>()->hend(1);
    HierarchicIterator end_copy(end); // works

    auto begin = grid->levelView(0).template begin<0>()->hbegin(1);
    while(begin != end) ++begin;
    HierarchicIterator end_from_begin_copy(begin); // doesn't work
  }
  catch( Dune::Exception &e )
  {
    std::cerr << e << std::endl;
    return 1;
  }
  catch( ... )
  {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
}
