#ifndef __ALU3DGRID_PARALLEL_CC_INCLUDED__
#define __ALU3DGRID_PARALLEL_CC_INCLUDED__

#define _ANSI_HEADER
// include all headers 
#include "stlheaders.h"

#define _DUNE_USES_BSGRID_

namespace BernhardSchuppGrid {

typedef enum {hexa,tetra} grid_t;

struct GatherScatter;
typedef GatherScatter GatherScatterType;

#include "serial/key.h"
#include "serial/serialize.h"
#include "serial/gitter_sti.h"

#include "parallel/gitter_pll_sti.h"

typedef GitterPll::helement_STI  HElemType;    // Interface Element
typedef GitterPll::hbndseg       HGhostType;

struct GatherScatter
{
  virtual void inlineData ( ObjectStream & str , HElemType & elem ) = 0;
  virtual void xtractData ( ObjectStream & str , HElemType & elem ) = 0;
  virtual void sendData ( ObjectStream & str , const HElemType  & elem ) = 0;
  virtual void recvData ( ObjectStream & str , HGhostType & elem ) = 0;
};
typedef GatherScatter GatherScatterType;


// bsgrid original sources 
#include "serial/gitter_geo.cc" 
#include "serial/mapp_cube_3d.cc"
#include "serial/myalloc.cc"
#include "serial/gitter_mgb.cc"
#include "serial/mapp_tetra_3d.cc"
#include "serial/gitter_sti.cc"
#include "serial/gitter_sti.cc"

// file for duneinterface 
#include "duneinterface/gitter_dune_impl.cc"

} //end namespace 

#endif
