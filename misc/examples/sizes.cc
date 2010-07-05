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
using namespace std;

//#define DONT_USE_ALUGRID_ALLOC

// include serial part of ALUGrid 
#include <alugrid_serial.h>
//#include <alugrid_parallel.h>
using namespace ALUGridSpace;

void printSizes()
{
  cout << "Hexa : " << sizeof(GitterDuneImpl :: Objects :: hexa_IMPL) << endl; 
}

// refine grid globally, i.e. mark all elements and then call adapt 
void globalRefine(GitterBasisImpl* grid, int refcount) {
    
   for (int count=refcount ; count > 0; count--) {
   cout << "Refine global: run " << refcount-count << endl;
       {
          // get LeafIterator which iterates over all leaf elements of the grid 
          LeafIterator < Gitter::helement_STI > w (*grid) ;
          
          for (w->first () ; ! w->done () ; w->next ())
          {
            // mark element for refinement 
            w->item ().tagForGlobalRefinement ();
          }
       }
       // adapt grid 
       grid->adapt ();

       // print size of grid 
       grid->printsize () ;
   }
}

// coarse grid globally, i.e. mark all elements for coarsening 
// and then call adapt 
void globalCoarsening(GitterBasisImpl* grid, int refcount) {
    
   for (int count=refcount ; count > 0; count--) {
   cout << "Global Coarsening: run " << refcount-count << endl;
       {
          // get LeafIterator which iterates over all leaf elements of the grid 
          LeafIterator < Gitter::helement_STI > w (*grid) ;
          
          for (w->first () ; ! w->done () ; w->next ())
          {
            // mark elements for coarsening  
            w->item ().resetRefinementRequest () ;
          }
       }
       // adapt grid 
       grid->adapt ();

       // print size of grid 
       grid->printsize () ;
   }
}

// perform walk over elements of a certain level  
void levelwalk(GitterBasisImpl* grid, int level) {
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
  //MPI_Init(&argc,&argv);

   int mxl = 0; 
   if (argc < 2) 
   {
      cout << "usage: "<< argv[0] << " <macro grid> <opt: level> \n";
      abort();
   }

   if (argc < 3)
     cout << "Default level = "<< mxl << " choosen! \n";
   else 
     mxl = atoi(argv[2]);

   std::string macroname( argv[1] );

   cout << "\n-----------------------------------------------\n";
   cout << "read macro grid from < " << macroname << " > !" << endl;
   cout << "-----------------------------------------------\n";

   {
   //MpAccessMPI a (MPI_COMM_WORLD);
   //GitterDunePll grid(macroname.c_str(),a);
   GitterDuneImpl grid(macroname.c_str());
   
   cout << "Grid generated! \n";
   grid.printsize(); 
   cout << "---------------------------------------------\n";
  
   printSizes();
   grid.printMemUsage();
   
   globalRefine(&grid, mxl);
   //levelwalk(&grid, mxl);
   //globalCoarsening(&grid, mxl);
   grid.printMemUsage();
   cin.get();
   }

   //MPI_Finalize();
   return 0;
}

