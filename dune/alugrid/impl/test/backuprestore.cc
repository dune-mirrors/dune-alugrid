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

// include serial part of ALUGrid 
#include <dune/alugrid/3d/alu3dinclude.hh>

typedef ALUGrid::Gitter::AdaptRestrictProlong AdaptRestrictProlongType;

typedef ALUGrid::Gitter::helement_STI  HElemType;    // Interface Element
typedef ALUGrid::Gitter::hface_STI     HFaceType;    // Interface Element
typedef ALUGrid::Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef ALUGrid::Gitter::vertex_STI    HVertexType;  // Interface Element
typedef ALUGrid::Gitter::hbndseg       HGhostType;

#if HAVE_MPI
#warning RUNNING PARALLEL VERSION
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
int main (int argc, char ** argv, const char ** envp) 
{
  MPI_Init(&argc,&argv);
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

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
  {
#if HAVE_MPI
    ALUGrid::MpAccessMPI a (MPI_COMM_WORLD);
    ALUGrid::GitterDunePll grid(macroname.c_str(),a);
    grid.duneLoadBalance() ;
#else 
    std::ifstream infile( macroname.c_str());
    ALUGrid::GitterDuneImpl grid1( infile );
    infile.close();

    std::ifstream infile2( macroname.c_str());
    ALUGrid::GitterDuneImpl grid2( infile2 );
    infile2.close();
    ALUGrid::GitterDuneImpl& grid = grid2; 
   
    std::cout << "Grid generated! \n";
    globalRefine(grid, mxl);
    std::cout << "---------------------------------------------\n";
#endif

    std::ofstream file( backupname.str().c_str() );
    grid.duneBackup( file );
    file.close();

    globalCoarsening(grid, mxl);
  }

  {
    std::ifstream file( backupname.str().c_str() );
    // read grid from file 
#if HAVE_MPI
    ALUGrid::MpAccessMPI a (MPI_COMM_WORLD);
    ALUGrid::GitterDunePll grid( file, a);
#else 
    ALUGrid::GitterDuneImpl grid( file );
#endif
    grid.printsize();

    grid.duneRestore( file );
    file.close();
    //globalRefine(grid, mxl);
    // adapt grid 

    grid.duneBackup( databuf );

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
    ALUGrid::GitterDunePll grid( databuf, a);
#else 
    ALUGrid::GitterDuneImpl grid( databuf );
#endif
    grid.printsize();
    grid.duneRestore( databuf );

    //grid.duneRestore( file );
    // adapt grid 

    std::cout << "Grid restored!" << std::endl;
    grid.printsize();
    std::cout << "---------------------------------------------\n";

    globalCoarsening(grid, mxl);
  }

  MPI_Finalize();
  return 0;
}

