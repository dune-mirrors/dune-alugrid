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
#if 0
     {
        // get LeafIterator which iterates over all leaf elements of the grid 
        LeafIterator < Gitter::helement_STI > w (grid) ;
        
        for (w->first () ; ! w->done () ; w->next ())
        {
          // mark element for refinement 
          // w->item ().tagForGlobalRefinement ();

          typedef typename GitterType :: Objects :: tetra_IMPL tetra_IMPL ;
          // mark element for refinement 
          tetra_IMPL* item = ((tetra_IMPL *) &w->item ());

          //item->checkTetra( item, item->nChild(), true );

          typedef Gitter ::Geometric :: TetraRule  TetraRule ;
          item->request ( TetraRule :: bisect );

	        // break ;
        }
     }
     // adapt grid 
     grid.adapt ();

#endif
     double center[3] = {0.2,0.2,0.2};
     double rad=0.4;
     grid.refineBall(center,rad,10);

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

     
      //cout << "P[ " << rank << " ] : Grid generated! \n";
      grid.printsize(); 
      cout << "---------------------------------------------\n";
    
      grid.printMemUsage();
      //int bla; 
      // cin >> bla;
      for( int i = 0; i <= mxl; ++i )
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(i) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
        if( i < mxl )
          globalRefine(grid, 1);
      }
     
#if 0
      checkRefinements( grid );

      std::ofstream file( "file.out" );
      grid.duneBackup( file );
      file.close();

      /*
      {
        ObjectStream os ;
        grid.duneBackup( os );

        char* buffer = ObjectStream ::allocateBuffer( os.size() );
        os.read( buffer, os.size() );
        
        std::ofstream obj( "obj.out" );
        const size_t size = os.size();
        obj.write( (const char *) &size, sizeof( size_t ) ); 
        obj.write( buffer, size );
        ObjectStream :: freeBuffer( buffer );
      }

      globalRefine(grid, mxl);

      {
        size_t size = 0;

        std::ifstream obj( "obj.out" );
        obj.read( (char *) &size, sizeof( size_t ) ); 
        char * buffer = ObjectStream ::allocateBuffer( size );
        obj.read( buffer, size );
        //ObjectStream :: freeBuffer( buffer );

        ObjectStream is;
        is.clear();
        is.write( buffer, size );

        {
          ObjectStream copy( is );
          std::ofstream obj( "check.out" );
          const size_t size = copy.size();
          obj.write( buffer, size );
          //ObjectStream :: freeBuffer( buffer );
        }
        }*/

      {
        std::ifstream file( "file.out" );
#ifdef PARALLEL
        MpAccessMPI a (MPI_COMM_WORLD);
        GitterDunePll grid2( file, a);
#else 
        GitterDuneImpl grid2( file );
#endif
        grid2.printsize();
      }
      //levelwalk(grid, mxl);
      // globalCoarsening(grid, mxl);
      //grid.printMemUsage();
      //cin.get();
#endif
    }
  }

  MPI_Finalize();
  return 0;
}

