#include <config.h>
#define INCLUDE_REGALUGRID_INLINE
#include "registeralugrid.hh"
template void registerALUGrid<3, 3, Dune::simplex>(pybind11::module);
