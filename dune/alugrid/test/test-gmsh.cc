// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "config.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/alugrid/grid.hh>
#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/test/gridcheck.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>
#include <dune/grid/io/file/gmshreader.hh>
#include <dune/grid/io/file/gmshwriter.hh>

using namespace Dune;

template <typename GridType>
void testReadingAndWritingGrid( const std::string& path, const std::string& gridName, const std::string& gridManagerName, int refinements)
{
  // Read the grid
  std::cout<<"Using "<<gridManagerName<<std::endl;
  GridFactory<GridType> gridFactory;
  std::vector<int> boundaryIDs;
  std::vector<int> elementsIDs;
  const std::string inputName(path+gridName+".msh");
  std::cout<<"Reading mesh file "<<inputName<<std::endl;
  GmshReader<GridType>::read(gridFactory,inputName,boundaryIDs,elementsIDs);
  auto grid=std::unique_ptr<GridType>(gridFactory.createGrid());

  // Reorder boundary IDs according to the inserction index
  const auto leafGridView(grid->leafGridView());
  if(!boundaryIDs.empty())
  {
    std::vector<int> tempIDs(boundaryIDs.size(),0);
    for(const auto& entity:elements(leafGridView))
      for(const auto& intersection:intersections(leafGridView,entity))
        if(intersection.boundary())
          tempIDs[intersection.boundarySegmentIndex()]=boundaryIDs[gridFactory.insertionIndex(intersection)];
    boundaryIDs=std::move(tempIDs);
  }

  // Load balancing and refinement
  grid->loadBalance();
  if ( refinements > 0 )
    grid->globalRefine( refinements );

  // Do some tests to make sure the grid has been properly read
  // gridcheck(*grid);

  // Write MSH
  Dune::GmshWriter<typename GridType::LeafGridView> writer( leafGridView );
  writer.setPrecision(10);
  const std::string outputName(gridName+"-"+gridManagerName+"-gmshtest-write.msh");
  writer.write(outputName);
  if(!elementsIDs.empty())
  {
    const std::string outputNameEntity(gridName+"-"+gridManagerName+"-gmshtest-write-entity.msh");
    writer.write(outputNameEntity,elementsIDs);
  }
  if((!boundaryIDs.empty())&&(!elementsIDs.empty()))
  {
    const std::string outputNameBoundary(gridName+"-"+gridManagerName+"-gmshtest-write-boundary.msh");
    writer.write(outputNameBoundary,elementsIDs,boundaryIDs);
  }

  // Write VTK
  std::ostringstream vtkName;
  vtkName << gridName << "-gmshtest-" << refinements;
  VTKWriter<typename GridType::LeafGridView> vtkWriter( leafGridView );
  vtkWriter.write( vtkName.str() );
  std::cout<<std::endl;
}


int main( int argc, char** argv )
try
{
  MPIHelper::instance( argc, argv );
  const int refinements = ( argc > 1 ) ? atoi( argv[1] ) : 0;
  const std::string path("./gmsh/");

  using ALU2dSimplex = Dune::ALUGrid<2, 2, Dune::simplex, Dune::nonconforming>;
  testReadingAndWritingGrid<ALU2dSimplex>( path, "curved2d", "ALU2dSimplex", refinements );
  testReadingAndWritingGrid<ALU2dSimplex>( path, "circle2ndorder", "ALU2dSimplex", refinements );

  using ALU2dCube = Dune::ALUGrid<2, 2, Dune::cube, Dune::nonconforming>;
  testReadingAndWritingGrid<ALU2dCube>( path, "unitsquare_quads_2x2", "ALU2dCube", refinements );

  using ALU3dSimplex = Dune::ALUGrid<3, 3, Dune::simplex, Dune::nonconforming>;
  testReadingAndWritingGrid<ALU3dSimplex>( path, "telescope", "ALU3dSimplex", refinements );

  return 0;
}
catch ( Dune::Exception &e )
{
  std::cerr << e << std::endl;
  return 1;
}
catch (std::exception &e) {
  std::cerr << e.what() << std::endl;
  return 1;
}
catch ( ... )
{
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
