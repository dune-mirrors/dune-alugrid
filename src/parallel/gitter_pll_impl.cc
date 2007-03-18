// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <malloc.h>
#include <time.h>
#include <stdlib.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <sstream>
  #include <set>
  #include <map>
  #include <functional>
  #include <algorithm>
#else
  #include <strstream.h>
  #include <set.h>
  #include <map.h>
  #include <function.h>
  #include <algo.h>
#endif

#include "mapp_cube_3d.h"
#include "mapp_tetra_3d.h"
  
#include "gitter_pll_impl.h"
#include "gitter_hexa_top_pll.h"
#include "gitter_tetra_top_pll.h"

const linkagePattern_t VertexPllBaseX :: nullPattern ;

extern float __STATIC_unpackCount;
extern float __STATIC_packCount;

extern float __STATIC_strEdge;
extern float __STATIC_resEdge;

extern float __STATIC_strTetra;
extern float __STATIC_resTetra;

VertexPllBaseX :: VertexPllBaseX (myvertex_t & v, linkagePatternMap_t & m) 
  : _v (v), _map (m), _lpn (), _moveTo (), _ref () {
  linkagePatternMap_t :: iterator pos = _map.find (nullPattern) ;
  _lpn = (pos != _map.end ()) ? pos : _map.insert (pair < const linkagePattern_t, int > (nullPattern,0)).first ;
  (*_lpn).second ++ ;
  return ;
}

VertexPllBaseX :: ~VertexPllBaseX () {
  assert (_moveTo.size () == 0) ;
  (*_lpn).second -- ;
  return ;
}

vector < int > VertexPllBaseX :: estimateLinkage () const {
  return vector < int > ((*_lpn).first) ;
}

LinkedObject :: Identifier VertexPllBaseX :: getIdentifier () const {
  return Identifier (myvertex().ident ()) ;
}

bool VertexPllBaseX :: setLinkage (vector < int > lp) {
  (*_lpn).second -- ;
  sort (lp.begin (), lp.end (), less < int > ()) ;
  linkagePatternMap_t :: iterator pos = _map.find (lp) ;
  _lpn = (pos != _map.end ()) ? pos : _map.insert (pair < const linkagePattern_t, int > (lp,0)).first ;
  (*_lpn).second ++ ;
  return true ;
}

void VertexPllBaseX :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  return ;
}

void VertexPllBaseX :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } else {
    (*pos).second ++ ;
  }
  return ;
}

bool VertexPllBaseX :: packAll (vector < ObjectStream > & osv) {
  bool action (false) ;
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    osv [j].writeObject (VERTEX) ;
    osv [j].writeObject (myvertex ().ident ()) ;
    osv [j].writeObject (myvertex ().Point ()[0]) ;
    osv [j].writeObject (myvertex ().Point ()[1]) ;
    osv [j].writeObject (myvertex ().Point ()[2]) ;
    
    inlineData (osv [j]) ;
    
    action = true ;
  }
  return action ;
}

void VertexPllBaseX :: unpackSelf (ObjectStream & os, bool i) {
  if (i) {
    xtractData (os) ;
  }
  return ;
}

vector < int > EdgePllBaseX :: estimateLinkage () const {
  return (abort (), vector < int > ()) ;
}

LinkedObject :: Identifier EdgePllBaseX :: getIdentifier () const {
  return (abort (), LinkedObject :: Identifier  ()) ;
}

void EdgePllBaseX :: getRefinementRequest (ObjectStream & os) const {
  os.writeObject (int(myhedge1 ().getrule ())) ;
  return ;
}

bool EdgePllBaseX :: setRefinementRequest (ObjectStream & os) {
  int i ;
  try {
    os.readObject (i) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  return myhedge1_t :: myrule_t (i) == myhedge1_t :: myrule_t :: nosplit ? 
    false : (myhedge1 ().refineImmediate (myhedge1_t :: myrule_t (i)), true) ;
}

void EdgePllBaseX :: unattach2 (int) {
  abort () ;
  return ;
}

void EdgePllBaseX :: attach2 (int) {
  abort () ;
  return ;
}

bool EdgePllBaseX :: packAll (vector < ObjectStream > &) {
  return (abort (), false) ;
}

void EdgePllBaseX :: unpackSelf (ObjectStream &,bool) {
  abort () ;
  return ;
}

bool EdgePllBaseX :: lockAndTry () {
  _lockCRS = true ;
  return myhedge1().coarse () ;
}

bool EdgePllBaseX :: lockedAgainstCoarsening () const {
  return _lockCRS ;
}

bool EdgePllBaseX :: unlockAndResume (bool r) {
  _lockCRS = false ;
  bool x ;
  if (r) {
    x = myhedge1().coarse () ;
  }
  else {
    x = false ;
  }
  return x ;
}

EdgePllBaseXMacro :: EdgePllBaseXMacro (myhedge1_t & e) : EdgePllBaseX (e), _moveTo (), _ref () {
  return ;
}

EdgePllBaseXMacro :: ~EdgePllBaseXMacro () {
  assert (0 == _moveTo.size ()) ;
  return ;
}

vector < int > EdgePllBaseXMacro :: estimateLinkage () const {
  vector < int > est ;
  vector < int > l0 = myhedge1 ().myvertex(0)->accessPllX ().estimateLinkage () ;
  vector < int > l1 = myhedge1 ().myvertex(1)->accessPllX ().estimateLinkage () ;
  set_intersection (l0.begin (), l0.end (), l1.begin (), l1.end (), back_inserter (est), less < int > ()) ;
  return est ;
}

LinkedObject :: Identifier EdgePllBaseXMacro :: getIdentifier () const {
  return Identifier (myhedge1 ().myvertex (0)->ident (), myhedge1 ().myvertex (1)->ident ()) ;
}

void EdgePllBaseXMacro :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  myhedge1 ().myvertex (0)->accessPllX ().unattach2 (i) ;
  myhedge1 ().myvertex (1)->accessPllX ().unattach2 (i) ;
  return ;
}

void EdgePllBaseXMacro :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } else {
    (*pos).second ++ ;
  }
  myhedge1 ().myvertex (0)->accessPllX ().attach2 (i) ;
  myhedge1 ().myvertex (1)->accessPllX ().attach2 (i) ;
  return ;
}

bool EdgePllBaseXMacro :: packAll (vector < ObjectStream > & osv) {
  bool action (false) ;
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    osv [j].writeObject (EDGE1) ;
    osv [j].writeObject (myhedge1 ().myvertex (0)->ident ()) ;
    osv [j].writeObject (myhedge1 ().myvertex (1)->ident ()) ;
    {
      strstream_t s ;
      myhedge1 ().backup (s) ;
      for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;
      osv [j].writeObject (ENDOFSTREAM) ;
      
      inlineData (osv [j]) ;
    }
    action = true ;
  }
  return action ;
}

void EdgePllBaseXMacro :: unpackSelf (ObjectStream & os, bool i) {
  strstream_t s ;
  int c ;
  try {
    Timer t; 
    for (os.readObject (c) ; c != ENDOFSTREAM ; os.readObject (c)) s.put ((char)c) ;
    __STATIC_strEdge += t.elapsed();
  } 
  catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) {
    Timer t; 
    myhedge1 ().restore (s) ;
    assert (!s.eof ()) ;
    
    xtractData (os) ;
    __STATIC_resEdge += t.elapsed();
  }
  return ;
}

// #######
// #        #       ######  #    #  ######  #    #   #####
// #        #       #       ##  ##  #       ##   #     #
// #####    #       #####   # ## #  #####   # #  #     #
// #        #       #       #    #  #       #  # #     #
// #        #       #       #    #  #       #   ##     #
// #######  ######  ######  #    #  ######  #    #     #

pair < ElementPllXIF_t *, int > ElementPllBaseX :: accessOuterPllX (const pair < ElementPllXIF_t *, int > & x, int) {
  return x ;
}

pair < ElementPllXIF_t *, int > ElementPllBaseX :: accessInnerPllX (const pair < ElementPllXIF_t *, int > &, int f) {
  return pair < ElementPllXIF_t *, int > (this,f) ;
}

pair < const ElementPllXIF_t *, int > ElementPllBaseX :: accessOuterPllX (const pair < const ElementPllXIF_t *, int > & x, int) const {
  return x ;
}

pair < const ElementPllXIF_t *, int > ElementPllBaseX :: accessInnerPllX (const pair < const ElementPllXIF_t *, int > &, int f) const {
  return pair < const ElementPllXIF_t *, int > (this,f) ;
}

bool ElementPllBaseX :: ldbUpdateGraphVertex (LoadBalancer :: DataBase &) {
  return (abort (), false) ;
}

int ElementPllBaseX :: ldbVertexIndex () const {
  return (abort (), 0) ;
}

int & ElementPllBaseX :: ldbVertexIndex () {
  return (abort (), *(int *)0) ;
}

void ElementPllBaseX :: writeStaticState (ObjectStream &, int) const {
  return ;
}

void ElementPllBaseX :: readStaticState (ObjectStream &, int) {
  assert(false);
  abort () ;
  return ;
}

void ElementPllBaseX :: readDynamicState (ObjectStream &, int) {
  assert(false);
  abort () ;
  return ;
}

void ElementPllBaseX :: unattach2 (int) {
  assert(false);
  abort () ;
  return ;
}

void ElementPllBaseX :: attach2 (int) {
  assert(false);
  abort () ;
  return ;
}

bool ElementPllBaseX :: packAll (vector < ObjectStream > &) { 
  return (abort (), false) ;
}

void ElementPllBaseX :: packAsBnd (int,int,ObjectStream &) const {
  assert(false);
  abort () ;
  return ;
}

void ElementPllBaseX :: unpackSelf (ObjectStream &, bool) {
  assert(false);
  abort () ;
  return ;
}

bool ElementPllBaseX :: erasable () const {
  return (abort (), false) ;
}

void ElementPllBaseX :: getRefinementRequest (ObjectStream &) {
  assert(false);
  abort () ;
  return ;
}

bool ElementPllBaseX :: setRefinementRequest (ObjectStream &) {
  return (abort (), false) ;
}

bool ElementPllBaseX :: lockAndTry () {
  return (abort (), false) ;
}

bool ElementPllBaseX :: unlockAndResume (bool) {
  return (abort (), false) ;
}

// #######
//    #     ######   #####  #####     ##
//    #     #          #    #    #   #  #
//    #     #####      #    #    #  #    #
//    #     #          #    #####   ######
//    #     #          #    #   #   #    #
//    #     ######     #    #    #  #    #
void TetraPllXBase :: writeDynamicState (ObjectStream & os, GatherScatterType & gs) const 
{
  gs.sendData( os , mytetra () );
  return ;
}

void TetraPllXBase :: writeDynamicState (ObjectStream & os, int face) const 
{
#ifndef _DUNE_NOT_USES_ALU3DGRID_
  // write level to know the level of ghost on the other side
  os.writeObject( mytetra().level() );
  os.writeObject( ((int) (mytetra().leaf() ? 1 : 0)) );
#endif
  return ;
}

void TetraPllXBase :: 
VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace) 
{
  mytetra().VertexData2os(os,gs,borderFace);
}

void TetraPllXBase :: EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  mytetra().EdgeData2os(os,gs,borderFace);
}

void TetraPllXBase :: FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  mytetra().FaceData2os(os,gs,borderFace);
} 

void HexaPllBaseX :: VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  myhexa().VertexData2os(os,gs,borderFace);
}

void HexaPllBaseX :: EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  myhexa().EdgeData2os(os,gs,borderFace);
}

void HexaPllBaseX :: FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  myhexa().FaceData2os(os,gs,borderFace);
}

TetraPllXBaseMacro :: TetraPllXBaseMacro (mytetra_t & t) : 
  TetraPllXBase (t), _ldbVertexIndex (-1), _moveTo (), _erasable (false) {
  static const double x = 0.25 ; 
  LinearMapping (mytetra ().myvertex (0)->Point (), mytetra ().myvertex (1)->Point (),
         mytetra ().myvertex (2)->Point (), mytetra ().myvertex (3)->Point ())
      .map2world (x,x,x,x,_center) ;
  return ;
}

TetraPllXBaseMacro :: ~TetraPllXBaseMacro () {
  vector < int > v ;
  {for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; v.push_back ((*i++).first)) ;}
  {for (vector < int > :: const_iterator i = v.begin () ; i != v.end () ; unattach2 (*i++)) ;}
  return ;
}

int TetraPllXBaseMacro :: ldbVertexIndex () const {
  return _ldbVertexIndex ;
}

int & TetraPllXBaseMacro :: ldbVertexIndex () {
  return _ldbVertexIndex ;
}

bool TetraPllXBaseMacro :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) {
  // parameter are: 
  // - macro vertex index
  // - number of elementes below macro element 
  // - bary center 
  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (mytetra ()).size (), _center) ) ;
  return true ;
}

void TetraPllXBaseMacro :: writeStaticState (ObjectStream & os, int) const {
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

void TetraPllXBaseMacro :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  mytetra ().myhface3 (0)->accessPllX ().unattach2 (i) ;
  mytetra ().myhface3 (1)->accessPllX ().unattach2 (i) ;
  mytetra ().myhface3 (2)->accessPllX ().unattach2 (i) ;
  mytetra ().myhface3 (3)->accessPllX ().unattach2 (i) ;
  return ;
}

void TetraPllXBaseMacro :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } else {
    if ((*pos).first == i) {
      cerr << "  TetraPllXBaseMacro :: attach2 () WARNUNG versuchte mehrfache Zuweisung ignoriert " << endl ;
      return ;
    }
  }
  mytetra ().myhface3 (0)->accessPllX ().attach2 (i) ;
  mytetra ().myhface3 (1)->accessPllX ().attach2 (i) ;
  mytetra ().myhface3 (2)->accessPllX ().attach2 (i) ;
  mytetra ().myhface3 (3)->accessPllX ().attach2 (i) ;
  return ;
}

bool TetraPllXBaseMacro :: packAll (vector < ObjectStream > & osv) {
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) 
  {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    assert (_moveTo.size () == 1) ;
    osv [j].writeObject (TETRA) ;
    osv [j].writeObject (mytetra ().myvertex (0)->ident ()) ;
    osv [j].writeObject (mytetra ().myvertex (1)->ident ()) ;
    osv [j].writeObject (mytetra ().myvertex (2)->ident ()) ;
    osv [j].writeObject (mytetra ().myvertex (3)->ident ()) ;
    {
      strstream_t s ;
      mytetra ().backup (s) ;
      for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;
      osv [j].writeObject (ENDOFSTREAM) ;
      inlineData (osv [j]) ;
    }
    _erasable = true ;
    return true ;
  }
  return false ;
}

bool TetraPllXBaseMacro :: dunePackAll (vector < ObjectStream > & osv,
    GatherScatterType & gs) {
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    assert (_moveTo.size () == 1) ;
    osv [j].writeObject (TETRA) ;
    osv [j].writeObject (mytetra ().myvertex (0)->ident ()) ;
    osv [j].writeObject (mytetra ().myvertex (1)->ident ()) ;
    osv [j].writeObject (mytetra ().myvertex (2)->ident ()) ;
    osv [j].writeObject (mytetra ().myvertex (3)->ident ()) ;
    {
      strstream_t s ;
      mytetra ().backup (s) ;
      for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;
      osv [j].writeObject (ENDOFSTREAM) ;
      
      inlineData (osv [j]) ;
      
      // count how long dune unpack lasts 
      const long start = clock();
    
      // pack Dune data 
      gs.inlineData( osv[j] , mytetra() );

      // count how long dune unpack lasts 
      const long end = clock();

      __STATIC_packCount += (float)(end-start)/(float)(CLOCKS_PER_SEC);
    
    }
    _erasable = true ;
    return true ;
  }
  return false ;
}

void TetraPllXBaseMacro :: packAsBndNow (int fce, ObjectStream & os) const 
{
  os.writeObject (HBND3INT) ;
  os.writeObject (Gitter :: hbndseg :: closure) ;
  os.writeObject ( mytetra ().myvertex (fce,0)->ident () ) ;
  os.writeObject ( mytetra ().myvertex (fce,1)->ident () ) ;
  os.writeObject ( mytetra ().myvertex (fce,2)->ident () ) ;
  
  // see method unpackHbnd3Int 
  int writePoint = MacroGridMoverIF :: POINTTRANSMITTED; // 1 == point is transmitted 
  os.writeObject ( writePoint ); // 1 == points are transmitted 

  // know which face is the internal bnd 
  os.writeObject (fce);

  // write the vertices of the tetra 
  for(int k=0; k<4; ++k) 
  {
    int vx = mytetra ().myvertex (k)->ident ();
    os.writeObject ( vx ) ;
  }

  {
    const Gitter :: Geometric :: VertexGeo * vertex = mytetra().myvertex(fce);
    assert( vertex );

    // know identifier of transmitted point 
    os.writeObject ( vertex->ident ()) ;

    // store the missing point to form a tetra 
    const double (&p)[3] = vertex->Point();
    os.writeObject ( p[0] ) ;
    os.writeObject ( p[1] ) ;
    os.writeObject ( p[2] ) ;
  }
}

// packs macro element as internal bnd for other proc 
void TetraPllXBaseMacro :: packAsBnd (int fce, int who, ObjectStream & os) const {
  bool hit = _moveTo.size () == 0 ? true : false ;
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; 
       i != _moveTo.end () ; i ++ )
  {
    if ((*i).first != who) hit = true ;
  }
  
  if (hit) 
  {
    // write data to stream 
    packAsBndNow(fce,os); 
  }
  return ;
}

// packs macro element as internal bnd for other proc 
void TetraPllXBaseMacro :: packAsGhost(ObjectStream & os, int fce) const 
{
  packAsBndNow(fce,os);
}

void TetraPllXBaseMacro :: unpackSelf (ObjectStream & os, bool i) {
  assert (i) ;
  strstream_t s ;
  int c ;
  try {
    Timer strT;
    for (os.readObject (c) ; c != ENDOFSTREAM ; os.readObject (c)) s.put ((char)c) ;
    __STATIC_strTetra += strT.elapsed();
  } 
  catch (ObjectStream :: EOFException) 
  {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) {
    Timer strT;
    mytetra ().restore (s) ;
    assert (!s.eof ()) ;
    xtractData (os) ;
    __STATIC_resTetra += strT.elapsed();
  }
  return ;
}

void TetraPllXBaseMacro :: duneUnpackSelf (ObjectStream & os, GatherScatterType
  & gs , bool i) 
{
  assert (i) ;
  strstream_t s ;
  int c ;
  try {
    for (os.readObject (c) ; c != ENDOFSTREAM ; os.readObject (c)) s.put ((char)c) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) 
  {
    mytetra ().restore (s) ;
    assert (!s.eof ()) ;
    xtractData (os) ;
    
    // count how long dune unpack lasts 
    const long start = clock();
    
    gs.xtractData( os , mytetra() );

    // count how long dune unpack lasts 
    const long end = clock();

    __STATIC_unpackCount += (float)(end-start)/(float)(CLOCKS_PER_SEC);
  }
  return ;
}

bool TetraPllXBaseMacro :: erasable () const {
  return _erasable ;
}

// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

void Periodic3PllXBase :: writeDynamicState (ObjectStream & os, int) const {

  // Der Schwerpunkt des "flachen" periodischen Randelements wird
  // auf die Mitte der linken Fl"ache gelegt. Per Definition.

  static const double x = 1./3. ;
  double p [3] ;
  LinearSurfaceMapping (myperiodic3 ().myvertex (0,0)->Point (), myperiodic3 ().myvertex (0,1)->Point (),
    myperiodic3 ().myvertex (0,2)->Point ()).map2world (x,x,x,p) ;
  os.writeObject (p [0]) ;
  os.writeObject (p [1]) ;
  os.writeObject (p [2]) ;
  return ;
}

Periodic3PllXBaseMacro :: Periodic3PllXBaseMacro (myperiodic3_t & p) : Periodic3PllXBase (p), _ldbVertexIndex (-1), _moveTo (), _erasable (false) {
  static const double x = 1./3. ;
  LinearSurfaceMapping (myperiodic3 ().myvertex (0,0)->Point (), myperiodic3 ().myvertex (0,1)->Point (),
         myperiodic3 ().myvertex (0,2)->Point ()).map2world (x,x,x,_center) ;
  return ;
}

Periodic3PllXBaseMacro :: ~Periodic3PllXBaseMacro () {
  vector < int > v ;
  {for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; v.push_back ((*i++).first)) ;}
  {for (vector < int > :: const_iterator i = v.begin () ; i != v.end () ; unattach2 (*i++)) ;}
  return ;
}

int Periodic3PllXBaseMacro :: ldbVertexIndex () const {
  return _ldbVertexIndex ;
}

int & Periodic3PllXBaseMacro :: ldbVertexIndex () {
  return _ldbVertexIndex ;
}

bool Periodic3PllXBaseMacro :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) {
  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (myperiodic3 ()).size (), _center)) ;
  return true ;
}

void Periodic3PllXBaseMacro :: writeStaticState (ObjectStream & os, int) const {
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

void Periodic3PllXBaseMacro :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  myperiodic3 ().myhface3 (0)->accessPllX ().unattach2 (i) ;
  myperiodic3 ().myhface3 (1)->accessPllX ().unattach2 (i) ;
  return ;
}

void Periodic3PllXBaseMacro :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } else {
    if ((*pos).first == i) {
      cerr << "  Periodic3PllXBaseMacro :: attach2 () WARNUNG versuchte mehrfache Zuweisung ignoriert " << endl ;
      return ;
    }
  }
  myperiodic3 ().myhface3 (0)->accessPllX ().attach2 (i) ;
  myperiodic3 ().myhface3 (1)->accessPllX ().attach2 (i) ;
  return ;
}

bool Periodic3PllXBaseMacro :: packAll (vector < ObjectStream > & osv) {
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    assert (_moveTo.size () == 1) ;
    osv [j].writeObject (PERIODIC3) ;
    osv [j].writeObject (myperiodic3 ().myvertex (0)->ident ()) ;
    osv [j].writeObject (myperiodic3 ().myvertex (1)->ident ()) ;
    osv [j].writeObject (myperiodic3 ().myvertex (2)->ident ()) ;
    osv [j].writeObject (myperiodic3 ().myvertex (3)->ident ()) ;
    osv [j].writeObject (myperiodic3 ().myvertex (4)->ident ()) ;
    osv [j].writeObject (myperiodic3 ().myvertex (5)->ident ()) ;
    {
      strstream_t s ;
      myperiodic3 ().backup (s) ;
      for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;
      osv [j].writeObject (ENDOFSTREAM) ;
      inlineData (osv [j]) ;
    }
    _erasable = true ;
    return true ;
  }
  return false ;
}

void Periodic3PllXBaseMacro :: packAsBnd (int fce, int who, ObjectStream & os) const {
  bool hit = _moveTo.size () == 0 ? true : false ;
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++ )
    if ((*i).first != who) hit = true ;
  if (hit) {
    os.writeObject (HBND3INT) ;
    os.writeObject (Gitter :: hbndseg :: closure) ;
    os.writeObject (myperiodic3 ().myvertex (fce,0)->ident ()) ;
    os.writeObject (myperiodic3 ().myvertex (fce,1)->ident ()) ;
    os.writeObject (myperiodic3 ().myvertex (fce,2)->ident ()) ;
    os.writeObject ( MacroGridMoverIF :: NO_POINT ); // 0 == no point transmitted 
  }
  return ;
}

void Periodic3PllXBaseMacro :: unpackSelf (ObjectStream & os, bool i) {
  assert (i) ;
  strstream_t s ;
  int c ;
  try {
    for (os.readObject (c) ; c != -1 ; os.readObject (c)) s.put ((char)c) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) {
    myperiodic3 ().restore (s) ;
    assert (!s.eof ()) ;
    xtractData (os) ;
  }
  return ;
}

bool Periodic3PllXBaseMacro :: erasable () const {
  return _erasable ;
}


// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

void Periodic4PllXBase :: writeDynamicState (ObjectStream & os, int) const {

  // Der Schwerpunkt des "flachen" periodischen Randelements wird
  // auf die Mitte der linken Fl"ache gelegt. Per Definition.

  static const double x = .0 ;
  double p [3] ;
  BilinearSurfaceMapping (myperiodic4 ().myvertex (0,0)->Point (), myperiodic4 ().myvertex (0,1)->Point (),
    myperiodic4 ().myvertex (0,2)->Point (),myperiodic4 ().myvertex (0,3)->Point ()).map2world (x,x,p) ;
  os.writeObject (p [0]) ;
  os.writeObject (p [1]) ;
  os.writeObject (p [2]) ;
  return ;
}

Periodic4PllXBaseMacro :: Periodic4PllXBaseMacro (myperiodic4_t & p) : Periodic4PllXBase (p), _ldbVertexIndex (-1), _moveTo (), _erasable (false) {
  static const double x = .0 ;
  BilinearSurfaceMapping (myperiodic4 ().myvertex (0,0)->Point (), myperiodic4 ().myvertex (0,1)->Point (),
         myperiodic4 ().myvertex (0,2)->Point (), myperiodic4 ().myvertex (0,3)->Point ()).map2world (x,x,_center) ;
  return ;
}

Periodic4PllXBaseMacro :: ~Periodic4PllXBaseMacro () {
  vector < int > v ;
  {for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; v.push_back ((*i++).first)) ;}
  {for (vector < int > :: const_iterator i = v.begin () ; i != v.end () ; unattach2 (*i++)) ;}
  return ;
}

int Periodic4PllXBaseMacro :: ldbVertexIndex () const {
  return _ldbVertexIndex ;
}

int & Periodic4PllXBaseMacro :: ldbVertexIndex () {
  return _ldbVertexIndex ;
}

bool Periodic4PllXBaseMacro :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) {
  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (myperiodic4 ()).size (), _center)) ;
  return true ;
}

void Periodic4PllXBaseMacro :: writeStaticState (ObjectStream & os, int) const {
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

void Periodic4PllXBaseMacro :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  myperiodic4 ().myhface4 (0)->accessPllX ().unattach2 (i) ;
  myperiodic4 ().myhface4 (1)->accessPllX ().unattach2 (i) ;
  return ;
}

void Periodic4PllXBaseMacro :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } else {
    if ((*pos).first == i) {
      cerr << "  Periodic4PllXBaseMacro :: attach2 () WARNUNG versuchte mehrfache Zuweisung ignoriert " << endl ;
      return ;
    }
  }
  myperiodic4 ().myhface4 (0)->accessPllX ().attach2 (i) ;
  myperiodic4 ().myhface4 (1)->accessPllX ().attach2 (i) ;
  return ;
}

bool Periodic4PllXBaseMacro :: packAll (vector < ObjectStream > & osv) {
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    assert (_moveTo.size () == 1) ;
    osv [j].writeObject (PERIODIC4) ;
    osv [j].writeObject (myperiodic4 ().myvertex (0)->ident ()) ;
    osv [j].writeObject (myperiodic4 ().myvertex (1)->ident ()) ;
    osv [j].writeObject (myperiodic4 ().myvertex (2)->ident ()) ;
    osv [j].writeObject (myperiodic4 ().myvertex (3)->ident ()) ;
    osv [j].writeObject (myperiodic4 ().myvertex (4)->ident ()) ;
    osv [j].writeObject (myperiodic4 ().myvertex (5)->ident ()) ;
    osv [j].writeObject (myperiodic4 ().myvertex (6)->ident ()) ;
    osv [j].writeObject (myperiodic4 ().myvertex (7)->ident ()) ;
    {
      strstream_t s ;
      myperiodic4 ().backup (s) ;
      for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;
      osv [j].writeObject ( ENDOFSTREAM ) ;
      inlineData (osv [j]) ;
    }
    _erasable = true ;
    return true ;
  }
  return false ;
}

void Periodic4PllXBaseMacro :: packAsBnd (int fce, int who, ObjectStream & os) const {
  bool hit = _moveTo.size () == 0 ? true : false ;
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++ )
    if ((*i).first != who) hit = true ;
  if (hit) {
    os.writeObject (HBND4INT) ;
    os.writeObject (Gitter :: hbndseg :: closure) ;
    os.writeObject (myperiodic4 ().myvertex (fce,0)->ident ()) ;
    os.writeObject (myperiodic4 ().myvertex (fce,1)->ident ()) ;
    os.writeObject (myperiodic4 ().myvertex (fce,2)->ident ()) ;
    os.writeObject (myperiodic4 ().myvertex (fce,3)->ident ()) ;
    os.writeObject ( MacroGridMoverIF :: NO_POINT ); // 0 == no point transmitted 
  }
  return ;
}

void Periodic4PllXBaseMacro :: unpackSelf (ObjectStream & os, bool i) {
  assert (i) ;
  strstream_t s ;
  int c ;
  try {
    for (os.readObject (c) ; c != ENDOFSTREAM ; os.readObject (c)) s.put ((char)c) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) {
    myperiodic4 ().restore (s) ;
    assert (!s.eof ()) ;
    xtractData (os) ;
  }
  return ;
}

bool Periodic4PllXBaseMacro :: erasable () const {
  return _erasable ;
}

  // #     #
  // #     #  ######  #    #    ##
  // #     #  #        #  #    #  #
  // #######  #####     ##    #    #
  // #     #  #         ##    ######
  // #     #  #        #  #   #    #
  // #     #  ######  #    #  #    #

void HexaPllBaseX  :: writeDynamicState (ObjectStream & os, GatherScatterType & gs) const {
  gs.sendData( os , myhexa () );
  return ;
}
void HexaPllBaseX :: writeDynamicState (ObjectStream & os, int face) const {
  // siehe writeDynamicState von Tetra 

#ifndef _DUNE_NOT_USES_ALU3DGRID_
  // write level to know the level of ghost on the other side
  os.writeObject( myhexa().level() );
  const int leaf = (myhexa().leaf() ? 1 : 0);
  os.writeObject( leaf );
#endif

  return ;
}

HexaPllBaseXMacro :: HexaPllBaseXMacro (myhexa_t & h) : HexaPllBaseX (h), _ldbVertexIndex (-1), _moveTo (), _erasable (false) {
  //double p [3] ;
  TrilinearMapping (myhexa ().myvertex (0)->Point (), myhexa ().myvertex (1)->Point (),
    myhexa ().myvertex (2)->Point (), myhexa ().myvertex (3)->Point (), myhexa ().myvertex (4)->Point (),
    myhexa ().myvertex (5)->Point (), myhexa ().myvertex (6)->Point (), myhexa ().myvertex (7)->Point ())
    .map2world (.0,.0,.0,_center) ;
  return ;
}

HexaPllBaseXMacro :: ~HexaPllBaseXMacro () {
  vector < int > v ;
  {for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; v.push_back ((*i++).first)) ;}
  {for (vector < int > :: const_iterator i = v.begin () ; i != v.end () ; unattach2 (*i++)) ;}
  return ;
}

int HexaPllBaseXMacro :: ldbVertexIndex () const {
  return _ldbVertexIndex ;
}

int & HexaPllBaseXMacro :: ldbVertexIndex () {
  return _ldbVertexIndex ;
}

bool HexaPllBaseXMacro :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) {
  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (myhexa ()).size (), _center)) ;
  return true ;
}

void HexaPllBaseXMacro :: writeStaticState (ObjectStream & os, int) const {
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

void HexaPllBaseXMacro :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  myhexa ().myhface4 (0)->accessPllX ().unattach2 (i) ;
  myhexa ().myhface4 (1)->accessPllX ().unattach2 (i) ;
  myhexa ().myhface4 (2)->accessPllX ().unattach2 (i) ;
  myhexa ().myhface4 (3)->accessPllX ().unattach2 (i) ;
  myhexa ().myhface4 (4)->accessPllX ().unattach2 (i) ;
  myhexa ().myhface4 (5)->accessPllX ().unattach2 (i) ;
  return ;
}

void HexaPllBaseXMacro :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } else {
    if ((*pos).first == i) {
      cerr << "  HexaPllBaseXMacro :: attach2 () WARNUNG versuchte mehrfache Zuweisung ignoriert " << endl ;
      return ;
    }
  }
  myhexa ().myhface4 (0)->accessPllX ().attach2 (i) ;
  myhexa ().myhface4 (1)->accessPllX ().attach2 (i) ;
  myhexa ().myhface4 (2)->accessPllX ().attach2 (i) ;
  myhexa ().myhface4 (3)->accessPllX ().attach2 (i) ;
  myhexa ().myhface4 (4)->accessPllX ().attach2 (i) ;
  myhexa ().myhface4 (5)->accessPllX ().attach2 (i) ;
  return ;
}

bool HexaPllBaseXMacro :: packAll (vector < ObjectStream > & osv) {
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    assert (_moveTo.size () == 1) ;
    osv [j].writeObject (HEXA) ;
    osv [j].writeObject (myhexa ().myvertex (0)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (1)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (2)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (3)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (4)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (5)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (6)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (7)->ident ()) ;
    {
      strstream_t s ;
      myhexa ().backup (s) ;
      for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;
      osv [j].writeObject ( ENDOFSTREAM ) ;
      inlineData (osv [j]) ;
    }
    _erasable = true ;
    return true ;
  }
  return false ;
}

// pack all function for dune 
bool HexaPllBaseXMacro :: dunePackAll (vector < ObjectStream > & osv,
    GatherScatterType & gs) {
  
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) 
  {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    assert (_moveTo.size () == 1) ;

    osv [j].writeObject (HEXA) ;
    osv [j].writeObject (myhexa ().myvertex (0)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (1)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (2)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (3)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (4)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (5)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (6)->ident ()) ;
    osv [j].writeObject (myhexa ().myvertex (7)->ident ()) ;
    {
      strstream_t s ;
      myhexa ().backup (s) ;
      for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;
      osv [j].writeObject ( ENDOFSTREAM ) ;
      inlineData (osv [j]) ;

      // pack Dune data 
      gs.inlineData( osv[j] , myhexa() );
    }
    _erasable = true ;
    return true ;
  }
  return false ;
}


void HexaPllBaseXMacro :: packAsBndNow(int fce, ObjectStream & os) const 
{
  os.writeObject (HBND4INT) ;
  os.writeObject (Gitter :: hbndseg :: closure) ;

  // write the four identifiers of the hexa 
  os.writeObject (myhexa ().myvertex (fce,0)->ident ()) ;
  os.writeObject (myhexa ().myvertex (fce,1)->ident ()) ;
  os.writeObject (myhexa ().myvertex (fce,2)->ident ()) ;
  os.writeObject (myhexa ().myvertex (fce,3)->ident ()) ;

  // see method unpackHbnd4Int 
  int writePoint = MacroGridMoverIF :: POINTTRANSMITTED; 
  os.writeObject ( writePoint ); // 1 == points are transmitted 

  // know which face is the internal bnd 
  os.writeObject (fce);
 
  for(int k=0; k<8; k++) 
  {
    int vx = myhexa ().myvertex (k)->ident ();
    os.writeObject ( vx ) ;
  }

  int oppFace = Gitter :: Geometric :: Hexa :: oppositeFace[fce];
  for(int vx=0; vx<4; vx++)
  {
    const Gitter :: Geometric :: VertexGeo * vertex = myhexa().myvertex(oppFace,vx); 
    os.writeObject( vertex->ident() );
    const double (&p)[3] = vertex->Point();
    os.writeObject ( p[0] ) ;
    os.writeObject ( p[1] ) ;
    os.writeObject ( p[2] ) ;
  }
}

void HexaPllBaseXMacro :: packAsGhost(ObjectStream & os, int fce) const 
{
  packAsBndNow(fce, os);
}

// packs macro element as internal bnd for other proc 
void HexaPllBaseXMacro :: packAsBnd (int fce, int who, ObjectStream & os) const 
{
  bool hit = _moveTo.size () == 0 ? true : false ;
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; 
       i != _moveTo.end () ; i ++ )
  {
    if ((*i).first != who) hit = true ;
  }

  if (hit) 
  {
    packAsBndNow( fce, os );
  }
  return ;
}

void HexaPllBaseXMacro :: unpackSelf (ObjectStream & os, bool i) {
  assert (i) ;
  strstream_t s ;
  int c ;
  try {
    for (os.readObject (c) ; c != ENDOFSTREAM ; os.readObject (c)) s.put ((char)c) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) {
    myhexa ().restore (s) ;
    assert (!s.eof ()) ;
    xtractData (os) ;
  }
  return ;
}

void HexaPllBaseXMacro :: duneUnpackSelf (ObjectStream & os, GatherScatterType & gs , bool i) {
  assert (i) ;
  strstream_t s ;
  int c ;
  try {
    for (os.readObject (c) ; c != ENDOFSTREAM ; os.readObject (c)) s.put ((char)c) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) {
    myhexa ().restore (s) ;
    assert (!s.eof ()) ;
    xtractData (os) ;
    gs.xtractData( os , myhexa() );
  }
  return ;
}

bool HexaPllBaseXMacro :: erasable () const {
  return _erasable ;
}

pair < ElementPllXIF_t *, int > BndsegPllBaseX :: accessOuterPllX (const pair < ElementPllXIF_t *, int > &, int f) {
  assert (!f) ;  
  return pair < ElementPllXIF_t *, int > (this,0) ;
}

pair < ElementPllXIF_t *, int > BndsegPllBaseX :: accessInnerPllX (const pair < ElementPllXIF_t *, int > & x, int) {
  return x ;
}

pair < const ElementPllXIF_t *, int > BndsegPllBaseX :: accessOuterPllX (const pair < const ElementPllXIF_t *, int > &, int f) const {
  assert (!f) ;
  return pair < const ElementPllXIF_t *, int > (this,0) ;
}

pair < const ElementPllXIF_t *, int > BndsegPllBaseX :: accessInnerPllX (const pair < const ElementPllXIF_t *, int > & x, int) const {
  return x ;
}

GitterBasisPll :: ObjectsPll :: VertexPllImplMacro :: VertexPllImplMacro (double x, double y, double z, int i, 
                    linkagePatternMap_t & map, IndexManagerType &im) 
  : GitterBasis :: Objects :: VertexEmptyMacro (x,y,z,i,im), _pllx (new mypllx_t (*this,map)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: VertexPllImplMacro :: ~VertexPllImplMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

VertexPllXIF_t & GitterBasisPll :: ObjectsPll :: VertexPllImplMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const VertexPllXIF_t & GitterBasisPll :: ObjectsPll :: VertexPllImplMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: VertexPllImplMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

EdgePllXIF_t & GitterBasisPll :: ObjectsPll :: Hedge1EmptyPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _pllx ;
}

const EdgePllXIF_t & GitterBasisPll :: ObjectsPll :: Hedge1EmptyPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _pllx ;
}

GitterBasisPll :: ObjectsPll :: Hedge1EmptyPllMacro :: Hedge1EmptyPllMacro (myvertex_t * a, myvertex_t * b, IndexManagerType & im) :
  GitterBasisPll :: ObjectsPll :: hedge1_IMPL (0, a, b, im)
    , _pllx (new mypllx_t (*this)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: Hedge1EmptyPllMacro :: ~Hedge1EmptyPllMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

EdgePllXIF_t & GitterBasisPll :: ObjectsPll :: Hedge1EmptyPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const EdgePllXIF_t & GitterBasisPll :: ObjectsPll :: Hedge1EmptyPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: Hedge1EmptyPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface3EmptyPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _pllx ;
}

const FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface3EmptyPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _pllx ;
}

GitterBasisPll :: ObjectsPll :: Hface3EmptyPllMacro :: Hface3EmptyPllMacro (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2, IndexManagerType & im)
  : GitterBasisPll :: ObjectsPll :: hface3_IMPL (0,e0,s0,e1,s1,e2,s2,im), _pllx (new mypllx_t (*this)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: Hface3EmptyPllMacro :: ~Hface3EmptyPllMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface3EmptyPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface3EmptyPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: Hface3EmptyPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface4EmptyPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _pllx ;
}

const FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface4EmptyPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _pllx ;
}

GitterBasisPll :: ObjectsPll :: Hface4EmptyPllMacro :: Hface4EmptyPllMacro (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2, myhedge1_t * e3, int s3, IndexManagerType & im)
  : GitterBasisPll :: ObjectsPll :: hface4_IMPL (0,e0,s0,e1,s1,e2,s2,e3,s3,im), _pllx (new mypllx_t (*this)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: Hface4EmptyPllMacro :: ~Hface4EmptyPllMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface4EmptyPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const FacePllXIF_t & GitterBasisPll :: ObjectsPll :: Hface4EmptyPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: Hface4EmptyPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: TetraEmptyPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: TetraEmptyPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _pllx ;
}

void GitterBasisPll :: ObjectsPll :: TetraEmptyPll :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  abort () ;  // Auf dem feinen Element ist die Aktion nicht zul"assig.
  return ;
}
  
GitterBasisPll :: ObjectsPll :: TetraEmptyPllMacro :: TetraEmptyPllMacro (myhface3_t * f0, int t0, myhface3_t * f1, int t1, myhface3_t * f2, int t2, 
                              myhface3_t * f3, int t3, IndexManagerType & im , Gitter * mygrid)
  : GitterBasisPll :: ObjectsPll :: tetra_IMPL (0,f0,t0,f1,t1,f2,t2,f3,t3,im,mygrid), _pllx (new mypllx_t (*this)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: TetraEmptyPllMacro :: ~TetraEmptyPllMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: TetraEmptyPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: TetraEmptyPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: TetraEmptyPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}


// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic3EmptyPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic3EmptyPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _pllx ;
}

void GitterBasisPll :: ObjectsPll :: Periodic3EmptyPll :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  abort () ;  // Auf dem feinen Element ist die Aktion nicht zul"assig.
  return ;
}

GitterBasisPll :: ObjectsPll :: Periodic3EmptyPllMacro :: Periodic3EmptyPllMacro (myhface3_t * f0, int t0, myhface3_t * f1, int t1)
  : GitterBasisPll :: ObjectsPll :: periodic3_IMPL (0,f0,t0,f1,t1), _pllx (new mypllx_t (*this)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: Periodic3EmptyPllMacro :: ~Periodic3EmptyPllMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic3EmptyPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic3EmptyPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: Periodic3EmptyPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}


// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic4EmptyPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic4EmptyPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _pllx ;
}

void GitterBasisPll :: ObjectsPll :: Periodic4EmptyPll :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  abort () ;  // Auf dem feinen Element ist die Aktion nicht zul"assig.
  return ;
}

GitterBasisPll :: ObjectsPll :: Periodic4EmptyPllMacro :: Periodic4EmptyPllMacro (myhface4_t * f0, int t0, myhface4_t * f1, int t1)
  : GitterBasisPll :: ObjectsPll :: periodic4_IMPL (0,f0,t0,f1,t1), _pllx (new mypllx_t (*this)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: Periodic4EmptyPllMacro :: ~Periodic4EmptyPllMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic4EmptyPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: Periodic4EmptyPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: Periodic4EmptyPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}


ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: HexaEmptyPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: HexaEmptyPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _pllx ;
}

void GitterBasisPll :: ObjectsPll :: HexaEmptyPll :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  abort () ;
  return ;
}

GitterBasisPll :: ObjectsPll :: HexaEmptyPllMacro :: HexaEmptyPllMacro 
  (myhface4_t * f0, int t0, myhface4_t * f1, int t1, myhface4_t * f2, int t2, 
   myhface4_t * f3, int t3, myhface4_t * f4, int t4, myhface4_t * f5, int t5, IndexManagerType & im, Gitter * gitter)
  : GitterBasisPll :: ObjectsPll :: hexa_IMPL (0,f0,t0,f1,t1,f2,t2,f3,t3,f4,t4,f5,t5,im,gitter), _pllx (new mypllx_t (*this)) {
  return ;
}

GitterBasisPll :: ObjectsPll :: HexaEmptyPllMacro :: ~HexaEmptyPllMacro () {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: HexaEmptyPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

const ElementPllXIF_t & GitterBasisPll :: ObjectsPll :: HexaEmptyPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_pllx) ;
  return * _pllx ;
}

void GitterBasisPll :: ObjectsPll :: HexaEmptyPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _pllx ;
  _pllx = 0 ;
  return ;
}

GitterBasisPll :: MacroGitterBasisPll :: MacroGitterBasisPll (Gitter * mygrid , istream & in) : GitterPll :: MacroGitterPll () , GitterBasis:: MacroGitterBasis (mygrid) {
  macrogridBuilder (in) ;
  return ;
}

GitterBasisPll :: MacroGitterBasisPll :: MacroGitterBasisPll (Gitter * mygrid) : GitterPll :: MacroGitterPll () , GitterBasis :: MacroGitterBasis (mygrid) {
  return ;
}

GitterBasisPll :: MacroGitterBasisPll :: ~MacroGitterBasisPll () {
  try {
    {
      AccessIterator < helement_STI > :: Handle w (*this) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().detachPllXFromMacro () ;
    }
    {
      AccessIterator < hface_STI > :: Handle w (*this) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().detachPllXFromMacro () ;
    }
    {
      AccessIterator < hedge_STI > :: Handle w (*this) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().detachPllXFromMacro () ;
    }
    {
      AccessIterator < vertex_STI > :: Handle w (*this) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().detachPllXFromMacro () ;
    }
  } catch (Parallel :: AccessPllException) {
    cerr << "**WARNUNG (AUSNAHME IGNORIERT) in " << __FILE__ << " " << __LINE__ << endl ;
  }
  {
    for (linkagePatternMap_t :: iterator p = _linkagePatterns.begin () ; p != _linkagePatterns.end () ; p ++) 
    {
      if( (*p).second != 0 ) 
        std::cout << (*p).second << " p sec \n";
      assert ((*p).second == 0) ;
    }
    _linkagePatterns.erase (_linkagePatterns.begin (), _linkagePatterns.end ()) ;
  }
  return ;
}

set < int, less < int > > GitterBasisPll :: MacroGitterBasisPll :: secondScan () {
  set < int, less < int > > s ;
  int n = 0 ;
  for (linkagePatternMap_t :: iterator p = _linkagePatterns.begin () ; p != _linkagePatterns.end () ; ) {
    if ((*p).second) {
      for (vector < int > :: const_iterator i = (*p).first.begin () ; i != (*p).first.end () ; s.insert (*i++)) ;
      p ++ ;
    } else {
      _linkagePatterns.erase (p ++) ;
      n ++ ;
    }
  }
  assert (debugOption (20) ? (cout << "  GitterBasisPll :: MacroGitterBasisPll :: secondScan () deleted " << n << " patterns" << endl, 1) : 1) ;
  return s ;
}

Gitter :: Geometric :: VertexGeo * GitterBasisPll :: MacroGitterBasisPll :: insert_vertex (double x,double y,double z,int i) {
  return new ObjectsPll :: VertexPllImplMacro (x,y,z,i,_linkagePatterns,indexManager(IM_Vertices)) ;
}

Gitter :: Geometric :: VertexGeo * GitterBasisPll :: MacroGitterBasisPll :: 
insert_ghostvx (double x, double y, double z, int i) 
{
  return GitterBasis :: MacroGitterBasis :: insert_vertex (x,y,z,i);
}

Gitter :: Geometric :: hedge1_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hedge1 (VertexGeo *a, VertexGeo *b) 
{
  return new ObjectsPll :: Hedge1EmptyPllMacro (a,b,indexManager(IM_Edges) ) ;
}

Gitter :: Geometric :: hedge1_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hedge1_twist (VertexGeo *a, int aid,  VertexGeo *b , int bid ) 
{
  if(aid < bid) 
    return GitterBasis :: MacroGitterBasis :: insert_hedge1 (a,b);
  else 
    return GitterBasis :: MacroGitterBasis :: insert_hedge1 (b,a);
} 

Gitter :: Geometric :: hface4_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hface4 (hedge1_GEO *(&e)[4], int (&s)[4]) 
{
  return new ObjectsPll :: Hface4EmptyPllMacro (e [0], s [0], e [1], s [1], e [2], s [2], e [3], s [3], indexManager(IM_Faces) ) ;
}

Gitter :: Geometric :: hface3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3]) 
{
  return new ObjectsPll :: Hface3EmptyPllMacro (e [0], s [0], e [1], s [1], e [2], s [2], indexManager(IM_Faces) ) ;
}

Gitter :: Geometric :: hexa_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hexa (hface4_GEO *(&f)[6], int (&t)[6]) 
{
  return new ObjectsPll :: HexaEmptyPllMacro (f [0], t[0], f [1], t[1], f [2], t[2], f[3], t[3], f[4], t[4], f[5], t[5], 
                                              indexManager(IM_Elements), this->_myGrid ) ;
}

Gitter :: Geometric :: tetra_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_tetra (hface3_GEO *(&f)[4], int (&t)[4]) 
{
  return new ObjectsPll :: TetraEmptyPllMacro (f [0], t[0], f [1], t[1], f [2], t[2], f[3], t[3], 
                                               indexManager(IM_Elements), this->_myGrid ) ;
}

Gitter :: Geometric :: periodic3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_periodic3 (hface3_GEO *(&f)[2], int (&t)[2]) 
{
  return new ObjectsPll :: Periodic3EmptyPllMacro (f [0], t[0], f [1], t[1]) ;
} 

Gitter :: Geometric :: periodic4_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_periodic4 (hface4_GEO *(&f)[2], int (&t)[2]) 
{
  return new ObjectsPll :: Periodic4EmptyPllMacro (f [0], t[0], f [1], t[1]) ;
}

Gitter :: Geometric :: hbndseg4_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd4 (hface4_GEO * f, int t, Gitter :: hbndseg_STI :: bnd_t b) 
{
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    // internal face always get dummy index manager      
    typedef GitterBasis :: Objects :: Hbnd4Default Hbnd4DefaultType;
    return new Hbnd4PllInternal < Hbnd4DefaultType , BndsegPllBaseXClosure < Hbnd4DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd4DefaultType > > :: macro_t (f,t, NULL, b, indexManager(IM_Internal) , this->_myGrid , *this ) ;
  } else {
    return new Hbnd4PllExternal < GitterBasis :: Objects :: Hbnd4Default, 
        BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, NULL, b, indexManager(IM_Bnd) , 0 ) ;
  }
}

Gitter :: Geometric :: hbndseg4_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd4_ghost (hface4_GEO * f, int t) 
{
  Gitter :: hbndseg_STI :: bnd_t b = Gitter :: hbndseg_STI :: ghost_closure;
  return new Hbnd4PllExternal < GitterBasis :: Objects :: Hbnd4Default, 
    BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, NULL, b, indexManager(IM_Internal) , 0 ) ;
}


Gitter :: Geometric :: hbndseg4_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd4 (hface4_GEO * f, int t, Gitter :: hbndseg_STI :: bnd_t b, 
              MacroGhostInfoHexa* ghInfo) 
{ 
  typedef GitterBasis :: Objects :: Hbnd4Default Hbnd4DefaultType;
  // if internal boundary create ghost 
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    assert( ghInfo );
    return new Hbnd4PllInternal < Hbnd4DefaultType , BndsegPllBaseXClosure < Hbnd4DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd4DefaultType > > :: 
          macro_t (f,t, NULL, b, indexManager(IM_Internal) , this->_myGrid , *this, ghInfo ) ;
  } 
  else 
  {
    return new Hbnd4PllExternal < Hbnd4DefaultType , 
        BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, NULL, b, indexManager(IM_Bnd) , 0 ) ;
  }
}

// version with point 
Gitter :: Geometric :: hbndseg3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd3 (hface3_GEO * f, int t, Gitter :: hbndseg_STI :: bnd_t b, 
              MacroGhostInfoTetra * ghInfo) 
{
  typedef GitterBasis :: Objects :: Hbnd3Default Hbnd3DefaultType;
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    assert( ghInfo );
    // this HbnPll has a ghost element so is dosent get and index ==> dummyindex == 5 (see gitter_sti.h)
    return new Hbnd3PllInternal < Hbnd3DefaultType , BndsegPllBaseXClosure < Hbnd3DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd3DefaultType > > :: 
              macro_t (f,t,NULL, b, indexManager(IM_Internal) , this->_myGrid , *this, ghInfo ) ;
  } 
  else 
  {
    return new Hbnd3PllExternal < Hbnd3DefaultType , 
        BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t,NULL, b, indexManager(IM_Bnd) , 0 ) ;
  }
}

// version without point 
Gitter :: Geometric :: hbndseg3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd3 (hface3_GEO * f, int t, Gitter :: hbndseg_STI :: bnd_t b ) 
{
  typedef GitterBasis :: Objects :: Hbnd3Default Hbnd3DefaultType;
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    // here we have a ghost of the ghost, therefor we need the element index manager 
    return new Hbnd3PllInternal < Hbnd3DefaultType , BndsegPllBaseXClosure < Hbnd3DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd3DefaultType > > :: macro_t (f,t,NULL, b, indexManager(IM_Internal) , this->_myGrid , *this ) ;
  } 
  else 
  {
    return new Hbnd3PllExternal < Hbnd3DefaultType , 
           BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t,NULL, b, indexManager(IM_Bnd), 0 ) ;
  }
}

Gitter :: Geometric :: hbndseg3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd3_ghost (hface3_GEO * f, int t) 
{
  Gitter :: hbndseg_STI :: bnd_t b = Gitter :: hbndseg_STI :: ghost_closure;
  typedef GitterBasis :: Objects :: Hbnd3Default Hbnd3DefaultType;

  return new Hbnd3PllExternal < Hbnd3DefaultType , 
         BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t,NULL, b, indexManager(IM_Bnd), 0 ) ;
}

IteratorSTI < Gitter :: vertex_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const vertex_STI * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: vertex_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const IteratorSTI < vertex_STI > * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: hedge_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const hedge_STI * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: hedge_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const IteratorSTI < hedge_STI > * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: hface_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const hface_STI * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: hface_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const IteratorSTI < hface_STI > * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: helement_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const helement_STI * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: helement_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const IteratorSTI < helement_STI > * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: hbndseg_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const hbndseg_STI * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

IteratorSTI < Gitter :: hbndseg_STI > * GitterBasisPll :: MacroGitterBasisPll :: iterator (const IteratorSTI < hbndseg_STI > * a) const {
  return GitterBasis :: MacroGitterBasis :: iterator (a) ;
}

GitterBasisPll :: GitterBasisPll (MpAccessLocal & mpa) : _mpaccess (mpa), _macrogitter (0) {
  _macrogitter = new MacroGitterBasisPll (this) ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

GitterBasisPll :: GitterBasisPll (const char * f, MpAccessLocal & mpa) : _mpaccess (mpa), _macrogitter (0) {
  assert (debugOption (20) ? (cout << "GitterBasisPll :: GitterBasisPll (const char * = \"" << f << "\" ...)" << endl, 1) : 1) ;

  // if still no macrogitter, try old method 
  if(!_macrogitter) 
  {
    char * extendedName = new char [strlen(f) + 200] ;
    sprintf (extendedName, "%s.%u", f, mpa.myrank ()) ;
    ifstream in (extendedName) ;
    if (in) {
      _macrogitter = new MacroGitterBasisPll (this,in) ;
    } 
    else 
    {
      assert (debugOption (5) ? 
        ( cerr << "  GitterBasisPll :: GitterBasisPll () file: " << extendedName 
           << " cannot be read. Try " << f << " instead. In " << __FILE__ << " line " << __LINE__ << endl, 1) : 1);
    }
    delete [] extendedName ;
  }

  // read normal macro gitter if myrank is 0 
  //if( (mpa.myrank () == 0) && !_macrogitter )
  if( !_macrogitter )
  {
    ifstream in ( f ) ;
    if (in) _macrogitter = new MacroGitterBasisPll (this,in) ;
  }
  
  // create empty macro gitter 
  if(!_macrogitter) _macrogitter = new MacroGitterBasisPll (this) ;

  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

GitterBasisPll :: ~GitterBasisPll () {
  delete _macrogitter ;
  return ;
}

