#ifndef _ALU3DGRID_PARALLEL_h_INCLUDED_
#define _ALU3DGRID_PARALLEL_h_INCLUDED_

#include "alu3dgrid_serial.h"

namespace ALU3dGridSpace {

// the parallel stuff 
#include "parallel/gitter_pll_sti.h"
#include "parallel/gitter_pll_impl.h"
#include "parallel/gitter_pll_ldb.h"
#include "parallel/gitter_tetra_top_pll.h"
#include "parallel/gitter_hexa_top_pll.h"
#include "parallel/mpAccess.h"
#include "parallel/mpAccess_MPI.h"
#include "parallel/gitter_pll_mgb.h"

// the duneinterface stuff 
#include "duneinterface/gitter_dune_pll_impl.h"

}
  
#endif
