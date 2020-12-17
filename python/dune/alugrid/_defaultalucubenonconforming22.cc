#include <config.h>
#define USING_DUNE_PYTHON 1
#include <dune/alugrid/python/registergrid.hh>

PYBIND11_MODULE( _defaultalucubenonconforming22, module )
{
  // register Dune::AluGrid< 2, 2, Dune::cube, Dune::nonconforming >
  Dune::Python::registerAluGrid< 2, 2, Dune::cube, Dune::nonconforming>( module );
}
