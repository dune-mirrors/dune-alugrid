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
  public :
    DuneParallelGridMover (BuilderIF &i) : ParallelGridMover (i) {}

    // if not finalized yet, finalize is called 
    ~DuneParallelGridMover () ;
    inline void finalize (); //former desctructor
    
    // overlaoded, because calles unpackHbnd3 
    inline void unpackAll (vector < ObjectStream > &) ;
    inline void duneUnpackAll (vector < ObjectStream > &, GatherScatterType & ) ;

  protected :
    void duneUnpackTetra (ObjectStream &,GatherScatterType &) ;
    // overloaded, because calles InsertUniqueHbnd3_withPoint
    inline void unpackHbnd3 (ObjectStream & os); 
    
    bool InsertUniqueHbnd3_withPoint (int (&)[3], Gitter :: hbndseg ::
            bnd_t,const double (&p) [3]) ;
        
};

// new method that gets coord of ghost point 
bool DuneParallelGridMover :: InsertUniqueHbnd3_withPoint (int (&v)[3],         
      Gitter :: hbndseg_STI ::bnd_t bt, const double (&p)[3]) {
  int twst = cyclicReorder (v,v+3) ;
  faceKey_t key (v [0], v [1], v [2]) ;
  if (bt == Gitter :: hbndseg_STI :: closure) {
    if (_hbnd3Int.find (key) == _hbnd3Int.end ()) {
      hface3_GEO * face =  InsertUniqueHface3 (v).first ;
      // here the point is stored 
      _hbnd3Int [key] = new Hbnd3IntStorage (face,twst,p) ;
      return true ;
    }
  } else {
    if (_hbnd3Map.find (key) == _hbnd3Map.end ()) {
      hface3_GEO * face =  InsertUniqueHface3 (v).first ;
      hbndseg3_GEO * hb3 = myBuilder ().insert_hbnd3 (face,twst,bt) ;
      _hbnd3Map [key] = hb3 ;
      return true ;
    }
  }
  return false ;
}

// overloaded method because here we call insertion with point 
inline void DuneParallelGridMover :: unpackHbnd3 (ObjectStream & os) {
  int b, v [3] ;
  double p [3];
  os.readObject (b) ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;

  // read vertex coord 
  if(Gitter :: hbndseg :: bnd_t (b) == Gitter :: hbndseg :: closure )
  {
    os.readObject (p[0]) ;
    os.readObject (p[1]) ;
    os.readObject (p[2]) ;
    //cout << "Insert Unique Hbnd3 p \n";
    InsertUniqueHbnd3_withPoint (v, Gitter :: hbndseg :: bnd_t (b), p ) ;
  }
  else
  {
    // old method defined in base class 
    InsertUniqueHbnd3 (v, Gitter :: hbndseg :: bnd_t (b)) ;
  }
  return ;
}


void DuneParallelGridMover :: duneUnpackTetra (ObjectStream & os, GatherScatterType & gs) {
  int v [4] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  pair < tetra_GEO *, bool > p = InsertUniqueTetra (v) ;
  p.first->accessPllX ().duneUnpackSelf (os,gs,p.second) ;
  return ;
}

void DuneParallelGridMover :: unpackAll (vector < ObjectStream > & osv) {
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

// destructor calles finalize if not finalized yet 
DuneParallelGridMover :: ~DuneParallelGridMover () 
{
  if(!_finalized) finalize();
}   

// overloaded, because here we use the new insertInternal method 
void DuneParallelGridMover :: finalize ()
{
  //cout << "finalize on DuneParallelGridMover called! \n";
  {for (elementMap_t :: iterator i = _hexaMap.begin () ; i != _hexaMap.end () ; _hexaMap.erase (i++))
    myBuilder ()._hexaList.push_back ((hexa_GEO *)(*i).second) ;
  }
  {for (elementMap_t :: iterator i = _tetraMap.begin () ; i != _tetraMap.end () ; _tetraMap.erase (i++))
    myBuilder ()._tetraList.push_back ((tetra_GEO *)(*i).second) ;
  }
  {for (elementMap_t :: iterator i = _periodic3Map.begin () ; i != _periodic3Map.end () ; _periodic3Map.erase (i++))
    myBuilder ()._periodic3List.push_back ((periodic3_GEO *)(*i).second) ;
  }
  
  {for (elementMap_t :: iterator i = _periodic4Map.begin () ; i != _periodic4Map.end () ; _periodic4Map.erase (i++))
    myBuilder ()._periodic4List.push_back ((periodic4_GEO *)(*i).second) ;
  }

  {for (faceMap_t :: iterator i = _hbnd4Map.begin () ; i != _hbnd4Map.end () ; )
    if (((hbndseg4_GEO *)(*i).second)->myhface4 (0)->ref == 1) {
      delete (hbndseg4_GEO *)(*i).second ;
      _hbnd4Map.erase (i++) ;
    } else {
      myBuilder ()._hbndseg4List.push_back ((hbndseg4_GEO *)(*i ++).second) ;
    }
  }
  {for (faceMap_t :: iterator i = _hbnd3Map.begin () ; i != _hbnd3Map.end () ; )
    if (((hbndseg3_GEO *)(*i).second)->myhface3 (0)->ref == 1) {
      delete (hbndseg3_GEO *)(*i).second ;
      _hbnd3Map.erase (i++) ;
    } else {
      myBuilder ()._hbndseg3List.push_back ((hbndseg3_GEO *)(*i ++).second) ;
    }
  }
  {for (faceMap_t :: iterator i = _hbnd4Int.begin () ; i != _hbnd4Int.end () ; i ++) {
    const pair < hface4_GEO *, int > & p = * (pair < hface4_GEO *, int > *)(*i).second ;
    if (p.first->ref == 1) {
      hbndseg4_GEO * hb4 = myBuilder ().insert_hbnd4 (p.first,p.second,Gitter :: hbndseg_STI :: closure) ;
      myBuilder ()._hbndseg4List.push_back (hb4) ;
    }
    delete (pair < hface4_GEO *, int > *)(*i).second ;
  }}

  // here the internal boundary elements are created 
  {for (hbndintMap_t :: iterator i = _hbnd3Int.begin () ; i != _hbnd3Int.end () ; i ++) {
    const Hbnd3IntStorage & p = * (Hbnd3IntStorage *) (*i).second ;
    if (p.first()->ref == 1) {
      hbndseg3_GEO * hb3 = myBuilder().insert_hbnd3 ( p.first(),p.second(),Gitter :: hbndseg_STI :: closure , p.getPoint() );
      myBuilder ()._hbndseg3List.push_back (hb3) ;
    }
    delete (pair < hface3_GEO *, int > *)(*i).second ;
  }}
  {for (faceMap_t :: iterator i = _face4Map.begin () ; i != _face4Map.end () ; )
    if (!((hface4_GEO *)(*i).second)->ref) {
      delete (hface4_GEO *)(*i).second ;
      _face4Map.erase (i++) ;
    } else {
      assert (((hface4_GEO *)(*i).second)->ref == 2) ;
      myBuilder ()._hface4List.push_back ((hface4_GEO *)(*i ++).second ) ;
    }
  }
  {for (faceMap_t :: iterator i = _face3Map.begin () ; i != _face3Map.end () ; ) {
    if (!((hface3_GEO *)(*i).second)->ref) {
      delete (hface3_GEO *)(*i).second ;
      _face3Map.erase (i++) ;
    } else {
      assert (((hface3_GEO *)(*i).second)->ref == 2) ;
      myBuilder ()._hface3List.push_back ((hface3_GEO *)(*i ++).second ) ;
    }
  }}
  {for (edgeMap_t :: iterator i = _edgeMap.begin () ; i != _edgeMap.end () ; )
    if (!(*i).second->ref) {
      delete (*i).second ;
      _edgeMap.erase (i++) ;
    } else {
      assert ((*i).second->ref >= 1) ;
      myBuilder ()._hedge1List.push_back ((*i ++).second) ;
    }
  }
  {for (vertexMap_t :: iterator i = _vertexMap.begin () ; i != _vertexMap.end () ; )
    if (!(*i).second->ref) {
      delete (*i).second ;
      _vertexMap.erase (i++) ;
    } else {
      assert ((*i).second->ref >= 2) ;
      myBuilder ()._vertexList.push_back ((*i ++).second) ;
    }
  }
  myBuilder ()._modified = true ;	// wichtig !
  this->_finalized = true;
  return ;
}

//*************************************************************************
//  repartition method of class GitterDunePll 
//*************************************************************************
// method was overloaded because here we use our DuneParallelGridMover 
void GitterDunePll :: repartitionMacroGrid (LoadBalancer :: DataBase & db) {

  if (db.repartition (mpAccess (), LoadBalancer :: DataBase :: method (_ldbMethod))) {
    
    const long start = clock () ;
    long lap1 (start), lap2 (start), lap3 (start), lap4 (start) ;
    mpAccess ().removeLinkage () ;
    mpAccess ().insertRequestSymetric (db.scan ()) ;
    const int me = mpAccess ().myrank (), nl = mpAccess ().nlinks () ;
    {
      AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
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
      DuneParallelGridMover pgm (containerPll ()) ;
      pgm.unpackAll (osv) ;
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
