//***********************************************************************
//
//  Example program how to use ALUGrid.
//  Author: Robert Kloefkorn
//
//  This little program read one of the macrogrids and generates a grid.
//  The  grid is refined and coarsend again.
//
//***********************************************************************
#include <config.h>
#include <iostream>

// include serial part of ALUGrid
#include <dune/alugrid/3d/alu3dinclude.hh>

//using namespace ALUGrid;
//using namespace std;

typedef ALUGrid::Gitter::AdaptRestrictProlong AdaptRestrictProlongType;

typedef ALUGrid::Gitter::helement_STI  HElemType;    // Interface Element
typedef ALUGrid::Gitter::hface_STI     HFaceType;    // Interface Element
typedef ALUGrid::Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef ALUGrid::Gitter::vertex_STI    HVertexType;  // Interface Element
typedef ALUGrid::Gitter::hbndseg       HGhostType;

#if HAVE_MPI
#warning RUNNING PARALLEL VERSION
#endif

//#define ENABLE_ALUGRID_VTK_OUTPUT

// refine grid globally, i.e. mark all elements and then call adapt
template <class GitterType>
void globalRefine(GitterType& grid, bool global, int step, int mxl,
                  const bool loadBalance = true, const bool printOutput = false )
{
   {
     if (global)
     {
       // get LeafIterator which iterates over all leaf elements of the grid
       ALUGrid::LeafIterator < HElemType > w (grid) ;

       for (w->first () ; ! w->done () ; w->next ())
       {
         // mark element for refinement
         w->item ().tagForGlobalRefinement ();
       }
     }
     else
     {
       double t = double(step)/10.;
       double center[3] = {0.2,0.2,0.2};
       double dir[3]    = {1.0,0.0,0.0};
       center[0] += dir[0]*t;
       center[1] += dir[1]*t;
       center[2] += dir[2]*t;
       double rad=0.6;

       grid.markForBallRefinement(center,rad,mxl);
     }

     // adapt grid
     grid.adaptWithoutLoadBalancing();

     if( printOutput )
     {
       // print size of grid
       grid.printsize () ;
     }
   }

}

// exmaple on read grid, refine global and print again
int main (int argc, char ** argv)
{
#if HAVE_MPI
  MPI_Init(&argc,&argv);
#endif
  const bool printOutput = true ;

  int mxl = 0, glb = 0;
  const char* filename = 0 ;
  if (argc < 2)
  {
    filename = "../macrogrids/reference.tetra";
    mxl = 1;
    glb = 1;
    std::cout << "usage: "<< argv[0] << " <macro grid> <opt: maxlevel> <opt: global refinement>\n";
  }
  else
  {
    filename = argv[ 1 ];
  }

  const bool useClosure = argc > 4 ;

  {
    int rank = 0;
#if HAVE_MPI
    ALUGrid::MpAccessMPI mpa (MPI_COMM_WORLD);
    rank = mpa.myrank();
#endif

    if (argc < 3)
    {
      if( rank == 0 )
        std::cout << "Default level = "<< mxl << " choosen! \n";
    }
    else
      mxl = atoi(argv[2]);
    if (argc < 4)
    {
      if( rank == 0 )
        std::cout << "Default global refinement = "<< glb << " choosen! \n";
    }
    else
      glb = atoi(argv[3]);

    std::string macroname( filename );

    if( rank == 0 )
    {
      std::cout << "\n-----------------------------------------------\n";
      std::cout << "read macro grid from < " << macroname << " > !" << std::endl;
      std::cout << "-----------------------------------------------\n";
    }

    const int dim = 3;
    {
#if HAVE_MPI
      ALUGrid::GitterDunePll* gridPtr = new ALUGrid::GitterDunePll(dim, macroname.c_str(),mpa);
      ALUGrid::GitterDunePll& grid = *gridPtr ;
#else
      ALUGrid::GitterDuneImpl* gridPtr = new ALUGrid::GitterDuneImpl(dim, macroname.c_str());
      ALUGrid::GitterDuneImpl& grid = *gridPtr ;
#endif

#ifdef ENABLE_ALUGRID_VTK_OUTPUT
      {
        std::ostringstream ss;
        ss << "start-" << ZeroPadNumber(mxl) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
#endif

      grid.printMemUsage();
      for (int i = 0; i < glb; ++i)
        globalRefine(grid, true, -1, mxl, true, true);

      grid.printsize();
      grid.printMemUsage();
    }
  }

#if HAVE_MPI
  MPI_Finalize();
#endif
  return 0;
}

