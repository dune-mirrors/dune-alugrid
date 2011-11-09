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

// avoid C++ bindings of MPI (-DMPIPP_H is not common enough)
// this is the only thing all MPI implementations have in common
// to do that we pretend that we are compiling C code 
#if defined(__cplusplus) 
#define rem__cplusplus __cplusplus
#undef __cplusplus
#endif

#if defined(c_plusplus) 
#define remc_plusplus c_plusplus
#undef c_plusplus
#endif

extern "C" {
  // the message passing interface (MPI) headers for C 
  #include <mpi.h>
}

// restore defines 
#if defined(rem__cplusplus) 
#define __cplusplus rem__cplusplus
#undef rem__cplusplus
#endif

#if defined(c_plusplus) 
#define c_plusplus remc_plusplus
#undef remc_plusplus
#endif


// METIS stuff 
#include "parallel/alumetis.hh"

// ParMETIS stuff 
#include "parallel/aluparmetis.hh"

// PARTY Lib stuff 
#include "parallel/aluparty_lib.hh"

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
