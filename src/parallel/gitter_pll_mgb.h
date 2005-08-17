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
    void unpackVertex (ObjectStream &) ;
    void unpackHedge1 (ObjectStream &) ;
    void unpackHface3 (ObjectStream &) ;
    void unpackHface4 (ObjectStream &) ;
    void unpackHexa (ObjectStream &) ;
    void unpackTetra (ObjectStream &) ;
    void unpackPeriodic3 (ObjectStream &) ;
    void unpackPeriodic4 (ObjectStream &) ;
    void unpackHbnd3Int (ObjectStream &) ;
    void unpackHbnd3Ext (ObjectStream &) ;
    void unpackHbnd4Int (ObjectStream &) ;
    void unpackHbnd4Ext (ObjectStream &) ;
  public :
    ParallelGridMover (BuilderIF &, bool init = true) ;
    // former constructor 
    void initialize ();
    ~ParallelGridMover () ;
    void unpackAll (vector < ObjectStream > &) ;
};
#endif
