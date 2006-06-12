#ifndef __ALUGRID_SERIAL_H_INCLUDED__
#define __ALUGRID_SERIAL_H_INCLUDED__

#ifndef NDEBUG
#warning -- Compiling ALUGrid code in debug mode !!! Use -DNDEBUG to get more optimized code!!!
#endif

#define _ANSI_HEADER
// include all headers 
#include "stlheaders.h"

#define _DUNE_USES_ALU3DGRID_

namespace ALUGridSpace {

typedef enum ALUElementType { tetra=4 , hexa=7 , hexa_periodic , tetra_periodic } grid_t;

struct GatherScatter;
typedef GatherScatter GatherScatterType;


#include "serial/key.h"
#include "serial/serialize.h"
#include "serial/parallel.h" // only tpyedefs mainly 
#include "serial/gitter_sti.h"

typedef Gitter::AdaptRestrictProlong AdaptRestrictProlongType;

typedef Gitter::helement_STI  HElemType;    // Interface Element
typedef Gitter::hface_STI     HFaceType;    // Interface Element
typedef Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef Gitter::vertex_STI    HVertexType;  // Interface Element
typedef Gitter::hbndseg       HGhostType;

struct GatherScatter
{
  typedef ObjectStreamImpl ObjectStreamType;

  virtual ~GatherScatter () {}

  virtual bool contains(int,int) const = 0;
    
  virtual bool containsItem(HElemType   & elem ) const { assert(false); abort(); }
  virtual bool containsItem(HFaceType   & elem ) const { assert(false); abort(); }
  virtual bool containsItem(HEdgeType   & elem ) const { assert(false); abort(); }
  virtual bool containsItem(HVertexType & elem ) const { assert(false); abort(); }
  
  virtual size_t size(HElemType   & elem ) { assert(false); abort(); }
  virtual size_t size(HFaceType   & elem ) { assert(false); abort(); }
  virtual size_t size(HEdgeType   & elem ) { assert(false); abort(); }
  virtual size_t size(HVertexType & elem ) { assert(false); abort(); }
  
  virtual void inlineData ( ObjectStreamType & str , HElemType & elem ) { assert(false); abort(); }
  virtual void xtractData ( ObjectStreamType & str , HElemType & elem ) { assert(false); abort(); }
  
  virtual void sendData ( ObjectStreamType & str , HFaceType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HFaceType & elem ) { assert(false); abort(); }
  virtual void setData    ( ObjectStreamType & str , HFaceType & elem ) { assert(false); abort(); }
  virtual void removeData ( ObjectStreamType & str , HFaceType & elem ) { assert(false); abort(); }
  
  virtual void sendData ( ObjectStreamType & str , HEdgeType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HEdgeType & elem ) { assert(false); abort(); }
  virtual void setData    ( ObjectStreamType & str , HEdgeType & elem ) { assert(false); abort(); }
  virtual void removeData ( ObjectStreamType & str , HEdgeType & elem ) { assert(false); abort(); }
  
  virtual void sendData ( ObjectStreamType & str , HVertexType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HVertexType & elem ) { assert(false); abort(); }
  virtual void setData    ( ObjectStreamType & str , HVertexType & elem ) { assert(false); abort(); }
  virtual void removeData ( ObjectStreamType & str , HVertexType & elem ) { assert(false); abort(); }

  virtual void sendData ( ObjectStreamType & str , const HElemType  & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HGhostType & elem ) { assert(false); abort(); }
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
