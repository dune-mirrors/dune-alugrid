#include <config.h>
#define INCLUDE_REGALUGRID_INLINE
#include "registeralugrid.hh"
#if not defined(DIM) || not defined(GEOMTYPE)
#error "DIM and GEOMTYPE need to be defined!"
#endif
template void registerALUGrid<DIM, DIM, GEOMTYPE>(pybind11::module);
