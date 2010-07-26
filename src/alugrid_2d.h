#ifndef __ALUGRID_2D_H_INCLUDED__
#define __ALUGRID_2D_H_INCLUDED__

#define ALUGRID_NOTEMPFILE_2D
#define ALUGRID_SURFACE_2D


#ifndef ALU2DGRID_COMPATIBILITY_LEVEL
#define ALU2DGRID_COMPATIBILITY_LEVEL 1
#endif

// include all headers 
#include "stlheaders.h"

//#define COUNT_ALUGRID_FLOPS

#ifdef COUNT_ALUGRID_FLOPS
#include "double.h"
// overload original double 
//#define double Double 
#endif

#include "indexstack.h"

#ifdef COUNT_ALUGRID_FLOPS
// overload original double 
#define double Double 
#endif

#include "projectvertex.h"

namespace ALU2DGrid
{

#ifdef COUNT_ALUGRID_FLOPS
  typedef ALUGridSpace :: Double Double ;
#endif

  using namespace std;

#include "alu2d/grid.h"
#include "alu2d/triang.h"
#include "alu2d/listwalk.h"
#include "alu2d/handle.h"
#include "alu2d/vtx_btree.h"
#include "alu2d/vmmap.h"
#include "alu2d/xdisplay_imp.hh"

}


#if (ALU2DGRID_COMPATIBILITY_LEVEL < 2)
/* For compatibility with DUNE 2.0 export the old types in ALUGridSpace */

namespace ALUGridSpace
{

  typedef ALU2DGrid :: Vertex < 2 > Vertex;
  typedef ALU2DGrid :: Thinelement < 2, 3 > Thinelement;
  typedef ALU2DGrid :: Element < 2, 3 > Element;
  typedef ALU2DGrid :: Bndel < 2, 3 > Bndel;

  typedef ALU2DGrid :: Hmesh_basic < 2, 3 > Hmesh_basic;
  typedef ALU2DGrid :: Hmesh < 2, 3 > Hmesh;

  using ALU2DGrid :: Listwalkptr;
  using ALU2DGrid :: Macro;
  using ALU2DGrid :: Refco;

  typedef ALU2DGrid :: AdaptRestrictProlong2d < 2, 3 > AdaptRestrictProlong2dType;
  typedef VertexProjection < 2 , double > ProjectVertex_t;

}
#endif // #if (ALU2DGRID_COMPATIBILITY_LEVEL < 2)

#endif // #ifndef __ALUGRID_2D_H_INCLUDED__
