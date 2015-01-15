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


// include serial part of ALUGrid
//#include <alugrid_serial.h>
#include <alugrid_parallel.h>
using namespace ALUGridSpace;


typedef GitterDunePll GitterType;

// refine grid globally, i.e. mark all elements and then call adapt
void globalRefine(GitterType* grid, int refcount, int rank) {

   for (int count=refcount ; count > 0; count--) {
   cout << "Refine global: run " << refcount-count << endl;
       {
          // get LeafIterator which iterates over all leaf elements of the grid
          LeafIterator < Gitter::helement_STI > w (*grid) ;
          cout << "we have " << w->size() << " elements on rank = " << rank << "\n";

          for (w->first () ; ! w->done () ; w->next ())
          {
            // mark element for refinement
            w->item ().tagForGlobalRefinement ();
          }
       }
       // adapt grid
       grid->adapt();

       // print size of grid
       //grid->printsize () ;
   }
}

// coarse grid globally, i.e. mark all elements for coarsening
// and then call adapt
void globalCoarsening(GitterType * grid, int refcount, int rank) {

   for (int count=refcount ; count > 0; count--) {
   cout << "Global Coarsening: run " << refcount-count << endl;
       {
          // get LeafIterator which iterates over all leaf elements of the grid
          LeafIterator < Gitter::helement_STI > w (*grid) ;

          for (w->first () ; ! w->done () ; w->next ())
          {
            // mark elements for coarsening
            w->item ().tagForGlobalCoarsening() ;
          }
       }
       // adapt grid
       grid->adapt ();

       // print size of grid
       grid->printsize () ;
   }

   {
     // get LeafIterator which iterates over all leaf elements of the grid
     LeafIterator < Gitter::helement_STI > w (*grid) ;
     cout << "we have " << w->size() << " elements on rank = " << rank << "\n";
   }
}

// perform walk over elements of a certain level
void levelwalk(GitterType * grid, int level) {
   typedef Insert <AccessIterator <
     Gitter::helement_STI>::Handle,
     TreeIterator <Gitter :: helement_STI, any_has_level <Gitter::helement_STI> > >
       LevelIterator;

   LevelIterator it (grid->container(), level);
   int i = 0;
   for (it.first(); !it.done(); it.next())
   {
      cout << "Element " << it.item().getIndex() << " has " << i++ << " as level index " << endl;
   }
   cout << endl;
}


// exmaple on read grid, refine global and print again
int main (int argc, char ** argv, const char ** envp)
{
   MPI_Init(&argc,&argv);
   int myrank = -1;

   MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

   {
     int mxl = 3;
     if (argc < 2)
     {
        cout << "usage: "<< argv[0] << " <macro grid> <opt: level> \n";
        abort();
     }

     if (argc < 3)
       cout << "Default level = "<< mxl << "choosen! \n";
     else
       mxl = atoi(argv[2]);

     std::string macroname( argv[1] );

     cout << "\n-----------------------------------------------\n";
     cout << "read macro grid from < " << macroname << " > !" << endl;
     cout << "-----------------------------------------------\n";

     MpAccessMPI mpAccess(MPI_COMM_WORLD);
     GitterDunePll grid(macroname.c_str(),mpAccess);
     //GitterDunePll grid(macroname.c_str(),mpAccess);
     cout << "Grid generated! \n";
     grid.duneLoadBalance();

     grid.printsize();
     cout << "---------------------------------------------\n";

     globalRefine(&grid, mxl, myrank);
     globalCoarsening(&grid, mxl, myrank );
   }

   MPI_Finalize();
   return 0;
}

