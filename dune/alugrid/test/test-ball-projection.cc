#include <config.h>

// iostream includes
#include <iostream>

// grid includes
#include <dune/alugrid/grid.hh>
#include <dune/alugrid/dgf.hh>

#include <dune/grid/common/partitionset.hh>
#include <dune/grid/common/rangegenerators.hh>

#include <dune/grid/io/file/vtk/vtkwriter.hh>


//#include <dune/grid/albertagrid.hh>
//#include <dune/grid/albertagrid/dgfparser.hh>

template <class HGridType >
void algorithm ( HGridType &grid, const int step, const bool writeVTK = false )
{
  int n = 0;
  double volume = 0;

  const auto gridView = grid.leafGridView();

  for( const auto& entity : Dune::elements( gridView, Dune::Partitions::interior ) )
  {
    volume += entity.geometry().volume();
    ++n;
  }

  volume = gridView.comm().sum( volume );
  n = gridView.comm().sum( n );

  if( gridView.comm().rank() == 0 )
  {
    std::cout << "level: " << step
              << " elements: " << n
              << " volume: " << volume
              << " error: " << std::abs( volume - 4.0 * M_PI / 3.0 )
              << std::endl;
  }

  // Write VTK
  std::ostringstream vtkName;
  vtkName << "test-ball-ref-" << step;
  Dune::VTKWriter<typename HGridType::LeafGridView> vtkWriter( gridView );
  vtkWriter.write( vtkName.str() );

}

// main
// ----

int main ( int argc, char **argv )
try
{
  Dune::MPIHelper &mpihelper = Dune::MPIHelper::instance( argc, argv );

  // create grid from DGF file
  const std::string gridFile = "dgf/ball.dgf";

  {
    // type of hierarchical grid
    typedef Dune :: ALUGrid< 3, 3, Dune::simplex, Dune::conforming > HGridType;

    std::cout << "P[ " << mpihelper.rank() << " ]:  Dune :: ALUGrid< 3, 3, Dune::simplex, Dune::conforming >" << std::endl;

    // the method rank and size from MPIManager are static
    std::cout << "P[ " << mpihelper.rank() << " ]:  Loading macro bulk grid: " << gridFile << std::endl;

    // construct macro using the DGF Parser
    Dune::GridPtr< HGridType > gridPtr( gridFile );
    HGridType& grid = *gridPtr ;

    // do initial load balance
    grid.loadBalance();

    const int refineStepsForHalf = Dune::DGFGridInfo< HGridType >::refineStepsForHalf();

    for( int step = 0; step < 5; ++step )
    {
      // refine globally such that grid with is bisected
      // and all memory is adjusted correctly
      grid.globalRefine( refineStepsForHalf );

      algorithm( grid, step );
    }
  }
#if 0
  {
    typedef Dune::AlbertaGrid< 3 > HGridType;
    std::cout << "Dune :: AlbertaGrid< 3 >" << std::endl;

    // the method rank and size from MPIManager are static
    std::cout << "Loading macro bulk grid: " << gridFile << std::endl;

    // construct macro using the DGF Parser
    Dune::GridPtr< HGridType > gridPtr( gridFile );
    HGridType& grid = *gridPtr ;

    // do initial load balance
    grid.loadBalance();

    const int refineStepsForHalf = Dune::DGFGridInfo< HGridType >::refineStepsForHalf();

    for( int step = 0; step <= 5; ++step )
    {
      // refine globally such that grid with is bisected
      // and all memory is adjusted correctly
      grid.globalRefine( refineStepsForHalf );

      algorithm( grid, step );
    }
  }
#endif

  return 0;
}
catch( const Dune::Exception &exception )
{
  std::cerr << "Error: " << exception << std::endl;
  return 1;
}
