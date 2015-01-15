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
//#define DONT_USE_ALUGRID_ALLOC

// include serial part of ALUGrid

template <class Item>
class GatherScatterImpl : public GatherScatter
{
  const int _codim;
public:
  GatherScatterImpl( const int codim ) : _codim( codim ) {}

  using GatherScatter :: inlineData ;
  using GatherScatter :: xtractData ;

  using GatherScatter :: sendData ;
  using GatherScatter :: recvData ;
  using GatherScatter :: setData ;

  typedef Item ItemType ;
  typedef ElementPllXIF_t PllElementType;

  bool contains(int dim, int codim) const
  {
    return codim == _codim ;
    // return codim == 0;
  }

  virtual bool containsInterior (const HFaceType  & face , ElementPllXIF_t & elif) const
  {
    return face.isInteriorLeaf();
  }
  virtual bool containsGhost (const HFaceType  & face , PllElementType& pll) const
  {
    return pll.ghostLeaf();
  }

  virtual bool containsItem (const ItemType & item) const
  {
    return item.isLeafEntity();
  }

  virtual void sendData ( ObjectStreamType & str , ItemType& item )
  {
    str.write(_codim);
  }

  virtual void recvData ( ObjectStreamType & str , ItemType& item )
  {
    int codim;
    str.read(codim);
  }

  virtual void setData ( ObjectStreamType & str , ItemType& item )
  {
    int codim;
    str.read( codim );
  }

  virtual void sendData ( ObjectStreamType & str, const HElemType& elem )
  {
    HElemType& el = const_cast< HElemType & > (elem);
    sendData( str, el );
  }

  virtual void sendData ( ObjectStreamType & str, const HGhostType& ghost )
  {
    // HElemType& item = getElement( ghost );
    str.write( _codim );
  }

  virtual void recvData ( ObjectStreamType & str, HGhostType& ghost )
  {
    // HElemType& item = getElement( ghost );
    int codim ;
    str.read( codim );
  }

  HElemType& getElement( const HGhostType& ghost )
  {
    HElemType* item = ghost.getGhost().first;

    // method getGhost can return 0, but then is something wrong
    assert(item);
    assert(item->isGhost());

    return *item;
  }
};


template <class GitterType>
void checkCommunication( GitterType& grid )
{
  typedef GatherScatterImpl< HVertexType > VertexData;
  typedef GatherScatterImpl< HEdgeType >   EdgeData;
  typedef GatherScatterImpl< HFaceType >   FaceData;
  typedef GatherScatterImpl< HElemType >   ElementData;

  VertexData vertexData( 3 );
  EdgeData edgeData( 2 );
  FaceData faceData( 1 );
  ElementData elementData( 0 );

  grid.interiorGhostCommunication(vertexData,edgeData,faceData,elementData);
  grid.borderBorderCommunication(vertexData,edgeData,faceData,elementData);
}

// refine grid globally, i.e. mark all elements and then call adapt
template <class GitterType>
void globalRefine(GitterType& grid, int refcount) {

   for (int count=refcount ; count > 0; count--) {
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
  MPI_Init(&argc,&argv);

  int rank = 0;
#ifdef PARALLEL
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
#endif

  int mxl = 0;
  const char* filename = 0 ;
  if (argc < 2)
  {
    filename = "../macrogrids/macro.big";
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

  {
#ifdef PARALLEL
    MpAccessMPI a (MPI_COMM_WORLD);
    GitterDunePll grid(macroname.c_str(),a);
    grid.printsize();
#else
    //GitterDuneImpl grid(macroname.c_str());
#endif

    //cout << "Grid generated! \n";
    //cout << "---------------------------------------------\n";

    //grid.printMemUsage();

    cout << "Start load balance " << endl;
    grid.duneLoadBalance();
    //globalRefine(&grid, mxl);


#ifdef PARALLEL
    grid.printsize();
    //checkCommunication( grid );
#endif

    //levelwalk(&grid, mxl);
    globalRefine(grid, mxl);
    checkCommunication( grid );
    //grid.printMemUsage();
  }

  MPI_Finalize();
  return 0;
}

