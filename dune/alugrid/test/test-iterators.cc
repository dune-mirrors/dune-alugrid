#define DISABLE_DEPRECATED_METHOD_CHECK 1

#include <config.h>


#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/tupleutility.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/geometry/referenceelements.hh>

#include <dune/grid/io/file/dgfparser/dgfwriter.hh>

#include <dune/grid/test/checkintersectionit.hh>
//#include "checktwists.cc"

#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <dune/alugrid/dgf.hh>

#if ALU3DGRID_PARALLEL && HAVE_MPI
#define USE_PARALLEL_TEST 1
#endif

template < class GeometryType >
void printGeometry(const GeometryType geo)
{
  std::cout << "{\n";
  for(int i=0; i<geo.corners(); ++i)
  {
    std::cout << " corner " << i << " ";
    std::cout << "{" << geo.corner(i) << "}";
    std::cout << std::endl;
  }
  std::cout << "} \n";
}



template <int codim, class GridType>
void checkIteratorCodim(GridType & grid)
{
  typedef typename GridType::template Codim<codim>::
     template Partition<Dune::InteriorBorder_Partition>::LeafIterator
        IteratorInteriorBorder;

  typedef typename GridType::template Codim<codim>:: Geometry Geometry ;
  typedef typename GridType:: ctype ctype;

  std::cout << "CODIM: " << codim << std::endl << std::endl;
  int cnt = 0;
  /** Loop only over the interior elements, not over ghost elements. */
  const IteratorInteriorBorder endIterator = grid.template leafend< codim, Dune::InteriorBorder_Partition >();
  for( IteratorInteriorBorder iter = grid.template leafbegin< codim, Dune::InteriorBorder_Partition >(); iter != endIterator; ++iter )
  {
    /** Provide geometry type of element. */
    const Geometry& geo = iter->geometry();
    std::cout << "Number: " << cnt << std::endl;

    printGeometry(geo);
    if( geo.corners() > 1 )
    {
      Dune::FieldVector<ctype, GridType::dimension>
        diff( geo.corner(0) - geo.corner(1) );
      if( diff.two_norm() < 1e-8 )
      {
        std::cout << diff << " twonorm = " << diff.two_norm() << " point 0 and 1 do not differ! " << std::endl;
        assert ( diff.two_norm() > 1e-8 );
      }
    }
    cnt++;
  }

  std::cout << std::endl << std::endl;
}

template <class GridType>
void checkIntersections( GridType& grid )
{
  grid.globalRefine(1);

  auto level_view = grid.levelView(1);
  for (auto && entity : Dune::elements(level_view)) {
    std::cout << "entity " << level_view.indexSet().index(entity) << std::endl;
    try {
    for (auto&& intersection : Dune::intersections(level_view, entity))
      std::cout << "  intersection " << intersection.indexInInside() << std::endl;
    }
    catch ( const Dune::NotImplemented& e )
    {
      std::cout << e.what() << std::endl;
    }
  }
}

template <class GridType>
void checkIterators( GridType& grid )
{
  checkIteratorCodim< 0 > ( grid );
  checkIteratorCodim< 1 > ( grid );
  checkIteratorCodim< 2 > ( grid );
  checkIteratorCodim< GridType :: dimension > ( grid );
}


template <class GridView>
void writeFile( const GridView& gridView )
{
  //Dune::DGFWriter< GridView > writer( gridView );
  //writer.write( "dump.dgf" );

  Dune::VTKWriter< GridView > vtk( gridView );
  vtk.write( "dump" );
}

template <class GridType>
void checkALUSerial(GridType & grid, int mxl = 2, const bool display = false)
{

  writeFile( grid.leafGridView() );

  std::cout << "  CHECKING: grid size = " << grid.size( 0 ) << std::endl;

  // check iterators
  //checkIterators( grid );
  checkIntersectionIterator(grid);

  std::cout << std::endl << std::endl;
}


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


    bool testALU2dSimplex = initialize ;
    bool testALU2dConform = initialize ;
    bool testALU2dCube    = initialize ;
    if( key == "2d" )
    {
      testALU2dSimplex = true ;
      testALU2dConform = true ;
      testALU2dCube   = true ;
    }
    if( key == "2dsimp" ) testALU2dSimplex = true ;
    if( key == "2dconf" ) testALU2dConform = true ;
    if( key == "2dcube" ) testALU2dCube    = true ;



    // extra-environment to check destruction
    {


      // check empty grid


      // check non-conform ALUGrid for 2d
      if( testALU2dCube )
      {
        typedef Dune::ALUGrid< 2, 2, Dune::cube, Dune::nonconforming > GridType;
        std::string filename( "./dgf/cube-testgrid-2-2.dgf" );
        std::cout << "READING from " << filename << std::endl;
        Dune::GridPtr< GridType > gridPtr(filename);
        std::cout << "Begin Cube test" << std::endl;
        checkALUSerial(*gridPtr, 2, display);

        checkIntersections( *gridPtr );

        //CircleBoundaryProjection<2> bndPrj;
        //GridType grid("alu2d.triangle", &bndPrj );
        //checkALUSerial(grid,2);

        /*
        typedef Dune::ALUGrid< 2, 3, Dune::cube, Dune::nonconforming > SurfaceGridType;
        std::string surfaceFilename( "./dgf/cube-testgrid-2-3.dgf" );
        std::cout << "READING from '" << surfaceFilename << "'..." << std::endl;
        Dune::GridPtr< SurfaceGridType > surfaceGridPtr( surfaceFilename );
        checkALUSerial( *surfaceGridPtr, 1, display );
        */
      }

      // check non-conform ALUGrid for 2d
      if( testALU2dSimplex )
      {
        typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::nonconforming > GridType;
        //std::string filename( "./dgf/simplex-testgrid-2-2.dgf" );
        std::string filename( "./dgf/cube-testgrid-2-2.dgf" );
        std::cout << "READING from " << filename << std::endl;
        Dune::GridPtr< GridType > gridPtr( filename );
        std::cout << "begin simplex test nonconforming" << std::endl;
        checkALUSerial(*gridPtr, 2, display);

        checkIntersections( *gridPtr );

        //CircleBoundaryProjection<2> bndPrj;
        //GridType grid("alu2d.triangle", &bndPrj );
        //checkALUSerial(grid,2);

        /*
        typedef Dune::ALUGrid< 2, 3, Dune::simplex, Dune::nonconforming > SurfaceGridType;
        std::string surfaceFilename( "./dgf/simplex-testgrid-2-3.dgf" );
        std::cout << "READING from '" << surfaceFilename << "'..." << std::endl;
        Dune::GridPtr< SurfaceGridType > surfaceGridPtr( surfaceFilename );
        checkALUSerial( *surfaceGridPtr, 1, display );
        */
      }

      // check conform ALUGrid for 2d
      if( testALU2dConform )
      {
        typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > GridType;
        std::string filename( "./dgf/simplex-testgrid-2-2.dgf");
        Dune::GridPtr<GridType> gridPtr( filename );
        std::cout << "begin simplex test conforming" << std::endl;
        checkALUSerial(*gridPtr, 2, display);

        checkIntersections( *gridPtr );

        //CircleBoundaryProjection<2> bndPrj;
        //GridType grid("alu2d.triangle", &bndPrj );
        //checkALUSerial(grid,2);

        /*
        typedef Dune::ALUGrid< 2, 3, Dune::simplex, Dune::conforming > SurfaceGridType;
        //typedef ALUConformGrid< 2, 3 > SurfaceGridType;
        std::string surfaceFilename( "./dgf/simplex-testgrid-2-3.dgf" );
        std::cout << "READING from '" << surfaceFilename << "'..." << std::endl;
        Dune::GridPtr< SurfaceGridType > surfaceGridPtr( surfaceFilename );
        checkALUSerial( *surfaceGridPtr, 1, display );
        */
      }

    };

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
