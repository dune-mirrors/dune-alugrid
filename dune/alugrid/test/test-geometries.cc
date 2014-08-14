//***********************************************************************
//
//  test geometry implementation of ALUGrid. 
//  Author: Robert Kloefkorn 
//
//***********************************************************************
#include <config.h>
#include <iostream>

#include <dune/common/parallel/mpihelper.hh>

// include dgf parser 
#include <dgfparser.hh>

// include serial part of ALUGrid 
#include <dune/alugrid/common/declaration.hh>
#include <dune/alugrid/common/alugrid_assert.hh>
#include <dune/alugrid/3d/alu3dinclude.hh>
#include <dune/alugrid/3d/geometry.hh>

#include <dune/geometry/test/checkgeometry.hh>

#if HAVE_MPI
#warning RUNNING PARALLEL VERSION
#endif

typedef ALUGrid::Gitter::helement_STI  HElemType;    // Interface Element
typedef ALUGrid::Gitter::hface_STI     HFaceType;    // Interface Element
typedef ALUGrid::Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef ALUGrid::Gitter::vertex_STI    HVertexType;  // Interface Element
typedef ALUGrid::Gitter::hbndseg       HGhostType;


template<class ALUGrid, ALUGridElementType eltype>
ALUGrid* createGrid2d ( std::istream &file, const std::string &filename )
{   
    DGFParser dgf(eltype);
    dgf.element = (eltype == simplex) ? 
        DuneGridFormatParser::Simplex : DuneGridFormatParser::Cube ;


    const bool isDGF = dgf.isDuneGridFormat( file );
    file.seekg( 0 );
    if( !isDGF )
      return false;
      
          
    // ALUGrid is taking ownership of bndProjections 
    // and is going to delete this pointer 
    Grid* grid = createGridObj( NULL , "name" );
    alugrid_assert ( grid );
    
    
    // insert grid using ALUGrid macro grid builder   
      ALU3DSPACE MacroGridBuilder mgb ( grid->getBuilder(), grid->vertexProjection() );

   
   //insert additional (3d)-vertices - 
   //a single one for all triangles
   // one for each quadrilateral vertex
    if(eltype == simplex){
     Dune::FieldVector<double, 3> pos(1.);
      mgb.InsertUniqueVertex( pos[ 0 ], pos[ 1 ], pos[ 2 ], globalId( 0 ) );
    }

    for( int n = 0; n < dgf.numVertices(); ++n )
    { 
      FieldVector< double, dimworld > pos;
      for( int i = 0; i < dimworld; ++i )
        pos[ i ] = dgf.vtx[ n ][ i ];
      mgb.InsertUniqueVertex( pos[ 0 ], pos[ 1 ], pos[ 2 ], globalId( n+1 ) );
      if (eltype == hexa)
      {
        mgb.InsertUniqueVertex( pos[ 0 ], pos[ 1 ], pos[ 2 ] + 1., globalId( n+1 ) );
      }
    }
    

    const int nFaces = (eltype == simplex) ? dimgrid+1 : 2*dimgrid;
    for( int n = 0; n < dgf.numElements(); ++n )
    {
      if(eltype == simplex){
        dgf.elements[n].resize(4,0);
        for(int j=2; j>-1; --j)
        {
          dgf.elements[n][j]+=1;
          dgf.elements[n][j+1]=dgf.elements[n][j]; 
        }
        dgf.elements[n][0]=0;
      }
      
      if(eltype == hexa)
      {
        dgf.elements[n].resize(8,0);
        for (int k = 0; k<4;++k)
        {
          dgf.elements[n][k] *=2;
          dgf.elements[n][k+4]= dgf.elements[n][k]+1;
        }
      }

      insertElement( elementType, dgf.elements[n] );
      for( int face = 0; face <nFaces; ++face )
      {
        typedef typename DuneGridFormatParser::facemap_t::key_type Key;
        typedef typename DuneGridFormatParser::facemap_t::iterator Iterator;

        const Key key = ElementFaceUtil::generateFace( dimworld, dgf.elements[n], face );
        const Iterator it = dgf.facemap.find( key );
        if( it != dgf.facemap.end() )
          insertBoundary( n, face, it->second.first );
      }
    }
 

      for( size_t el = 0; el<elemSize; ++el )
      {
        if( elementType == hexa )
        {
          int element[ 8 ];
          for( unsigned int i = 0; i < 8; ++i )
          {
            const unsigned int j = ElementTopologyMappingType::dune2aluVertex( i );
            element[ j ] = globalId( elements_[ elemIndex ][ i ] );
          }
          mgb.InsertUniqueHexa( element );
        }
        else if( elementType == tetra )
        {
          int element[ 4 ];
          for( unsigned int i = 0; i < 4; ++i )
          {
            const unsigned int j = ElementTopologyMappingType::dune2aluVertex( i );
            element[ j ] = globalId( elements_[ elemIndex ][ i ] );
          }
          mgb.InsertUniqueTetra( element, (elemIndex % 2) );
        }
        else 
          DUNE_THROW( GridError, "Invalid element type");
      }
    
    return grid;
}

template < class G > 
G generateALU2dGrid(  )
{
    DuneGridFormatParser dgf;
  
    

  return createGrid( facemap.empty(), true, filename );

}







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

  const int nFaces = 6;
  for( int i=0; i<nFaces; ++i )
  {
    typedef Dune :: ALU3dGridGeometry< Grid::dimension-1, Grid::dimensionworld, const Grid > FaceGeometry;
    FaceGeometry faceGeom;
    const GEOFaceType* face = elem.myhface( i );
    faceGeom.buildGeom( *face, elem.twist( i ), i );
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
      edgeGeom.buildGeom( *edge, elem.twist( i ), i );
      // perform geometry check
      checkGeometry( edgeGeom );
      edgeGeom.print( std::cout );
    }
  }*/

  const int nVerts = 8;
  for( int i=0; i<nVerts; ++i )
  {
    typedef Dune :: ALU3dGridGeometry< 0, Grid::dimensionworld, const Grid > PointGeometry;
    PointGeometry point ;
    const GEOVertexType* vertex = static_cast<const GEOVertexType*> (elem.myvertex( i ));
    point.buildGeom( *vertex,0,0 );
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
  Dune :: MPIHelper& mpi = Dune :: MPIHelper :: instance(argc,argv);
#endif
  const char* filename = 0 ;
  if (argc < 2) 
  {
    filename = "reference.tetra";
    std::cout << "usage: "<< argv[0] << " <macro grid> <opt: maxlevel> <opt: global refinement>\n";
  }
  else 
  {
    filename = argv[ 1 ];
  }

  {
    int rank = 0;
#if HAVE_MPI
    ALUGrid::MpAccessMPI mpa (MPI_COMM_WORLD);
    rank = mpa.myrank();
#endif

    std::string macroname( filename );

    if( rank == 0 ) 
    {
      std::cout << "\n-----------------------------------------------\n";
      std::cout << "read macro grid from < " << macroname << " > !" << std::endl;
      std::cout << "-----------------------------------------------\n";
    }

    {
//#if HAVE_MPI
//      ALUGrid::GitterDunePll* gridPtr = new ALUGrid::GitterDunePll(macroname.c_str(),mpa);
//      ALUGrid::GitterDunePll& grid = *gridPtr ;
//#else 
      ALUGrid::GitterDuneImpl* gridPtr = new ALUGrid::GitterDuneImpl(macroname.c_str());
      ALUGrid::GitterDuneImpl& grid = *gridPtr ;
//#endif

      checkGeometries( grid );
    
      const bool output = false ;
      if( output )
      {
        std::ostringstream ss;
        ss << macroname << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
    }
  }

  return 0;
}

