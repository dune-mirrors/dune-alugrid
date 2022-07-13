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

  // TODO: dynamic_cast of entity fails
//  {
//    // 2D
//    std::cout << "============= 2D =============" << std::endl;
//
//    typedef Dune::ALUGrid<2, 2, Dune::cube, Dune::nonconforming> GridType;
//    Dune::GridPtr<GridType> gridPtr( "./dgf/cube-testgrid-2-2.dgf" );
//    GridType& alugrid = *gridPtr;
//    alugrid.globalRefine(4);
//    Dune::VirtualizedGrid<2, 2> vgrid( alugrid );
//
//    timer.reset();
//    gridcheck(alugrid);
//    std::cout << "ALUGrid<2, 2, Dune::simplex, Dune::nonconforming>: " << timer.elapsed() << std::endl;
//    std::cout << "------------------------------" << std::endl;
//
//    timer.reset();
//    gridcheck(vgrid);
//    std::cout << "Virtualized<2, 2>: " << timer.elapsed() << std::endl;
//    std::cout << "=============================" << std::endl << std::endl;
//  }m

  {
    // 3D
    std::cout << "============= 2D-3D =============" << std::endl;

    typedef Dune::ALUGrid<2, 3, Dune::simplex, Dune::nonconforming> GridType;
    Dune::GridPtr<GridType> gridPtr( "./dgf/simplex-testgrid-2-3.dgf" );
    GridType& alugrid = *gridPtr;
    Dune::VirtualizedGrid<2, 3> vgrid( alugrid );

    timer.reset();
    gridcheck(alugrid);
    std::cout << "ALUGrid<2, 3, Dune::simplex, Dune::nonconforming>: " << timer.elapsed() << std::endl;
    std::cout << "------------------------------" << std::endl;

    timer.reset();
    gridcheck(vgrid);
    std::cout << "Virtualized<2, 3>: " << timer.elapsed() << std::endl;
    std::cout << "=============================" << std::endl << std::endl;
  }

  {
    // 3D
    std::cout << "============= 3D =============" << std::endl;

    typedef Dune::ALUGrid<3, 3, Dune::simplex, Dune::nonconforming> GridType;
    Dune::GridPtr<GridType> gridPtr( "./dgf/simplex-testgrid-3-3.dgf" );
    GridType& alugrid = *gridPtr;
    Dune::VirtualizedGrid<3, 3> vgrid( alugrid );

    timer.reset();
    gridcheck(alugrid);
    std::cout << "ALUGrid<3, 3, Dune::simplex, Dune::nonconforming>: " << timer.elapsed() << std::endl;
    std::cout << "------------------------------" << std::endl;

    timer.reset();
    gridcheck(vgrid);
    std::cout << "Virtualized<3, 3>: " << timer.elapsed() << std::endl;
    std::cout << "=============================" << std::endl << std::endl;
  }

  return 0;
}
