#ifndef __ALUGRID_2D_H_INCLUDED__
#define __ALUGRID_2D_H_INCLUDED__

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode !!! Use -DNDEBUG to get more optimized code!!!
#endif

#define _ANSI_HEADER
// include all headers 
#include "stlheaders.h"

namespace ALU2dGridSpace { 

#undef INDEXSTACK_H_INCLUDED
#include "serial/indexstack.h"

#include "grid.h"
#include "triang.h"
#include "handle.h"
#include "vtx_btree.h"
#include "vmmap.h"

}
#include "alugrid_2d.cc"
#endif
