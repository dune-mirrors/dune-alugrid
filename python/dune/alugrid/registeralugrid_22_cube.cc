#include <config.h>
#define INCLUDE_REGALUGRID_INLINE
#include "registeralugrid.hh"
template void registerALUGrid<2, 2, Dune::cube>(pybind11::module);
