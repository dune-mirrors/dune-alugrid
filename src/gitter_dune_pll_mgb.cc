#ifndef GITTER_DUNE_PLL_MGB_CC_INCLUDED
#define GITTER_DUNE_PLL_MGB_CC_INCLUDED

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
#include "gitter_pll_mgb.h"

class DuneParallelGridMover : public ParallelGridMover {
  protected :
    inline void duneUnpackTetra (ObjectStream &,GatherScatterType &) ;
  public :
    DuneParallelGridMover (BuilderIF &i) : ParallelGridMover (i) {};
    inline ~DuneParallelGridMover () ;
    void duneUnpackAll (vector < ObjectStream > &, GatherScatterType & ) ;
} ;

inline DuneParallelGridMover :: ~DuneParallelGridMover () {
    return ;
}


inline void DuneParallelGridMover :: duneUnpackTetra (ObjectStream & os, GatherScatterType & gs) {
  int v [4] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  pair < tetra_GEO *, bool > p = InsertUniqueTetra (v) ;
  p.first->accessPllX ().duneUnpackSelf (os,gs,p.second) ;
  return ;
}

void DuneParallelGridMover :: duneUnpackAll (vector < ObjectStream > & osv,
    GatherScatterType & gs) {
  for (vector < ObjectStream > :: iterator j = osv.begin () ; j != osv.end () ; j ++) {
    ObjectStream & os (*j) ;
    int code = MacroGridMoverIF :: ENDMARKER ;
    for (os.readObject (code) ; code != MacroGridMoverIF :: ENDMARKER ; os.readObject (code)) {
      switch (code) {
      case MacroGridMoverIF:: VERTEX :
	unpackVertex (os) ;
	break ;
      case MacroGridMoverIF :: EDGE1 :
        unpackHedge1 (os) ;
	break ;
      case MacroGridMoverIF :: FACE3 :
        unpackHface3 (os) ;
	break ;
      case MacroGridMoverIF :: FACE4 :
	unpackHface4 (os) ;
	break ;
      case MacroGridMoverIF :: TETRA :
        duneUnpackTetra (os,gs) ;
        break ;
      case MacroGridMoverIF :: HEXA :
	unpackHexa (os) ;
        break ;
      case MacroGridMoverIF :: PERIODIC3 :
        unpackPeriodic3 (os) ;
        break ;
// Anfang - Neu am 23.5.02 (BS)
      case MacroGridMoverIF :: PERIODIC4 :
        unpackPeriodic4 (os) ;
        break ;
// Ende - Neu am 23.5.02 (BS)
      case MacroGridMoverIF :: HBND3INT :
      case MacroGridMoverIF :: HBND3EXT :
        unpackHbnd3 (os) ;
	break ;
      case MacroGridMoverIF :: HBND4INT :
      case MacroGridMoverIF :: HBND4EXT :
	unpackHbnd4 (os) ;
        break ;
      default :
	cerr << "**FEHLER (FATAL) Unbekannte Gitterobjekt-Codierung gelesen [" << code << "]\n" ;
	cerr << "  Weitermachen unm\"oglich. In " << __FILE__ << " " << __LINE__ << endl ;
	abort () ;
        break ;
      }
    }
  }  
  return ;
}

void GitterDunePll :: duneRepartitionMacroGrid (LoadBalancer :: DataBase & db, GatherScatterType & gs) {
  if (db.repartition (mpAccess (), LoadBalancer :: DataBase :: method (_ldbMethod))) {
    const long start = clock () ;
    long lap1 (start), lap2 (start), lap3 (start), lap4 (start) ;
    mpAccess ().removeLinkage () ;
    mpAccess ().insertRequestSymetric (db.scan ()) ;
    const int me = mpAccess ().myrank (), nl = mpAccess ().nlinks () ;
    {
      AccessIterator < helement > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) {
      int to = db.getDestination (w.item ().accessPllX ().ldbVertexIndex ()) ;
        if (me != to)
          w.item ().accessPllX ().attach2 (mpAccess ().link (to)) ;
      }
    }
    lap1 = clock () ;
    vector < ObjectStream > osv (nl) ;
    {
      AccessIterator < vertex_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      AccessIterator < hedge_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().dunePackAll (osv,gs) ;
    }
    {
      for (vector < ObjectStream > :: iterator i = osv.begin () ; i != osv.end () ; 
        (*i++).writeObject (MacroGridMoverIF :: ENDMARKER)) ;
    }
    lap2 = clock () ;
    osv = mpAccess ().exchange (osv) ;
    lap3 = clock () ;
    {
      DuneParallelGridMover pgm (containerPll ()) ;
      pgm.duneUnpackAll (osv,gs) ;
    }
    lap4 = clock () ;
    if (MacroGridBuilder :: debugOption (20)) {
      cout << "**INFO GitterDunePll :: repartitionMacroGrid () [ass|pck|exc|upk|all] " ;
      cout << setw (5) << (float)(lap1 - start)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap4 - start)/(float)(CLOCKS_PER_SEC) << " sec." << endl ;
    }
  }
  return ;
}
#endif
