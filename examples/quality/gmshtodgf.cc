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
#include <dune/alugrid/dgf.hh>

#include <dune/grid/io/file/vtk/vtkwriter.hh>
#include <dune/grid/io/file/gmshwriter.hh>
#include <dune/grid/io/file/dgfparser/dgfwriter.hh>

using namespace Dune;

int main( int argc, char** argv )
try
{
  MPIHelper::instance( argc, argv );

  std::string filename;
  if( argc > 1 )
  {
    filename = std::string(argv[1]);
  }
  else
  {
    std::cerr << "usage: " << argv[0] << " <filename>" << std::endl;
    return 0;
  }

  typedef typename GridSelector::GridType GridType;
  std::unique_ptr< GridType > gridPtr;
  typedef typename GridSelector::GridType GridType;
  gridPtr.reset( Dune::GridPtr< GridType >( filename ).release() );

  GridType& grid = *gridPtr;
  typedef typename GridType::LeafGridView  LeafGridView;
  const LeafGridView& leafGridView = grid.leafGridView();

  // Write MSH
  {
    Dune::GmshWriter< LeafGridView > writer( leafGridView );
    writer.setPrecision(10);

    const std::string outputName(filename+".msh");
    writer.write(outputName);
  }

  // Write DGF
  {
    Dune::DGFWriter< LeafGridView > writer( leafGridView );

    const std::string outputName(filename+".dgf");
    writer.write(outputName);
  }

  // Write VTK
  {
    std::ostringstream vtkName;
    vtkName << filename << ".vtk";
    VTKWriter< LeafGridView > vtkWriter( leafGridView );
    vtkWriter.write( vtkName.str() );
  }

  return 0;
}
catch ( const Dune::Exception &e )
{
  std::cerr << e << std::endl;
  return 1;
}
catch (const std::exception &e) {
  std::cerr << e.what() << std::endl;
  return 1;
}
catch ( ... )
{
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
