#include <config.h>
#define USING_DUNE_PYTHON 1
#include <dune/alugrid/python/registergrid.hh>

PYBIND11_MODULE( _defaultalucubenonconforming33, module )
{
  // register Dune::AluGrid< 3, 3, Dune::cube, Dune::nonconforming >
  Dune::Python::registerAluGrid< 3, 3, Dune::cube, Dune::nonconforming>( module );
}
