#ifndef __ALUGRID_SERIAL_H_INCLUDED__
#define __ALUGRID_SERIAL_H_INCLUDED__

#ifndef NDEBUG
#warning -- Compiling ALUGrid library in debug mode !!!
#endif

#define _ANSI_HEADER
// include all headers 
#include "stlheaders.h"

#define _DUNE_USES_ALU3DGRID_

namespace ALUGridSpace {

typedef enum ALUElementType { hexa , tetra , hexa_periodic , tetra_periodic } grid_t;

struct GatherScatter;
typedef GatherScatter GatherScatterType;


#include "serial/key.h"
#include "serial/serialize.h"
#include "serial/gitter_sti.h"

typedef Gitter::AdaptRestrictProlong AdaptRestrictProlongType;


//#ifndef _ALU3DGRID_PARALLEL_
typedef Gitter::helement_STI  HElemType;    // Interface Element
typedef Gitter::hbndseg       HGhostType;
//#else 
//#include "parallel/gitter_pll_sti.h"
//typedef GitterPll::helement_STI  HElemType;    // Interface Element
//typedef GitterPll::hbndseg       HGhostType;
//#endif

struct GatherScatter
{
  virtual void inlineData ( ObjectStream & str , HElemType & elem ) = 0;
  virtual void xtractData ( ObjectStream & str , HElemType & elem ) = 0;
  virtual void sendData ( ObjectStream & str , const HElemType  & elem ) = 0;
  virtual void recvData ( ObjectStream & str , HGhostType & elem ) = 0;
};
typedef GatherScatter GatherScatterType;

#include "serial/alloca.h"
#include "serial/mapp_cube_3d.h"
#include "serial/mapp_tetra_3d.h"
#include "serial/gitter_hexa_top.h"
#include "serial/mapp_tetra_3d_ext.h"
#include "serial/gitter_tetra_top.h"
#include "serial/walk.h"
#include "serial/gitter_impl.h"
#include "serial/xdrclass.h"
#include "serial/gitter_mgb.h"
#include "serial/key.h"
#include "serial/lock.h"
#include "serial/myalloc.h"

#include "duneinterface/gitter_dune_impl.h"

}
#endif
