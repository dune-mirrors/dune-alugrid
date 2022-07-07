#include "config.h"

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/timer.hh>
#include <dune/grid/virtualizedgrid.hh>
#include <dune/grid/test/gridcheck.hh>
#include <dune/alugrid/grid.hh>
#include <doc/grids/gridfactory/testgrids.hh>


int main(int argc, char** argv)
{
  Dune::MPIHelper::instance(argc, argv);
  Dune::Timer timer;

  {
    // 2D
    std::cout << "============= 2D =============" << std::endl;

    typedef Dune::ALUGrid< 2, 2, Dune::cube, Dune::nonconforming > GridType;
    Dune::GridPtr<GridType> gridPtr( "./dgf/cube-testgrid-2-2.dgf" );
    GridType& alugrid2 = *gridPtr;
    alugrid2.globalRefine(4);
    Dune::VirtualizedGrid<2, 2> vgrid2( alugrid2 );

    timer.reset();
    gridcheck(alugrid2);
    std::cout << "ALUGrid<2>: " << timer.elapsed() << std::endl;
    std::cout << "------------------------------" << std::endl;

    timer.reset();
    gridcheck(vgrid2);
    std::cout << "Virtualized<2>: " << timer.elapsed() << std::endl;
    std::cout << "=============================" << std::endl << std::endl;
  }

  return 0;
}
