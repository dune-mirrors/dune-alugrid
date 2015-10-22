#include <config.h>

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/grid/common/rangegenerators.hh>

#include <dune/alugrid/dgf.hh>
#include <dune/alugrid/grid.hh>


template< class GridType >
void checkAdaptivity ( GridType &grid, int steps )
{
  std::vector< std::size_t > refineSizes( steps+1 ), coarseSizes( steps+1 );

  refineSizes[ 0 ] = grid.leafGridView().size(0);
  for( int i =0; i < steps; ++i )
  {
    for( const auto & entity : Dune::elements( grid.leafGridView() ) )
      grid.mark( 1, entity );
    grid.preAdapt();
    grid.adapt();
    grid.postAdapt();

    refineSizes[ i + 1 ] = grid.leafGridView().size(0);
  }

  for( int i =0; i < steps; ++i )
  {
    for( const auto & entity : Dune::elements( grid.leafGridView() ) )
      grid.mark( -1, entity );
    grid.preAdapt();
    grid.adapt();
    grid.postAdapt();

    coarseSizes[ i ] = grid.leafGridView().size(0);
  }
  coarseSizes[ steps ] = grid.leafGridView().size(0);

  std::cout<<"Refinement Sizes: "<<std::endl;
  for( int size : refineSizes )
    std::cout<< size <<std::endl;

  std::cout<<"Coarsening Sizes: "<<std::endl;
  for( int size : coarseSizes )
    std::cout<< size <<std::endl;
}


int main (int argc , char **argv) {

  // this method calls MPI_Init, if MPI is enabled
  Dune::MPIHelper::instance( argc, argv );

  try {
    // 2-2 nonconform cube
    {
      typedef Dune::ALUGrid< 2, 2, Dune::cube, Dune::nonconforming > GridType;
      Dune::GridPtr<GridType> gridPtr( "./dgf/cube-testgrid-2-2.dgf" );
      GridType & grid = *gridPtr;
      grid.loadBalance();

      checkAdaptivity( grid, 3 );
    }

    // 2-2 nonconform
    {
      typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::nonconforming > GridType;
      Dune::GridPtr<GridType> gridPtr( "./dgf/simplex-testgrid-2-2.dgf" );
      GridType & grid = *gridPtr;
      grid.loadBalance();

      checkAdaptivity( grid, 3 );
    }

    // 2-2 conform
    {
      typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > GridType;
      Dune::GridPtr<GridType> gridPtr( "./dgf/simplex-testgrid-2-2.dgf" );
      GridType & grid = *gridPtr;
      grid.loadBalance();

      checkAdaptivity( grid, 3 );
    }

    // 3-3 nonconform cube
    {
      typedef Dune::ALUGrid< 3, 3, Dune::cube, Dune::nonconforming > GridType;
      Dune::GridPtr<GridType> gridPtr( "./dgf/simplex-testgrid-3-3.dgf" );
      GridType & grid = *gridPtr;
      grid.loadBalance();

      checkAdaptivity( grid, 3 );
    }

    // 3-3 nonconform
    {
      typedef Dune::ALUGrid< 3, 3, Dune::simplex, Dune::nonconforming > GridType;
      Dune::GridPtr< GridType > gridPtr( "./dgf/simplex-testgrid-3-3.dgf" );
      GridType & grid = *gridPtr;
      grid.loadBalance();

      checkAdaptivity( grid, 3 );
    }

    // 3-3 conform
    {
      typedef Dune::ALUGrid< 3, 3, Dune::simplex, Dune::conforming > GridType;
      Dune::GridPtr< GridType > gridPtr( "./dgf/simplex-testgrid-3-3.dgf" );
      GridType & grid = *gridPtr;
      grid.loadBalance();
      checkAdaptivity( grid, 3 );
    }

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
