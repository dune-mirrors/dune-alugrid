#ifndef __ALUGRID_PARALLEL_CC_INCLUDED__
#define __ALUGRID_PARALLEL_CC_INCLUDED__

#include "alugrid_serial.h"

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode!!! Use -DNDEBUG to get more optimized code!!!
#endif

namespace ALUGridSpace {

// the code needs this 
using namespace std; 

// bsgrid original sources 
#include "serial/gitter_geo.cc" 
#include "serial/mapp_cube_3d.cc"
#include "serial/myalloc.cc"
#include "serial/gitter_impl.cc"
#include "serial/gitter_mgb.cc"
#include "serial/mapp_tetra_3d.cc"
#include "serial/gitter_sti.cc"
#include "serial/gitter_sti.cc"
#include "serial/ghost_elements.cc"
#include "serial/gitter_tetra_top.cc"
#include "serial/gitter_hexa_top.cc"

// file for duneinterface 
#include "duneinterface/gitter_dune_impl.cc"

} //end namespace 

#endif
