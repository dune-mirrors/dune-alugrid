//***********************************************************************
//
//  test geometry implementation of ALUGrid. 
//  Author: Robert Kloefkorn 
//
//***********************************************************************
#include <config.h>
#include <iostream>

// include serial part of ALUGrid 
#include <dune/alugrid/3d/alu3dinclude.hh>
#include <dune/alugrid/3d/geometry.hh>

#if HAVE_MPI
#warning RUNNING PARALLEL VERSION
#endif

// exmaple on read grid, refine global and print again 
int main (int argc, char ** argv, const char ** envp) 
{
#if HAVE_MPI
  Dune :: MPIHelper& mpi = Dune :: MPIHelper :: instance(&argc,&argv);
#endif
  const char* filename = 0 ;
  if (argc < 2) 
  {
    filename = "reference.tetra";
    std::cout << "usage: "<< argv[0] << " <macro grid> <opt: maxlevel> <opt: global refinement>\n";
  }
  else 
  {
    filename = argv[ 1 ];
  }

  {
    int rank = 0;
#if HAVE_MPI
    ALUGrid::MpAccessMPI mpa (MPI_COMM_WORLD);
    rank = mpa.myrank();
#endif

    std::string macroname( filename );

    if( rank == 0 ) 
    {
      std::cout << "\n-----------------------------------------------\n";
      std::cout << "read macro grid from < " << macroname << " > !" << std::endl;
      std::cout << "-----------------------------------------------\n";
    }

    {
#if HAVE_MPI
      ALUGrid::GitterDunePll* gridPtr = new ALUGrid::GitterDunePll(macroname.c_str(),mpa);
      ALUGrid::GitterDunePll& grid = *gridPtr ;
#else 
      ALUGrid::GitterDuneImpl* gridPtr = new ALUGrid::GitterDuneImpl(macroname.c_str());
      ALUGrid::GitterDuneImpl& grid = *gridPtr ;
#endif
    
      if( output )
      {
        std::ostringstream ss;
        ss << "start-" << ZeroPadNumber(mxl) << ".vtu";
        grid.tovtk(  ss.str().c_str() );
      }
    }
  }

  return 0;
}

