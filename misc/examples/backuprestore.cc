//***********************************************************************
//
//  Example program how to use ALUGrid.
//  Author: Robert Kloefkorn
//
//  This little program read one of the macrogrids and generates a grid.
//  The  grid is refined and coarsend again.
//
//***********************************************************************
//
#include <config.h>
#include <iostream>

// include serial part of ALUGrid
#include <dune/alugrid/grid.hh>

using namespace ALUGrid;
using namespace std;

typedef Gitter::AdaptRestrictProlong AdaptRestrictProlongType;

typedef Gitter::helement_STI  HElemType;    // Interface Element
typedef Gitter::hface_STI     HFaceType;    // Interface Element
typedef Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef Gitter::vertex_STI    HVertexType;  // Interface Element
typedef Gitter::hbndseg       HGhostType;

#if HAVE_MPI
  #define PARALLEL 1
#warning RUNNING PARALLEL VERSION
#else
  #define PARALLEL 0
#endif

#define COUNT_FLOPS
#define DONT_USE_ALUGRID_ALLOC


template <class GitterType, class element_t>
void checkElement( GitterType& grid, element_t& elem )
{
  if( elem.type() == tetra )
  {
    typedef typename GitterType :: Objects :: tetra_IMPL tetra_IMPL ;
    // mark element for refinement
    tetra_IMPL& item = ((tetra_IMPL &) elem);

    for( int i=0; i<4; ++i )
    {
      assert( item.myneighbour( i ).first->isRealObject() );
    }
  }
  else
  {
    typedef typename GitterType :: Objects :: hexa_IMPL hexa_IMPL ;
    // mark element for refinement
    hexa_IMPL& item = ((hexa_IMPL &) elem);

    for( int i=0; i<6; ++i )
    {
      assert( item.myneighbour( i ).first->isRealObject() );
    }
  }
}

// refine grid globally, i.e. mark all elements and then call adapt
template <class GitterType>
void globalRefine(GitterType& grid, int refcount)
{
   for (int count=refcount ; count > 0; count--)
   {
     cout << "Refine global: run " << refcount-count << endl;
     {
        // get LeafIterator which iterates over all leaf elements of the grid
        LeafIterator < Gitter::helement_STI > w (grid) ;

        for (w->first () ; ! w->done () ; w->next ())
        {
          // mark element for refinement
          w->item ().tagForGlobalRefinement ();
          checkElement( grid, w->item() );
        }
     }

     // adapt grid
     grid.adaptWithoutLoadBalancing ();

     // print size of grid
     grid.printsize () ;
   }

}

// coarse grid globally, i.e. mark all elements for coarsening
// and then call adapt
template <class GitterType>
void globalCoarsening(GitterType& grid, int refcount) {

  for (int count=refcount ; count > 0; count--)
  {
    cout << "Global Coarsening: run " << refcount-count << endl;
    {
       // get LeafIterator which iterates over all leaf elements of the grid
       LeafIterator < Gitter::helement_STI > w (grid) ;

       for (w->first () ; ! w->done () ; w->next ())
       {
         checkElement( grid, w->item() );
         // mark elements for coarsening
         w->item ().tagForGlobalCoarsening() ;
       }
    }

    // adapt grid
    grid.adaptWithoutLoadBalancing ();

    // print size of grid
    grid.printsize () ;

  }
}

// exmaple on read grid, refine global and print again
int main (int argc, char ** argv, const char ** envp)
{
  MPI_Init(&argc,&argv);
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int mxl = 0;
  const char* filename = 0 ;
  if (argc < 2)
  {
    filename = "../macrogrids/reference.tetra";
    mxl = 1;
    cout << "usage: "<< argv[0] << " <macro grid> <opt: level> \n";
  }
  else
    filename = argv[ 1 ];

  if (argc < 3)
    cout << "Default level = "<< mxl << " choosen! \n";
  else
    mxl = atoi(argv[2]);

  std::string macroname( filename );

  if( rank == 0 )
  {
    cout << "\n-----------------------------------------------\n";
    cout << "read macro grid from < " << macroname << " > !" << endl;
    cout << "-----------------------------------------------\n";
  }

  std::stringstream backupname ;
  backupname << "file." << rank ;
  std::stringstream databuf;
  {
#ifdef PARALLEL
    MpAccessMPI a (MPI_COMM_WORLD);
    GitterDunePll grid(macroname.c_str(),a);
    grid.duneLoadBalance() ;
#else
    std::ifstream infile( macroname.c_str());
    GitterDuneImpl grid1( infile );
    infile.close();

    std::ifstream infile2( macroname.c_str());
    GitterDuneImpl grid2( infile2 );
    infile2.close();
    GitterDuneImpl& grid = grid2;

    cout << "Grid generated! \n";
    globalRefine(grid, mxl);
    cout << "---------------------------------------------\n";
#endif

    std::ofstream file( backupname.str().c_str() );
    grid.duneBackup( file );
    file.close();

    globalCoarsening(grid, mxl);
  }

  {
    std::ifstream file( backupname.str().c_str() );
    // read grid from file
#ifdef PARALLEL
    MpAccessMPI a (MPI_COMM_WORLD);
    GitterDunePll grid( file, a);
#else
    GitterDuneImpl grid( file );
#endif
    grid.printsize();

    grid.duneRestore( file );
    file.close();
    //globalRefine(grid, mxl);
    // adapt grid

    grid.duneBackup( databuf );

    cout << "Grid restored!" << std::endl;
    grid.printsize();
    cout << "---------------------------------------------\n";

    globalCoarsening(grid, mxl);
  }

  {
    cout << "Try to read stringbuf:" << std::endl;
    cout << "Data Buffer size: " << databuf.str().size() << std::endl;
    // read grid from file
#ifdef PARALLEL
    MpAccessMPI a (MPI_COMM_WORLD);
    GitterDunePll grid( databuf, a);
#else
    GitterDuneImpl grid( databuf );
#endif
    grid.printsize();
    grid.duneRestore( databuf );

    //grid.duneRestore( file );
    // adapt grid

    cout << "Grid restored!" << std::endl;
    grid.printsize();
    cout << "---------------------------------------------\n";

    globalCoarsening(grid, mxl);
  }

  MPI_Finalize();
  return 0;
}

