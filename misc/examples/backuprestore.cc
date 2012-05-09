//***********************************************************************
//
//  Example program how to use ALUGrid. 
//  Author: Robert Kloefkorn 
//
//  This little program read one of the macrogrids and generates a grid. 
//  The  grid is refined and coarsend again. 
//
//***********************************************************************
#include <iostream>
#include <mpi.h>

using namespace std;

//#define PARALLEL

#define COUNT_FLOPS

#define DONT_USE_ALUGRID_ALLOC

// include serial part of ALUGrid 
#ifdef PARALLEL
  #include <alugrid_parallel.h>
#else
  #include <alugrid_serial.h>
#endif

using namespace ALUGridSpace;


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
        }
     }

     // adapt grid 
     grid.adapt ();

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
         // mark elements for coarsening  
         w->item ().tagForGlobalCoarsening() ;
       }
    }

    // adapt grid 
    grid.adapt ();

    // print size of grid 
    grid.printsize () ;

  }
}

// exmaple on read grid, refine global and print again 
int main (int argc, char ** argv, const char ** envp) 
{
  MPI_Init(&argc,&argv);

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

  cout << "\n-----------------------------------------------\n";
  cout << "read macro grid from < " << macroname << " > !" << endl;
  cout << "-----------------------------------------------\n";

  {
#ifdef PARALLEL
    MpAccessMPI a (MPI_COMM_WORLD);
    GitterDunePll grid(macroname.c_str(),a);
#else 
    GitterDuneImpl grid(macroname.c_str());
#endif
   
    cout << "Grid generated! \n";
    globalRefine(grid, mxl);
    cout << "---------------------------------------------\n";

    std::ofstream file( "file.out" );
    grid.duneBackup( file );
    file.close();

  }

  {
    std::ifstream file( "file.out" );
    // read grid from file 
#ifdef PARALLEL
    MpAccessMPI a (MPI_COMM_WORLD);
    GitterDunePll grid( file, a);
#else 
    GitterDuneImpl grid( file );
#endif
    grid.printsize();
    grid.duneRestore( file );
    cout << "Grid restored!" << std::endl;
    grid.printsize();
    cout << "---------------------------------------------\n";

    globalCoarsening(grid, mxl);
  }

  MPI_Finalize();
  return 0;
}

