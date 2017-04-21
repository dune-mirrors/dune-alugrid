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
#include <stdlib.h>
#include <time.h>

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



// refine grid globally, i.e. mark all elements and then call adapt
template <class GitterType>
void checkRefinements( GitterType& grid, int n )
{
  // if bisection is not enabled do nothing here
  bool isHexa = false ;
  {
    // get LeafIterator which iterates over all leaf elements of the grid
    ALUGrid::LeafIterator < HElemType > w (grid) ;
    w->first();
    if( ! w->done() )
    {
      if(  w->item ().type() != ALUGrid::tetra )
      {
        isHexa = true;
      }
    }
  }

  if( isHexa )
  {
    typedef ALUGrid::Gitter ::Geometric :: HexaRule  HexaRule ;
    const HexaRule rule = HexaRule::regular;

    {

      std::cout << "*********************************************" <<std::endl;
      std::cout << "Refinement rule " << rule << std::endl;
      std::cout << "*********************************************" <<std::endl;

      // get LeafIterator which iterates over all leaf elements of the grid
      ALUGrid::LeafIterator < HElemType > w (grid) ;

      // create empty gather scatter
      EmptyAdaptRestrictProlong rp;

      for(int j = 0; j<n ; ++j)
      {

        for (w->first () ; ! w->done () ; w->next ())
        {
          if( w->item ().type() == ALUGrid::hexa )
          {
            typedef typename GitterType :: Objects :: hexa_IMPL hexa_IMPL ;
            // mark element for refinement
            hexa_IMPL* item = ((hexa_IMPL *) &w->item ());

            item->request ( rule );
          }
        }

        // adapt grid
        grid.duneAdapt( rp );

        // print size of grid
        grid.printsize () ;

      }

      // coarsen again
      globalCoarsening( grid , n );
    }
  }
  else // tetra
  {
    typedef ALUGrid::Gitter ::Geometric :: TetraRule  TetraRule ;
    const TetraRule rules[ 2 ] =
    { TetraRule::regular,
     // TetraRule :: e01, TetraRule :: e12, TetraRule :: e20,
     // TetraRule :: e23, TetraRule :: e30, TetraRule :: e31,
     // TetraRule::iso4_2d,
      TetraRule :: bisect
    };

    for (int i=0; i<2; ++i )
    {
      std::cout << "*********************************************" <<std::endl;
      std::cout << "Refinement rule " << rules[ i ] << std::endl;
      std::cout << "*********************************************" <<std::endl;

      {
        // get LeafIterator which iterates over all leaf elements of the grid
        ALUGrid::LeafIterator < HElemType > w (grid) ;

        if (rules[ i ] == TetraRule::bisect ) grid.enableConformingClosure();


      // create empty gather scatter
      EmptyAdaptRestrictProlong rp;

      //initialize random seed
      srand(time(NULL));


      for(int j=0; j<n ; ++j){
         int cnt =0;

      for (w->first () ; ! w->done () ; w->next ())
        {
        ++cnt;
          if( w->item ().type() == ALUGrid::tetra )
          {
            typedef typename GitterType :: Objects :: tetra_IMPL tetra_IMPL ;
            // mark element for refinement
            tetra_IMPL* item = ((tetra_IMPL *) &w->item ());

           //if(rand() % 100 > 50){ //do some random refinement to simulate adaptive behavior

           if(cnt < 3){ //always refine the first  elements
              item->request ( rules[ i ] );
            }
          }
        }


      // adapt grid
      grid.duneAdapt( rp );

      // print size of grid
      grid.printsize () ;
       }
      }

      // coarsen again - to be on the safe side because of conforming Closure take 2 times refinement steps
      globalCoarsening( grid , 2*n );
    } // end for
  }

  std::cout << "*********************************************" <<std::endl;
  std::cout << " Check of rules done " << std::endl;
  std::cout << "*********************************************" <<std::endl;
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

    const int dim = 2;
    {
#if HAVE_MPI
      ALUGrid::GitterDunePll* gridPtr = new ALUGrid::GitterDunePll(dim, macroname.c_str(),mpa);
#else
      ALUGrid::GitterDuneImpl* gridPtr = new ALUGrid::GitterDuneImpl(dim, macroname.c_str());
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


      checkRefinements( *gridPtr , 3);
    }
  }

#if HAVE_MPI
  MPI_Finalize();
#endif
  return 0;
}

