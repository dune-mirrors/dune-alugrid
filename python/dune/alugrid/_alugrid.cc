#include <config.h>

#include <dune/python/pybind11/pybind11.h>

#ifdef DUNE_ENABLE_PYTHONMODULE_PRECOMPILE
#include "registeralugrid.hh"
#endif

PYBIND11_MODULE( _alugrid, module )
{
#ifdef DUNE_ENABLE_PYTHONMODULE_PRECOMPILE
  // pre-compiled objects for ALUGrid
  registerALUGrid< 2, 2, Dune::simplex > ( module );
  registerALUGrid< 2, 2, Dune::cube    > ( module );

  registerALUGrid< 3, 3, Dune::simplex > ( module );
  registerALUGrid< 3, 3, Dune::cube    > ( module );
#endif
}
