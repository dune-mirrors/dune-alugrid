#ifndef __ALUGRID_PARALLEL_CC_INCLUDED__
#define __ALUGRID_PARALLEL_CC_INCLUDED__

#include "alugrid_serial.h"

namespace ALUGridSpace {

// bsgrid original sources 
#include "serial/gitter_geo.cc" 
#include "serial/mapp_cube_3d.cc"
#include "serial/myalloc.cc"
#include "serial/gitter_mgb.cc"
#include "serial/mapp_tetra_3d.cc"
#include "serial/gitter_sti.cc"
#include "serial/gitter_sti.cc"

// file for duneinterface 
#include "duneinterface/gitter_dune_impl.cc"

} //end namespace 

#endif
