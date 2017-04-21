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

struct EmptyGatherScatter : public ALUGrid::GatherScatter
{
  typedef ALUGrid::GatherScatter :: ObjectStreamType  ObjectStreamType;
  const int _rank;
  const int _size;
  const bool _userPartitioning;

  EmptyGatherScatter (const int rank, const int size, const bool useUserPart )
    : _rank( rank ), _size( size ), _userPartitioning( useUserPart ) {}

  virtual bool userDefinedPartitioning () const { return _userPartitioning; }
  virtual bool userDefinedLoadWeights  () const { return false ; }
  virtual bool repartition () { return true ; }
  virtual int destination( const ALUGrid::Gitter::helement_STI &elem ) const
  {
    return _rank < (_size-1) ? _rank+1 : 0 ;
  }

  bool hasUserData () const { return true; }

  bool contains ( int, int ) const { return true ;}

  virtual void inlineData ( ObjectStreamType & str , HElemType & elem, const int ) {}
  virtual void xtractData ( ObjectStreamType & str , HElemType & elem ) {}
};

struct EmptyAdaptRestrictProlong : public ALUGrid::Gitter :: AdaptRestrictProlong
{
  virtual int preCoarsening (HElemType & elem )   { return 1; }
  virtual int postRefinement (HElemType & elem ) { return 1; }
  virtual int preCoarsening (HGhostType & bnd )     { return 1; }
  virtual int postRefinement (HGhostType & bnd )    { return 1; }
};


// refine grid globally, i.e. mark all elements and then call adapt
template <class GitterType>
bool needConformingClosure( GitterType& grid, bool useClosure )
{
  bool needClosure = false ;
  {
    // get LeafIterator which iterates over all leaf elements of the grid
    ALUGrid::LeafIterator < HElemType > w (grid) ;
    w->first();
    if( ! w->done() )
    {
      if( w->item ().type() == ALUGrid::tetra )
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
    ALUGrid::LeafIterator < HElemType > w (grid) ;
    w->first();
    if( ! w->done() )
    {
      if( w->size() > 1 || w->item ().type() != ALUGrid::tetra ) return ;
    }
  }

  typedef ALUGrid::Gitter ::Geometric :: TetraRule  TetraRule ;
  const ALUGrid::Gitter ::Geometric :: TetraRule rules[ 6 ] =
  { TetraRule :: e01, TetraRule :: e12, TetraRule :: e20,
    TetraRule :: e23, TetraRule :: e30, TetraRule :: e31 };

  for (int i=0; i<6; ++i )
  {
    std::cout << "*********************************************" <<std::endl;
    std::cout << "Refinement rule " << rules[ i ] << std::endl;
    std::cout << "*********************************************" <<std::endl;

    {
      // get LeafIterator which iterates over all leaf elements of the grid
      ALUGrid::LeafIterator < HElemType > w (grid) ;

      for (w->first () ; ! w->done () ; w->next ())
      {
        if( w->item ().type() == ALUGrid::tetra )
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

  std::cout << "*********************************************" <<std::endl;
  std::cout << " Check of rules done " << std::endl;
  std::cout << "*********************************************" <<std::endl;
}

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

     // create empty gather scatter
     EmptyAdaptRestrictProlong rp;

     // adapt grid
     grid.duneAdapt( rp );

#if HAVE_MPI
     if( loadBalance )
     {
       EmptyGatherScatter gs ( grid.mpAccess().myrank(), grid.mpAccess().psize(), false );
       // load balance
       grid.loadBalance( &gs );
     }
#endif

     if( printOutput )
     {
       // print size of grid
       grid.printsize () ;
     }
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
      ALUGrid::LeafIterator < HElemType > w (grid) ;

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

    {
#if HAVE_MPI
      ALUGrid::GitterDunePll* gridPtr = new ALUGrid::GitterDunePll(3, macroname.c_str(),mpa);
#else
      ALUGrid::GitterDuneImpl* gridPtr = new ALUGrid::GitterDuneImpl(3, macroname.c_str());
#endif
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
      EmptyGatherScatter gs ( mpa.myrank(), mpa.psize(), false );
      gridPtr->loadBalance();
      //if( ! closure )
      //  gridPtr = ALUGrid::GitterDunePll::compress( gridPtr );
      ALUGrid::GitterDunePll& grid = *gridPtr ;
#else
      ALUGrid::GitterDuneImpl& grid = *gridPtr ;
#endif

      //std::cout << "P[ " << rank << " ] : Grid generated! \n";
      if( printOutput )
      {
        grid.printsize();
        std::cout << "---------------------------------------------\n";
      }

#ifdef ENABLE_ALUGRID_VTK_OUTPUT
      {
        std::ostringstream ss;
        ss << "start-" << ZeroPadNumber(mxl) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
#endif

      for (int i = 0; i < glb; ++i)
        globalRefine(grid, true, -1, mxl, true, printOutput);
      for (int i = 0; i < glb; ++i)
        globalRefine(grid, false,0, mxl, true, printOutput);
      for( int i = 0; i < 4*mxl; ++i )
      {
#ifdef ENABLE_ALUGRID_VTK_OUTPUT
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(i) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
#endif
        globalRefine(grid, false,i, mxl, true, printOutput);
      }
      /*
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(mxl) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
      */
      /*
      globalCoarsening(grid,3*glb);
      {
        std::ostringstream ss;
        ss << "out-" << ZeroPadNumber(mxl+1) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
      */
      grid.printsize();
    }
  }

#if HAVE_MPI
  MPI_Finalize();
#endif
  return 0;
}

