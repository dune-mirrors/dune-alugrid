//***********************************************************************
//
//  test geometry implementation of ALUGrid.
//  Author: Robert Kloefkorn
//
//***********************************************************************
#include <config.h>
#include <iostream>
#include <fstream>

#include <dune/common/parallel/mpihelper.hh>

// include serial part of ALUGrid
#include <dune/alugrid/common/declaration.hh>
#include <dune/alugrid/common/alugrid_assert.hh>
#include <dune/alugrid/3d/alu3dinclude.hh>
#include <dune/alugrid/3d/geometry.hh>
#include <dune/alugrid/3d/entity.hh>

#include <dune/grid/io/file/dgfparser/parser.hh>

#include <dune/geometry/test/checkgeometry.hh>

// DGFParser
// ---------
#include "../../../utils/convert-macrogrid/dgfparser.hh"

#include "insertgrid.hh"


#if HAVE_MPI
#warning RUNNING PARALLEL VERSION
#endif

typedef ALUGrid::Gitter::helement_STI  HElemType;    // Interface Element
typedef ALUGrid::Gitter::hface_STI     HFaceType;    // Interface Element
typedef ALUGrid::Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef ALUGrid::Gitter::vertex_STI    HVertexType;  // Interface Element
typedef ALUGrid::Gitter::hbndseg       HGhostType;


// fake class for Geometry Implementation
template <int dim, int dimworld, Dune::ALU3dGridElementType eltype >
struct GridImp
{
  static const Dune::ALU3dGridElementType elementType = eltype;
  typedef Dune :: ALUGridNoComm     MPICommunicatorType ;
  static const int dimension      = dim ;
  static const int dimensionworld = dimworld ;
  typedef Dune :: alu3d_ctype       ctype ;
};

template < class Grid >
void checkGeom( HElemType* item )
{
  typedef Dune :: ALU3dGridGeometry< Grid::dimension, Grid::dimensionworld, const Grid > GeometryImpl;
  typedef typename GeometryImpl :: IMPLElementType IMPLElementType ;
  typedef typename GeometryImpl :: GEOFaceType     GEOFaceType;
  typedef typename GeometryImpl :: GEOEdgeType     GEOEdgeType;
  typedef typename GeometryImpl :: GEOVertexType     GEOVertexType;
  const IMPLElementType& elem = *(dynamic_cast<IMPLElementType *> (item));

  GeometryImpl geometry ;
  geometry.buildGeom( elem );
  // perform geometry check
  checkGeometry( geometry );
  geometry.print( std::cout );

  const int nFaces = (Grid::elementType == Dune::tetra) ? 4 : 6;
  for( int i=0; i<nFaces; ++i )
  {
    typedef Dune :: ALU3dGridGeometry< Grid::dimension-1, Grid::dimensionworld, const Grid > FaceGeometry;
    FaceGeometry faceGeom;
    const GEOFaceType* face = elem.myhface( i );
    faceGeom.buildGeom( *face, elem.twist( i ) );
    std::cout << "FACE: " << i << std::endl;
    // perform geometry check
    checkGeometry( faceGeom );
    faceGeom.print( std::cout );
  }

  /*
  // check edges
  const int nEdges = 6;
  if( Grid::dimension > 2 )
  {
    for( int i=0; i<nEdges; ++i )
    {
      typedef Dune :: ALU3dGridGeometry< Grid::dimension-2, Grid::dimensionworld, const Grid > EdgeGeometry;
      EdgeGeometry edgeGeom;
      const GEOEdgeType* edge = elem.myhedge( i );
      edgeGeom.buildGeom( *edge, elem.twist( i ) );
      // perform geometry check
      checkGeometry( edgeGeom );
      edgeGeom.print( std::cout );
    }
  }*/

  const int nVerts = (Grid::elementType == Dune::tetra) ? 4 : 8;
  for( int i=0; i<nVerts; ++i )
  {
    typedef Dune :: ALU3dGridGeometry< 0, Grid::dimensionworld, const Grid > PointGeometry;
    PointGeometry point ;
    const GEOVertexType* vertex = static_cast<const GEOVertexType*> (elem.myvertex( i ));
    point.buildGeom( *vertex, 0 );
    // perform geometry check
    checkGeometry( point );
    point.print( std::cout );
  }
}

template <class Gitter>
void checkGeometries( Gitter& grid )
{
  // get LeafIterator which iterates over all leaf elements of the grid
  ALUGrid::LeafIterator < HElemType > w (grid) ;
  int numberofelement = 0;
  for (w->first () ; ! w->done () ; w->next ())
  {
    HElemType* item =  &w->item ();
    // mark element for refinement
    std::cout<< "ELEMENT: " << numberofelement << std::endl;
    if( item->type() == ALUGrid::tetra )
    {
      checkGeom< GridImp< 2, 2, Dune::tetra > >( item );
      //checkGeom< GridImp< 2, 3, Dune::tetra > >( item );
      checkGeom< GridImp< 3, 3, Dune::tetra > >( item );
    }
    else
    {
      checkGeom< GridImp< 2, 2, Dune::hexa > >( item );
      //checkGeom< GridImp< 2, 3, Dune::hexa > >( item );
      checkGeom< GridImp< 3, 3, Dune::hexa > >( item );
    }
    ++numberofelement;
  }
}

// exmaple on read grid, refine global and print again
int main (int argc, char ** argv, const char ** envp)
{
#if HAVE_MPI
  //Dune :: MPIHelper& mpi =
  Dune :: MPIHelper :: instance(argc,argv);
#endif
  std::string filename;

  if (argc < 2)
  {
    filename = "reference.tetra";
    std::cout << "usage: "<< argv[0] << " <macro grid> <opt: maxlevel> <opt: global refinement>\n";
  }
  else
  {
    filename = argv[ 1 ];
  }


  ALUGrid::GitterDuneImpl* gridPtr = 0;
  std::ifstream input( filename );
  if( DGFParser::isDuneGridFormat( input ) )
  {
    DGFParser dgf( Dune::simplex, 2, 2 );
    if( !dgf.readDuneGrid( input, 2, 2 ) )
    {
      std::cerr << "ERROR: Invalid DGF file." << std::endl;
      std::exit( 1 );
    }
    gridPtr = new ALUGrid::GitterDuneImpl( 2 );
    insertGrid( dgf, gridPtr );
  }
  else
  {
    gridPtr = new ALUGrid::GitterDuneImpl(3, filename.c_str());
  }
  alugrid_assert( gridPtr );

  ALUGrid::GitterDuneImpl& grid = *gridPtr ;

  checkGeometries( grid );

  const bool output = false ;
  if( output )
  {
    std::ostringstream ss;
    ss << filename << ".vtu";
    grid.tovtk(  ss.str().c_str() );
  }

  return 0;
}

