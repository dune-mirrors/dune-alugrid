#ifndef __ALUGRID_PARALLEL_CC_INCLUDED__
#define __ALUGRID_PARALLEL_CC_INCLUDED__

#include <config.h>

#include "alugrid_parallel.h"

#ifdef COUNT_ALUGRID_FLOPS
#undef double 
#endif

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode!!! Use -DNDEBUG to get more optimized code!!!
#endif



namespace ALUGridSpace {
// sfc stuff 
#include "parallel/alusfc.hh"

// METIS stuff 
#include "parallel/alumetis.hh"

// ParMETIS stuff 
#include "parallel/aluparmetis.hh"
}

#ifdef COUNT_ALUGRID_FLOPS
#define double Double
#endif


namespace ALUGridSpace {

#include "parallel/gitter_pll_sti.cc"
#include "parallel/gitter_pll_impl.cc"
#include "parallel/gitter_pll_sti.cc"
#include "parallel/gitter_pll_mgb.cc"

#include "parallel/gitter_tetra_top_pll.cc"
#include "parallel/gitter_hexa_top_pll.cc"

#include "parallel/mpAccess.cc"
#include "parallel/mpAccess_MPI.cc"  
#include "parallel/mpAccess_STAR.cc"  

#include "parallel/gitter_pll_ldb.cc"
#include "parallel/gitter_pll_idn.cc"
// file for duneinterface 
#include "duneinterface/gitter_dune_pll_impl.cc"

} //end namespace 

#endif
