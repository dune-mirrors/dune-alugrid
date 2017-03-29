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
#include <fstream>

#define ENABLE_ALUGRID_VTK_OUTPUT

// include serial part of ALUGrid
#include <dune/alugrid/3d/alu3dinclude.hh>

typedef ALUGrid::Gitter::AdaptRestrictProlong AdaptRestrictProlongType;

typedef ALUGrid::Gitter::helement_STI  HElemType;    // Interface Element
typedef ALUGrid::Gitter::hface_STI     HFaceType;    // Interface Element
typedef ALUGrid::Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef ALUGrid::Gitter::vertex_STI    HVertexType;  // Interface Element
typedef ALUGrid::Gitter::hbndseg       HGhostType;


struct ExchangeBaryCenter : public ALUGrid::GatherScatter
{
  typedef ALUGrid::GatherScatter :: ObjectStreamType  ObjectStreamType;
  typedef ALUGrid::Gitter::helement_STI  helement_STI;
  typedef ALUGrid::Gitter::hface_STI     hface_STI;
  typedef ALUGrid::Gitter::hbndseg       hbndseg;

#if HAVE_MPI
  typedef ALUGrid::GitterDunePll  GitterType ;
#else
  typedef ALUGrid::GitterDuneImpl GitterType ;
#endif

  ExchangeBaryCenter () {}

  bool contains ( int dim, int codim ) const { return codim == 0; }


  virtual bool containsItem(const helement_STI &elem ) const { return elem.isLeafEntity(); }
  virtual bool containsItem(const HGhostType & ghost) const { return ghost.isLeafEntity(); }
  virtual bool containsInterior (const hface_STI  & face , ALUGrid::ElementPllXIF_t & elif) const
  {
    return face.isInteriorLeaf();
  }

  void computeBaryCenter( const helement_STI& elem, double (&center)[3] ) const
  {
    if( elem.type() == ALUGrid::tetra )
    {
      typedef typename GitterType :: Objects :: tetra_IMPL tetra_IMPL ;
      // mark element for refinement
      tetra_IMPL& tetra = ((tetra_IMPL &) elem);
      ALUGrid::LinearMapping::
        barycenter(
          tetra.myvertex (0)->Point (),
          tetra.myvertex (1)->Point (),
          tetra.myvertex (2)->Point (),
          tetra.myvertex (3)->Point (),
          center);
    }
    else
    {
      typedef typename GitterType :: Objects :: hexa_IMPL hexa_IMPL ;
      // mark element for refinement
      hexa_IMPL& hexa = ((hexa_IMPL &) elem);
      ALUGrid::TrilinearMapping::
        barycenter(
          hexa.myvertex (0)->Point (),
          hexa.myvertex (1)->Point (),
          hexa.myvertex (2)->Point (),
          hexa.myvertex (3)->Point (),
          hexa.myvertex (4)->Point (),
          hexa.myvertex (5)->Point (),
          hexa.myvertex (6)->Point (),
          hexa.myvertex (7)->Point (),
          center);
    }
  }

  void writeBaryCenter( ObjectStreamType & str, const helement_STI& elem ) const
  {
    double center[ 3 ] = { 0 };
    computeBaryCenter( elem, center );
    for( int i=0; i<3; ++i )
      str.write( center[ i ] );
  }

  void readBaryCenter( ObjectStreamType & str, const helement_STI& elem ) const
  {
    double checkCenter[ 3 ] = { 0 };
    computeBaryCenter( elem, checkCenter );
    double center[ 3 ] = { 0 };
    double sum = 0 ;
    for( int i=0; i<3; ++i )
    {
      str.read( center[ i ] );
      double diff = center[ i ] - checkCenter[ i ];
      sum += (diff * diff);
    }

    //std::cout << "Got   c = { " << center[ 0 ] << ", " << center[ 1 ] << ", " << center[ 2 ] << " }" << std::endl;
    //std::cout << "Check b = { " << checkCenter[ 0 ] << ", " << checkCenter[ 1 ] << ", " << checkCenter[ 2 ] << " }" << std::endl << std::endl;

    if( sum > 1e-10 )
    {
      std::cerr << "ERROR: barycenter do not match!!!" << std::endl;
    }
  }

  virtual void sendData ( ObjectStreamType & str , const helement_STI  & elem )
  {
    writeBaryCenter( str, elem );
  }

  virtual void recvData ( ObjectStreamType & str , hbndseg & ghost )
  {
    helement_STI* elem = ghost.getGhost().first;
    if( elem == 0 )
    {
      std::cerr << "ERROR: no ghost element found!!!" << std::endl;
      abort();
    }
    readBaryCenter( str, *elem );
  }

  // only needed for backward communication
  virtual void sendData ( ObjectStreamType & str , const hbndseg & elem ) { alugrid_assert (false); abort(); }
  virtual void recvData ( ObjectStreamType & str , helement_STI  & elem ) { alugrid_assert (false); abort(); }

  virtual void inlineData ( ObjectStreamType & str , HElemType & elem ) {}
  virtual void xtractData ( ObjectStreamType & str , HElemType & elem ) {}
};



#if HAVE_MPI
#warning "RUNNING PARALLEL VERSION"
#endif

template <class GitterType, class element_t>
void checkElement( GitterType& grid, element_t& elem )
{
  if( elem.type() == ALUGrid::tetra )
  {
    typedef typename GitterType :: Objects :: tetra_IMPL tetra_IMPL ;
    // mark element for refinement
    tetra_IMPL& item = ((tetra_IMPL &) elem);

    for( int i=0; i<4; ++i )
    {
      assert( item.myneighbour( i ).first->isRealObject() );
    }
  }
  else
  {
    typedef typename GitterType :: Objects :: hexa_IMPL hexa_IMPL ;
    // mark element for refinement
    hexa_IMPL& item = ((hexa_IMPL &) elem);

    for( int i=0; i<6; ++i )
    {
      assert( item.myneighbour( i ).first->isRealObject() );
    }
  }
}

// refine grid globally, i.e. mark all elements and then call adapt
template <class GitterType>
void globalRefine(GitterType& grid, int refcount)
{
   for (int count=refcount ; count > 0; count--)
   {
     std::cout << "Refine global: run " << refcount-count << std::endl;
     {
        // get LeafIterator which iterates over all leaf elements of the grid
       ALUGrid::LeafIterator < ALUGrid::Gitter::helement_STI > w (grid) ;

        for (w->first () ; ! w->done () ; w->next ())
        {
          // mark element for refinement
          w->item ().tagForGlobalRefinement ();
          checkElement( grid, w->item() );
        }
     }

     // adapt grid
     grid.adaptWithoutLoadBalancing ();

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
    std::cout << "Global Coarsening: run " << refcount-count << std::endl;
    {
       // get LeafIterator which iterates over all leaf elements of the grid
      ALUGrid::LeafIterator < ALUGrid::Gitter::helement_STI > w (grid) ;

       for (w->first () ; ! w->done () ; w->next ())
       {
         checkElement( grid, w->item() );
         // mark elements for coarsening
         w->item ().tagForGlobalCoarsening() ;
       }
    }

    // adapt grid
    grid.adaptWithoutLoadBalancing ();

    // print size of grid
    grid.printsize () ;

  }
}

// exmaple on read grid, refine global and print again
int main (int argc, char ** argv)
{
  int rank = 0;
#if HAVE_MPI
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
#endif

  int mxl = 0;
  const char* filename = 0 ;
  if (argc < 2)
  {
    filename = "./grids/reference.tetra";
    mxl = 1;
    std::cout << "usage: "<< argv[0] << " <macro grid> <opt: level> \n";
  }
  else
    filename = argv[ 1 ];

  if (argc < 3)
    std::cout << "Default level = "<< mxl << " choosen! \n";
  else
    mxl = atoi(argv[2]);

  std::string macroname( filename );

  if( rank == 0 )
  {
    std::cout << "\n-----------------------------------------------\n";
    std::cout << "read macro grid from < " << macroname << " > !" << std::endl;
    std::cout << "-----------------------------------------------\n";
  }

  std::stringstream backupname ;
  backupname << "file." << rank ;
  std::stringstream databuf;

  const int dim = 3;
  {
#if HAVE_MPI
    ALUGrid::MpAccessMPI a (MPI_COMM_WORLD);
    ALUGrid::GitterDunePll grid(dim, macroname.c_str(),a);
    grid.loadBalance() ;

    grid.tovtk( "out" );
#else
    std::ifstream infile( macroname.c_str());
    ALUGrid::GitterDuneImpl grid1( dim, infile );
    infile.close();

    std::ifstream infile2( macroname.c_str());
    ALUGrid::GitterDuneImpl grid2( dim, infile2 );
    infile2.close();
    ALUGrid::GitterDuneImpl& grid = grid2;

    std::cout << "Grid generated! \n";
    globalRefine(grid, mxl);
    std::cout << "---------------------------------------------\n";
#endif

    std::ofstream file( backupname.str().c_str() );
    grid.backup( file );
    file.close();

    globalCoarsening(grid, mxl);
  }

  {
    std::ifstream file( backupname.str().c_str() );
    // read grid from file
#if HAVE_MPI
    ALUGrid::MpAccessMPI a (MPI_COMM_WORLD);
    ALUGrid::GitterDunePll grid( dim, file, a);
#else
    ALUGrid::GitterDuneImpl grid( dim, file );
#endif
    grid.printsize();

    grid.restore( file );
    file.close();
    //globalRefine(grid, mxl);
    // adapt grid
    grid.backup( databuf );

    std::cout << "Grid restored!" << std::endl;
    grid.printsize();
    std::cout << "---------------------------------------------\n";

    globalCoarsening(grid, mxl);
  }

  {
    std::cout << "Try to read stringbuf:" << std::endl;
    std::cout << "Data Buffer size: " << databuf.str().size() << std::endl;
    // read grid from file
#if HAVE_MPI
    ALUGrid::MpAccessMPI a (MPI_COMM_WORLD);
    ALUGrid::GitterDunePll grid( dim, databuf, a);
#else
    ALUGrid::GitterDuneImpl grid( dim, databuf );
#endif
    grid.printsize();
    grid.restore( databuf );

    //grid.restore( file );
    // adapt grid

#if HAVE_MPI
    ExchangeBaryCenter dataHandle ;
    grid.interiorGhostCommunication( dataHandle, dataHandle, dataHandle, dataHandle );
#endif

    std::cout << "Grid restored!" << std::endl;
    grid.printsize();
    std::cout << "---------------------------------------------\n";

    globalCoarsening(grid, mxl);
  }

#if HAVE_MPI
  MPI_Finalize();
#endif
  return 0;
}

