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

//#define DONT_USE_ALUGRID_ALLOC

// include serial part of ALUGrid 
#ifdef PARALLEL
  #include <alugrid_parallel.h>
#else
  #include <alugrid_serial.h>
#endif

using namespace ALUGridSpace;

// refine grid globally, i.e. mark all elements and then call adapt 
template <class GitterType>
void checkRefinements( GitterType& grid ) 
{
  {
    // get LeafIterator which iterates over all leaf elements of the grid 
    LeafIterator < Gitter::helement_STI > w (grid) ;
    w->first(); 
    if( ! w->done() ) 
    {
      if( w->size() > 1 || w->item ().type() != tetra ) return ;
    }
  }
       
  typedef Gitter ::Geometric :: TetraRule  TetraRule ;
  const Gitter ::Geometric :: TetraRule rules[ 6 ] = 
  { TetraRule :: e01, TetraRule :: e12, TetraRule :: e20, 
    TetraRule :: e23, TetraRule :: e30, TetraRule :: e31 };

  for (int i=0; i<6; ++i ) 
  {
    cout << "*********************************************" <<endl;
    cout << "Refinement rule " << rules[ i ] << endl;
    cout << "*********************************************" <<endl;

    {
      // get LeafIterator which iterates over all leaf elements of the grid 
      LeafIterator < Gitter::helement_STI > w (grid) ;
       
      for (w->first () ; ! w->done () ; w->next ())
      {
        if( w->item ().type() == tetra ) 
        {
          typedef typename GitterType :: Objects :: tetra_IMPL tetra_IMPL ;
          // mark element for refinement 
          tetra_IMPL* item = ((tetra_IMPL *) &w->item ());
          item->request ( rules[ i ] );
        }
      }
    }

    // adapt grid 
    grid.adapt ();

    // coarsen again 
    globalCoarsening( grid , 1 );
  }

  cout << "*********************************************" <<endl;
  cout << " Check of rules done " << endl;
  cout << "*********************************************" <<endl;
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
    grid.printsize(); 
    cout << "---------------------------------------------\n";
  
    //grid.printMemUsage();
   
    checkRefinements( grid );

    std::ofstream file( "file.out" );
    grid.duneBackup( file );

    {
      ObjectStream os ;
      grid.duneBackup( os );
      double value = 5;
      os << value ;
      char* buffer = ObjectStream ::allocateBuffer( os.size() );
      os.read( buffer, os.size() );
      
      std::ofstream obj( "obj.out" );
      obj.write( buffer, os.size() );
      ObjectStream :: freeBuffer( buffer );
    }

    globalRefine(grid, mxl);
    //levelwalk(&grid, mxl);
    //globalCoarsening(&grid, mxl);
    //grid.printMemUsage();
    cin.get();
  }

  MPI_Finalize();
  return 0;
}

