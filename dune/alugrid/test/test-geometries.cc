//***********************************************************************
//
//  test geometry implementation of ALUGrid. 
//  Author: Robert Kloefkorn 
//
//***********************************************************************
#include <config.h>
#include <iostream>

#include <dune/common/parallel/mpihelper.hh>

// include serial part of ALUGrid 
#include <dune/alugrid/common/declaration.hh>
#include <dune/alugrid/3d/alu3dinclude.hh>
#include <dune/alugrid/3d/geometry.hh>

#if HAVE_MPI
#warning RUNNING PARALLEL VERSION
#endif

typedef ALUGrid::Gitter::helement_STI  HElemType;    // Interface Element
typedef ALUGrid::Gitter::hface_STI     HFaceType;    // Interface Element
typedef ALUGrid::Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef ALUGrid::Gitter::vertex_STI    HVertexType;  // Interface Element
typedef ALUGrid::Gitter::hbndseg       HGhostType;

template <int dim, int dimworld, Dune::ALU3dGridElementType eltype >
struct GridImp 
{
  static const Dune::ALU3dGridElementType elementType = eltype;
  typedef Dune :: ALUGridNoComm MPICommunicatorType ;
  static const int dimension      = dim ;
  static const int dimensionworld = dimworld ;
  typedef Dune :: alu3d_ctype ctype ;
};

template < Dune::ALUGridElementType type, int dim > 
void checkGeom( HElemType& elem ) 
{
  static const int cd = 0 ;
  static const int dimworld = dim ; // change appropriately 
  static const Dune::ALU3dGridElementType eltype = ( type == Dune :: simplex  ) 
    ? Dune::tetra : Dune::hexa ;

  typedef GridImp< dim, dimworld, eltype > Grid ;
  typedef Dune :: ALU3dGridGeometry< dim-cd, dimworld, const Grid > GeometryImpl;

  GeometryImpl geometry ; 
  geometry.buildGeom( elem );

  geometry.print( std::cout );
}

template <class Gitter> 
void checkGeometries( Gitter& grid ) 
{

  // get LeafIterator which iterates over all leaf elements of the grid 
  ALUGrid::LeafIterator < HElemType > w (grid) ;

  for (w->first () ; ! w->done () ; w->next ())
  {
    // mark element for refinement 
    if( w->item ().type() == ALUGrid::tetra )
    {
      checkGeom< Dune::simplex, 2 >( w->item() );
      checkGeom< Dune::simplex, 3 >( w->item() );
    }
    else 
    {
      checkGeom< Dune::cube, 2 >( w->item() );
      checkGeom< Dune::cube, 3 >( w->item() );
    }
  }

}

// exmaple on read grid, refine global and print again 
int main (int argc, char ** argv, const char ** envp) 
{
#if HAVE_MPI
  Dune :: MPIHelper& mpi = Dune :: MPIHelper :: instance(&argc,&argv);
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
#if HAVE_MPI
      ALUGrid::GitterDunePll* gridPtr = new ALUGrid::GitterDunePll(macroname.c_str(),mpa);
      ALUGrid::GitterDunePll& grid = *gridPtr ;
#else 
      ALUGrid::GitterDuneImpl* gridPtr = new ALUGrid::GitterDuneImpl(macroname.c_str());
      ALUGrid::GitterDuneImpl& grid = *gridPtr ;
#endif

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

