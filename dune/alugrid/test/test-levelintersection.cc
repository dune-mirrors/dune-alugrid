#define DISABLE_DEPRECATED_METHOD_CHECK 1

#include <config.h>


#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/tupleutility.hh>
#include <dune/common/tuples.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/geometry/referenceelements.hh>

#include <dune/grid/io/file/dgfparser/dgfwriter.hh>

#include <dune/grid/test/checkintersectionit.hh>

#include <dune/grid/common/rangegenerators.hh>

#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <dune/alugrid/dgf.hh>

#if ALU3DGRID_PARALLEL && HAVE_MPI
#define USE_PARALLEL_TEST 1
#endif


int main (int argc , char **argv) {

  // this method calls MPI_Init, if MPI is enabled
  Dune::MPIHelper &mpihelper = Dune::MPIHelper::instance( argc, argv );
  int myrank = mpihelper.rank();
  int mysize = mpihelper.size();

  try {
    /* use grid-file appropriate for dimensions */

    std::string key;
    bool initialize = true ;
    if( argc >= 2 )
    {
      key = argv[1];
      initialize = false;
    }
    else
    {
      std::cout << "usage:" << argv[0] << " <2d|2dsimp|2dcube|2dconf|3d|3dsimp|3dconf|3dcube> <display>" << std::endl;
    }

    const char *newfilename = 0;
    bool display = false;
    if( argc > 2 )
    {
      display = (std::string( argv[ 2 ] ) == "display");
      newfilename = (display ? 0 : argv[ 2 ]);
      if( newfilename && argc > 3 )
        display = true ;
    }



    // extra-environment to check destruction
    {
      typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > GridType;
      std::string filename( "./dgf/simplex-testgrid-2-2.dgf");
      Dune::GridPtr<GridType> gridPtr( filename );
      gridPtr->globalRefine(2);
      GridType &grid = *gridPtr;

      GridType::LevelGridView levelGrid = grid.levelGridView( 2 );

      for( auto entity : Dune::elements( levelGrid ) )
      {
        for( auto intersection : Dune::intersections( levelGrid, entity ) )
        {
          auto inside = intersection.inside();
          std::cout << "entity.level(): "<< entity.level() <<std::endl;
          std::cout << "intersection.inside().level(): " << intersection.inside().level() <<std::endl;
          if( intersection.neighbor() )
            std::cout << "intersection.outside().level(): " << intersection.outside().level() <<std::endl;
          std::cout << std::endl;
        }
      }
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
