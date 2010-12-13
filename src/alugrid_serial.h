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

//! type of coordinate storage 
typedef double alucoord_t ;

typedef enum ALUElementType { tetra=4 , hexa=7 , hexa_periodic , tetra_periodic } grid_t;

struct GatherScatter;
typedef GatherScatter GatherScatterType;

#include "serial/key.h"
#include "serial/serialize.h"

#include "parallel/mpAccess.h" // for def of MPAccessGlobal needed by LoadBalancer 
#include "parallel/gitter_pll_ldb.h" // for def of LoadBalancer needed in parallel.h

//#include "serial/parallel.h" // only tpyedefs mainly 
#include "serial/gitter_sti.h"

typedef Gitter::AdaptRestrictProlong AdaptRestrictProlongType;

typedef Gitter::helement_STI  HElemType;    // Interface Element
typedef Gitter::hface_STI     HFaceType;    // Interface Element
typedef Gitter::hedge_STI     HEdgeType;    // Interface Element
typedef Gitter::vertex_STI    HVertexType;  // Interface Element
typedef Gitter::hbndseg       HGhostType;

struct GatherScatter
{
  // type of used object stream 
  typedef ObjectStreamImpl ObjectStreamType;

  virtual ~GatherScatter () {}

  virtual bool contains(int,int) const = 0;

  virtual bool containsItem(const HElemType   & elem ) const { assert(false); abort(); return false; }
  virtual bool containsItem(const HFaceType   & elem ) const { assert(false); abort(); return false; }
  virtual bool containsItem(const HEdgeType   & elem ) const { assert(false); abort(); return false; }
  virtual bool containsItem(const HVertexType & elem ) const { assert(false); abort(); return false; }
  
  virtual bool containsInterior (const HFaceType  & face , ElementPllXIF_t & elif) const { assert(false); abort(); return false; }
  virtual bool containsGhost    (const HFaceType  & face , ElementPllXIF_t & elif) const { assert(false); abort(); return false; }
  
  virtual void inlineData ( ObjectStreamType & str , HElemType & elem ) { assert(false); abort(); }
  virtual void xtractData ( ObjectStreamType & str , HElemType & elem ) { assert(false); abort(); }
  
  virtual void sendData ( ObjectStreamType & str , HFaceType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HFaceType & elem ) { assert(false); abort(); }
  virtual void setData  ( ObjectStreamType & str , HFaceType & elem ) { assert(false); abort(); }
  
  virtual void sendData ( ObjectStreamType & str , HEdgeType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HEdgeType & elem ) { assert(false); abort(); }
  virtual void setData  ( ObjectStreamType & str , HEdgeType & elem ) { assert(false); abort(); }
  
  virtual void sendData ( ObjectStreamType & str , HVertexType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HVertexType & elem ) { assert(false); abort(); }
  virtual void setData  ( ObjectStreamType & str , HVertexType & elem ) { assert(false); abort(); }

  virtual void sendData ( ObjectStreamType & str , const HElemType  & elem ) { assert(false); abort(); }
  virtual void sendData ( ObjectStreamType & str , const HGhostType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HGhostType & elem ) { assert(false); abort(); }
  virtual void recvData ( ObjectStreamType & str , HElemType  & elem ) { assert(false); abort(); }
};
typedef GatherScatter GatherScatterType;

#include "serial/gitter_hexa_top.h"
#include "serial/mapp_tetra_3d_ext.h"
#include "serial/gitter_tetra_top.h"
#include "serial/walk.h"
#include "serial/gitter_impl.h"
#include "serial/gitter_mgb.h"
#include "serial/key.h"
#include "serial/lock.h"
#include "serial/myalloc.h"

#include "duneinterface/gitter_dune_impl.h"
}

#endif
