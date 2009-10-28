#ifndef __ALUGRID_PARALLEL_CC_INCLUDED__
#define __ALUGRID_PARALLEL_CC_INCLUDED__

#include <config.h>

#include "alugrid_parallel.h"

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode!!! Use -DNDEBUG to get more optimized code!!!
#endif

#define _ANSI_HEADER 

// ParMETIS stuff 
#include "parallel/aluparmetis.hh"

// METIS stuff 
#include "parallel/alumetis.hh"

// PARTY Lib stuff 
#include "parallel/aluparty_lib.hh"

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

#undef _ANSI_HEADER 
#endif
