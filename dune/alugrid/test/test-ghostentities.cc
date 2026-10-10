#include <config.h>

#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>

#include <dune/common/hybridutilities.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/grid/common/rangegenerators.hh>
#include <dune/grid/io/file/dgfparser/dgfparser.hh>

#include <dune/alugrid/grid.hh>
#include <dune/alugrid/dgf.hh>

template <class GridView, int codim>
int checkCodim (const GridView& gridView, const std::string& name)
{
  const auto& indexSet = gridView.indexSet();
  const auto& idSet = gridView.grid().globalIdSet();
  const int rank = gridView.comm().rank();

  using IdType = typename GridView::Grid::GlobalIdSet::IdType;
  struct Info {
    std::set<std::size_t> indices;
    std::set<Dune::PartitionType> partitionTypes;
    Dune::FieldVector<double, GridView::dimensionworld> center;
    std::optional<typename GridView::template Codim<codim>::Entity> first;
    bool allEqual = true;
  };
  std::map<IdType, Info> subEntities;

  // collect all codim subentities of all elements, including ghost elements
  for (const auto& element : elements(gridView, Dune::Partitions::all))
  {
    for (unsigned int i = 0; i < element.subEntities(codim); ++i) {
      const auto subEntity = element.template subEntity<codim>(i);
      auto& info = subEntities[idSet.id(subEntity)];
      info.indices.insert(indexSet.index(subEntity));
      info.partitionTypes.insert(subEntity.partitionType());
      info.center = subEntity.geometry().center();
      if (!info.first)
        info.first = subEntity;
      else if (!(*info.first == subEntity))
        info.allEqual = false;
    }
  }

  // count the entities visited by the codim iterator
  std::size_t numIterated = 0;
  for ([[maybe_unused]] const auto& entity : entities(gridView, Dune::Codim<codim>{}, Dune::Partitions::all))
    ++numIterated;

  int errors = 0;
  for (const auto& [id, info] : subEntities) {
    if (info.indices.size() > 1 || info.partitionTypes.size() > 1) {
      ++errors;
      std::cout << "[" << rank << "] " << name << ", codim " << codim
                << ": entity with id " << id << " at " << info.center
                << " has " << info.indices.size() << " indices {";
      for (auto index : info.indices)
        std::cout << " " << index;
      std::cout << " } and partition types {";
      for (auto pt : info.partitionTypes)
        std::cout << " " << Dune::PartitionName(pt);
      std::cout << " }, entities compare " << (info.allEqual ? "equal" : "unequal") << std::endl;
    }
  }

  if (numIterated != subEntities.size() || std::size_t(indexSet.size(codim)) != subEntities.size()) {
    ++errors;
    std::cout << "[" << rank << "] " << name << ", codim " << codim
              << ": " << subEntities.size() << " distinct ids among element subentities, but "
              << numIterated << " entities visited by the codim iterator and indexSet.size(codim) = "
              << indexSet.size(codim) << std::endl;
  }

  return errors;
}

template <class Grid>
int checkGrid (const std::string& name, const std::string& elementType, int numCells)
{
  std::stringstream dgf;
  dgf << "DGF\nInterval\n";
  for (int i = 0; i < Grid::dimension; ++i) dgf << "0 ";
  dgf << "\n";
  for (int i = 0; i < Grid::dimension; ++i) dgf << "1 ";
  dgf << "\n";
  for (int i = 0; i < Grid::dimension; ++i) dgf << numCells << " ";
  dgf << "\n#\n" << elementType << "\n#\n";
  dgf << "GRIDPARAMETER\nREFINEMENTEDGE ARBITRARY\n#\n";

  Dune::GridPtr<Grid> gridPtr(dgf, Dune::MPIHelper::getCommunicator());
  Grid& grid = *gridPtr;
  grid.loadBalance();

  const auto gridView = grid.leafGridView();
  int errors = 0;
  Dune::Hybrid::forEach(Dune::range(std::integral_constant<int,1>{}, std::integral_constant<int,Grid::dimension+1>{}),
    [&](auto codim) { errors += checkCodim<decltype(gridView), codim>(gridView, name); });

  errors = grid.comm().sum(errors);
  if (grid.comm().rank() == 0)
    std::cout << name << ": " << (errors == 0 ? "OK" : std::to_string(errors) + " error(s)") << std::endl;
  return errors;
}

int main (int argc, char** argv)
{
  Dune::MPIHelper::instance(argc, argv);

  int errors = 0;
  try {
    errors += checkGrid<Dune::ALUGrid<2,2,Dune::simplex,Dune::nonconforming>>("ALUGrid<2,2,simplex,nonconforming>", "Simplex", 8);
    errors += checkGrid<Dune::ALUGrid<2,2,Dune::simplex,Dune::conforming>>("ALUGrid<2,2,simplex,conforming>", "Simplex", 8);
    errors += checkGrid<Dune::ALUGrid<2,2,Dune::cube,Dune::nonconforming>>("ALUGrid<2,2,cube,nonconforming>", "Cube", 8);
    //errors += checkGrid<Dune::ALUGrid<3,3,Dune::simplex,Dune::nonconforming>>("ALUGrid<3,3,simplex,nonconforming>", "Simplex", 4);
    //errors += checkGrid<Dune::ALUGrid<3,3,Dune::simplex,Dune::conforming>>("ALUGrid<3,3,simplex,conforming>", "Simplex", 4);
    errors += checkGrid<Dune::ALUGrid<3,3,Dune::cube,Dune::nonconforming>>("ALUGrid<3,3,cube,nonconforming>", "Cube", 4);
  }
  catch (Dune::Exception& e) {
    std::cerr << e << std::endl;
    return 1;
  }

  return errors == 0 ? 0 : 1;
}
