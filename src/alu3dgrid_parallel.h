#ifndef _ALU3DGRID_h_INCLUDED_
#define _ALU3DGRID_h_INCLUDED_

// the serial stuff 
#include "serial/alloca.h"
#include "serial/mapp_cube_3d.h"
#include "serial/mapp_tetra_3d.h"
#include "serial/gitter_hexa_top.h"
#include "serial/gitter_sti.h"
#include "serial/mapp_tetra_3d_ext.h"
#include "serial/serialize.h"
#include "serial/gitter_tetra_top.h"
#include "serial/walk.h"
#include "serial/gitter_impl.h"
#include "serial/xdrclass.h"
#include "serial/gitter_mgb.h"
#include "serial/key.h"
#include "serial/lock.h"
#include "serial/myalloc.h"


// the parallel stuff 
#include "parallel/gitter_pll_sti.h"
#include "parallel/gitter_pll_impl.h"
#include "parallel/gitter_pll_ldb.h"
#include "parallel/gitter_tetra_top_pll.h"
#include "parallel/gitter_hexa_top_pll.h"
#include "parallel/mpAccess.h"
#include "parallel/mpAccess_MPI.h"
#include "parallel/parallel.h"
#include "parallel/gitter_pll_mgb.h"

// the duneinterface stuff 
#include "duneinterface/gitter_dune_pll_impl.h"
#endif
