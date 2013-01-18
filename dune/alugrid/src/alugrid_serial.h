#ifndef __ALUGRID_SERIAL_H_INCLUDED__
#define __ALUGRID_SERIAL_H_INCLUDED__

//#ifdef _OPENMP 
#define ITERATORS_WITHOUT_MYALLOC 
//#endif
 
#define ALUGRID_PERIODIC_BOUNDARY
#define ALUGRID_PERIODIC_BOUNDARY_PARALLEL
#define ALUGRID_CONSTRUCTION_WITH_STREAMS
#define ALUGRID_3D_CONFORMING_REFINEMENT

#define ALUGRID_VERTEX_PROJECTION

#include "serial/myalloc.h"
#include "serial/key.h"
#include "serial/serialize.h"

#include "parallel/mpAccess.h" // for def of MPAccessGlobal needed by LoadBalancer 
#include "parallel/gitter_pll_ldb.h" // for def of LoadBalancer needed in parallel.h

#include "serial/gitter_sti.h"

#include "serial/gitter_hexa_top.h"
#include "serial/mapp_tetra_3d_ext.h"
#include "serial/gitter_tetra_top.h"
#include "serial/walk.h"
#include "serial/gitter_impl.h"
#include "serial/gitter_mgb.h"
#include "serial/key.h"
#include "serial/lock.h"

#include "duneinterface/gitter_dune_impl.h"

#endif
