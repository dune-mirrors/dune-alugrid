#ifndef __ALUGRID_2D_CC_INCLUDED__
#define __ALUGRID_2D_CC_INCLUDED__

#define ALU2DGRID_COMPATIBILITY_LEVEL 2
#include "alugrid_2d.h"

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode!!! Use -DNDEBUG to get more optimized code!!!
#endif

namespace ALU2DGrid
{

// this is needed since the code does not used std :: 
using namespace std; 

#include "2d/grid.cc"
#include "2d/triang.cc"
#include "2d/hdlrw.cc"
#include "2d/mknbrs.cc"
#include "2d/handle.cc"
#include "2d/vmmap.cc"
#include "2d/vtx_btree.cc"

} //end namespace 

#endif
