#ifndef _ALUGRID_PARALLEL_h_INCLUDED_
#define _ALUGRID_PARALLEL_h_INCLUDED_

// is this is defined the internal faces are used as ghost elements 
// needs to be defined before all headers are included 
//#define __USE_INTERNAL_FACES__


// the message passing interface (MPI) headers 
#include <mpi.h>

#include "alugrid_serial.h"

#define _ANSI_HEADER 

namespace ALUGridSpace {

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

#undef _ANSI_HEADER
#endif
