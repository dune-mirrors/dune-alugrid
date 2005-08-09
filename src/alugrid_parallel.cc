#ifndef __ALUGRID_PARALLEL_CC_INCLUDED__
#define __ALUGRID_PARALLEL_CC_INCLUDED__

#include "alugrid_parallel.h"

// partitioning libs 
// METIS if not found here then dummy version is included 
extern "C" {
// METISTITLE is defined by the metis header file. 
#undef METISTITLE 
#include <metis.h>
// if METISTITLE is still undefined then the dummy is included 
#include "parallel/metis.c"
}


// PARTY_LIB if not found here then dummy version is included 
#ifdef VERSION 
#define _VER_SAVE VERSION 
#undef VERSION 
#endif

#include <party_lib.h>
#ifdef VERSION 
#define PARTY_LIB_H_INCLUDED
#ifdef _VER_SAVE
#undef VERSION 
#define VERSION _VER_SAVE
#endif // end _VER_SAVE 
#else 
#include "parallel/party_lib.c"
#endif

namespace ALUGridSpace {

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
