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
#include <mpi.h>

//#define COUNT_FLOPS

// enable vtk output 
#define PRINT_OUTPUT
//#define ENABLE_ALUGRID_VTK_OUTPUT

// include serial part of ALUGrid 
#include <dune/alugrid/3d/alu3dinclude.hh>

using namespace ALUGrid;

struct EmptyGatherScatter : public GatherScatter
{
  typedef GatherScatter :: ObjectStreamType  ObjectStreamType;
  typedef Gitter::helement_STI HElemType ;

  EmptyGatherScatter () {}

  virtual void inlineData ( ObjectStreamType & str , HElemType & elem ) {}
  virtual void xtractData ( ObjectStreamType & str , HElemType & elem ) {}
};

struct EmptyAdaptRestrictProlong : public Gitter :: AdaptRestrictProlong
{
  typedef Gitter::helement_STI HElemType ;
  typedef Gitter::hbndseg      HGhostType ;
  virtual int preCoarsening (HElemType & elem )   { return 1; } 
  virtual int postRefinement (HElemType & elem )  { return 1; }
  virtual int preCoarsening (HGhostType & bnd )   { return 1; }
  virtual int postRefinement (HGhostType & bnd )  { return 1; } 
};

template <class GitterType>
GitterType* memoryCompress( GitterType* grid )
{
  return GitterType :: compress( grid );
  //return grid;
}

// refine grid globally, i.e. mark all elements and then call adapt 
template <class GitterType>
bool needConformingClosure( GitterType& grid, bool useClosure ) 
{
  bool needClosure = false ;
  {
    // get LeafIterator which iterates over all leaf elements of the grid 
    LeafIterator < Gitter::helement_STI > w (grid) ;
    w->first(); 
    if( ! w->done() ) 
    {
      if( w->item ().type() == tetra )
      {
        needClosure = useClosure ;
      }
    }
  }
  return needClosure ;
}

       
// refine grid globally, i.e. mark all elements and then call adapt 
template <class GitterType>
void checkRefinements( GitterType& grid ) 
{
  // if bisection is not enabled do nothing here
  if( ! grid.conformingClosureNeeded() ) return ;

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
    std::cout << "*********************************************" << std::endl;
    std::cout << "Refinement rule " << rules[ i ] << std::endl;
    std::cout << "*********************************************" << std::endl;

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

    // create empty gather scatter 
    EmptyAdaptRestrictProlong rp;

    // adapt grid 
    grid.duneAdapt( rp );


    // coarsen again 
    globalCoarsening( grid , 1 );
  }

  std::cout << "*********************************************" << std::endl;
  std::cout << " Check of rules done " << std::endl;
  std::cout << "*********************************************" << std::endl;
}

// refine grid globally, i.e. mark all elements and then call adapt 
template <class GitterType>
void globalRefine(GitterType& grid, bool global, int step, int mxl,
                  const bool loadBalance = true )
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

     // create empty gather scatter 
     EmptyAdaptRestrictProlong rp;

     // adapt grid 
     grid.duneAdapt( rp );

#if HAVE_MPI
     if( loadBalance ) 
     {
       // load balance 
       grid.duneLoadBalance();
     }
#endif

#ifdef PRINT_OUTPUT
     // print size of grid 
     grid.printsize () ;
#endif
   }

}

// coarse grid globally, i.e. mark all elements for coarsening 
// and then call adapt 
template <class GitterType> 
void globalCoarsening(GitterType& grid, int refcount) {
    
  for (int count=refcount ; count > 0; count--) 
  {
    std::cout << "Global Coarsening: run " << refcount-count << std::endl;
    {
       // get leafiterator which iterates over all leaf elements of the grid 
       LeafIterator < Gitter::helement_STI > w (grid) ;
       
       for (w->first () ; ! w->done () ; w->next ())
       {
         // mark elements for coarsening  
         w->item ().tagForGlobalCoarsening() ;
       }
    }

    // create empty gather scatter 
    EmptyAdaptRestrictProlong rp;

    // adapt grid 
    grid.duneAdapt( rp );

    // print size of grid 
    grid.printsize () ;
  }

  //int i ;
  //std::cin >> i ;
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
     std::cout << "Element " << it.item().getIndex() << " has " << i++ << " as level index " << std::endl;
   }
   std::cout << std::endl;
}


// exmaple on read grid, refine global and print again 
int main (int argc, char ** argv, const char ** envp) 
{
#if HAVE_MPI
  MPI_Init(&argc,&argv);
#endif

  int mxl = 0, glb = 0; 
  const char* filename = 0 ;
  if (argc < 2) 
  {
    filename = "./grids/reference.tetra";
    mxl = 1;
    glb = 1;
    std::cout << "usage: "<< argv[0] << " <macro grid> <opt: level global> \n";
  }
  else 
  {
    filename = argv[ 1 ];
  }

  const bool useClosure = argc > 4 ;

  {
    int rank = 0;
#if HAVE_MPI
    MpAccessMPI mpa (MPI_COMM_WORLD);
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

    {
#if HAVE_MPI
      GitterDunePll* gridPtr = new GitterDunePll(macroname.c_str(),mpa);
#else 
      GitterDuneImpl* gridPtr = new GitterDuneImpl(macroname.c_str());
#endif
      // grid.disableGhostCells();
      //if( rank == 0 ) 
     // {
      //  int i ;
       // cin >> i ;
      //}

      bool closure = needConformingClosure( *gridPtr, useClosure );
#if HAVE_MPI
      closure = mpa.gmax( closure );
#endif
      if( closure ) 
      {
        gridPtr->enableConformingClosure() ;
        gridPtr->disableGhostCells();
      }
#if HAVE_MPI
      {
        GitterDunePll& grid = *gridPtr ;
        grid.duneLoadBalance();
      }
      gridPtr = memoryCompress( gridPtr );
      GitterDunePll& grid = *gridPtr ;
#else
      //printMemoryBytesUsed ();
      gridPtr = memoryCompress( gridPtr );
      GitterDuneImpl& grid = *gridPtr ;
#endif

      //cout << "P[ " << rank << " ] : Grid generated! \n";
#ifdef PRINT_OUTPUT
      grid.printsize(); 
      std::cout << "---------------------------------------------\n";
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
        globalRefine(grid, true, -1, mxl);
      for (int i = 0; i < glb; ++i)
        globalRefine(grid, false,0, mxl);
      for( int i = 0; i < 2*mxl; ++i )
      {
#ifdef ENABLE_ALUGRID_VTK_OUTPUT
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(i) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
#endif
        globalRefine(grid, false,i, mxl);
      }
      //int i;
      //std::cin >> i;

      /*
      {
        size_t memsize = std::pow( 2.0, 30.0 );
        std::vector< int >* vec = new std::vector< int >( memsize, int(1) );
        for( size_t i = 0; i<memsize; ++ i ) 
        {
          (*vec)[ i ] = -1;
        }

        delete vec;
      }
      */

      std::cout << "vec " << std::endl;
      //{
      //  int i;
      //  std::cin >> i;
      //}
      /*
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(mxl) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
      */
     // globalCoarsening(grid,3*glb);
#ifdef ENABLE_ALUGRID_VTK_OUTPUT
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(mxl+1) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
#endif
      grid.printsize(); 
      std::cout << "done" << std::endl;
      /*
      {
        int i;
        std::cin >> i;
      }
      */

      delete gridPtr ;
    }
  }

  // MyAlloc :: clearFreeMemory ();
  // while( 1 ){}
#if HAVE_MPI
  MPI_Finalize();
#endif
  return 0;
}

