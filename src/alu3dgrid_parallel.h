#ifndef _ALU3DGRID_PARALLEL_h_INCLUDED_
#define _ALU3DGRID_PARALLEL_h_INCLUDED_

#include "alu3dgrid_serial.h"

#if 0
// partitioning libs 
// METIS if not found here then dummy version is included 
extern "C" {
#undef METISTITLE 
#include <metis.h>
}


// PARTY_LIB if not found here then dummy version is included 
#ifdef VERSION 
#define _VER_SAVE VERSION 
#undef VERSION 
#endif
#include <party_lib.h>
#ifdef VERSION 
#define PARTY_LIB_H_INCLUDED
#endif
#endif

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
