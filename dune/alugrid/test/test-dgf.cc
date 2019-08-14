// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "config.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/alugrid/grid.hh>
#include <dune/alugrid/dgf.hh>

#include <dune/grid/io/file/vtk/vtkwriter.hh>

using namespace Dune;

int main( int argc, char** argv )
try
{
  auto& mpiHelper = MPIHelper::instance( argc, argv );
  const int rank = mpiHelper.rank();

  std::string filename;
  if( argc > 1 )
  {
    filename = std::string(argv[1]);
  }
  else
  {
    std::cerr << "usage: " << argv[0] << " <filename>" << std::endl;
    return 0;
  }

#if 0
  using GridType = Dune::ALUGrid<2, 3, Dune::simplex, Dune::nonconforming>;
  Dune::GridPtr< GridType > gridPtr( filename );
  GridType& grid = *gridPtr;
  grid.loadBalance();

  if( rank == 0 )
  {
    for(const auto& elem : Dune::elements( grid.leafGridView() ) )
    {
      grid.mark( 1, elem );
    }
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();
  // grid.globalRefine( 5 );

  Dune::VTKWriter<typename GridType::LeafGridView> vtkWriter( grid.leafGridView());
  vtkWriter.write( "sphere-out" );
#else
  {
    using GridType = Dune::ALUGrid<3, 3, Dune::simplex, Dune::conforming>;
    Dune::GridPtr< GridType > gridPtr( filename );
    gridPtr.loadBalance();

    // grid is ready and load balanced at that point

    std::cout << "P[ " << rank << " ] parameters = " << gridPtr.nofParameters( 0 ) << std::endl;
    auto lvlView = gridPtr->levelGridView( 0 );
    const auto end = lvlView.end< 0 > ();
    for( auto it = lvlView.begin< 0 > (); it != end; ++it )
    {
      const auto& entity = *it;
      std::cout << "P[ " << rank << " ], entity " << lvlView.indexSet().index( entity );
      const auto& param  = gridPtr.parameters( entity );
      for( const auto& p : param )
      {
        std::cout << " " << p;
      }
      std::cout << std::endl;
    }
  }
#endif

  return 0;
}
catch ( Dune::Exception &e )
{
  std::cerr << e << std::endl;
  return 1;
}
catch (std::exception &e) {
  std::cerr << e.what() << std::endl;
  return 1;
}
catch ( ... )
{
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
