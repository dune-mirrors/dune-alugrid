// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 

#include "mapp_cube_3d.h"
#include "mapp_tetra_3d.h"
  
#include "gitter_pll_impl.h"
#include "gitter_hexa_top_pll.h"
#include "gitter_tetra_top_pll.h"

const linkagePattern_t VertexPllBaseX :: nullPattern ;

VertexPllBaseX :: VertexPllBaseX (myvertex_t & v, linkagePatternMap_t & m) 
  : _v (v), _map (m), _lpn (), _moveTo (), _ref () 
{
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
  typedef map < int, int, less < int > > :: const_iterator  const_iterator;
  const const_iterator iEnd =  _moveTo.end () ; 
  for (const_iterator i = _moveTo.begin () ; i != iEnd ; ++i) 
  {
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

template < class A >
vector < int > EdgePllBaseXMacro< A > :: estimateLinkage () const {
  vector < int > est ;
  vector < int > l0 = myhedge1 ().myvertex(0)->accessPllX ().estimateLinkage () ;
  vector < int > l1 = myhedge1 ().myvertex(1)->accessPllX ().estimateLinkage () ;
  set_intersection (l0.begin (), l0.end (), l1.begin (), l1.end (), back_inserter (est), less < int > ()) ;
  return est ;
}

template < class A >
LinkedObject :: Identifier EdgePllBaseXMacro< A > :: getIdentifier () const {
  return LinkedObject :: Identifier (myhedge1 ().myvertex (0)->ident (), myhedge1 ().myvertex (1)->ident ()) ;
}

template < class A >
void EdgePllBaseXMacro< A > :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  myhedge1 ().myvertex (0)->accessPllX ().unattach2 (i) ;
  myhedge1 ().myvertex (1)->accessPllX ().unattach2 (i) ;
  return ;
}

template < class A >
void EdgePllBaseXMacro< A > :: attach2 (int i) 
{
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

template < class A >
bool EdgePllBaseXMacro< A > :: packAll (vector < ObjectStream > & osv) 
{
  bool action (false) ;
  typedef map < int, int, less < int > > :: const_iterator const_iterator;
  const const_iterator iEnd =  _moveTo.end () ;
  for (const_iterator i = _moveTo.begin () ; i != iEnd ; ++i) 
  {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;

    {
      ObjectStream & os = osv[j];
      os.writeObject (EDGE1) ;
      os.writeObject (myhedge1 ().myvertex (0)->ident ()) ;
      os.writeObject (myhedge1 ().myvertex (1)->ident ()) ;
      
      // make sure ENDOFSTREAM is not a valid refinement rule 
      assert( ! myhedge1_t :: myrule_t :: isValid (ObjectStream :: ENDOFSTREAM) ) ;

      // pack refinement information 
      myhedge1 ().backup ( os ) ;
      os.put( ObjectStream :: ENDOFSTREAM );
      
      inlineData ( os ) ;
    }
    action = true ;
  }
  return action ;
}

template < class A >
void EdgePllBaseXMacro< A > :: unpackSelf (ObjectStream & os, bool i) 
{
  if (i) 
  {
    myhedge1 ().restore ( os ) ;
    
    // stream should be at position ENDOFSTREAM now
    char c = os.get(); 
    if( c != ObjectStream :: ENDOFSTREAM )
    {
      cerr << "**FEHLER (FATAL) c != ENDOFSTREAM ! in " << __FILE__ << " " << __LINE__ << endl;
      abort();
    }
    
    // remove data if have any 
    xtractData (os) ;
  }
  else 
  {
    try 
    {
      char c = os.get() ; 
      // read stream until ENDOFSTREAM 
      while ( c != ObjectStream :: ENDOFSTREAM ) 
      {
        os.read(c) ;
      }
    } 
    catch (ObjectStream :: EOFException) 
    {
      cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
  }
  return ;
}

// Template Instantiation 
template class EdgePllBaseXMacro< GitterBasisPll :: ObjectsPll :: hedge1_IMPL > ;

// #######   
// #         #####   #####  ###### 
// #        #     # #       # 
// #####    ####### #       ######
// #        #     # #       #  
// #        #     # #       #    
// #        #     #  #####  ######

// constructor for hface3 
template <> FacePllBaseXMacro<GitterBasisPll :: ObjectsPll :: hface3_IMPL> :: 
FacePllBaseXMacro(int l, myhedge1_t * e0, int s0, myhedge1_t * e1, int s1,
                  myhedge1_t * e2, int s2)
 : GitterBasisPll :: ObjectsPll :: hface3_IMPL(l, e0, s0, e1, s1, e2, s2), _moveTo(), _ref() 
{
}

// constructor for hface4
template <> FacePllBaseXMacro<GitterBasisPll :: ObjectsPll :: hface4_IMPL> :: 
FacePllBaseXMacro(int l, myhedge1_t * e0, int s0, myhedge1_t * e1, int s1,
                  myhedge1_t * e2, int s2, myhedge1_t * e3, int s3)
 : GitterBasisPll :: ObjectsPll :: hface4_IMPL(l, e0, s0, e1, s1, e2, s2, e3, s3), _moveTo(), _ref() 
{
}

// destructor 
template < class A > FacePllBaseXMacro < A > :: ~FacePllBaseXMacro() 
{
  assert (0 == _moveTo.size ()); 
}

template < class A > vector < int > FacePllBaseXMacro < A > :: estimateLinkage () const {

  // Diese Methode sch"atzt den Verbindungsstern der Grobgitterfl"ache,
  // indem sie die Schnittmenge der Verbindungssterne der anliegenden
  // Grobgitterknoten bildet. Je besser die Sch"atzung, desto schneller
  // arbeitet das Identifikationsmodul. Falls keine Sch"atzung m"oglich
  // ist, kann man auch einfach aller log. Teilgiternummern in einem 
  // Vektor zur"uckgeben. Dann geht die Identifikation eben langsam.

  vector < int > t1, t2, est ;
  vector < int > l0 = this->myhface ().myhedge1 (0)->estimateLinkage () ;
  vector < int > l1 = this->myhface ().myhedge1 (1)->estimateLinkage () ;
  vector < int > l2 = this->myhface ().myhedge1 (2)->estimateLinkage () ;
  vector < int > l3 = this->myhface ().myhedge1 (A :: polygonlength == 3 ? 2 : 3)->estimateLinkage () ;
  set_intersection (l0.begin (), l0.end (), l1.begin (), l1.end (), back_inserter (t1), less < int > ()) ;
  set_intersection (l2.begin (), l2.end (), l3.begin (), l3.end (), back_inserter (t2), less < int > ()) ;
  set_intersection (t1.begin (), t1.end (), t2.begin (), t2.end (), back_inserter (est), less < int > ()) ;
  return est ;
}

template < class A > LinkedObject :: Identifier FacePllBaseXMacro < A > :: getIdentifier () const {

  // Diese Methode erzeugt den Schl"ussel einer Grobgitterfl"ache im Auftrag
  // des Identifikationsmoduls 'gitter_pll_idn.cc'. Der Schl"ussel besteht
  // aus drei Integer Werten 'key.h', die aus der globelen Ausrichtung der
  // Fl"ache folgen und auch f"ur Fl"achen mit mehr als drei Knoten korrekt
  // (d.h. gleiche Fl"ache in versch. Teilgittern -> gleicher Schl"ussel) sind.

  return LinkedObject :: Identifier (this->myhface ().myvertex (0)->ident (), 
      this->myhface ().myvertex (1)->ident (), this->myhface ().myvertex (2)->ident ()) ;
}

template < class A > bool FacePllBaseXMacro < A > :: ldbUpdateGraphEdge (LoadBalancer :: DataBase & db) {
  
  // Diese Methode erzeugt eine Kante im Graphen f"ur die Berechnung
  // der Neupartitionierung, der sie das Gewicht der Anzahl aller feinsten
  // Fl"achen "uber der verwalteten Grobgitterfl"ache gibt.
  typedef typename myhface_t :: myconnect_t myconnect_t;
  
  const myconnect_t * mycon1 = this->myhface().nb.front().first;
  const myconnect_t * mycon2 = this->myhface().nb.rear ().first;

  if(mycon1 && mycon2)
  {
    // if one of them is periodic, increase factor
    // this should reduce cutting of edges between 
    // periodic and normal elements 
    
    int ldbVx1 = -1 ;
    int ldbVx2 = -1 ; 

    if( mycon1->isperiodic() ) 
    {
      assert( ! mycon2->isperiodic() ); 
      ldbVx1 = mycon1->otherLdbVertexIndex( this->myhface() );
      ldbVx2 = mycon2->accessPllX ().ldbVertexIndex ();
    }

    if( mycon2->isperiodic() ) 
    {
      assert( ! mycon1->isperiodic() ); 
      ldbVx1 = mycon1->accessPllX ().ldbVertexIndex ();
      ldbVx2 = mycon2->otherLdbVertexIndex( myhface() );
    }
    
    // count leaf faces for this macro face 
    const int weight =  TreeIterator < typename Gitter :: hface_STI, 
                                       is_leaf < Gitter :: hface_STI > > (this->myhface ()).size ();

    // if we have a periodic situation 
    if( ldbVx1 < ldbVx2 ) 
    {
      assert( mycon1->isperiodic() || mycon2->isperiodic() );
      assert( ldbVx1 >= 0 && ldbVx2 >= 0 );
      // increase the edge weight for periodic connections 
      db.edgeUpdate ( LoadBalancer :: GraphEdge ( ldbVx1, ldbVx2, weight*4 ) );
    }
    else if( ldbVx1 == ldbVx2 ) 
    {
      // the default graph edge 
      db.edgeUpdate ( LoadBalancer :: GraphEdge 
        ( mycon1->accessPllX ().ldbVertexIndex (),
          mycon2->accessPllX ().ldbVertexIndex (),
          weight ) );
    
    }
  }
  return true ;
}

template < class A > void FacePllBaseXMacro < A > :: unattach2 (int i) {

  // Diese Methode bindet die Fl"ache von einer Zuweisung zu einem neuen
  // Teilgitter ab. D.h. der Eintrag in der Zuweisungsliste wird gel"oscht,
  // und dann wird die M"oglichkeit an die anliegenden Kanten weitervermittelt.

  //assert (_moveTo.find (i) != _moveTo.end ()) ;
  if( _moveTo.find (i) ==  _moveTo.end () ) 
    return ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  {for (int j = 0 ; j < A :: polygonlength ; ++j ) 
    this->myhface ().myhedge1 (j)->unattach2 (i) ;}
  return ;
}

template < class A > void FacePllBaseXMacro < A > :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) 
  {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } 
  else 
  {
    (*pos).second ++ ;
  }

  {
    for (int j = 0 ; j < A :: polygonlength ; ++j ) 
      this->myhface ().myhedge1 (j)->attach2 (i) ;
  }
  return ;  
}

template < class A > bool FacePllBaseXMacro < A > :: packAll (vector < ObjectStream > & osv) {

  // Die Methode packAll () verpackt die Fl"ache auf alle Datenstr"ome,
  // die zu Teilgittern f"uhren, an die sie zugewiesen wurde mit attach2 ().
  // Ausserdem geht die Methode noch an die anliegenden Elemente (Randelemente)
  // "uber.

  bool action = false ;
  typedef map < int, int, less < int > > :: const_iterator const_iterator;
  const const_iterator iEnd =  _moveTo.end () ;
  for (const_iterator i = _moveTo.begin () ; i != iEnd ; ++i) 
  {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    
    ObjectStream& os = osv[j];
    if (A :: polygonlength == 4) 
    {
      os.writeObject (MacroGridMoverIF :: FACE4) ;
    }
    else if (A :: polygonlength == 3) 
    {
      os.writeObject (MacroGridMoverIF :: FACE3) ;
    }
    else 
    {
      // something wrong 
      assert(false);
      abort () ;
    }
    
    {
      // write vertex idents 
      for (int k = 0 ; k < A :: polygonlength ; ++ k)
      {
        os.writeObject (this->myhface ().myvertex (k)->ident ()) ;
      }
    }
    try {
    
      // Sicherheitshalber testen, ob das ENDOFSTREAM Tag nicht auch
      // mit einer Verfeinerungsregel identisch ist - sonst gibt's
      // nachher beim Auspacken nur garbage.
    
      assert (! myhface_t :: myrule_t :: isValid (ObjectStream :: ENDOFSTREAM) ) ;
    
      this->myhface ().backup ( os );
      os.put( ObjectStream :: ENDOFSTREAM );

      // inline internal data if has any 
      inlineData ( os ) ;

    } catch (ObjectStream :: OutOfMemoryException) {
      cerr << "**FEHLER (FATAL) ObjectStream :: OutOfMemoryException aufgetreten in " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    try {
    
      // Wenn die Fl"ache auf den j. Strom des Lastverschiebers
      // geschrieben wurde, dann mu"ussen auch die anliegenden
      // Elemente daraufhin untersucht werden, ob sie sich nicht
      // als Randelemente dorthin schreiben sollen - das tun sie
      // aber selbst.
    
      this->myhface ().nb.front ().first->accessPllX ().packAsBnd (this->myhface ().nb.front ().second, j, os ) ;
      this->myhface ().nb.rear  ().first->accessPllX ().packAsBnd (this->myhface ().nb.rear  ().second, j, os ) ;
    } 
    catch (Parallel :: AccessPllException) 
    {
      cerr << "**FEHLER (FATAL) AccessPllException aufgetreten in " << __FILE__ << " " << __LINE__ << ". Ende." << endl ;
      abort () ;
    }
    action = true ;
  }
  return action ;
}

//- --unpackSelf
template < class A > void FacePllBaseXMacro < A > :: unpackSelf (ObjectStream & os, bool i) 
{

  // Die Methode wird eine Fl"ache aus dem Datenstrom rekonstruieren,
  // die der MacrogridBuilder angelegt hat, oder sie wird nur die
  // Verfeinerungsregeln aus dem Strom nehmen, falls schon vorher
  // eine identische Fl"ache bestand. Dies wird vom MacrogridBuilder
  // durch die 'bool' Variable mitgeteilt. Trotzdem k"onnen auch auf
  // einen bestehenden Fl"achenbaum durch die Lastverschiebung neue
  // Daten aufgebracht werden - dies ist dann hier zu realisieren.

  if (i) 
  {
    // Sobald der Stringstream mit den 'byte' Verfeinerungsregeln
    // voll ist, kann mit dem normalen restore der ganze Fl"achen-
    // baum wieder hochgezogen werden. Analog zur Wiederherstellung
    // aus einer Datei.
    
    this->myhface ().restore ( os ) ;

    char c = os.get();
    if( c != ObjectStream :: ENDOFSTREAM )
    {
      cerr << "**FEHLER (FATAL) c != ENDOFSTREAM ! in " << __FILE__ << " " << __LINE__ << endl;
      abort();
    }

    // restore internal data if have any 
    xtractData (os) ;
  }
  else 
  {
    try 
    {
      char c = os.get() ; 
      // read stream until ENDOFSTREAM 
      while ( c != ObjectStream :: ENDOFSTREAM ) 
      {
        os.read(c) ;
      }
    } 
    catch (ObjectStream :: EOFException) 
    {
      cerr << "**FEHLER EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
  }

  return ;
}

// Template Instantiation 
template class FacePllBaseXMacro < GitterBasisPll :: ObjectsPll :: hface3_IMPL > ;
template class FacePllBaseXMacro < GitterBasisPll :: ObjectsPll :: hface4_IMPL > ;

//////////////////////////////////////////////////////////////////////
//
//  --BndsegPllBaseXMacroClosure
//
//////////////////////////////////////////////////////////////////////
template < class A > void BndsegPllBaseXClosure < A > :: getRefinementRequest (ObjectStream & os) {
  os.writeObject (int (_rul)) ;
  _rul = balrule_t :: nosplit ;
  return ;
}

template < class A > bool BndsegPllBaseXClosure < A > :: setRefinementRequest (ObjectStream & os) {

  // Die Methode schickt die Regel aus dem Nachbargitter in das
  // eigene Teilgitter hinein. Sie liefert "true" immer dann, wenn
  // sich das eigene Gitter g"andert hat -> f"uhrt zu einer weiteren
  // Iteration des parallelen refine ().

  int i ;
  try {
    os.readObject (i) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) BndsegPllBaseXClosure :: setRefinementRequest (..)\n" ;
    cerr << "  EOF gelesen. In " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  balrule_t r (i)  ;
  if (r == balrule_t :: nosplit) {
    return false ;
  } else {
    if (myhbnd ().getrule () == r) {
      return false ;
    } else {
      if (myhbnd ().refineLikeElement (r)) {
        // Verfeinerung erfolgreich
        return true ;
      } else {
      
        // Verfeinerung verhindert irgendwo im Gitter. Dies ist ein Vorbehalt
        // f"ur den parallelen anisotropen Verfeinerungsalgorithmus. Daher
        // sollte die Situation im isotropen Fall nicht auftreten.
      
        cerr << "**FEHLER (FATAL, weil nicht vorgesehen): Verfeinerung wurde\n" ;
        cerr << "  verhindert am inneren Rand. In " << __FILE__ << " " << __LINE__ << endl ;
        abort () ;
      }
    }
  }
  return (abort (), false) ;
}

template < class A > void BndsegPllBaseXClosure < A > :: writeDynamicState (ObjectStream & os, GatherScatterType & gs ) const 
{
  gs.sendData( os , myhbnd () );
  return ;
}

template < class A > void BndsegPllBaseXClosure < A > :: readDynamicState (ObjectStream & os, GatherScatterType & gs ) 
{
  gs.recvData( os , myhbnd () );
  return ;
}

template < class A > void BndsegPllBaseXClosure < A > :: 
readDynamicState (ObjectStream & os, int) 
{
  try 
  {

    // read the real level of ghost 
    assert(myhbnd().leafRefCount()==0 || myhbnd().leafRefCount()==1);
    const bool wasLeaf = this->ghostLeaf() ;
   
    // read level and leaf of interior element on other side
    // this changes the state of this leaf or not leaf
    os.readObject( _ghostLevel );
    os.readObject( _ghostLeaf );

    const bool nowLeaf = this->ghostLeaf() ;

    // if leaf state has changed the attach or detach 
    if (! wasLeaf && nowLeaf) 
    {
      myhbnd().attachleafs();
    }
    else if (wasLeaf && !nowLeaf)
    {
      myhbnd().detachleafs();
    }
    
    assert( myhbnd().leafRefCount()==0 || myhbnd().leafRefCount()==1 );
    assert( (!nowLeaf) ? (! myhbnd().isLeafEntity()) : 1);
    assert( ( nowLeaf) ? (  myhbnd().isLeafEntity()) : 1);
    
  } 
  catch (ObjectStream :: EOFException) 
  {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }

  return ;
}

template < class A > void BndsegPllBaseXMacroClosure < A > :: readStaticState (ObjectStream & os, int) {
  try {
    os.readObject (_extGraphVertexIndex) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  //assert (_extGraphVertexIndex >= 0) ;
  return ;
}

template < class A > void BndsegPllBaseXMacroClosure < A > :: 
packAsBnd (int fce, int who, ObjectStream & os) const 
{
  assert (!fce) ; // fce should be 0, because we only have 1 face 
  assert (this->myhbnd ().bndtype () == Gitter :: hbndseg :: closure) ;

  if (myhface_t :: polygonlength == 3) os.writeObject (MacroGridMoverIF :: HBND3INT) ;
  else if (myhface_t :: polygonlength == 4) os.writeObject (MacroGridMoverIF :: HBND4INT) ;
  else 
  {
    cerr << "BndsegPllBaseXMacroClosure :: packAsBnd: Wrong face type!in: "<<__FILE__ << " line: " <<__LINE__ << endl; 
    abort () ;
  }
  os.writeObject (this->myhbnd ().bndtype ()) ;
  
  {
    for (int i = 0 ; i < myhface_t :: polygonlength ; ++i) 
      os.writeObject (this->myhbnd ().myvertex (fce,i)->ident ()) ; 
  }

  if(_ghInfo) // is stored ghost point exists
  {
    os.writeObject ( MacroGridMoverIF :: POINTTRANSMITTED ); 
    // see ghost_info.h for implementation of this functions 
    _ghInfo->inlineGhostElement(os);
  }
  else 
  {
    os.writeObject ( MacroGridMoverIF :: NO_POINT ); // no point transmitted 
  }
   
  return ;
}

template < class A > inline void BndsegPllBaseXMacroClosure < A > :: 
insertGhostCell(ObjectStream & os, int fce)
{
  assert( _ghInfo == 0 );
  _ghInfo = this->myhbnd().buildGhostCell(os , fce);
  assert( _ghInfo );
}

// template Instantiation 
template class BndsegPllBaseXMacroClosure< GitterBasis :: Objects :: Hbnd4Default >;
template class BndsegPllBaseXMacroClosure< GitterBasis :: Objects :: Hbnd3Default >;
  
// #######
//    #     ######   #####  #####     ##
//    #     #          #    #    #   #  #
//    #     #####      #    #    #  #    #
//    #     #          #    #####   ######
//    #     #          #    #   #   #    #
//    #     ######     #    #    #  #    #
template < class A >
void TetraPllXBase< A > :: writeDynamicState (ObjectStream & os, GatherScatterType & gs) const 
{
  gs.sendData( os , mytetra () );
  return ;
}

template < class A >
void TetraPllXBase< A > :: writeDynamicState (ObjectStream & os, int face) const 
{
  // write level to know the level of ghost on the other side
  os.writeObject( mytetra().level() );
  os.writeObject( mytetra().leaf()  );
  return ;
}

template < class A >
TetraPllXBaseMacro< A > :: 
TetraPllXBaseMacro (int l, myhface3_t *f0, int s0, myhface3_t *f1, int s1,
                           myhface3_t *f2, int s2, myhface3_t *f3, int s3) 
  : A(l, f0, s0, f1, s1, f2, s2, f3, s3 )
  , _moveTo ( -1 )
  , _ldbVertexIndex (-1)
  , _erasable (false) 
{
#ifdef GRAPHVERTEX_WITH_CENTER
  LinearMapping :: barycenter(
      mytetra ().myvertex (0)->Point (), 
      mytetra ().myvertex (1)->Point (),
      mytetra ().myvertex (2)->Point (),
      mytetra ().myvertex (3)->Point (),
      _center);
#endif
  return ;
}

template < class A >
TetraPllXBaseMacro< A > :: ~TetraPllXBaseMacro () 
{
  if( _moveTo >= 0 ) 
  {
    unattach2 ( _moveTo ) ;
  }
}

template < class A >
int TetraPllXBaseMacro< A > :: ldbVertexIndex () const {
  return _ldbVertexIndex ;
}

template < class A >
void TetraPllXBaseMacro< A > :: setLoadBalanceVertexIndex ( const int ldbVx ) {
  _ldbVertexIndex = ldbVx ;
}

template < class A >
bool TetraPllXBaseMacro< A > :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) 
{
  // parameter are: 
  // - macro vertex index
  // - number of elementes below macro element 
  // - bary center 
  const int factor = this->numberOfPeriodicBoundaries( mytetra() ) + 1 ;
   
  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (mytetra ()).size () * factor 
#ifdef GRAPHVERTEX_WITH_CENTER
      , _center
#endif
      ) ) ;
  return true ;
}

template < class A >
void TetraPllXBaseMacro< A > :: writeStaticState (ObjectStream & os, int) const {
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

template < class A >
void TetraPllXBaseMacro< A > :: unattach2 (int i) 
{
  assert( i >= 0 );
  mytetra ().myhface3 (0)->unattach2 (i) ;
  mytetra ().myhface3 (1)->unattach2 (i) ;
  mytetra ().myhface3 (2)->unattach2 (i) ;
  mytetra ().myhface3 (3)->unattach2 (i) ;

  // reset move to 
  _moveTo = -1;

  return ;
}

template < class A >
void TetraPllXBaseMacro< A > :: attachElement2 (const int destination, const int face) 
{
  attach2( destination );
  // check all neighbours 
  // face is the face this method was called from 
  for( int f=0; f<4; ++f )
  {
    if( face == f ) continue ;

    myneighbour_t nb = myneighbour( f );
    if( nb.first->isperiodic() )
    {
      nb.first->attachPeriodic( destination );
    }
  }
}

template < class A >
void TetraPllXBaseMacro< A > :: attach2 (int i) 
{
  // don't attach elements twice 
  if( _moveTo == -1 ) 
  {
    // set my destination 
    _moveTo = i ;
    
    // also move all faces to the same process 
    mytetra ().myhface3 (0)->attach2 (i) ;
    mytetra ().myhface3 (1)->attach2 (i) ;
    mytetra ().myhface3 (2)->attach2 (i) ;
    mytetra ().myhface3 (3)->attach2 (i) ;
  }
  return ;
}

template < class A >
bool TetraPllXBaseMacro< A > :: packAll (vector < ObjectStream > & osv) 
{
  return doPackAll( osv, (GatherScatterType * ) 0 );
}

template < class A >
bool TetraPllXBaseMacro< A > :: doPackAll (vector < ObjectStream > & osv,
                                           GatherScatterType* gs) 
{
  if( _moveTo >= 0 ) 
  {
    assert ((osv.begin () + _moveTo) < osv.end ()) ;
      ObjectStream& os = osv[ _moveTo ];

    os.writeObject (TETRA) ;
    os.writeObject (mytetra ().myvertex (0)->ident ()) ;
    os.writeObject (mytetra ().myvertex (1)->ident ()) ;
    os.writeObject (mytetra ().myvertex (2)->ident ()) ;
    os.writeObject (mytetra ().myvertex (3)->ident ()) ;

    // make sure ENDOFSTREAM is not a valid refinement rule 
    assert( ! mytetra_t :: myrule_t :: isValid (ObjectStream :: ENDOFSTREAM) ) ;
    
    // pack refinement information 
    mytetra ().backup ( os );
    os.put( ObjectStream :: ENDOFSTREAM );

    // pack internal data if has any 
    inlineData ( os ) ;
    
    // if gather scatter was passed 
    if( gs ) 
    {
      // pack Dune data 
      gs->inlineData( os , mytetra() );
    }

    _erasable = true ;
    return true ;
  }
  return false ;
}

template < class A >
bool TetraPllXBaseMacro< A > :: dunePackAll (vector < ObjectStream > & osv,
                                             GatherScatterType & gs) 
{
  return doPackAll( osv, &gs );
}

template < class A >
void TetraPllXBaseMacro< A > :: packAsBndNow (int fce, ObjectStream & os) const 
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
    const alucoord_t (&p)[3] = vertex->Point();
    os.writeObject ( p[0] ) ;
    os.writeObject ( p[1] ) ;
    os.writeObject ( p[2] ) ;
  }
}

// packs macro element as internal bnd for other proc 
template < class A >
void TetraPllXBaseMacro< A > :: packAsBnd (int fce, int who, ObjectStream & os) const 
{
  if( _moveTo != who ) 
  {
    // write data to stream 
    packAsBndNow(fce,os); 
  }
  return ;
}

// packs macro element as internal bnd for other proc 
template < class A >
void TetraPllXBaseMacro< A > :: packAsGhost(ObjectStream & os, int fce) const 
{
  packAsBndNow(fce,os);
}

template < class A >
void TetraPllXBaseMacro< A > :: 
unpackSelf (ObjectStream & os, bool i )
{
  duneUnpackSelf( os, i , ( GatherScatterType * ) 0 );
}

template < class A >
void TetraPllXBaseMacro< A > :: 
duneUnpackSelf (ObjectStream & os, const bool i, GatherScatterType* gatherScatter )
{
  doUnpackSelf( os, i, gatherScatter );
}

template < class A >
void TetraPllXBaseMacro< A > :: 
doUnpackSelf (ObjectStream & os, const bool i, GatherScatterType* gatherScatter )
{
  assert (i) ;
  if (i) 
  {
    // restore refinement information 
    mytetra ().restore (os) ;

    char c = os.get(); 
    if( c != ObjectStream :: ENDOFSTREAM )
    {
      cerr << "**FEHLER (FATAL) c != ENDOFSTREAM ! in " << __FILE__ << " " << __LINE__ << endl;
      abort();
    }
    
    // restore internal data if have any 
    xtractData (os) ;
    
    // unpack dune data if present, pointer can be zero 
    if( gatherScatter ) 
    {
      // unpack Dune data 
      gatherScatter->xtractData( os , mytetra() );
    }
  }
  else 
  {
    cerr << "**FEHLER (FATAL) i=false, should be true! in " << __FILE__ << " " << __LINE__ << endl;
    abort();
  }
  return ;
}

template < class A >
bool TetraPllXBaseMacro< A > :: erasable () const {
  return _erasable ;
}

// template instantiation 
template class TetraPllXBase< GitterBasisPll :: ObjectsPll :: TetraEmpty > ;
template class TetraPllXBaseMacro< GitterBasisPll :: ObjectsPll :: tetra_IMPL > ;

// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

template < class A > 
void Periodic3PllXBase< A > :: writeDynamicState (ObjectStream & os, int) const {

  // Der Schwerpunkt des "flachen" periodischen Randelements wird
  // auf die Mitte der linken Fl"ache gelegt. Per Definition.

  static const double x = 1./3. ;
  alucoord_t p [3] ;
  LinearSurfaceMapping (myperiodic ().myvertex (0,0)->Point (), myperiodic ().myvertex (0,1)->Point (),
    myperiodic ().myvertex (0,2)->Point ()).map2world (x,x,x,p) ;
  os.writeObject (p [0]) ;
  os.writeObject (p [1]) ;
  os.writeObject (p [2]) ;
  return ;
}

template < class A >
Periodic3PllXBaseMacro< A > :: 
Periodic3PllXBaseMacro ( int level, myhface3_t* f0,int s0, myhface3_t *f1,int s1 )
: A(level, f0, s0, f1, s1 )
, _moveTo ()
, _ldbVertexIndex ( -1 )
, _erasable (false) 
{
#ifdef GRAPHVERTEX_WITH_CENTER
  static const double x = 1./3. ;
  LinearSurfaceMapping (myperiodic ().myvertex (0,0)->Point (), myperiodic ().myvertex (0,1)->Point (),
         myperiodic ().myvertex (0,2)->Point ()).map2world (x,x,x,_center) ;
#endif
  return ;
}

template < class A > 
Periodic3PllXBaseMacro< A > :: ~Periodic3PllXBaseMacro () 
{
  if( _moveTo >= 0 ) 
  {
    unattach2 ( _moveTo );
  }
}

template <class A>
bool Periodic3PllXBaseMacro< A > :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) 
{
  /*
  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (myperiodic ()).size ()
#ifdef GRAPHVERTEX_WITH_CENTER
      , _center
#endif
      )) ;
  */

  return true ;
}

template < class A >
void Periodic3PllXBaseMacro< A > :: writeStaticState (ObjectStream & os, int) const {
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

template < class A >
void Periodic3PllXBaseMacro< A > :: unattach2 (int i) 
{
  assert ( i>= 0 );
  myperiodic ().myhface3 (0)->unattach2 (i) ;
  myperiodic ().myhface3 (1)->unattach2 (i) ;
  _moveTo = -1;
  return ;
}

// return the first element's ldbVertexIndex (used in Periodic3PllXBaseMacro)
template < class A >
pair<int,int> Periodic3PllXBaseMacro< A > :: insideLdbVertexIndex() const
{
  return pair<int,int>  ( myneighbour( 0 ).first->firstLdbVertexIndex(),
                          myneighbour( 1 ).first->firstLdbVertexIndex() );
}

template < class A >
int Periodic3PllXBaseMacro< A > :: otherLdbVertexIndex( const hface_STI& face ) const
{
  if( myhface3( 0 ) == &face )
    return myneighbour( 1 ).first->firstLdbVertexIndex() ;
  else
    return myneighbour( 0 ).first->firstLdbVertexIndex() ;
}

template < class A >
void Periodic3PllXBaseMacro< A > :: attachPeriodic(const int destination)
{
  attach2( destination );
}

template < class A >
void Periodic3PllXBaseMacro< A > :: attach2 (int i) 
{
  if( _moveTo == -1 ) 
  {
    // store new destination 
    _moveTo = i; 

    // attach other elements 
    myperiodic ().myhface3 (0)->attach2 (i) ;
    myperiodic ().myhface3 (1)->attach2 (i) ;
  }

  return ;
}

template < class A >
bool Periodic3PllXBaseMacro< A > :: packAll (vector < ObjectStream > & osv) 
{
  if( _moveTo >= 0 ) 
  {
    assert ((osv.begin () + _moveTo) < osv.end ()) ;
    ObjectStream& os = osv[ _moveTo ];

    os.writeObject (PERIODIC3) ;
    os.writeObject (myperiodic ().myvertex (0)->ident ()) ;
    os.writeObject (myperiodic ().myvertex (1)->ident ()) ;
    os.writeObject (myperiodic ().myvertex (2)->ident ()) ;
    os.writeObject (myperiodic ().myvertex (3)->ident ()) ;
    os.writeObject (myperiodic ().myvertex (4)->ident ()) ;
    os.writeObject (myperiodic ().myvertex (5)->ident ()) ;
    
    // make sure ENDOFSTREAM is not a valid refinement rule 
    assert( ! myperiodic_t :: myrule_t :: isValid (ObjectStream :: ENDOFSTREAM) ) ;
    
    // pack refinement information 
    myperiodic ().backup ( os ) ;
    os.put( ObjectStream :: ENDOFSTREAM );

    // pack internal data if has any 
    inlineData ( os ) ;

    _erasable = true ;
    return true ;
  }
  return false ;
}

template < class A >
void Periodic3PllXBaseMacro< A > :: packAsBnd (int fce, int who, ObjectStream & os) const 
{
  // we require that periodic element are never packed as boundary 
  // since they are on the same process as their faces 
  assert( _moveTo == who );
}

template < class A >
void Periodic3PllXBaseMacro< A > :: unpackSelf (ObjectStream & os, bool i) 
{
  assert (i) ;

  if (i) 
  {
    myperiodic ().restore ( os ) ;
    
    char c = os.get();
    if( c != ObjectStream :: ENDOFSTREAM )
    {
      cerr << "**FEHLER (FATAL) c != ENDOFSTREAM ! in " << __FILE__ << " " << __LINE__ << endl;
      abort();
    }
    
    // unpack internal data if has any 
    xtractData (os) ;
  }
  else 
  {
    cerr << "**FEHLER (FATAL) i=false, should be true! in " << __FILE__ << " " << __LINE__ << endl;
    abort();
  }
  return ;
}


// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

template < class A > 
void Periodic4PllXBase< A > :: writeDynamicState (ObjectStream & os, int) const {

  // Der Schwerpunkt des "flachen" periodischen Randelements wird
  // auf die Mitte der linken Fl"ache gelegt. Per Definition.

  static const double x = .0 ;
  alucoord_t p [3] ;
  BilinearSurfaceMapping (myperiodic ().myvertex (0,0)->Point (), myperiodic ().myvertex (0,1)->Point (),
    myperiodic ().myvertex (0,2)->Point (),myperiodic ().myvertex (0,3)->Point ()).map2world (x,x,p) ;
  os.writeObject (p [0]) ;
  os.writeObject (p [1]) ;
  os.writeObject (p [2]) ;
  return ;
}

template < class A > 
Periodic4PllXBaseMacro< A > :: 
Periodic4PllXBaseMacro ( int level, myhface4_t* f0,int s0, myhface4_t *f1,int s1) 
  : A(level, f0, s0, f1, s1 )
  , _moveTo ()
  , _ldbVertexIndex (-1)
  , _erasable (false) 
{
#ifdef GRAPHVERTEX_WITH_CENTER
  static const double x = .0 ;
  BilinearSurfaceMapping (myperiodic ().myvertex (0,0)->Point (), myperiodic ().myvertex (0,1)->Point (),
         myperiodic ().myvertex (0,2)->Point (), myperiodic ().myvertex (0,3)->Point ()).map2world (x,x,_center) ;
#endif
  return ;
}

template < class A > 
Periodic4PllXBaseMacro< A > :: ~Periodic4PllXBaseMacro () {
  vector < int > v ;
  {
    v.reserve( _moveTo.size() ); 
    typedef map < int, int, less < int > > :: const_iterator const_iterator;
    const const_iterator iEnd =  _moveTo.end () ;
    for (const_iterator i = _moveTo.begin () ; i != iEnd ; v.push_back ((*i++).first)) ;
  }
  {
    const vector < int > :: const_iterator iEnd = v.end () ;
    for (vector < int > :: const_iterator i = v.begin () ; i != iEnd ; unattach2 (*i++)) ;}
  return ;
}

template < class A > 
int Periodic4PllXBaseMacro< A > :: ldbVertexIndex () const {
  return _ldbVertexIndex ;
}

template < class A > 
void Periodic4PllXBaseMacro< A > :: setLoadBalanceVertexIndex ( const int ldbVx ) {
  _ldbVertexIndex = ldbVx ;
}

template < class A > 
bool Periodic4PllXBaseMacro< A > :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) {
  /*
  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (myperiodic ()).size ()
#ifdef GRAPHVERTEX_WITH_CENTER
      , _center
#endif
      )) ;
      */
  return true ;
}

template < class A > 
void Periodic4PllXBaseMacro< A > :: writeStaticState (ObjectStream & os, int) const {
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

template < class A > 
void Periodic4PllXBaseMacro< A > :: unattach2 (int i) {
  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  myperiodic ().myhface4 (0)->unattach2 (i) ;
  myperiodic ().myhface4 (1)->unattach2 (i) ;
  return ;
}

// return the first element's ldbVertexIndex (used in Periodic3PllXBaseMacro)
template < class A >
pair<int,int> Periodic4PllXBaseMacro< A > :: insideLdbVertexIndex() const
{
  return pair<int,int>  ( myneighbour( 0 ).first->firstLdbVertexIndex(),
                          myneighbour( 1 ).first->firstLdbVertexIndex() );
}

template < class A >
int Periodic4PllXBaseMacro< A > :: otherLdbVertexIndex( const hface_STI& face ) const
{
  if( myhface4( 0 ) == &face )
    return myneighbour( 1 ).first->firstLdbVertexIndex() ;
  else
    return myneighbour( 0 ).first->firstLdbVertexIndex() ;
}

template < class A >
void Periodic4PllXBaseMacro< A > :: attachPeriodic(const int destination)
{
  attach2( destination );
}

template < class A > 
void Periodic4PllXBaseMacro< A > :: attach2 (int i) 
{
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) 
  {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } 
  else 
  {
    // should only be attached once 
    if ((*pos).first == i) 
    {
      cerr << "  Periodic4PllXBaseMacro :: attach2 () WARNUNG versuchte mehrfache Zuweisung ignoriert " << endl ;
      return ;
    }
  }

  // attach both neighbours to the same process 
  for(int n=0; n<2; ++n ) 
  {
    typename A :: myneighbour_t nb = this->myneighbour( n );
    nb.first->attachElement2( i, nb.second );
  }

  /*
    hexa_IMPL* hexa = ((hexa_IMPL *) nb.first);
    hexa->attach2( i );
    for( int j=0; j<6; ++j ) 
    {
      // skip the face connected to this periodic bnd 
      if( j == nb.second ) continue ;

      // check neighboring element 
      myneighbour_t faceNb = hexa->myneighbour( j );
      if( nb.first->isperiodic() )
      {
        
      }
    }
  }
  */

  //attachElementNeighbours();

  //myperiodic ().myhface4 (0)->attach2 (i) ;
  //myperiodic ().myhface4 (1)->attach2 (i) ;
  return ;
}

template < class A > 
bool Periodic4PllXBaseMacro< A > :: packAll (vector < ObjectStream > & osv) 
{
  typedef map < int, int, less < int > > :: const_iterator const_iterator;
  const const_iterator iEnd =  _moveTo.end () ;
  for (const_iterator i = _moveTo.begin () ; i != iEnd ; ++i) 
  {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    assert (_moveTo.size () == 1) ;
    {
      ObjectStream& os = osv[j];
      
      os.writeObject (PERIODIC4) ;
      os.writeObject (myperiodic ().myvertex (0)->ident ()) ;
      os.writeObject (myperiodic ().myvertex (1)->ident ()) ;
      os.writeObject (myperiodic ().myvertex (2)->ident ()) ;
      os.writeObject (myperiodic ().myvertex (3)->ident ()) ;
      os.writeObject (myperiodic ().myvertex (4)->ident ()) ;
      os.writeObject (myperiodic ().myvertex (5)->ident ()) ;
      os.writeObject (myperiodic ().myvertex (6)->ident ()) ;
      os.writeObject (myperiodic ().myvertex (7)->ident ()) ;

      // make sure ENDOFSTREAM is not a valid refinement rule 
      assert( ! myperiodic_t :: myrule_t :: isValid (ObjectStream :: ENDOFSTREAM) ) ;
      
      // pack refinement information 
      myperiodic ().backup ( os ) ;
      os.put( ObjectStream :: ENDOFSTREAM );
      
      // pack internal data if has any 
      inlineData ( os ) ;
    }
    _erasable = true ;
    return true ;
  }
  return false ;
}

template < class A > 
void Periodic4PllXBaseMacro< A > :: packAsBnd (int fce, int who, ObjectStream & os) const 
{
  // we require that periodic element are never packed as boundary 
  // since they are on the same process as their faces 
  // assert( _moveTo[ 0 ].first == who );
}

template < class A > 
void Periodic4PllXBaseMacro< A > :: unpackSelf (ObjectStream & os, bool i) 
{
  assert (i) ;
  if (i) 
  {
    myperiodic ().restore (os) ;
    
    char c = os.get();
    if( c != ObjectStream :: ENDOFSTREAM )
    {
      cerr << "**FEHLER (FATAL) c != ENDOFSTREAM ! in " << __FILE__ << " " << __LINE__ << endl;
      abort();
    }
    
    // unpack internal data if has any 
    xtractData (os) ;
  }
  else 
  {
    cerr << "**FEHLER (FATAL) i=false, should be true! in " << __FILE__ << " " << __LINE__ << endl;
    abort();
  }
  return ;
}

template < class A > 
bool Periodic4PllXBaseMacro< A > :: erasable () const {
  return _erasable ;
}

  // #     #
  // #     #  ######  #    #    ##
  // #     #  #        #  #    #  #
  // #######  #####     ##    #    #
  // #     #  #         ##    ######
  // #     #  #        #  #   #    #
  // #     #  ######  #    #  #    #

template < class A >
void HexaPllBaseX< A >  :: writeDynamicState (ObjectStream & os, GatherScatterType & gs) const 
{
  gs.sendData( os , myhexa () );
  return ;
}

template < class A >
void HexaPllBaseX< A > :: writeDynamicState (ObjectStream & os, int face) const 
{
  // siehe writeDynamicState von Tetra 

  // write level to know the level of ghost on the other side
  os.writeObject( myhexa().level() );
  os.writeObject( myhexa().leaf()  );

  return ;
}

template < class A >
HexaPllBaseXMacro< A > :: 
HexaPllBaseXMacro(int l, myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                         myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                         myhface4_t *f4, int s4, myhface4_t *f5, int s5)
: A(l, f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5)
, _moveTo ( -1 )
, _ldbVertexIndex (-1)
, _erasable (false) 
{
#ifdef GRAPHVERTEX_WITH_CENTER
  // calculate bary center 
  TrilinearMapping :: barycenter (
      myhexa ().myvertex (0)->Point (), 
      myhexa ().myvertex (1)->Point (),
      myhexa ().myvertex (2)->Point (), 
      myhexa ().myvertex (3)->Point (), 
      myhexa ().myvertex (4)->Point (),
      myhexa ().myvertex (5)->Point (), 
      myhexa ().myvertex (6)->Point (), 
      myhexa ().myvertex (7)->Point (),
      _center );
#endif
  return ;
}

template < class A >
HexaPllBaseXMacro< A > :: ~HexaPllBaseXMacro () 
{
  if( _moveTo >= 0 ) 
  {
    unattach2( _moveTo );
  }
}

template < class A >
int HexaPllBaseXMacro< A > :: ldbVertexIndex () const {
  return _ldbVertexIndex ;
}

template < class A >
void HexaPllBaseXMacro< A > :: setLoadBalanceVertexIndex ( const int ldbVx ) {
  _ldbVertexIndex = ldbVx  ;
}

template < class A >
bool HexaPllBaseXMacro< A > :: ldbUpdateGraphVertex (LoadBalancer :: DataBase & db) 
{
  // element with periodic boundaries get higher weight 
  const int factor = this->numberOfPeriodicBoundaries( myhexa() ) + 1 ;

  db.vertexUpdate (LoadBalancer :: GraphVertex (ldbVertexIndex (), 
      TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI > > (myhexa ()).size () * factor 
#ifdef GRAPHVERTEX_WITH_CENTER
      , _center
#endif
      )) ;
  return true ;
}

template < class A >
void HexaPllBaseXMacro< A > :: writeStaticState (ObjectStream & os, int) const 
{
  os.writeObject (ldbVertexIndex ()) ;
  return ;
}

template < class A >
void HexaPllBaseXMacro< A > :: unattach2 (int i) 
{
  assert( i >= 0 );
  myhexa ().myhface4 (0)->unattach2 (i) ;
  myhexa ().myhface4 (1)->unattach2 (i) ;
  myhexa ().myhface4 (2)->unattach2 (i) ;
  myhexa ().myhface4 (3)->unattach2 (i) ;
  myhexa ().myhface4 (4)->unattach2 (i) ;
  myhexa ().myhface4 (5)->unattach2 (i) ;

  // reset moveTo 
  _moveTo = -1;
  return ;
}

template < class A >
void HexaPllBaseXMacro< A > :: attachElement2 (const int destination, const int face) 
{
  attach2( destination );
  // check all neighbours 
  // face is the face this method was called from 
  for( int f=0; f<6; ++f )
  {
    if( face == f ) continue ;

    myneighbour_t nb = myneighbour( f );
    if( nb.first->isperiodic() )
    {
      nb.first->attachPeriodic( destination );
    }
  }
}

template < class A >
void HexaPllBaseXMacro< A > :: attach2 (int i) 
{
  // don't attach elements twice 
  if( _moveTo == -1 ) 
  {
    // store new destination 
    _moveTo = i ;

    // also attach all my faces 
    myhexa ().myhface4 (0)->attach2 (i) ;
    myhexa ().myhface4 (1)->attach2 (i) ;
    myhexa ().myhface4 (2)->attach2 (i) ;
    myhexa ().myhface4 (3)->attach2 (i) ;
    myhexa ().myhface4 (4)->attach2 (i) ;
    myhexa ().myhface4 (5)->attach2 (i) ;
  }
  return ;
}
// pack all function for dune 
template < class A >
bool HexaPllBaseXMacro< A > :: doPackAll (vector < ObjectStream > & osv,
                                          GatherScatterType* gs) 
{
  if( _moveTo >= 0 ) 
  {
    assert ((osv.begin () + _moveTo) < osv.end ()) ;
    ObjectStream& os = osv[ _moveTo ];
    
    os.writeObject (HEXA) ;
    os.writeObject (myhexa ().myvertex (0)->ident ()) ;
    os.writeObject (myhexa ().myvertex (1)->ident ()) ;
    os.writeObject (myhexa ().myvertex (2)->ident ()) ;
    os.writeObject (myhexa ().myvertex (3)->ident ()) ;
    os.writeObject (myhexa ().myvertex (4)->ident ()) ;
    os.writeObject (myhexa ().myvertex (5)->ident ()) ;
    os.writeObject (myhexa ().myvertex (6)->ident ()) ;
    os.writeObject (myhexa ().myvertex (7)->ident ()) ;

    // make sure ENDOFSTREAM is not a valid refinement rule 
    assert( ! myhexa_t :: myrule_t :: isValid (ObjectStream :: ENDOFSTREAM) ) ;
    
    // backup refinement information 
    myhexa(). backup ( os );
    os.put( ObjectStream :: ENDOFSTREAM );
    
    // pack internal data if has any 
    inlineData ( os ) ;

    if( gs ) 
    {
      // pack Dune data 
      gs->inlineData( os , myhexa() );
    }

    _erasable = true ;
    return true ;
  }
  return false ;
}


// packall without gather scatter 
template < class A >
bool HexaPllBaseXMacro< A > :: packAll (vector < ObjectStream > & osv) 
{
  return doPackAll( osv, ( GatherScatterType* ) 0 );
}

// pack all function for dune 
template < class A >
bool HexaPllBaseXMacro< A > :: dunePackAll (vector < ObjectStream > & osv,
                                            GatherScatterType & gs) 
{
  return doPackAll( osv, &gs );
}

template < class A >
void HexaPllBaseXMacro< A > :: packAsBndNow(int fce, ObjectStream & os) const 
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
 
  for(int k=0; k<8; ++k) 
  {
    int vx = myhexa ().myvertex (k)->ident ();
    os.writeObject ( vx ) ;
  }

  int oppFace = Gitter :: Geometric :: Hexa :: oppositeFace[fce];
  for(int vx=0; vx<4; ++vx)
  {
    const Gitter :: Geometric :: VertexGeo * vertex = myhexa().myvertex(oppFace,vx); 
    os.writeObject( vertex->ident() );
    const alucoord_t (&p)[3] = vertex->Point();
    os.writeObject ( p[0] ) ;
    os.writeObject ( p[1] ) ;
    os.writeObject ( p[2] ) ;
  }
}

template < class A >
void HexaPllBaseXMacro< A > :: packAsGhost(ObjectStream & os, int fce) const 
{
  packAsBndNow(fce, os);
}

// packs macro element as internal bnd for other proc 
template < class A >
void HexaPllBaseXMacro< A > :: packAsBnd (int fce, int who, ObjectStream & os) const 
{
  if ( _moveTo != who ) 
  {
    packAsBndNow( fce, os );
  }
  return ;
}

template < class A >
void HexaPllBaseXMacro< A > :: 
unpackSelf (ObjectStream & os, 
            const bool i ) 
{
  doUnpackSelf( os, i , ( GatherScatterType * ) 0 );
}

template < class A >
void HexaPllBaseXMacro< A > :: 
duneUnpackSelf (ObjectStream & os, 
                const bool i, 
                GatherScatterType* gatherScatter ) 
{
  doUnpackSelf( os, i, gatherScatter );
}

template < class A >
void HexaPllBaseXMacro< A > :: 
doUnpackSelf (ObjectStream & os, 
              const bool i, 
              GatherScatterType* gatherScatter ) 
{
  assert (i) ;
  if (i) 
  {
    // unpack refinement data and restore 
    myhexa ().restore ( os ) ;

    // stream should now be at position ENDOFSTREAM 
    char c = os.get();
    if( c != ObjectStream :: ENDOFSTREAM )
    {
      cerr << "**FEHLER (FATAL) c != ENDOFSTREAM ! in " << __FILE__ << " " << __LINE__ << endl;
      abort();
    }
    
    // unpack internal data if has any 
    xtractData (os) ;

    // unpack dune data if present, pointer can be zero 
    if( gatherScatter ) 
    {
      gatherScatter->xtractData( os , myhexa() );
    }
  }
  else 
  {
    cerr << "**FEHLER (FATAL) i=false, should be true! in " << __FILE__ << " " << __LINE__ << endl;
    abort();
  }
  return ;
}

template < class A >
bool HexaPllBaseXMacro< A > :: erasable () const {
  return _erasable ;
}

// template instatiation 
template class HexaPllBaseX< GitterBasisPll :: ObjectsPll :: HexaEmpty > ;
template class HexaPllBaseXMacro< GitterBasisPll :: ObjectsPll :: hexa_IMPL > ;

///////////////////////////////////////////////////////////
//
//  --BndsegPllBaseX
//
///////////////////////////////////////////////////////////

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

GitterBasisPll :: ObjectsPll :: VertexPllImplMacro :: 
    VertexPllImplMacro (double x, double y, double z, int i, 
                        IndexManagerStorageType& ims,
                        linkagePatternMap_t & map) 
  : GitterBasis :: Objects :: VertexEmptyMacro (x,y,z,i,ims),
    _pllx (new mypllx_t (*this,map)) {
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

GitterBasisPll :: ObjectsPll :: Hface3EmptyPllMacro :: 
Hface3EmptyPllMacro (myhedge1_t * e0, int s0, myhedge1_t *e1,int s1, myhedge1_t *e2, int s2) 
  : Base_t(0, e0, s0, e1, s1, e2, s2) // 0 == level 0
{
} 

GitterBasisPll :: ObjectsPll :: Hface4EmptyPllMacro :: 
Hface4EmptyPllMacro (myhedge1_t *e0, int s0, myhedge1_t *e1, int s1, 
                     myhedge1_t *e2, int s2, myhedge1_t *e3, int s3) 
  : Base_t(0, e0, s0, e1, s1, e2, s2, e3, s3) // 0 == level 0
{
} 

////////////////////////////////////////////////////////////////
//  --MacroGitterBasisPll
////////////////////////////////////////////////////////////////
GitterBasisPll :: MacroGitterBasisPll :: MacroGitterBasisPll (Gitter * mygrid , istream & in) : GitterPll :: MacroGitterPll () , GitterBasis:: MacroGitterBasis (mygrid) 
{
  macrogridBuilder (in ) ;
  return ;
}

GitterBasisPll :: MacroGitterBasisPll :: MacroGitterBasisPll (Gitter * mygrid) : 
  GitterPll :: MacroGitterPll () , 
  GitterBasis :: MacroGitterBasis (mygrid) 
{
  return ;
}

GitterBasisPll :: MacroGitterBasisPll :: ~MacroGitterBasisPll () {
  try {
    {
      AccessIterator < helement_STI > :: Handle w (*this) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().detachPllXFromMacro () ;
    }
    {
      AccessIterator < vertex_STI > :: Handle w (*this) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().detachPllXFromMacro () ;
    }
  } 
  catch (Parallel :: AccessPllException) 
  {
    cerr << "**WARNUNG (AUSNAHME IGNORIERT) in " << __FILE__ << " " << __LINE__ << endl ;
  }
  {

#ifndef NDEBUG
    for (linkagePatternMap_t :: iterator p = _linkagePatterns.begin () ; p != _linkagePatterns.end () ; p ++) 
    {
      assert ((*p).second == 0) ;
    }
#endif
    
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

Gitter :: Geometric :: VertexGeo * GitterBasisPll :: MacroGitterBasisPll :: 
insert_vertex (double x,double y,double z,int i) {
  return new ObjectsPll :: VertexPllImplMacro (x,y,z,i,indexManagerStorage(),_linkagePatterns) ;
}

Gitter :: Geometric :: VertexGeo * GitterBasisPll :: MacroGitterBasisPll :: 
insert_ghostvx (double x, double y, double z, int i) 
{
  return GitterBasis :: MacroGitterBasis :: insert_vertex (x,y,z,i);
}

Gitter :: Geometric :: hedge1_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hedge1 (VertexGeo *a, VertexGeo *b) 
{
  return new ObjectsPll :: Hedge1EmptyPllMacro (a,b) ;
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
  return new ObjectsPll :: Hface4EmptyPllMacro (e [0], s [0], e [1], s [1], e [2], s [2], e [3], s [3]) ;
}

Gitter :: Geometric :: hface3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3]) 
{
  return new ObjectsPll :: Hface3EmptyPllMacro (e [0], s [0], e [1], s [1], e [2], s [2] ) ;
}

Gitter :: Geometric :: hexa_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hexa (hface4_GEO *(&f)[6], int (&t)[6]) 
{
  return new ObjectsPll :: HexaEmptyPllMacro (f [0], t[0], f [1], t[1], f [2], t[2], f[3], t[3], f[4], t[4], f[5], t[5]) ;
}

Gitter :: Geometric :: tetra_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_tetra (hface3_GEO *(&f)[4], int (&t)[4]) 
{
  return new ObjectsPll :: TetraEmptyPllMacro (f [0], t[0], f [1], t[1], f [2], t[2], f[3], t[3]); 
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
  typedef GitterBasis :: Objects :: Hbnd4Default Hbnd4DefaultType;
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    // internal face always get dummy index manager      
    return new Hbnd4PllInternal < Hbnd4DefaultType , BndsegPllBaseXClosure < Hbnd4DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd4DefaultType > > :: macro_t (f,t, b, *this ) ;
  } 
  else 
  {
    return new Hbnd4PllExternal < Hbnd4DefaultType, BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, b ) ;
  }
}

Gitter :: Geometric :: hbndseg4_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd4_ghost (hface4_GEO * f, int t) 
{
  Gitter :: hbndseg_STI :: bnd_t b = Gitter :: hbndseg_STI :: ghost_closure;
  return new Hbnd4PllExternal < GitterBasis :: Objects :: Hbnd4Default, 
    BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, b ) ;
}


Gitter :: Geometric :: hbndseg4_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd4 (hface4_GEO * f, int t,
              Gitter :: hbndseg_STI :: bnd_t b, 
              MacroGhostInfoHexa* ghInfo) 
{ 
  typedef GitterBasis :: Objects :: Hbnd4Default Hbnd4DefaultType;
  // if internal boundary create ghost 
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    assert( ghInfo );
    return new Hbnd4PllInternal < Hbnd4DefaultType , BndsegPllBaseXClosure < Hbnd4DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd4DefaultType > > :: 
          macro_t (f,t, b, *this, ghInfo ) ;
  } 
  else 
  {
    return new Hbnd4PllExternal < Hbnd4DefaultType , 
        BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, b ) ;
  }
}

// version with point 
Gitter :: Geometric :: hbndseg3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd3 (hface3_GEO * f, int t,
              Gitter :: hbndseg_STI :: bnd_t b, 
              MacroGhostInfoTetra * ghInfo) 
{
  typedef GitterBasis :: Objects :: Hbnd3Default Hbnd3DefaultType;
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    assert( ghInfo );
    // this HbnPll has a ghost element so is dosent get and index ==> dummyindex == 5 (see gitter_sti.h)
    return new Hbnd3PllInternal < Hbnd3DefaultType , BndsegPllBaseXClosure < Hbnd3DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd3DefaultType > > :: 
              macro_t (f,t, b, *this, ghInfo ) ;
  } 
  else 
  {
    return new Hbnd3PllExternal < Hbnd3DefaultType , 
        BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t, b ) ;
  }
}

// version without point 
Gitter :: Geometric :: hbndseg3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd3 (hface3_GEO * f, int t,
              Gitter :: hbndseg_STI :: bnd_t b ) 
{
  typedef GitterBasis :: Objects :: Hbnd3Default Hbnd3DefaultType;
  if (b == Gitter :: hbndseg_STI :: closure) 
  {
    // here we have a ghost of the ghost, therefor we need the element index manager 
    return new Hbnd3PllInternal < Hbnd3DefaultType , BndsegPllBaseXClosure < Hbnd3DefaultType > , 
          BndsegPllBaseXMacroClosure < Hbnd3DefaultType > > :: macro_t (f,t, b, *this ) ;
  } 
  else 
  {
    return new Hbnd3PllExternal < Hbnd3DefaultType , 
           BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t, b ) ;
  }
}

Gitter :: Geometric :: hbndseg3_GEO * GitterBasisPll :: MacroGitterBasisPll :: 
insert_hbnd3_ghost (hface3_GEO * f, int t) 
{
  Gitter :: hbndseg_STI :: bnd_t b = Gitter :: hbndseg_STI :: ghost_closure;
  typedef GitterBasis :: Objects :: Hbnd3Default Hbnd3DefaultType;

  return new Hbnd3PllExternal < Hbnd3DefaultType , 
         BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t, b );
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

GitterBasisPll :: GitterBasisPll (MpAccessLocal & mpa) 
  : _mpaccess (mpa), _macrogitter (0) , _ppv( 0 ) {
  _macrogitter = new MacroGitterBasisPll (this) ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

GitterBasisPll :: GitterBasisPll (const string filename, 
                                  MpAccessLocal & mpa, ProjectVertex* ppv ) 
  : GitterPll(mpa.myrank() == 0) , _mpaccess (mpa), _macrogitter (0) , _ppv( ppv ) 
{
  assert (debugOption (20) ? (cout << "GitterBasisPll :: GitterBasisPll (const char * = \"" << filename << "\" ...)" << endl, 1) : 1) ;

  const int myrank = mpa.myrank();
  stringstream rank;
  rank << "." << myrank;

  // if still no macrogitter, try old method 
  if(!_macrogitter) 
  {
    string extendedName ( filename );
    extendedName += rank.str();

    ifstream in (extendedName.c_str()) ;
    if (in) 
    {
      _macrogitter = new MacroGitterBasisPll (this, in) ;
    } 
    else 
    {
      assert (debugOption (5) ? 
        ( cerr << "  GitterBasisPll :: GitterBasisPll () file: " << extendedName 
           << " cannot be read. Try " << filename << " instead. In " << __FILE__ << " line " << __LINE__ << endl, 1) : 1);
    }
  }

  // only check this for higher ranks 
  // we assume that filename already contains rank info
  // if not empty grid is created 
  bool validFilename = true ;
  if( ! _macrogitter && myrank > 0 )
  {
    // search rank info in filename 
    const int result = filename.rfind( rank.str() );
    // if not found then filename is not valid 
    // and empty grid should be created 
    if( result == -1 )
    {
      validFilename = false ;
    }
  }

  // read normal macro gitter if not created yet and 
  // filename is valid for this rank  
  if( ! _macrogitter && validFilename ) 
  {
    ifstream in ( filename.c_str() ) ;
    if (in) _macrogitter = new MacroGitterBasisPll (this, in) ;
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

void GitterBasisPll :: printMemUsage ()
{
  typedef GitterBasisPll :: ObjectsPll :: TetraEmptyPllMacro tetra_MACRO ; 
  typedef GitterBasisPll :: ObjectsPll :: HexaEmptyPllMacro  hexa_MACRO ; 
  typedef GitterBasisPll :: ObjectsPll :: hbndseg3_IMPL hbndseg3_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: hbndseg4_IMPL hbndseg4_IMPL ; 
  //typedef GitterBasisPll :: ObjectsPll :: Hface3EmptyPllMacro hface3_IMPL ; 
  //typedef GitterBasisPll :: ObjectsPll :: Hface4EmptyPllMacro hface4_IMPL ; 
  //typedef GitterBasisPll :: ObjectsPll :: Hedge1EmptyPllMacro hedge1_IMPL ; 
  //typedef GitterBasisPll :: ObjectsPll :: VertexPllImplMacro VertexMacro; 
  typedef GitterBasisPll :: ObjectsPll :: tetra_IMPL tetra_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: hexa_IMPL  hexa_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: hbndseg3_IMPL hbndseg3_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: hbndseg4_IMPL hbndseg4_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: hface3_IMPL hface3_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: hface4_IMPL hface4_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: hedge1_IMPL hedge1_IMPL ; 
  typedef GitterBasisPll :: ObjectsPll :: VertexPllImplMacro VertexMacro; 
  typedef GitterBasis :: DuneIndexProvider DuneIndexProvider; 
  typedef GitterBasis :: Objects :: VertexEmptyMacro VertexEmptyMacro; 
  typedef GitterBasis :: Objects :: VertexEmpty VertexEmpty; 
  typedef Gitter :: Geometric :: VertexGeo VertexGeo; 
  cout << "bool   = " << sizeof(bool) << endl;
  cout << "char   = " << sizeof(unsigned char) << endl;
  cout << "signed char   = " << sizeof(signed char) << endl;
  cout << "moveto = " << sizeof( map < int, int, less < int > > ) << endl;
  cout << "MyAlloc = " << sizeof(MyAlloc) << "\n";
  cout << "Refcount = " << sizeof(Refcount) << "\n";
  cout << "HedgeRule  = " << sizeof(Gitter :: Geometric :: Hedge1Rule) <<"\n";
  cout << "Hface3Rule = " << sizeof(Gitter :: Geometric :: Hface3Rule) <<"\n";
  cout << "Hface4Rule = " << sizeof(Gitter :: Geometric :: Hface4Rule) <<"\n";
  cout << "DuneIndexProvider = "<< sizeof(DuneIndexProvider) << "\n\n";
  
  cout << "******** TETRA *************************8\n";
  cout << "Tetrasize  = " << sizeof(tetra_IMPL) << endl;
  cout << "TetraMacro = " << sizeof(tetra_MACRO) << endl;
  cout << "MacroGhostTetra = " << sizeof(MacroGhostTetra) << endl;
  cout << "Hface3_IMPL = " << sizeof(hface3_IMPL) << endl;
  cout << "Hface3_GEO = " << sizeof( Gitter :: Geometric :: hface3_GEO ) << endl;
  cout << "Hface3::nb = " << sizeof( Gitter :: Geometric :: hface3 :: face3Neighbour ) << endl;
  cout << "HEdge1_IMPL = " << sizeof(hedge1_IMPL) << endl;
  cout << "HEdge1_GEO = " << sizeof(Gitter :: Geometric ::hedge1_GEO) << endl;
  cout << "VertexMacro = " << sizeof(VertexEmptyMacro) << endl;
  cout << "VertexGeo   = " << sizeof(VertexGeo) << endl;
  cout << "Vertex = " << sizeof(VertexEmpty) << endl;
  cout << "Hbnd3_IMPL  = " << sizeof(hbndseg3_IMPL) << endl << endl;

  cout << "******** HEXA *************************8\n";
  cout << "Hexasize  = " << sizeof(hexa_IMPL) << endl;
  cout << "HexaMacro = " << sizeof(hexa_MACRO) << endl;
  cout << "MacroGhostHexa = " << sizeof(MacroGhostHexa) << endl;
  cout << "Hface4_IMPL = " << sizeof(hface4_IMPL) << endl;
  cout << "Hface4_GEO = " << sizeof( Gitter :: Geometric :: hface4_GEO ) << endl;
  cout << "Hface4::nb = " << sizeof( Gitter :: Geometric :: hface4 :: face4Neighbour ) << endl;
  cout << "Hbnd4_IMPL  = " << sizeof(hbndseg4_IMPL) << endl << endl;

  cout << "******** Number of Elements ************************8\n";
  {
    size_t totalSize = 0; 
    bool simplex = false;
    {
      AccessIterator < helement_STI > :: Handle iter (container ());
      int size = iter.size();
      iter.first(); 
      if( !iter.done() )
      {
        if( iter.item().type() == tetra )
        {
          simplex = true;
          size *= sizeof(tetra_IMPL);
        } 
        else
        {
          size *= sizeof(hexa_IMPL);
        } 
      } 
      totalSize += size;
      cout << "Macro elements: size = " << size/1024/1024 << " MB \n";
    } 
    
    {
      int size = AccessIterator < hbndseg_STI > :: Handle (container ()).size();
      size *= (simplex) ?  sizeof(hbndseg3_IMPL) : sizeof(hbndseg4_IMPL);
      cout << "Macro boundary : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < hface_STI > :: Handle (container ()).size();
      size *= (simplex) ?  sizeof(hface3_IMPL) : sizeof(hface4_IMPL);
      cout << "Macro faces : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < hedge_STI > :: Handle (container ()).size();
      size *= sizeof(hedge1_IMPL);
      cout << "Macro edges : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < vertex_STI > :: Handle (container ()).size();
      size *= sizeof(VertexEmptyMacro);
      cout << "Macro vertices : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    size_t allSize = 0;
    size_t numElements = 0;
    {
      LeafIterator< helement_STI > it( *this );
      int size = it->size();
      numElements = size;
      size *= (simplex ? sizeof( tetra_IMPL ) : sizeof( hexa_IMPL ));
      cout << "Elements : size = " << size/1024/1024 << " MB" << endl;
      allSize += size;
    }

    {
      LeafIterator< hbndseg_STI > it( *this );
      const int size = it->size() * (simplex ? sizeof( hbndseg3_IMPL ) : sizeof( hbndseg4_IMPL ));
      cout << "Boundaries : size = " << size/1024/1024 << " MB" << endl;
      allSize += size;
    }

    {
      LeafIterator< hface_STI > it( *this );
      const int size = it->size() * (simplex ? sizeof( hface3_IMPL ) : sizeof( hface4_IMPL ));
      cout << "Faces : size = " << size/1024/1024 << " MB" << endl;
      allSize += size;
    }

    {
      LeafIterator< hedge_STI > it( *this );
      const int size = it->size() * sizeof( hedge1_IMPL );
      cout << "Edges : size = " << size/1024/1024 << " MB" << endl;
      allSize += size;
    }

    {
      LeafIterator< vertex_STI > it( *this );
      const int size = it->size() * sizeof( VertexEmpty );
      cout << "Vertices : size = " << size/1024/1024 << " MB" << endl;
      allSize += size;
    }

    {
      size_t indexMem = 0;
      for(int i=0; i<4; ++i) 
        indexMem += indexManager( i ).getMaxIndex() * sizeof( int );
      cout << "Indices : size = " << indexMem/1024/1024 << " MB" << endl;
      allSize += indexMem;
    }

    cout << "All leaf size : " << allSize << " MB" << endl;
    cout << "bytes per Element: " << allSize/numElements << endl; 
    cout << "Estimated all size : " << (9*long(allSize) / 8) << " MB" << endl;

    size_t build = container().memUsage();
    cout << "BuilderIF size = " << build/1024/1024 << " MB \n";
    totalSize += build;
    cout << "Overall size = " << totalSize/1024/1024 << " MB \n";
    cout << "\n" ;
  }
}

