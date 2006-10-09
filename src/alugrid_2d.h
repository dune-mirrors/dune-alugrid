#ifndef __ALUGRID_2D_H_INCLUDED__
#define __ALUGRID_2D_H_INCLUDED__

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode!!! Use -DNDEBUG to get more optimized code!!!
#endif

#define _ANSI_HEADER
// include all headers 
#include "stlheaders.h"

#include "indexstack.h"

namespace ALUGridSpace { 
#define _ANSI_HEADER

using namespace std;
#include "alu2d/grid.h"
#include "alu2d/triang.h"
#include "alu2d/handle.h"
#include "alu2d/vtx_btree.h"
#include "alu2d/vmmap.h"
#include "alu2d/xdisplay_imp.hh"
typedef AdaptRestrictProlong2d AdaptRestrictProlong2dType;

}
#undef _ANSI_HEADER
#endif
