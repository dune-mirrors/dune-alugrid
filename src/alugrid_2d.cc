#ifndef __ALUGRID_2D_CC_INCLUDED__
#define __ALUGRID_2D_CC_INCLUDED__

#include "alugrid_2d.h"

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode!!! Use -DNDEBUG to get more optimized code!!!
#endif

namespace ALUGridSpace {

#define _ANSI_HEADER 
#include "alu2d/handle.cc"
#include "alu2d/triang.cc"
#include "alu2d/hdlrw.cc"
#include "alu2d/mknbrs.cc"
#include "alu2d/vmmap.cc"
#include "alu2d/vtx_btree.cc"

#undef _ANSI_HEADER 
  
} //end namespace 

#endif
