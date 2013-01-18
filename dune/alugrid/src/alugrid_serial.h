#ifndef __ALUGRID_SERIAL_H_INCLUDED__
#define __ALUGRID_SERIAL_H_INCLUDED__

// toggle ALUGRID alloc at once during refinement 
//#define USE_MALLOC_AT_ONCE 

// toggle ALUGRID alloc for grid objects like elements 
// use with care 
//#define DONT_USE_ALUGRID_ALLOC 

//#ifdef _OPENMP 
#define ITERATORS_WITHOUT_MYALLOC 
//#endif
 
//#define ALUGRID_USE_COMM_BUFFER_IN_ITEM
#define ALUGRID_PERIODIC_BOUNDARY
#define ALUGRID_PERIODIC_BOUNDARY_PARALLEL
#define ALUGRID_CONSTRUCTION_WITH_STREAMS
#define ALUGRID_3D_CONFORMING_REFINEMENT

// include all headers 
#include "stlheaders.h"

//#define COUNT_ALUGRID_FLOPS

#ifdef COUNT_ALUGRID_FLOPS
#include "double.h"
// overload original double 
//#define double Double 
#endif

// defines IndexManagerType
#include "indexstack.h"

// defines VertexProjection Interface 
#include "projectvertex.h"


namespace ALUGridSpace {

#ifdef COUNT_ALUGRID_FLOPS
// overload original double 
#define double Double 
#endif

// the code needs this 
using namespace std;   


struct GatherScatter;
typedef GatherScatter GatherScatterType;

#include "serial/myalloc.h"
#include "serial/key.h"
#include "serial/serialize.h"

#include "parallel/mpAccess.h" // for def of MPAccessGlobal needed by LoadBalancer 
#include "parallel/gitter_pll_ldb.h" // for def of LoadBalancer needed in parallel.h

//#include "serial/parallel.h" // only tpyedefs mainly 
#include "serial/gitter_sti.h"

typedef Gitter::AdaptRestrictProlong AdaptRestrictProlongType;


#include "serial/gitter_hexa_top.h"
#include "serial/mapp_tetra_3d_ext.h"
#include "serial/gitter_tetra_top.h"
#include "serial/walk.h"
#include "serial/gitter_impl.h"
#include "serial/gitter_mgb.h"
#include "serial/key.h"
#include "serial/lock.h"

#include "duneinterface/gitter_dune_impl.h"
}

#endif
