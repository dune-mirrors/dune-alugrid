// Reproducer: concurrent calls of Entity::geometryInFather() in ALUGrid.
//
// ALUGrid declares Capabilities::viewThreadSafe = true. Each thread works on a
// disjoint set of leaf elements and calls only const methods. Still, the program
// aborts with "free(): double free detected" or a segmentation fault.
//
// Usage: ./alugrid_geometryinfather_threads [grid=0..4] [mode=0|1] [threads=8]
//   grid: 0 ALUGrid<2,2,cube,nonconforming>    1 ALUGrid<3,3,cube,nonconforming>
//         2 ALUGrid<2,2,simplex,nonconforming> 3 ALUGrid<3,3,simplex,nonconforming>
//         4 ALUGrid<2,2,simplex,conforming>
//   mode: 0 geometryInFather(), 1 intersection geometryInInside() (for comparison)
#include <config.h>
#include <array>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/grid/common/capabilities.hh>
#include <dune/grid/utility/structuredgridfactory.hh>
#include <dune/alugrid/grid.hh>

template <class Grid>
void test (int mode, int numThreads)
{
  constexpr int dim = Grid::dimension;
  static_assert(Dune::Capabilities::viewThreadSafe<Grid>::v);

  Dune::FieldVector<double,dim> lower(0.0), upper(1.0);
  std::array<unsigned int,dim> cells; cells.fill(dim == 2 ? 32 : 12);
  auto grid = Dune::Capabilities::hasSingleGeometryType<Grid>::topologyId == Dune::GeometryTypes::cube(dim).id()
    ? Dune::StructuredGridFactory<Grid>::createCubeGrid(lower, upper, cells)
    : Dune::StructuredGridFactory<Grid>::createSimplexGrid(lower, upper, cells);
  grid->globalRefine(2);

  using Seed = typename Grid::template Codim<0>::EntitySeed;
  std::vector<Seed> seeds;
  for (auto const& e : elements(grid->leafGridView()))
    seeds.push_back(e.seed());

  for (int rep = 0; rep < 20; ++rep) {
    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t)
      threads.emplace_back([&,t]{
        double sum = 0.0;
        auto gv = grid->leafGridView();
        for (std::size_t i = t; i < seeds.size(); i += numThreads) {
          auto e = grid->entity(seeds[i]);
          if (mode == 0)
            sum += e.geometryInFather().volume();
          else
            for (auto const& is : intersections(gv, e))
              sum += is.geometryInInside().volume();
        }
      });
    for (auto& th : threads)
      th.join();
  }
  std::cout << "finished without error" << std::endl;
}

int main (int argc, char** argv)
{
  Dune::MPIHelper::instance(argc, argv);
  int grid = argc > 1 ? std::atoi(argv[1]) : 0;
  int mode = argc > 2 ? std::atoi(argv[2]) : 0;   // 0: geometryInFather, 1: intersection geometryInInside
  int numThreads = argc > 3 ? std::atoi(argv[3]) : 8;
  if (grid == 0) test<Dune::ALUGrid<2,2,Dune::cube,Dune::nonconforming>>(mode, numThreads);
  if (grid == 1) test<Dune::ALUGrid<3,3,Dune::cube,Dune::nonconforming>>(mode, numThreads);
  if (grid == 2) test<Dune::ALUGrid<2,2,Dune::simplex,Dune::nonconforming>>(mode, numThreads);
  if (grid == 3) test<Dune::ALUGrid<3,3,Dune::simplex,Dune::nonconforming>>(mode, numThreads);
  if (grid == 4) test<Dune::ALUGrid<2,2,Dune::simplex,Dune::conforming>>(mode, numThreads);
}
