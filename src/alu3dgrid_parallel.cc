#ifndef __ALU3DGRID_PARALLEL_CC_INCLUDED__
#define __ALU3DGRID_PARALLEL_CC_INCLUDED__

#define _BSGRID_PARALLEL_

#include "alu3dgrid_parallel.h"

namespace BernhardSchuppGrid {

#include "parallel/gitter_pll_sti.cc"
#include "parallel/gitter_pll_ldb.cc"
#include "parallel/gitter_pll_impl.cc"
#include "parallel/gitter_pll_sti.cc"
#include "parallel/gitter_pll_mgb.cc"
#include "parallel/gitter_pll_idn.cc"
#include "parallel/mpAccess.cc"
#include "parallel/mpAccess_MPI.cc"

// file for duneinterface 
#include "duneinterface/gitter_dune_pll_impl.cc"

} //end namespace 

#endif
