#ifndef GITTER_PLL_MGB_H_INCLUDED
#define GITTER_PLL_MGB_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif
	
#include <assert.h>
#include <time.h>
#include <stdio.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
  #include <iomanip>
  #include <algorithm>
#else
  #include <iostream.h>
  #include <iomanip.h>
  #include <algo.h>
#endif

#include "serialize.h"
#include "gitter_mgb.h"
#include "gitter_pll_sti.h"

class ParallelGridMover : public MacroGridBuilder {
  protected :
    inline void unpackVertex (ObjectStream &) ;
    inline void unpackHedge1 (ObjectStream &) ;
    inline void unpackHface3 (ObjectStream &) ;
    inline void unpackHface4 (ObjectStream &) ;
    inline void unpackHexa (ObjectStream &) ;
    inline void unpackTetra (ObjectStream &) ;
    inline void unpackPeriodic3 (ObjectStream &) ;
    inline void unpackPeriodic4 (ObjectStream &) ;
    inline void unpackHbnd3 (ObjectStream &) ;
    inline void unpackHbnd4 (ObjectStream &) ;
  public :
    ParallelGridMover (BuilderIF &) ;
    inline ~ParallelGridMover () ;
    void unpackAll (vector < ObjectStream > &) ;
};
#endif
