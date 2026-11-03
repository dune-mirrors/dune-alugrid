// Reproducer: ALUGrid macro grid with periodic (torus) connectivity. Vertices on
// opposite boundaries are inserted only once, so the grid is topologically periodic.
//
// Usage: ./alugrid_periodic_macrogrid [grid=0..4] [n=4]
//   grid: 0 ALUGrid<2,2,cube>  1 ALUGrid<2,2,simplex>  2 ALUGrid<2,3,simplex>
//         3 ALUGrid<2,3,cube>  4 ALUGrid<3,3,cube>   (all nonconforming)
#include <config.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/grid/common/gridfactory.hh>
#include <dune/alugrid/grid.hh>

template <class Grid>
void test (int n, bool simplex)
{
  constexpr int dim = Grid::dimension;
  constexpr int dow = Grid::dimensionworld;
  Dune::GridFactory<Grid> factory;
  auto vid = [n](int i, int j) { return unsigned((j%n)*n + (i%n)); };
  int vCount=0;
  for (int j = 0; j < n; ++j)
    for (int i = 0; i < n; ++i) {
      Dune::FieldVector<double,dow> x(0.0);
      x[0] = double(i)/n; x[1] = double(j)/n;
      factory.insertVertex(x);
      std::cout << "vectex id=" << vCount << " coordinates (" << x[0] << "," << x[1] << ")" << std::endl;
      vCount++;
    }
  int elCount=0;
  for (int j = 0; j < n; ++j)
    for (int i = 0; i < n; ++i) {
      if (simplex) {
        factory.insertElement(Dune::GeometryTypes::simplex(dim), {vid(i,j), vid(i+1,j), vid(i,j+1)});
        factory.insertElement(Dune::GeometryTypes::simplex(dim), {vid(i+1,j), vid(i+1,j+1), vid(i,j+1)});
        //std::cout << "element id=" << elCount << " with corner ids "
        //          << "(" << vid(i,j) << "," << vid(i+1,j) << "," << vid(i,j+1) << ")" << std::endl;
        //std::cout << "element id=" << elCount+1 << " with corner ids "
        //          << "(" << vid(i+1,j) << "," << vid(i+1,j+1) << "," << vid(i,j+1) << ")" << std::endl;
        elCount += 2;
      } else
        factory.insertElement(Dune::GeometryTypes::cube(dim), {vid(i,j), vid(i+1,j), vid(i,j+1), vid(i+1,j+1)});
    }
  factory.unsetAllFlags()
  auto grid = factory.createGrid();
  std::cout << "elements: " << grid->size(0) << ", vertices: " << grid->size(dim) << std::endl;

  auto gv = grid->leafGridView();
  std::size_t numIntersections = 0, numNeighbors = 0;
  for (auto const& e : elements(gv))
    for (auto const& is : intersections(gv, e)) {
      ++numIntersections;
      numNeighbors += is.neighbor();
    }
  std::cout << "intersections: " << numIntersections << ", with neighbor: " << numNeighbors << std::endl;
}

// 3d: n x n x n hexahedra on the 3-torus
template <class Grid>
void test3d (int n)
{
  Dune::GridFactory<Grid> factory;
  auto vid = [n](int i, int j, int k) { return unsigned(((k%n)*n + (j%n))*n + (i%n)); };
  for (int k = 0; k < n; ++k)
    for (int j = 0; j < n; ++j)
      for (int i = 0; i < n; ++i)
        factory.insertVertex({double(i)/n, double(j)/n, double(k)/n});
  for (int k = 0; k < n; ++k)
    for (int j = 0; j < n; ++j)
      for (int i = 0; i < n; ++i)
        factory.insertElement(Dune::GeometryTypes::cube(3),
          {vid(i,j,k), vid(i+1,j,k), vid(i,j+1,k), vid(i+1,j+1,k),
           vid(i,j,k+1), vid(i+1,j,k+1), vid(i,j+1,k+1), vid(i+1,j+1,k+1)});
  auto grid = factory.createGrid();
  std::cout << "elements: " << grid->size(0) << ", vertices: " << grid->size(3) << std::endl;
  auto gv = grid->leafGridView();
  std::size_t numIntersections = 0, numNeighbors = 0;
  for (auto const& e : elements(gv))
    for (auto const& is : intersections(gv, e)) {
      ++numIntersections;
      numNeighbors += is.neighbor();
    }
  std::cout << "intersections: " << numIntersections << ", with neighbor: " << numNeighbors << std::endl;
}

int main (int argc, char** argv)
{
  Dune::MPIHelper::instance(argc, argv);
  int n = argc > 1 ? std::atoi(argv[1]) : 4;
  int g = argc > 2 ? std::atoi(argv[2]) : 22;
  int s = argc > 3 ? std::atoi(argv[3]) : 1;
  if (g==22)
  {
    if (s)
      test<Dune::ALUGrid<2,2,Dune::simplex,Dune::conforming>>(n, true);
    else
      test<Dune::ALUGrid<2,2,Dune::cube,Dune::nonconforming>>(n, false);
  }
  if (g==32)
  {
    if (s)
      test<Dune::ALUGrid<2,3,Dune::simplex,Dune::conforming>>(n, true);
    else
      test<Dune::ALUGrid<2,3,Dune::cube,Dune::nonconforming>>(n, false);
  }
  if (g==33)
    test3d<Dune::ALUGrid<3,3,Dune::cube,Dune::nonconforming>>(n);
}
