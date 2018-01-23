#define DISABLE_DEPRECATED_METHOD_CHECK 1
#define USE_ALUGRID_SFC_ORDERING 1

#include <config.h>

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <dune/alugrid/grid.hh>

int main (int argc , char **argv)
{
  // this method calls MPI_Init, if MPI is enabled
  Dune::MPIHelper::instance( argc, argv );

  try {
    static const int dim = 3;
    typedef Dune::ALUGrid< dim, dim, Dune::cube, Dune::nonconforming > GridType;
    typedef double ctype;
    Dune::FieldVector< ctype, dim > lower ( 0 );
    Dune::FieldVector< ctype, dim > upper ( 1 );
    std::array<unsigned int, dim> elements = {{ 8, 8, 8 }} ;

    auto gridPtr = Dune::StructuredGridFactory< GridType > :: createCubeGrid( lower, upper, elements );

    Dune::VTKWriter< typename GridType::LeafGridView > writer( gridPtr->leafGridView() );
    if( gridPtr->comm().rank() == 0 )
      std::cout <<"Writing VTK output! " << std::endl;
    writer.write( "dump" );

  }
  catch( Dune::Exception &e )
  {
    std::cerr << e << std::endl;
    return 1;
  }
  catch( ... )
  {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
}
