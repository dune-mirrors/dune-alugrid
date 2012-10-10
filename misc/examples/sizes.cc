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

#define PARALLEL

#define COUNT_FLOPS

#define DONT_USE_ALUGRID_ALLOC

// enable vtk output 
#define ENABLE_ALUGRID_VTK_OUTPUT

// include serial part of ALUGrid 
#ifdef PARALLEL
  #include <alugrid_parallel.h>
#else
  #include <alugrid_serial.h>
#endif

using namespace ALUGridSpace;

// refine grid globally, i.e. mark all elements and then call adapt 
template <class GitterType>
bool needConformingClosure( GitterType& grid, bool useClosure ) 
{
  bool needClosure = true ;
  {
    // get LeafIterator which iterates over all leaf elements of the grid 
    LeafIterator < Gitter::helement_STI > w (grid) ;
    w->first(); 
    if( ! w->done() ) 
    {
      if( w->item ().type() != tetra )
        needClosure = false ;
      else 
        needClosure = useClosure ;
    }
  }
  return needClosure ;
}
       
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
void globalRefine(GitterType& grid, bool global, int step, const bool bisection = true ) 
{
   {
     if (global)
     {
       // get LeafIterator which iterates over all leaf elements of the grid 
       LeafIterator < Gitter::helement_STI > w (grid) ;
        
       for (w->first () ; ! w->done () ; w->next ())
       {
         // mark element for refinement 
         w->item ().tagForGlobalRefinement ();
       }
       // adapt grid 
       grid.adapt ();
       grid.printsize () ;
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
       grid.refineBall(center,rad,10);
     }
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
       // get leafiterator which iterates over all leaf elements of the grid 
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

  int mxl = 0, glb = 0; 
  const char* filename = 0 ;
  if (argc < 2) 
  {
    filename = "../macrogrids/reference.tetra";
    mxl = 1;
    glb = 1;
    cout << "usage: "<< argv[0] << " <macro grid> <opt: level global> \n";
  }
  else 
  {
    filename = argv[ 1 ];
  }

  {
    int rank = 0;
#ifdef PARALLEL
    MpAccessMPI mpa (MPI_COMM_WORLD);
    rank = mpa.myrank();
#endif

    if (argc < 3)
    {
      if( rank == 0 ) 
        cout << "Default level = "<< mxl << " choosen! \n";
    }
    else 
      mxl = atoi(argv[2]);
    if (argc < 4)
    {
      if( rank == 0 ) 
        cout << "Default global refinement = "<< glb << " choosen! \n";
    }
    else 
      glb = atoi(argv[3]);

    std::string macroname( filename );

    if( rank == 0 ) 
    {
      cout << "\n-----------------------------------------------\n";
      cout << "read macro grid from < " << macroname << " > !" << endl;
      cout << "-----------------------------------------------\n";
    }

    {
#ifdef PARALLEL
      GitterDunePll grid(macroname.c_str(),mpa);
      grid.duneLoadBalance();
#else 
      GitterDuneImpl grid(macroname.c_str());
#endif
      bool closure = needConformingClosure( grid, true );
#ifdef PARALLEL
      closure = mpa.gmax( closure );
#endif
      if( closure ) grid.enableConformingClosure() ;

      //cout << "P[ " << rank << " ] : Grid generated! \n";
      grid.printsize(); 
      cout << "---------------------------------------------\n";
    
      grid.printMemUsage();
      for (int i = 0; i < glb; ++i)
        globalRefine(grid, true,-1);
      for (int i = 0; i < glb; ++i)
        globalRefine(grid, false,0);
      for( int i = 0; i < mxl; ++i )
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(i) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
        globalRefine(grid, false,i);
      }
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(mxl) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
      globalCoarsening(grid,3*glb);
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(mxl+1) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
    }
  }

  MPI_Finalize();
  return 0;
}

