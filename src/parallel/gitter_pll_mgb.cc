// (c) bernhard schupp 1997 - 1998

#ifndef GITTER_PLL_MGB_CC_INCLUDED
#define GITTER_PLL_MGB_CC_INCLUDED

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

#ifndef SERIALIZE_H_INCLUDED
#include "serialize.h"
#endif

#ifndef GITTER_MGB_H_INCLUDED
#include "gitter_mgb.h"
#endif
  
#include "gitter_pll_sti.h"
#include "gitter_pll_mgb.h"
  

ParallelGridMover :: ParallelGridMover (BuilderIF & b, bool init) : MacroGridBuilder (b,init) 
{
  if(init) initialize();
}

void ParallelGridMover ::initialize ()
{
  assert(_initialized);
  vector < elementKey_t > toDelete ;
  {for (elementMap_t :: iterator i = _hexaMap.begin () ; i != _hexaMap.end () ; i ++)
    if (Gitter :: InternalElement ()(*((hexa_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    } 
  }
  {for (elementMap_t :: iterator i = _tetraMap.begin () ; i != _tetraMap.end () ; i ++)
    if (Gitter :: InternalElement ()(*((tetra_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    }
  }
  {for (elementMap_t :: iterator i = _periodic3Map.begin () ; i != _periodic3Map.end () ; i ++)
    if (Gitter :: InternalElement ()(*((periodic3_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    }
  }
// Anfang - Neu am 23.5.02 (BS)
  {for (elementMap_t :: iterator i = _periodic4Map.begin () ; i != _periodic4Map.end () ; i ++)
    if (Gitter :: InternalElement ()(*((periodic4_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    }
  }
// Ende - Neu am 23.5.02 (BS)
  {for (vector < elementKey_t > :: iterator i = toDelete.begin () ; i != toDelete.end () ; i ++ )
    removeElement (*i) ;
  }

  return ;
}

inline ParallelGridMover :: ~ParallelGridMover () {
  assert(_initialized);
  return ;
}

inline void ParallelGridMover :: unpackVertex (ObjectStream & os) {
  int id ;
  double x, y, z ;
  os.readObject (id) ;
  os.readObject (x) ;
  os.readObject (y) ;
  os.readObject (z) ;
  pair < VertexGeo *, bool > p = InsertUniqueVertex (x,y,z,id) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}

inline void ParallelGridMover :: unpackHedge1 (ObjectStream & os) {
  int left, right ;
  os.readObject (left) ;
  os.readObject (right) ;
  pair < hedge1_GEO *, bool > p = InsertUniqueHedge1 (left,right) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}

inline void ParallelGridMover :: unpackHface3 (ObjectStream & os) {
  int v [3] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  pair < hface3_GEO *, bool > p = InsertUniqueHface3 (v) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}

inline void ParallelGridMover :: unpackHface4 (ObjectStream & os) {
  int v [4] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  pair < hface4_GEO *, bool > p = InsertUniqueHface4 (v) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}

inline void ParallelGridMover :: unpackTetra (ObjectStream & os) {
  int v [4] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  pair < tetra_GEO *, bool > p = InsertUniqueTetra (v) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}

	// Neu >
inline void ParallelGridMover :: unpackPeriodic3 (ObjectStream & os) {
  int v [6] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  os.readObject (v[4]) ;
  os.readObject (v[5]) ;
  pair < periodic3_GEO *, bool > p = InsertUniquePeriodic3 (v) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}	// < Neu

// Anfang - Neu am 23.5.02 (BS)

inline void ParallelGridMover :: unpackPeriodic4 (ObjectStream & os) {
  int v [8] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  os.readObject (v[4]) ;
  os.readObject (v[5]) ;
  os.readObject (v[6]) ;
  os.readObject (v[7]) ;
  pair < periodic4_GEO *, bool > p = InsertUniquePeriodic4 (v) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}

// Ende - Neu am 23.5.02 (BS)

inline void ParallelGridMover :: unpackHexa (ObjectStream & os) {
  int v [8] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  os.readObject (v[4]) ;
  os.readObject (v[5]) ;
  os.readObject (v[6]) ;
  os.readObject (v[7]) ;
  pair < hexa_GEO *, bool > p = InsertUniqueHexa (v) ;
  p.first->accessPllX ().unpackSelf (os,p.second) ;
  return ;
}

inline void ParallelGridMover :: unpackHbnd3Int (ObjectStream & os) {

  int b, v [3] ;
  os.readObject (b) ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;

  // read vertex coord , is neccessary because we dont want to overload the
  // Tetra packAsBnd method, so we read vertex here but do the same as
  // before 
  int fake = 0;
  os.readObject( fake );

  if(fake) 
  {
    double p [3] = {0.0,0.0,0.0};
    os.readObject (p[0]) ;
    os.readObject (p[1]) ;
    os.readObject (p[2]) ;
  }

  InsertUniqueHbnd3 (v, Gitter :: hbndseg :: bnd_t (b)) ;
  return ;
}

inline void ParallelGridMover :: unpackHbnd3Ext (ObjectStream & os) {
  int b, v [3] ;
  os.readObject (b) ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  InsertUniqueHbnd3 (v, Gitter :: hbndseg :: bnd_t (b)) ;
  return ;
}
	
inline void ParallelGridMover :: unpackHbnd4 (ObjectStream & os) {
  int b, v [4] ;
  os.readObject (b) ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  InsertUniqueHbnd4 (v, Gitter :: hbndseg :: bnd_t (b)) ;
  return ;
}

void ParallelGridMover :: unpackAll (vector < ObjectStream > & osv) {
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
        unpackTetra (os) ;
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
        unpackHbnd3Int (os) ;
      	break ;
      case MacroGridMoverIF :: HBND3EXT :
        unpackHbnd3Ext (os) ;
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

void GitterPll :: repartitionMacroGrid (LoadBalancer :: DataBase & db) {
  assert(false);
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
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      for (vector < ObjectStream > :: iterator i = osv.begin () ; i != osv.end () ; 
      	(*i++).writeObject (MacroGridMoverIF :: ENDMARKER)) ;
    }
    lap2 = clock () ;
    osv = mpAccess ().exchange (osv) ;
    lap3 = clock () ;
    {
      ParallelGridMover pgm (containerPll ()) ;
      pgm.unpackAll (osv) ;
    }
    lap4 = clock () ;
    if (MacroGridBuilder :: debugOption (20)) {
      cout << "**INFO GitterPll :: repartitionMacroGrid () [ass|pck|exc|upk|all] " ;
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
