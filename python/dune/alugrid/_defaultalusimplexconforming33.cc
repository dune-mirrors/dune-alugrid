#include <config.h>
#define USING_DUNE_PYTHON 1
#include <dune/alugrid/python/registergrid.hh>

PYBIND11_MODULE( _defaultalusimplexconforming33, module )
{
  // register Dune::AluGrid< 3, 3, Dune::simplex, Dune::conforming >
  Dune::Python::registerAluGrid< 3, 3, Dune::simplex, Dune::conforming>( module );
}
