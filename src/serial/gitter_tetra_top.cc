// (c) Robert Kloefkorn 2010 

#include "mapp_tetra_3d.h"
#include "gitter_tetra_top.h"

// #     #                                  #####  #######
// #     #  ######    ##     ####   ###### #     #    #      ####   #####
// #     #  #        #  #   #    #  #            #    #     #    #  #    #
// #######  #####   #    #  #       #####   #####     #     #    #  #    #
// #     #  #       ######  #       #            #    #     #    #  #####
// #     #  #       #    #  #    #  #      #     #    #     #    #  #
// #     #  #       #    #   ####   ######  #####     #      ####   #

template < class A > 
typename Hface3Top < A > :: myvertex_t* 
Hface3Top < A > :: vertexNotOnSplitEdge( const int splitEdge )
{
  const myhedge1_t* edge = myhedge1( splitEdge );
  const myvertex_t* edgeVx[ 2 ] = { edge->myvertex( 0 ), edge->myvertex( 1 ) };
  int iVx = (splitEdge + 2) % 3;
  myvertex_t* vx = myvertex( iVx );
  while ( vx == edgeVx[ 0 ] || vx == edgeVx[ 1 ] )
  {
    iVx = ( iVx + 1 ) % 3;
    vx = myvertex( iVx );
    assert( iVx != (splitEdge+2)%3 );
  }

  return vx ;
}

template < class A > typename Hface3Top < A > :: edgepair_t 
Hface3Top < A > :: subEdges( myhedge1_t* edge, const myvertex_t* vx0, const myvertex_t* vx1 )  
{
  assert( vx0 );
  assert( vx1 );

  // get sub faces 
  myhedge1_t* subEdge[ 2 ] = { edge->subedge1( 0 ), edge->subedge1( 1 ) };

  // check face order with vertex0
  int sub0 = 1 ;
  for( int i=0; i<2; ++i ) 
  {
    if( subEdge[ 0 ]->myvertex( i ) == vx0 ) 
    {
      sub0 = 0;
      break;
    }
  }

#ifndef NDEBUG 
  // make sure the vertex is on the other face 
  bool found0 = false ;
  bool found1 = false ;
  // check face order with vertex0
  for( int i=0; i<2; ++i ) 
  {
    if( subEdge[ sub0 ]->myvertex( i ) == vx0 ) 
    {
      found0 = true ;
    }
    if( subEdge[ ! sub0 ]->myvertex( i ) == vx1 ) 
    {
      found1 = true ;
    }
  }
  if( ! found0 || ! found1 ) 
  {
    cout << "Problem: " << edge << endl;
    cout << " vx0 " << vx0 << endl;
    cout << " vx1 " << vx1 << endl;
    cout << "sub0 " << subEdge[ sub0 ] << endl;
    cout << "sub1 " << subEdge[ ! sub0 ] << endl;
  }
  assert( found0 );
  assert( found1 );

#endif
  return edgepair_t( subEdge[ sub0 ], subEdge[ ! sub0 ] );
}


template < class A > void Hface3Top < A > :: split_e01 () 
{
  // NOTE: edge numbering is not opposite vertex !!!
  // see gitter_geo.cc 
  // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})
 
  /* 
                    2  
                    *\
                  /2 2\
                 /  *  \
                /       \
               /    *    \
           2  /           \ 1
             /2    1*2    1\
            /               \
           /        *        \
          /                   \
         /          *          \
        /0    0    1 0    0    1\
        ------------*------------
      0            0               1 
    */

  assert( _inner == 0 );
  const int newLevel = 1 + level () ;
  myhedge1_t* splitEdge = myhedge1(0);

  myvertex_t * ev0 = splitEdge->subvertex (0) ;
  myvertex_t * vx2 = vertexNotOnSplitEdge( 0 );

  edgepair_t subEdge = subEdges( splitEdge, myvertex(0), myvertex(1) ); 
  //myvertex_t * vx2 = myvertex( 2 );
  assert(ev0) ;
  inneredge_t * e0 = new inneredge_t (newLevel, ev0, vx2 ) ;
  assert( e0 ) ;
  innerface_t * f0 = new innerface_t (newLevel, 
                                      subEdge.first, twist(0), 
                                      e0, 0, 
                                      myhedge1(2), twist(2), 
                                      0) ; // child number 

  innerface_t * f1 = new innerface_t (newLevel, 
                                      subEdge.second, twist(0), 
                                      myhedge1(1), twist(1), 
                                      e0, 1, 
                                      1) ; // child number 

  //std::cout << "split_e01 " << ev0 << endl;
  //cout << "Split face " << this << " into " << endl;
  //cout << "New subface 0" << f0 << endl;
  //cout << "New subface 1" << f1 << endl;

  assert (f0 && f1 ) ;
  f0->append(f1) ;

  _inner = new inner_t( f0 , e0 );
  _rule = myrule_t :: e01 ;
  return ;
}

template < class A >  void Hface3Top < A > :: split_e12 () 
{
  // NOTE: edge numbering is not opposite vertex !!!
  // see gitter_geo.cc 
  // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})
  
  /*                  2 
                    
                     / \
                    / 2 \
                   /     \
                  /     1 \
                 /         \
                /           \
             2 / 2         1 \  1 
              /             * \
             /           *   2 \
            /      0  *         \
           /       *           1 \
          /     *   2             \
         /0  *                     \
        / *0                      1 \
        -----------------------------
      0              0                 1 
    */
  assert( _inner == 0 );
  const int newLevel= 1 + level () ;
  myhedge1_t* splitEdge = myhedge1(1);

  myvertex_t * ev0 = splitEdge->subvertex (0) ;
  assert(ev0) ;

  myvertex_t * vxOld = vertexNotOnSplitEdge( 1 );
  //myvertex_t * vxOld = myvertex(0);

  edgepair_t subEdge = subEdges( splitEdge, myvertex(1), myvertex(2) ); 

  // create new inner edge 
  inneredge_t * e0 = new inneredge_t (newLevel, ev0, vxOld ) ;
  assert( e0 ) ;

  //std::cout << "split_e12 " << ev0 << endl;
  //cout << "new inner edge " << e0 << endl;
  
  innerface_t * f0 = new innerface_t (newLevel, // level 
                                      myhedge1(0), twist(0),         // edge 0, twist 
                                      subEdge.first, twist(1), // edge 0, twist 
                                      e0, 0,                         // edge 1, twist 
                                      0 ) ; // child number 

  innerface_t * f1 = new innerface_t (newLevel, // level 
                                      e0, 1,                         // edge 1, twist 
                                      subEdge.second, twist(1), // edge 2, twist 
                                      myhedge1(2), twist(2),         // edge 0, twist
                                      1 ) ; // child number 
  assert (f0 && f1 ) ;
  f0->append(f1) ;

  //cout << "Split face " << this << " into " << endl;
  //cout << "New subface 0" << f0 << endl;
  //cout << "New subface 1" << f1 << endl;

  _inner = new inner_t( f0 , e0 );
  _rule = myrule_t :: e12 ;


  return ;
}

template < class A >  void Hface3Top < A > :: split_e20 () 
{
  // NOTE: edge numbering is not opposite vertex !!!
  // see gitter_geo.cc 
  // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})

  /*                  2 
                    
                     / \
                    / 2 \
                   /     \
                  /2      \
                 /         \
                /           \
             2 /0            \  1 
              / *            1\
             / 2   *           \
            /         * 0       \
           /2          1 *       \
          /                 *     \
         /                     * 1 \
        / 0          0          1 * \
        -----------------------------
      0              0                 1 
    */

  assert( _inner == 0 );
  const int newLevel= 1 + level () ;

  myhedge1_t* splitEdge = myhedge1( 2 );

  // get vertices of new edge 
  myvertex_t * ev0 = splitEdge->subvertex (0) ;
  myvertex_t * vxOld = vertexNotOnSplitEdge( 2 );

  edgepair_t subEdge = subEdges( splitEdge, myvertex(0), myvertex(2) ); 
  //myvertex_t * vxOld = myvertex( 1 );
  
  assert(ev0) ;
  inneredge_t * e0 = new inneredge_t (newLevel, ev0, vxOld ) ;
  assert( e0 ) ;

  innerface_t * f0 = new innerface_t (newLevel, // level 
                                      e0, 0,                         // edge 0, twist 
                                      myhedge1(1), twist(1),         // edge 1, twist 
                                      subEdge.second, twist(2), // edge 2, twist 
                                      0 ) ; // child number 

  innerface_t * f1 = new innerface_t (newLevel, // level 
                                      myhedge1(0), twist(0),         // edge 0, twist
                                      e0, 1,                         // edge 1, twist 
                                      subEdge.first, twist(2), // edge 2, twist 
                                      1 ) ; // child number 

  assert (f0 && f1 ) ;

  //std::cout << "split_e20 " << ev0 << endl;
  //cout << "Split face " << this << " into " << endl;
  //cout << "New subface 0" << f0 << endl;
  //cout << "New subface 1" << f1 << endl;

  f0->append(f1) ;
  _inner = new inner_t( f0 , e0 );
  _rule = myrule_t :: e20 ;

  return ;
}

template < class A >  void Hface3Top < A > :: split_iso4 () 
{
  assert( _inner == 0 );
  int l = 1 + level () ;
  myvertex_t * ev0 = myhedge1(0)->subvertex (0) ;
  myvertex_t * ev1 = myhedge1(1)->subvertex (0) ;
  myvertex_t * ev2 = myhedge1(2)->subvertex (0) ;
  assert(ev0 && ev1 && ev2 ) ;
  inneredge_t * e0 = new inneredge_t (l, ev0, ev1) ;
  inneredge_t * e1 = new inneredge_t (l, ev1, ev2) ;
  inneredge_t * e2 = new inneredge_t (l, ev2, ev0) ;
  assert( e0 && e1 && e2 ) ;
  e0->append(e1) ;
  e1->append(e2) ;
  innerface_t * f0 = new innerface_t (l, this->subedge1(0,0), twist(0), e2, 1, this->subedge1(2,1), twist(2), 0) ;
  innerface_t * f1 = new innerface_t (l, this->subedge1(0,1), twist(0), this->subedge1(1,0), twist(1), e0, 1, 1) ;
  innerface_t * f2 = new innerface_t (l, e1, 1, this->subedge1(1,1), twist(1), this->subedge1(2,0), twist(2), 2) ;
  innerface_t * f3 = new innerface_t (l, e0, 0, e1, 0, e2, 0, 3 ) ;
  assert (f0 && f1 && f2 && f3) ;
  f0->append(f1) ;
  f1->append(f2) ;
  f2->append(f3) ;
  _inner = new inner_t( f0 , e0 );
  _rule = myrule_t :: iso4 ;
  return ;
}

template < class A > void Hface3Top < A > :: refineImmediate (myrule_t r) 
{
  if ( r != getrule () ) 
  {
    if( getrule() != myrule_t :: nosplit )
    {
      std::cout << " getrule(): " << getrule() << " ";
     std::cout << " r:         " << r << std::endl;
    }
    assert (getrule () == myrule_t :: nosplit) ;

    switch(r) 
    {
      typedef typename myhedge1_t :: myrule_t myhedge1rule_t;

      // rotate of hedge rule does nothing, 
      // so its actually useless 

      case myrule_t :: e01 :
        myhedge1 (0)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (twist (0))) ;
        split_e01 () ;
        break ;
      case myrule_t :: e12 :
        myhedge1 (1)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (twist (1))) ;
        split_e12 () ;
        break ;
      case myrule_t :: e20 :
        myhedge1 (2)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (twist (2))) ;
        split_e20 () ;
        break ;
      case myrule_t :: iso4 :
        myhedge1 (0)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (twist (0))) ;
        myhedge1 (1)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (twist (1))) ;
        myhedge1 (2)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (twist (2))) ;
        split_iso4 () ;
        break ;
      default :
        cerr << "**FEHLER (FATAL) falsche Verfeinerungsregel [" << r << "] in " << __FILE__ << " " << __LINE__ << endl ;
        abort () ;
        break ;
    }

    // set parent rule 
    {
      myrule_t myRule = getrule();
      for (innerface_t * f = dwnPtr() ; f ; f = f->next ()) 
      {
        f->nb._parRule = myRule;
      }
    }
    this->postRefinement () ;
  }
  return ;
}

template < class A > bool Hface3Top < A > :: refine (myrule_t r, int twist) 
{
  if (r != getrule ()) 
  {
    assert (getrule () == myrule_t :: nosplit ? 1 : 
      (cerr << "**FEHLER beim Verfeinern mit Regel " << r << " auf " << getrule () << endl, 0)) ;
    switch(r) {
      case myrule_t :: e01 :
      case myrule_t :: e12 :
      case myrule_t :: e20 :
      case myrule_t :: iso4 :
      {
        bool a = (twist < 0) 
               ? this->nb.front ().first->refineBalance (r,this->nb.front ().second)
               : this->nb.rear  ().first->refineBalance (r,this->nb.rear  ().second) ;
        if (a) 
        {  
          if (getrule () == myrule_t :: nosplit) 
          {
            refineImmediate (r) ;
            { 
              for (innerface_t * f = dwnPtr() ; f ; f = f->next ()) f->nb = this->nb ; 
            }
          } 
          else 
          {
            // Als Test absichern, da"s die Verfeinerung durchgekommen ist. Im
            // anisotropen Fall darf das so nicht mehr gemacht werden.
            assert (getrule () == r) ;
          }

          this->postRefinement () ;
          return true ;
        } 
        else 
        {
          return false ;
        }
      }
      default :
        cerr << "**WARNUNG (IGNORIERT) falsche Verfeinerungsregel gefunden: " ;
        cerr << "[" << r << "] in " << __FILE__ << " " << __LINE__ << endl ;
        return false ;
    }
  }
  return true ;
}

template < class A > bool Hface3Top < A > :: coarse () 
{
  innerface_t * f = dwnPtr() ;
  if (!f) return false ;
  bool x = true ;
  do {

  // Falls eine Kind-Fl"ache noch referenziert wird, kann
  // nicht auf diesem Level vergr"obert werden.
  // Daher wird nur die nichtkonforme Nachbarschaft ver-
  // vollst"andigt, die eventuell durch Elementvergr"oberung
  // durcheinander gekommen war. Die Vergr"oberung geht dann
  // auf das n"achste Level "uber.
  
    if (f->ref) {
      if (f->ref == 1) f->nb.complete (this->nb) ;
      f->coarse () ;
      x = false ;
    }
  } while ( (f = f->next()) ) ;
  if (x) {
    
  // Hier wird tats"achlich vergr"obert, d.h. alle Kinder 
  // werden beseitigt, und das Bezugsobjekt wird zum neuen
  // Blatt im Baum.
    
    delete _inner; 
    _inner = 0 ;

    _rule = myrule_t :: nosplit ;
    {for (int i = 0 ; i < 3 ; i ++ ) myhedge1 (i)->coarse () ; }
  }
  return x ;
}

template < class A > void Hface3Top < A > :: backup (ostream & os) const 
{
  doBackup( os );
}

template < class A > void Hface3Top < A > :: backup (ObjectStream & os) const 
{
  doBackup( os );
}

template < class A > 
template < class OutStream_t>
void Hface3Top < A > :: doBackup (OutStream_t & os) const 
{
  os.put ((char) getrule ()) ;
  {for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ; }
  {for (const innerface_t * c = dwnPtr() ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > void Hface3Top < A > :: restore (istream & is) 
{
  doRestore( is );
}

template < class A > void Hface3Top < A > :: restore (ObjectStream& is) 
{
  doRestore( is );
}

template < class A > template <class InStream_t> 
void Hface3Top < A > :: doRestore (InStream_t & is) 
{
  refineImmediate (myrule_t ((char) is.get ())) ;
  {for (inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->restore (is) ; }
  {for (innerface_t * c = dwnPtr() ; c ; c = c->next ()) c->restore (is) ; }
  return ;
}

// #     #                          #####  #######
// #     #  #####   #    #  #####  #     #    #      ####   #####
// #     #  #    #  ##   #  #    #       #    #     #    #  #    #
// #######  #####   # #  #  #    #  #####     #     #    #  #    #
// #     #  #    #  #  # #  #    #       #    #     #    #  #####
// #     #  #    #  #   ##  #    # #     #    #     #    #  #
// #     #  #####   #    #  #####   #####     #      ####   #

template < class A >  void Hbnd3Top < A > :: 
setBoundaryId (const int id ) 
{
  // set my id to the same as bnd 
  this->setBndId( id );
  myhface3_t & face = *(myhface3(0));
  face.setBndId( id );
  // 3 vertices and edges 
  for(int i=0; i<3; ++i) 
  {
    face.myvertex(i)->setBndId( id );
    face.myhedge1(i)->setBndId( id );
  }
}

template < class A >  void Hbnd3Top < A > :: split_e01 () 
{
  int l = 1 + level () ;
  int gFace = this->getGhost().second ;
  innerbndseg_t * b0 = new innerbndseg_t (l, subface3 (0,0), twist (0), this , _bt, 0 , gFace) ;
  innerbndseg_t * b1 = new innerbndseg_t (l, subface3 (0,1), twist (0), this , _bt, 0 , gFace) ;
  assert (b0 && b1) ;
  b0->append(b1) ;
  _dwn = b0 ;
  return ;
}

template < class A >  void Hbnd3Top < A > :: split_e12 () 
{
  int l = 1 + level () ;
  int gFace = this->getGhost().second ;
  innerbndseg_t * b0 = new innerbndseg_t (l, subface3 (0,0), twist (0), this , _bt, 0 , gFace) ;
  innerbndseg_t * b1 = new innerbndseg_t (l, subface3 (0,1), twist (0), this , _bt, 0 , gFace) ;
  assert (b0 && b1) ;
  b0->append(b1) ;
  _dwn = b0 ;
  return ;
}

template < class A >  void Hbnd3Top < A > :: split_e20 () 
{
  int l = 1 + level () ;
  int gFace = this->getGhost().second ;
  innerbndseg_t * b0 = new innerbndseg_t (l, subface3 (0,0), twist (0), this , _bt, 0, gFace) ;
  innerbndseg_t * b1 = new innerbndseg_t (l, subface3 (0,1), twist (0), this , _bt, 0, gFace) ;
  assert (b0 && b1) ;
  b0->append(b1) ;
  _dwn = b0 ;
  return ;
}

template < class A >  void Hbnd3Top < A > :: split_iso4 () 
{
  int l = 1 + level () ;
 
  typedef typename Gitter :: GhostChildrenInfo GhostChildrenInfo; 
  GhostChildrenInfo ghostInfo;
  // ghostInfo is filled by splitGhost, see gitter_tetra_top_pll.h
  this->splitGhost( ghostInfo );

  innerbndseg_t * b0 = new innerbndseg_t (l, subface3 (0,0), twist (0), this , _bt, ghostInfo.child(0), ghostInfo.face(0)) ;
  innerbndseg_t * b1 = new innerbndseg_t (l, subface3 (0,1), twist (0), this , _bt, ghostInfo.child(1), ghostInfo.face(1)) ;
  innerbndseg_t * b2 = new innerbndseg_t (l, subface3 (0,2), twist (0), this , _bt, ghostInfo.child(2), ghostInfo.face(2)) ;
  innerbndseg_t * b3 = new innerbndseg_t (l, subface3 (0,3), twist (0), this , _bt, ghostInfo.child(3), ghostInfo.face(3)) ;
  assert (b0 && b1 && b2 && b3) ;
  b0->append(b1) ;
  b1->append(b2) ;
  b2->append(b3) ;
  _dwn = b0 ;
  
  return ;
}

template < class A >  bool Hbnd3Top < A > :: coarse () 
{
  innerbndseg_t * b = down () ;
  if (!b) return false ;
  bool x = true ;
  do {
    if( (b->myhface3(0)->ref > 1) ) ((b->coarse ()), x = false) ;
  } while ( (b = b->next()) ) ;
  if (x) 
  {
    if (!this->lockedAgainstCoarsening ()) 
    {
      this->preCoarsening () ;
      
      delete _dwn ; _dwn = 0 ;
      myhface3 (0)->coarse () ;

      this->coarseGhost();
    }
  }
  return x ;
}

template < class A >  bool Hbnd3Top < A > :: bndNotifyCoarsen () {
  return coarse () ;
}

template < class A >  bool Hbnd3Top < A > :: refineBalance (balrule_t r, int b) 
{

  // Die Methode refineBalance () f"uhrt auf dem Randabschluss entweder
  // unbedingt die Verfeinerung durch, da im Verlauf der Verfeinerung keine
  // weiteren Anforerungen mehr an den Randabschluss  gerichtet werden 
  // ODER gibt die Verfeinerung als nicht erf"ullt zur"uck: Dann liegt
  // es am Aufrufer die Verfeinerung nochmals anzuforern.

  assert (b == 0) ;
  assert (this->leaf ()) ;
  if (! this->bndNotifyBalance (r,b) ) 
  {
  
    // Hier kann der innere Rand [parallel] die Verfeinerung
    // verhindern, damit z.B. das Durchverfeinern im anisotropen
    // Fall erstmal nicht stattfindet, wenn nicht klar ist, wie die
    // weitere Rekursion aussieht. Dazu muss auf dem Niveau der Klasse
    // des Template-Arguments die Methode bndNotifyBalance () "uber-
    // schrieben werden. Die Defaultmethode liefert immer 'true'.
  
    return false ;
  } 
  else 
  {
    //cout << "HbndTop: refineFace (twst = " << twist( 0 ) << ") ";
    //cout << myhface3 (0) << endl;

    if(r == myrule_t :: iso4) 
    {
      // Der Rand verfeinert unbedingt die anliegende Fl"ache und dann
      // sich selbst, weil die Anforderung durch die Fl"ache kam, und
      // dahinter keine Balancierung stattfinden muss.
    
      myhface3 (0)->refineImmediate (r) ;
      split_iso4 () ;
    } 
    else if (r == myrule_t :: e01) {
      myhface3 (0)->refineImmediate (r) ;
      split_e01 () ;
    } 
    else if (r == myrule_t :: e12) {
      myhface3 (0)->refineImmediate (r) ;
      split_e12 () ;
    } 
    else if (r == myrule_t :: e20) {
      myhface3 (0)->refineImmediate (r) ;
      split_e20 () ;
    } 
    else {
      cerr << "**FEHLER (FATAL, weil nicht vorgesehen) beim Verfeinern am " ;
      cerr << "Randst\"uck mit der Regel [" << r << "] in " ;
      cerr << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    
    // postRefinement () gibt die M"oglichkeit auf dem Niveau des
    // Template-Arguments eine Methode aufzurufen, um eventuelle
    // Operationen auf dem verfeinerten Randst"uck durchzuf"uhren.
    
    this->postRefinement () ;
    return true ;
  }
}

template < class A >  bool Hbnd3Top < A > :: refineLikeElement (balrule_t r) 
{

  // Mit der Methode refineLikeElement () verh"alt sich ein Randabschluss
  // in der Verfeinerung wie ein Element: Es wird zuerst gepr"uft ob eine
  // Balancierung der Vererfeinerung durch die Fl"ache hindurch erfolgreich
  // ist und nur genau dann die Verfeinerung durchgef"uhrt mit R"uckgabewert
  // 'true'. Diese Methode bedient eigentlich nur die parallele Verfeinerung
  // kann aber auch auf jedem beliebigen Randelement im seriellen Fall auf-
  // gerufen werden ohne Schaden anzurichten: Eine 1-Level Verfeinerung am
  // Rand ist jedoch wirkungslos, da sie beim n"achsten Vergr"obern wieder
  // aufgel"ost ist. Erst die mehrfache Anwendung f"uhrt durch die 
  // Balancierung zu einer "Anderung am Elementgitter.

  if (r == myrule_t :: nosplit) {
  
    cerr << "**WARNUNG (IGNORIERT) beim Versuch mit nosplit zu Verfeinern" ;
    cerr << "  in " << __FILE__ << " " << __LINE__ << endl ;
    
      // Eine Anforderung mit nosplit zu Verfeinern nur erf"ullt,
  // falls die zugeh"orige Fl"achenregel auch nosplit ist, sonst
  // wird die Anforderung als nicht erf"ullt zur"uckgegeben.
    
    return this->getrule () == balrule_t :: nosplit ? true : false ;
    
  } else {
    if (this->getrule () == r) {
    
      // Alles schon wie es sein soll -> true.
    
      return true ;
    } else {
  
  // Der nachfolgende Test bezieht sich auf die Verfeinerungssituation
  // der Fl"ache, da getrule () auf myhface3 (0)->getrule () umgeleitet
  // ist.
  
      // assert (this->getrule () == myrule_t :: nosplit) ;
      switch (r) {
      case balrule_t :: e01 :
        //cout << "refLikeEl: e01 " << endl;
        // if (!myhface3 (0)->refine (balrule_t (balrule_t :: e01).rotate (twist (0)), twist (0))) return false ;
        if (!myhface3 (0)->refine (r, twist (0))) return false ;
        split_e01 () ;
        break;
      case balrule_t :: e12 :
        //cout << "refLikeEl: e12 " << endl;
        //if (!myhface3 (0)->refine (balrule_t (balrule_t :: e12).rotate (twist (0)), twist (0))) return false ;
        if (!myhface3 (0)->refine (r, twist (0))) return false ;
        split_e12 () ;
        break;
      case balrule_t :: e20 :
        // cout << "refLikeEl: e20 " << " " << balrule_t (balrule_t :: e20).rotate (twist (0)) << endl;
        //if (!myhface3 (0)->refine (balrule_t (balrule_t :: e20).rotate (twist (0)), twist (0))) return false ;
        if (!myhface3 (0)->refine (r, twist (0))) return false ;
        split_e20 () ;
        break;
      case balrule_t :: iso4 :
        //if (!myhface3 (0)->refine (balrule_t (balrule_t :: iso4).rotate (twist (0)), twist (0))) return false ;
        if (!myhface3 (0)->refine (r, twist (0))) return false ;
        split_iso4 () ;
        break;
      default :
        cerr << "**WARNUNG (FEHLER IGNORIERT) falsche Verfeinerungsregel [" << this->getrule () ;
        cerr << "] (ignoriert) in " << __FILE__ << " " << __LINE__ << endl ;
        return false ;
      }

      // postRefinement () gibt die M"oglichkeit auf dem Niveau des
  // Template-Arguments eine Methode aufzurufen, um eventuelle
  // Operationen auf dem verfeinerten Randst"uck durchzuf"uhren.
      this->postRefinement () ;
      return true ;
    }
  }
}

template < class A >  void Hbnd3Top < A > :: restoreFollowFace () 
{
  // retoreFollowFace () veranlasst das Randelement sich am 
  // bestehenden Fl"achenbaum wiederherzustellen durch die
  // entsprechende Verfeinerung.
  
  myhface3_t & f (*(myhface3 (0))) ;
  if (!f.leaf ()) {
    balrule_t r = f.getrule () ;
    switch (r) {
      case myrule_t :: e01 :
        split_e01 () ;
        break ;
      case myrule_t :: e12 :
        split_e12 () ;
        break ;
      case myrule_t :: e20 :
        split_e20 () ;
        break ;
      case myrule_t :: iso4 :
        split_iso4 () ;
        break ;
      default :
        cerr << "**FEHLER (FATAL) beim Verfeinern am Randst\"uck mit der Regel [" << r << "] in " << __FILE__ << " " << __LINE__ << endl ;
        abort () ;
        break ;
    }
    this->postRefinement () ;
    {for (innerbndseg_t * b = down () ; b ; b = b->next ()) b->restoreFollowFace () ; }
  }
  return ;
}

// #######                                 #######
//    #     ######   #####  #####     ##      #      ####   #####
//    #     #          #    #    #   #  #     #     #    #  #    #
//    #     #####      #    #    #  #    #    #     #    #  #    #
//    #     #          #    #####   ######    #     #    #  #####
//    #     #          #    #   #   #    #    #     #    #  #
//    #     ######     #    #    #  #    #    #      ####   #

template < class A > TetraTop < A > 
:: TetraTop (int l, myhface3_t * f0, int t0,
             myhface3_t * f1, int t1, myhface3_t * f2, int t2, 
             myhface3_t * f3, int t3, innertetra_t *up, int nChild, double vol) 
  : A (f0, t0, f1, t1, f2, t2, f3, t3), 
    _bbb (0), _up(up)
  , _inner( 0 )  
  , _volume( (vol < 0.0) ?  
            quadraturTetra3D < VolumeCalc > (
                LinearMapping ( myvertex(0)->Point(), 
                                myvertex(1)->Point(),
                                myvertex(2)->Point(), 
                                myvertex(3)->Point())).integrate1 (0.0) 
            : vol )
  , _lvl (l) 
  , _nChild(nChild)
  , _rule (myrule_t :: nosplit)
{
  // vxMap is set by the setNewMapping routine 
  _vxMap[ 0 ] = _vxMap[ 1 ] = _vxMap[ 2 ] = _vxMap[ 3 ] = -1;

  // set level 
  assert( this->level() == l );
  
  // _up wird im Constructor uebergeben
  this->setIndex( indexManager().getIndex() );

  // we need boundary id now, for elements is the same as fathers 
  this->_bndid = _up->bndId();

#ifndef NDEBUG 
  // check that _volume has the correct value 
  const double calculatedVolume = 
    std::abs( quadraturTetra3D < VolumeCalc > (
      LinearMapping ( myvertex(0)->Point(), 
                      myvertex(1)->Point(),
                      myvertex(2)->Point(), 
                      myvertex(3)->Point())).integrate1 (0.0) );
  //if( std::abs( calculatedVolume - _volume ) >1e-10 ) 
  //  cout << "Determinant of Tetra[" << this->getIndex() << "] is wrong" << endl;
  assert( std::abs( calculatedVolume - _volume ) / _volume  < 1e-10 ); 
#endif
}

// constrcutor mit IndexManager uebergabe
// this is the macro element constructor 
template < class A > TetraTop < A > :: 
TetraTop (int l,  // level 
          myhface3_t * f0, int t0, // face, twist 
          myhface3_t * f1, int t1, // face, twist
          myhface3_t * f2, int t2, // face, twist
          myhface3_t * f3, int t3, // face, twist
          int orientation ) 
  : A (f0, t0, f1, t1, f2, t2, f3, t3)
  , _bbb (0), _up(0)
  , _inner( 0 )  
  , _volume( quadraturTetra3D < VolumeCalc > 
    (LinearMapping ( myvertex(0)->Point(), myvertex(1)->Point(),
                     myvertex(2)->Point(), myvertex(3)->Point())).integrate1 (0.0) )
  , _lvl (l) 
  , _nChild(0)  // we are macro ==> nChild 0 
  , _rule (myrule_t :: nosplit)
{ 
  assert( this->level() == l );

  // _up wird im Constructor uebergeben
  this->setIndex( indexManager().getIndex() );

  // initial mapping is has to be adjusted according 
  // to the make-6 algorithm 
  // NOTE: the _vxMap numbers only affect the bisection refinement
  std::cout << "Create Tetra with orientation " << orientation << endl;
  const int mod = 1 - orientation ;
  _vxMap[ 0 ] = 0; 
  _vxMap[ 1 ] = 1;
  _vxMap[ 2 ] = 2 + mod ;
  _vxMap[ 3 ] = 3 - mod ;
}

template < class A > TetraTop < A > :: ~TetraTop () 
{
  this->freeIndex( indexManager() );
  // attachleafs is called in constructor of TetraEmpty
  // if delete is called on macro we only call this method on leaf
  if (! _inner ) this->detachleafs();
  if (_bbb) delete _bbb ;
  if (_inner) delete _inner ;
}

//- --subedge1
template < class A >  typename TetraTop < A > :: myhedge1_t * TetraTop < A > :: subedge1 (int face, int edge) 
{
  switch ( myhface3( face )->getrule() ) 
  {
  case myhface3_t :: myrule_t :: e01 :
    assert( edge == 0 );  // for bisection we only get one subEdge of a face
    return myhface3 ( face )->subedge1 ( edge ) ;
  case myhface3_t :: myrule_t :: e12 :
    assert( edge == 0 );  // for bisection we only get one subEdge of a face
    return myhface3 ( face )->subedge1 ( edge ) ;
  case myhface3_t :: myrule_t :: e20 :
    assert( edge == 0 );  // for bisection we only get one subEdge of a face
    return myhface3 ( face )->subedge1 ( edge ) ;
  case myhface3_t :: myrule_t :: iso4 :
    assert( edge < 3 );
    return ((twist ( face ) < 0) ? myhface3 ( face )->subedge1 ((8 - edge + twist ( face )) % 3) : myhface3 ( face )->subedge1 ((edge + twist ( face )) % 3)) ;
  case myhface3_t :: myrule_t :: nosplit :
    cerr << "**ERROR (FATAL): subedge1 () called on non-refined face. In " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
    return 0 ;
  }
  return 0 ;
}

template < class A >  const typename TetraTop < A > :: myhedge1_t * TetraTop < A > :: subedge1 (int i, int j) const {
  return ((TetraTop < A > *)this)->subedge1 (i,j) ;
}


// --subFaces
template < class A >  
typename TetraTop < A > :: facepair_t
TetraTop < A > :: subFaces ( const int i ) 
{
  // get face that we want sub faces from 
  myhface3_t* face = myhface3( i );

  typedef typename myhface3_t :: myrule_t  myrule_t;
  // get split rule of face 
  const myrule_t rule = face->getrule() ;
  assert( rule == myrule_t :: e01 || 
          rule == myrule_t :: e12 || 
          rule == myrule_t :: e20 );
  assert( -3 <= twist( i ) && twist( i ) <= 2 ); 

#ifndef NDEBUG
  if( rule == myrule_t :: iso4 )
  {
    cerr << "**ERROR (FATAL): subFaces () not implemented for iso4 " << i << ". In " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  else if ( rule == myrule_t :: nosplit )
  {
    cerr << "**ERROR (FATAL): subFaces () called on non-refined face " << i << ". In " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
#endif

  // obtain rule id 
  const unsigned int ruleId = int(rule) - 2;
  assert( ruleId <= 2 );

  // cout << "subFaces rule " << ruleId << " of face " << i << " with twist " << twist( i ) << endl;

  //                                twists  -3  -2  -1   0   1   2 
  static const int subFace[ 3 ][ 6 ] = {  {  1,  1,  0,  0,  0,  1  }, // rule e01 
                                          {  1,  0,  0,  0,  1,  1  }, // rule e12  
                                          {  0,  0,  0,  0,  1,  1  }  // rule e20 
                                       }; 
  // sub face 0 and 1 
  unsigned int sub0 = subFace[ ruleId ][ twist( i ) + 3 ];

  /*
  if( elementType () == 0 && ruleId == 2 && twist( i ) == 0 ) 
  {
    sub0 = 0;
  }
  */
  // return sub face 0 and 1 of face i
  return facepair_t ( face->subface3( sub0 ), face->subface3( ! sub0 ) );
}

// --subFaces
template < class A >  
typename TetraTop < A > :: facepair_t
TetraTop < A > :: subFaces ( const int i, 
                             const myvertex_t* vx0, 
                             const myvertex_t* vx1 ) 
{
  assert( vx0 );
  assert( vx1 );

  // get face that we want sub faces from 
  myhface3_t* face = myhface3( i );

  // get sub faces 
  myhface3_t* subFce[ 2 ] = { face->subface3( 0 ), face->subface3( 1 ) };

  // check face order with vertex0
  int sub0 = 1 ;
  for( int i=0; i<3; ++i ) 
  {
    if( subFce[ 0 ]->myvertex( i ) == vx0 ) 
    {
      sub0 = 0;
      break;
    }
  }

#ifndef NDEBUG 
  // make sure the vertex is on the other face 
  bool found0 = false ;
  bool found1 = false ;
  // check face order with vertex0
  for( int i=0; i<3; ++i ) 
  {
    if( subFce[ sub0 ]->myvertex( i ) == vx0 ) 
    {
      found0 = true ;
    }
    if( subFce[ ! sub0 ]->myvertex( i ) == vx1 ) 
    {
      found1 = true ;
    }
  }
  if( ! found0 || ! found1 ) 
  {
    cout << "Problem: " << face << endl;
    cout << " vx0 " << vx0 << endl;
    cout << " vx1 " << vx1 << endl;
    cout << "sub0 " << subFce[ sub0 ] << endl;
    cout << "sub1 " << subFce[ ! sub0 ] << endl;
  }
  assert( found0 );
  assert( found1 );

#endif
  return facepair_t( subFce[ sub0 ], subFce[ ! sub0 ] );
}

// --subface3
template < class A >  typename TetraTop < A > ::  myhface3_t * TetraTop < A > :: subface3 (int i, int j) 
{
  switch ( myhface3( i )->getrule() ) 
  {
  case myhface3_t :: myrule_t :: e01 :
    assert( j < 2 );
    if ( twist(i) == 0 ||  twist(i) == 1 ||  twist(i) == -1 )
      return myhface3(i)->subface3( j ) ;
    if ( twist(i) == 2 ||  twist(i) == -2 || twist(i) == -3 )
      return myhface3(i)->subface3(!j) ;
    cerr << __FILE__ << " " << __LINE__ << "myhface3(i)->subface3()" << endl;
    assert( false );
    return 0;
  case myhface3_t :: myrule_t :: e12 :
    assert( j < 2 );
    if ( twist(i) == 0 ||  twist(i) == 2 ||  twist(i) == -3 )
      return myhface3(i)->subface3(j) ;
    if ( twist(i) == -1 || twist(i) == 1 ||  twist(i) == -2 )
      return myhface3(i)->subface3(!j) ;
    cerr << __FILE__ << " " << __LINE__ << "myhface3(i)->subface3()" << endl;
    return 0;
  case myhface3_t :: myrule_t :: e20 :
    assert( j < 2 );
    if ( twist(i) == 1 ||  twist(i) == 2 ||  twist(i) == -2 )
      return myhface3(i)->subface3(j) ;
    if ( twist(i) == 0 ||  twist(i) == -1 || twist(i) == -3 )
      return myhface3(i)->subface3(!j) ;
    cerr << __FILE__ << " " << __LINE__ << "myhface3(i)->subface3()" << endl;
    return 0;
  case myhface3_t :: myrule_t :: iso4 :
    assert( j < 4 );
    if ( j == 3 )
      return myhface3(i)->subface3(3);
    if ( j < 3 )
      return myhface3(i)->subface3(twist(i) < 0 ? (7 - j + twist(i)) % 3 : (j + twist(i)) % 3) ;
  case myhface3_t :: myrule_t :: nosplit :
    cerr << "**ERROR (FATAL): subface3 () called on non-refined face. In " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
    return 0 ;
  default:
    cerr << "**FEHLER (FATAL): Falsche Verfeinerungsregel [" << myhface3(i)->getrule() << "] in " << __FILE__ << " " << __LINE__ << endl ;
    abort() ;
  } 
  return 0 ;
}

template < class A >  const typename TetraTop < A > ::  myhface3_t * TetraTop < A > :: subface3 (int i, int j) const {
  return ((TetraTop < A > *)this)->subface3 (i,j) ;
}

template < class A >  void TetraTop < A > :: split_e01 () 
{
  assert( _inner == 0 );
  const int newLevel = 1 + this->level();

  splitInfo( myrule_t :: e01 );

  myhedge1_t* subEdge = this->subedge1 (2, 0);
  myhedge1_t* subEdge2 = this->subedge1 (3, 0);
  myhedge1_t* orgEdge = this->myhedge1( 5 ) ;
 
  const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

  // new inner face 
  innerface_t * newFace = 
    new innerface_t (newLevel, 
                     subEdge,  0, // from face 2 get subedge 0  
                     orgEdge,  edgeTwst,
                     subEdge2, 1 // from face 1 get subedge 0
                    ) ;
  assert( newFace );

  // cout << "New inner " << newFace << endl;

  //facepair_t subFace2 = subFaces( 2 ); // get sub face 0 and 1 of face 2
  //facepair_t subFace3 = subFaces( 3 ); // get sub face 0 and 1 of face 3

  facepair_t subFace2 = subFaces( 2, myvertex( 0 ), myvertex( 1 ) ); // get sub face 0 and 1 of face 2
  facepair_t subFace3 = subFaces( 3, myvertex( 0 ), myvertex( 1 ) ); // get sub face 0 and 1 of face 3

  //cout << "Sub face 2 : " << endl << subFace2.first << subFace2.second << endl;
  //cout << "Sub face 3 : " << endl << subFace3.first << subFace3.second << endl;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  /* 
    
    3               2
       ___________
      |3         2|     new inner face ( 4, 3 , 2 )
      |*\       .*|     
      |  \     .  |     child 0 is the child which contains node 0 
      | * \   . * |     child 1 is the child which contains node 1 
      |    \ .    |
      |  *  \  *  |      
      |    . \    |     4 becomes node 1 in child 0
      |   *.  \   |     4 becomes node 0 in child 1
      |  .     \  |
      | .  * *  \ |
      |0   1 0   1|
      ------*------
    0       4       1

  */

  innertetra_t * h0 = new innertetra_t (newLevel, 
                                        newFace, 0, 
                                        myhface3( 1 ),  twist( 1 ),
                                        subFace2.first, twist( 2 ),
                                        subFace3.first, twist( 3 ), 
                                        this, 0, childVolume) ;

  innertetra_t * h1 = new innertetra_t (newLevel, 
                                        myhface3( 0 ), twist( 0 ),
                                        newFace, -1,
                                        subFace2.second, twist( 2 ),
                                        subFace3.second, twist( 3 ),
                                        this, 1, childVolume) ;

  assert(h0 && h1) ;

  // the new vertices are the ones that are missing
  // i.e. 3 in child 0  and  0 in child 1 
  assert( h0->myvertex( 0 ) == myvertex( 0 ) );
  assert( h0->myvertex( 2 ) == myvertex( 2 ) );
  assert( h0->myvertex( 3 ) == myvertex( 3 ) );

  assert( h1->myvertex( 1 ) == myvertex( 1 ) );
  assert( h1->myvertex( 2 ) == myvertex( 2 ) );
  assert( h1->myvertex( 3 ) == myvertex( 3 ) );

  // this is always the edge combo, i.e. if we 
  // split e30 then 3 is new in child 0 and 0 is new in child 1 
  assert( h0->myvertex( 1 ) == h1->myvertex( 0 ) );

  // remap vertices of children 
  setNewMapping( h0, h1, newFace, 1, 0);

  // store refinement rule that was used to split this tetra 
  _rule = myrule_t :: e01;
  return ;
}

// --split_e12
template < class A >  void TetraTop < A > :: split_e12 () 
{
  assert( _inner == 0 );
  const int newLevel = 1 + this->level();

  splitInfo( myrule_t :: e12 );

  myhedge1_t* subEdge = this->subedge1 (3, 0);
  myhedge1_t* subEdge2 = this->subedge1 (0, 0);
  myhedge1_t* orgEdge = this->myhedge1( 2 ) ;
 
  const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 1 : 0;

  // new inner face 
  innerface_t * newFace = 
    new innerface_t (newLevel, 
                     subEdge,  1, // from face 1 get subedge 0
                     subEdge2, 0, // from face 2 get subedge 0  
                     orgEdge, edgeTwst
                    ) ;
  assert( newFace );

  //cout << "New inner " << newFace << endl;

  facepair_t subFace0 = subFaces( 0, myvertex( 1 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 0
  facepair_t subFace3 = subFaces( 3, myvertex( 1 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 3

  //cout << "Sub face 1 : " << endl << subFace0.first << subFace0.second << endl;
  //cout << "Sub face 3 : " << endl << subFace3.first << subFace3.second << endl;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  /* 
    
    3               2
       ___________
      |3         2|     new inner face ( 0, 4 , 3 )
      | \ *     . |     
      |  \   *  . |     child 0 is the child which contains node 0 
      |   \   .* 1|     child 1 is the child which contains node 1 
      |    \ .    * 4
      |     \   *2|      
      |    . \*   |     4 becomes node 2 in child 0
      |   . * \   |     4 becomes node 1 in child 1
      |  .*    \  |
      | *       \ |
      *0         1|
      -------------
    0               1

  */

  innertetra_t * h0 = new innertetra_t (newLevel, 
                                        subFace0.first, twist( 0 ),
                                        newFace, 0, 
                                        myhface3( 2 ),  twist( 2 ),
                                        subFace3.first, twist( 3 ), 
                                        this, 0, childVolume) ;

  innertetra_t * h1 = new innertetra_t (newLevel, 
                                        subFace0.second, twist( 0 ),
                                        myhface3( 1 ), twist( 1 ),
                                        newFace, -1,
                                        subFace3.second, twist( 3 ),
                                        this, 1, childVolume) ;

  assert(h0 && h1) ;

  // the new vertices are the ones that are missing
  // i.e. 3 in child 0  and  0 in child 1 
  assert( h0->myvertex( 0 ) == myvertex( 0 ) );
  assert( h0->myvertex( 1 ) == myvertex( 1 ) );
  assert( h0->myvertex( 3 ) == myvertex( 3 ) );

  assert( h1->myvertex( 0 ) == myvertex( 0 ) );
  assert( h1->myvertex( 2 ) == myvertex( 2 ) );
  assert( h1->myvertex( 3 ) == myvertex( 3 ) );

  // this is always the edge combo, i.e. if we 
  // split e30 then 3 is new in child 0 and 0 is new in child 1 
  assert( h0->myvertex( 2 ) == h1->myvertex( 1 ) );

  // remap vertices of children 
  setNewMapping( h0, h1, newFace, 1, 2 );

  // set refinement rule that was used to refine this tetra 
  _rule = myrule_t :: e12 ;
}

template < class A >  void TetraTop < A > :: split_e20 () 
{
  assert( _inner == 0 );
  const int newLevel = 1 + this->level();

  splitInfo( myrule_t :: e20 );

  myhedge1_t* subEdge2 = this->subedge1 (1, 0);
  myhedge1_t* subEdge = this->subedge1 (3, 0);
  myhedge1_t* orgEdge = this->myhedge1( 4 ) ;
 
  const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

  // new inner face 
  innerface_t * newFace = 
    new innerface_t (newLevel, 
                     orgEdge, edgeTwst,
                     subEdge2, 1, // from face 2 get subedge 0  
                     subEdge,  0 // from face 1 get subedge 0
                    ) ;
  assert( newFace );

  // cout << "New inner face " << newFace << endl;

  //facepair_t subFace1 = subFaces( 1 ); // get sub face 0 and 1 of face 1
  //facepair_t subFace3 = subFaces( 3 ); // get sub face 0 and 1 of face 3

  facepair_t subFace1 = subFaces( 1, myvertex( 0 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 0
  facepair_t subFace3 = subFaces( 3, myvertex( 0 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 1

  //cout << "Sub face 1 : " << endl << subFace1.first << subFace1.second << endl;
  //cout << "Sub face 3 : " << endl << subFace3.first << subFace3.second << endl;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  /* 
    
    3       4       2
       ___________
      |3         2|     new inner face ( 1, 4 , 2 )
      |*\       . |     
      |  \     .  |     child 0 is the child which contains node 0 
      | * \   .   |     child 1 is the child which contains node 2 
      |    \ .    |
      |  *  \     |      
      |   0. \    |     4 becomes node 2 in child 0
      |  4*   \   |     4 becomes node 0 in child 1
      |  .2 *  \  |
      | .     * \ |
      |0         1|
      -------------
    0               1

  */

  innertetra_t * h0 = new innertetra_t (newLevel, 
                                        newFace, 0, 
                                        subFace1.first, twist( 1 ),
                                        myhface3( 2 ),  twist( 2 ),
                                        subFace3.first, twist( 3 ), 
                                        this, 0, childVolume) ;

  innertetra_t * h1 = new innertetra_t (newLevel, 
                                        myhface3( 0 ), twist( 0 ),
                                        subFace1.second, twist( 1 ),
                                        newFace, -2,
                                        subFace3.second, twist( 3 ),
                                        this, 1, childVolume) ;

  assert(h0 && h1) ;

  // the new vertices are the ones that are missing
  // i.e. 3 in child 0  and  0 in child 1 
  assert( h0->myvertex( 0 ) == myvertex( 0 ) );
  assert( h0->myvertex( 1 ) == myvertex( 1 ) );
  assert( h0->myvertex( 3 ) == myvertex( 3 ) );

  assert( h1->myvertex( 1 ) == myvertex( 1 ) );
  assert( h1->myvertex( 2 ) == myvertex( 2 ) );
  assert( h1->myvertex( 3 ) == myvertex( 3 ) );

  // this is always the edge combo, i.e. if we 
  // split e30 then 3 is new in child 0 and 0 is new in child 1 
  assert( h0->myvertex( 2 ) == h1->myvertex( 0 ) );
  
  // remap vertices of children 
  setNewMapping( h0, h1, newFace, 2, 0 );

  // set refinement rule that was used to refine this tetra 
  _rule = myrule_t :: e20;
}

template < class A >  void TetraTop < A > :: split_e23 () 
{
  assert( _inner == 0 );
  const int newLevel = 1 + this->level();

  splitInfo( myrule_t :: e23 );

  myhedge1_t* subEdge2 = this->subedge1 (1, 0);
  myhedge1_t* subEdge = this->subedge1 (0, 0);
  myhedge1_t* orgEdge = this->myhedge1( 0 ) ;
 
  const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

  // new inner face 
  innerface_t * newFace = 
    new innerface_t (newLevel, 
                     subEdge2, 1, // from face 2 get subedge 0  
                     subEdge,  0, // from face 1 get subedge 0
                     orgEdge, edgeTwst 
                    ) ;
  assert( newFace );

  //cout << "New inner " << newFace << endl;

  facepair_t subFace0 = subFaces( 0, myvertex( 2 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 0
  facepair_t subFace1 = subFaces( 1, myvertex( 2 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 1

  //cout << "Sub face 0 : " << endl << subFace0.first << subFace0.second << endl;
  //cout << "Sub face 1 : " << endl << subFace1.first << subFace1.second << endl;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  /* 
    
    3       4       2
       ___________
      |3   2*3   2|     new inner face ( 1, 4 , 2 )
      | \       . |     
      |  \ * * .  |     child 0 is the child which contains node 2 
      |   \   .   |     child 1 is the child which contains node 3 
      |   *\ .*   |
      |     \     |      
      |  * . \ *  |     4 becomes node 3 in child 0
      |3  .   \   |     4 becomes node 0 in child 1
      | *.     \* |
      | .       \ |
      |0         1|
      -------------
    0               1

  */

  innertetra_t * h0 = new innertetra_t (newLevel, 
                                        subFace0.first, twist( 0 ), 
                                        subFace1.first, twist( 1 ),
                                        newFace, 0, 
                                        myhface3( 3 ),  twist( 3 ),
                                        this, 0, childVolume) ;

  innertetra_t * h1 = new innertetra_t (newLevel, 
                                        subFace0.second, twist( 0 ),
                                        subFace1.second, twist( 1 ),
                                        myhface3( 2 ),  twist( 2 ),
                                        newFace, -1,
                                        this, 1, childVolume) ;

  assert(h0 && h1) ;

  // the new vertices are the ones that are missing
  // i.e. 3 in child 0  and  0 in child 1 
  assert( h1->myvertex( 0 ) == myvertex( 0 ) );
  assert( h1->myvertex( 1 ) == myvertex( 1 ) );
  assert( h1->myvertex( 3 ) == myvertex( 3 ) );

  assert( h0->myvertex( 0 ) == myvertex( 0 ) );
  assert( h0->myvertex( 1 ) == myvertex( 1 ) );
  assert( h0->myvertex( 2 ) == myvertex( 2 ) );

  // this is always the edge combo, i.e. if we 
  assert( h0->myvertex( 3 ) == h1->myvertex( 2 ) );

  // remap vertices of children 
  setNewMapping( h0, h1, newFace, 2, 3 );

  // set refinement rule that was used to refine this tetra 
  _rule = myrule_t :: e23 ;
}

template < class A >  void TetraTop < A > :: split_e30 () 
{
  assert( _inner == 0 );
  const int newLevel = 1 + this->level();

  splitInfo( myrule_t :: e30 );
  /* 
    
    3               2
       ___________
      |3        *2|     new inner face ( 1, 4 , 2 )
      | \     * . |     
      |  \  *  .  |     child 0 is the child which contains node 0 
      |   \  .    |     child 1 is the child which contains node 3 
      |0*  \.     |
    4 *    .\     |
      | *  . \    |     4 becomes node 3 in child 0
      |3 .*   \   |     4 becomes node 0 in child 1
      | .   *  \  |
      |.      * \ |
      |0         1|
      -------------
    0               1

  */

  myhedge1_t* subEdge2 = this->subedge1 (2, 0);
  myhedge1_t* subEdge = this->subedge1 (1, 0);
  myhedge1_t* orgEdge = this->myhedge1( 3 ) ;
 
  const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

  // new inner face 
  innerface_t * newFace = 
    new innerface_t (newLevel, 
                     subEdge2, 1, // from face 2 get subedge 0  
                     subEdge,  0, // from face 1 get subedge 0
                     orgEdge, edgeTwst 
                    ) ;

  //cout << "New inner face " << newFace << endl;
  assert( newFace ) ;

  facepair_t subFace1 = subFaces( 1, myvertex( 0 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 1
  facepair_t subFace2 = subFaces( 2, myvertex( 0 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 2

  //cout << "Sub face 1 : " << endl << subFace1.first << subFace1.second << endl;
  //cout << "Sub face 2 : " << endl << subFace2.first << subFace2.second << endl;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );

  innertetra_t * h0 = new innertetra_t (newLevel, 
                                        newFace, 0, 
                                        subFace1.first, twist (1), 
                                        subFace2.first, twist (2), 
                                        myhface3(3), twist (3), 
                                        this, 0, childVolume) ;

  innertetra_t * h1 = new innertetra_t (newLevel, 
                                        myhface3( 0 ), twist( 0 ),
                                        subFace1.second, twist (1), 
                                        subFace2.second, twist (2), 
                                        newFace, -3, 
                                        this, 1, childVolume) ;

  assert(h0 && h1) ;

  // the new vertices are the ones that are missing
  // i.e. 3 in child 0  and  0 in child 1 
  assert( h0->myvertex( 0 ) == myvertex( 0 ) );
  assert( h0->myvertex( 1 ) == myvertex( 1 ) );
  assert( h0->myvertex( 2 ) == myvertex( 2 ) );

  assert( h1->myvertex( 1 ) == myvertex( 1 ) );
  assert( h1->myvertex( 2 ) == myvertex( 2 ) );
  assert( h1->myvertex( 3 ) == myvertex( 3 ) );

  // this is always the edge combo, i.e. if we 
  // split e30 then 3 is new in child 0 and 0 is new in child 1 
  assert( h0->myvertex( 3 ) == h1->myvertex( 0 ) );

  // remap vertices of children 
  setNewMapping( h0, h1, newFace, 3, 0 );

  // set refinement rule that was used to refine this tetra 
  _rule = myrule_t :: e30;
}

template < class A >  void TetraTop < A > :: split_e31 () 
{
  assert( _inner == 0 );
  const int newLevel = 1 + this->level () ;

  splitInfo( myrule_t :: e31 );

  myhedge1_t* subEdge2 = this->subedge1 (2, 0);
  myhedge1_t* subEdge = this->subedge1 (0, 0);
  myhedge1_t* orgEdge = this->myhedge1( 1 ) ;
 
  const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 1 : 0;

  // new inner face 
  innerface_t * newFace = 
    new innerface_t (newLevel, 
                     orgEdge, edgeTwst,
                     subEdge, 1,  // from face 1 get subedge 0
                     subEdge2, 0  // from face 2 get subedge 0  
                    ) ;

  assert( newFace ) ;

  //cout << "New inner face " << newFace << endl;

  /* 
    
    3               2
       ___________
      |3         2|     new inner face ( 0, 4, 2 )
      | \       .*|     
      |  \     .  |     child 0 is the child which contains node 1 
      |   \   . * |     child 0 is the child which contains node 3 
      |    \ .    |
      |     \  *  |
      |    . \    |      4 becomes node 1 in child 0
      |   .  1*  <--- 4  4 becomes node 3 in child 1
      |  .  * 3\  |
      | . *     \ |
      |0*       1\|
      -------------
    0               1

  */
  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  //facepair_t subFace0 = subFaces( 0 ); // get sub face 0 and 1 of face 0
  //facepair_t subFace2 = subFaces( 2 ); // get sub face 0 and 1 of face 2
  facepair_t subFace0 = subFaces( 0, myvertex( 1 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 0
  facepair_t subFace2 = subFaces( 2, myvertex( 1 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 1


  //cout << "Sub face 0 : " << endl << subFace0.first << subFace0.second << endl;
  //cout << "Sub face 2 : " << endl << subFace2.first << subFace2.second << endl;

  innertetra_t * h0 = new innertetra_t (newLevel, 
                                        subFace0.first, twist( 0 ),
                                        newFace, 0, 
                                        subFace2.first, twist ( 2 ), 
                                        myhface3( 3 ), twist ( 3 ),
                                        this, 0, childVolume) ;

  innertetra_t * h1 = new innertetra_t (newLevel, 
                                        subFace0.second, twist( 0 ),
                                        myhface3( 1 ), twist( 1 ),
                                        subFace2.second, twist( 2 ), 
                                        newFace, -1, 
                                        this, 1, childVolume) ;

  assert(h0 && h1) ;

  // the new vertices are the ones that are missing
  // i.e. 3 in child 0  and  0 in child 1 
  assert( h0->myvertex( 0 ) == myvertex( 0 ) );
  assert( h0->myvertex( 1 ) == myvertex( 1 ) );
  assert( h0->myvertex( 2 ) == myvertex( 2 ) );

  assert( h1->myvertex( 0 ) == myvertex( 0 ) );
  assert( h1->myvertex( 2 ) == myvertex( 2 ) );
  assert( h1->myvertex( 3 ) == myvertex( 3 ) );

  // this is always the edge combo, i.e. if we 
  // split e30 then 3 is new in child 0 and 0 is new in child 1 
  assert( h0->myvertex( 3 ) == h1->myvertex( 1 ) );
  
  // remap vertices of children 
  setNewMapping( h0, h1, newFace, 3, 1 );

  // set refinement rule that was used to refine this tetra 
  _rule = myrule_t :: e31;
}

// --setNewMapping  
template < class A >  void 
TetraTop < A > :: setNewMapping( innertetra_t* h0, innertetra_t* h1, 
                                 innerface_t* newFace,  
                                 const int newVx0, const int newVx1 ) 
{
  // vertex 0 is always containd in child 0, and not in child 1
  myvertex_t* vx0 = this->myvertex( _vxMap[ 0 ] );
  bool found = false ;
  for( int i=0; i<4; ++i ) 
  {
    if( h0->myvertex( i ) == vx0 )
    {
      found = true ;
      break ;
    }
  }

  // if vx0 was not found in child 0 we have to swap the children 
  innertetra_t* t0 = ( found ) ? h0 : h1;
  innertetra_t* t1 = ( found ) ? h1 : h0;

  if( stevensonRefinement_ ) 
  {
    ///////////////////////////////////////////////////
    //  Stevenson refinement, always refine edge 0--3
    ///////////////////////////////////////////////////

    // set vertex mapping (child 0)
    t0->_vxMap[ 0 ] = _vxMap[ 0 ]; 
    t0->_vxMap[ 1 ] = _vxMap[ 3 ];
    t0->_vxMap[ 2 ] = _vxMap[ 1 ];
    t0->_vxMap[ 3 ] = _vxMap[ 2 ];

    // set vertex mapping (child 1)
    t1->_vxMap[ 0 ] = _vxMap[ 3 ]; 
    t1->_vxMap[ 1 ] = _vxMap[ 0 ];
    const char face3 = ( elementType () == 0 ) ? 1 : 0;
    t1->_vxMap[ 2 ] = _vxMap[ 1 + face3 ]; // for type 0   2 else 1 
    t1->_vxMap[ 3 ] = _vxMap[ 2 - face3 ]; // for type 0   1 else 2 
  }
  else 
  {
    ///////////////////////////////////////////////////
    //  ALBERTA refinement, always refine edge 0--1
    ///////////////////////////////////////////////////

    // set vertex mapping (child 0)
    t0->_vxMap[ 0 ] = _vxMap[ 0 ]; 
    t0->_vxMap[ 1 ] = _vxMap[ 2 ];
    t0->_vxMap[ 2 ] = _vxMap[ 3 ];
    t0->_vxMap[ 3 ] = _vxMap[ 1 ];

    // set vertex mapping (child 1)
    t1->_vxMap[ 0 ] = _vxMap[ 1 ]; 
    t1->_vxMap[ 3 ] = _vxMap[ 0 ];
    const char face3 = ( elementType () == 0 ) ? 1 : 0;
    t1->_vxMap[ 1 ] = _vxMap[ 2 + face3 ]; // for type 0   2 else 1 
    t1->_vxMap[ 2 ] = _vxMap[ 3 - face3 ]; // for type 0   1 else 2 
  }

#ifndef NDEBUG
  /*
  cout << "Map0 = ( " ;
  for( int i=0; i<4 ; ++ i )
  {
    cout << int(h0->_vxMap[i]) << " " ;
  }
  cout << " ) " << endl;
  cout << "Map1 = ( " ;
  for( int i=0; i<4 ; ++ i )
  {
    cout << int(h1->_vxMap[i]) << " " ;
  }
  cout << " ) " << endl;
  */

  for(int i=0; i<4; ++i ) 
  {
    for(int j=i+1; j<4; ++ j) 
    {
      if( i != j ) 
      {
        assert( h0->_vxMap[ i ] != h0->_vxMap[ j ] );
        assert( h1->_vxMap[ i ] != h1->_vxMap[ j ] );
      }
    }
  }
#endif

  //cout << "New tetra " << h0 << endl;
  assert( checkTetra( h0, 0 ) );

  //cout << "New tetra " << h1 << endl;
  assert( checkTetra( h1, 1 ) );

  //cout << "For Tetra[" << h0->getIndex() << "] we suggest " << h0->suggestRule() << endl;
  //cout << "For Tetra[" << h1->getIndex() << "] we suggest " << h1->suggestRule() << endl;

  // append h1 to h0 
  h0->append( h1 );

  // create inner storage with first child and new face 
  _inner = new inner_t( h0, newFace ); 
  assert( _inner );

  // detach myself from being leaf element 
  this->detachleafs();
}


template < class A >  int 
TetraTop < A > :: vertexTwist( const int twst, const int vx ) const 
{
  return twst < 0 ? (7-vx+twst)%3 : (vx+twst)%3;
}

template < class A > int 
TetraTop < A > :: calculateFace2Twist( const int vxIndex, const myhface3_t* subFace ) const 
{
  const int faceIndices[ 3 ] = { subFace->myvertex( 0 )->getIndex(),
                                 subFace->myvertex( 1 )->getIndex(),
                                 subFace->myvertex( 2 )->getIndex() };

  for(int twst = -3; twst<3; ++twst ) 
  {
    // search for vx 1 
    if( vxIndex == faceIndices[ vertexTwist(twst, 1) ] )
    {
      return twst;
    }
  }

  cout << "Valid twist not found!!!" << endl;
  return 0;
  // we should not get here 
  assert( false );
  abort();
  return -5;
}

template < class A > int 
TetraTop < A > :: calculateFace3Twist( const int (&vx)[2], const myhface3_t* subFace, const int secondVx ) const 
{
  //cout << "check v0 = " << vx[0] << " v1 = " << vx[1] << endl;

  const int faceIndices[ 3 ] = { subFace->myvertex( 0 )->getIndex(),
                                 subFace->myvertex( 1 )->getIndex(),
                                 subFace->myvertex( 2 )->getIndex() };
  //cout << faceIndices[0] << " " << faceIndices[1] << " " << faceIndices[2] << " " << endl;

  for(int twst = -3; twst<3; ++twst ) 
  {
    // if vx0 and vx1 match we are done 
    if( vx[ 0 ] == faceIndices[ vertexTwist(twst, 0) ] && 
        vx[ 1 ] == faceIndices[ vertexTwist(twst, secondVx ) ] )
    {
      return twst;
    }
  }

  cout << "Valid twist not found!!!" << endl;
  return 0;

  // we should not get here 
  assert( false );
  abort();
  return -5;
}

// --checkTetra
template < class A > bool 
TetraTop < A > :: checkTetra( const innertetra_t *tetra, const int nChild ) const 
{
  // make sure face twists are ok 
  bool twistOk = true ;

  set< int > verticesFound ;
  assert( tetra->nChild() == nChild );

  const bool isGhost = tetra->isGhost();
  for(int fce=0; fce<4; ++fce ) 
  {
    for(int i=0; i<3; ++i ) 
    {
      verticesFound.insert( tetra->myvertex( fce, i )->getIndex() ); 
    }

    for(int i=0; i<3; ++i ) 
    {
      verticesFound.insert( tetra->myvertex( fce, i )->getIndex() ); 
      // use proto type to check face twists 
      if( tetra->myvertex( Gitter :: Geometric :: Tetra :: prototype[ fce ][ i ] ) 
            != tetra->myvertex( fce, i ) )
      {
        const int vx0 = Gitter :: Geometric :: Tetra :: prototype[ fce ][ 0 ] ;
        const int vx1 = Gitter :: Geometric :: Tetra :: prototype[ fce ][ 1 ] ;

        const int vx[2] = { tetra->myvertex( vx0 )->getIndex(),
                            tetra->myvertex( vx1 )->getIndex() 
                          };

        int twst = calculateFace3Twist( vx, tetra->myhface3( fce ), 1 ); 
        cout << "Twist is wrong, it should be " << twst << endl;
        twistOk = false ;
        continue ;
      }
    }

    if( ! isGhost && ! tetra->myneighbour( fce ).first->isRealObject()  ) 
    {
      cout << "Neighbour(type="<<tetra->isInterior() << ") " << fce << " of Tetra " << tetra->getIndex()  << " is wrong " << endl;
      cout << "Check face " << tetra->myhface3( fce )->getIndex() << endl;
    }
    // make sure neighbor is something meaningful 
    //assert( tetra->myneighbour( fce ).first->isRealObject() );
  }
  
  // make sure we have only 4 different vertices 
  assert( verticesFound.size() == 4 );

  return twistOk;
}

// --bisect
template < class A >  void TetraTop < A > :: bisect () 
{
  abort();
}

template < class A >  void TetraTop < A > :: 
splitISO8 () 
{
  assert( _inner == 0 );
  typedef typename A :: myvertex_t  myvertex_t;
  typedef typename A :: inneredge_t inneredge_t;
  const int l = 1 + this->level () ; 

  myvertex_t * e31 = myhface3 (0)->myhedge1 ((twist(0) < 0) ? ((9+twist(0))%3) : (twist(0)%3))->subvertex (0) ;
  myvertex_t * e20 = myhface3 (1)->myhedge1 ((twist(1) < 0) ? ((9+twist(1))%3) : (twist(1)%3))->subvertex (0) ;
  assert(e31 && e20);
  inneredge_t * e0 = new inneredge_t (l, e31, e20) ;
  assert(e0) ;
  innerface_t * f0 = new innerface_t (l, subedge1 (3, 2), ((twist(3)>=0)?1:0), subedge1 (1, 2), ((twist(1)>=0)?1:0), subedge1 (2, 2), ((twist(2)>=0)?1:0)) ;
  innerface_t * f1 = new innerface_t (l, subedge1 (3, 0), ((twist(3)>=0)?1:0), subedge1 (2, 1), ((twist(2)>=0)?1:0), subedge1 (0, 2), ((twist(0)>=0)?1:0)) ;
  innerface_t * f2 = new innerface_t (l, subedge1 (3, 1), ((twist(3)>=0)?1:0), subedge1 (0, 1), ((twist(0)>=0)?1:0), subedge1 (1, 0), ((twist(1)>=0)?1:0)) ;
  innerface_t * f3 = new innerface_t (l, subedge1 (2, 0), ((twist(2)>=0)?0:1), subedge1 (0, 0), ((twist(0)>=0)?0:1), subedge1 (1, 1), ((twist(1)>=0)?0:1)) ;
  innerface_t * f4 = new innerface_t (l, e0, 0, subedge1 (3, 2), ((twist(3)>=0)?0:1), subedge1 (2, 1), ((twist(2)>=0)?1:0)) ;
  innerface_t * f5 = new innerface_t (l, e0, 0, subedge1 (3, 1), ((twist(3)>=0)?1:0), subedge1 (0, 2), ((twist(0)>=0)?0:1)) ; 
  innerface_t * f6 = new innerface_t (l, e0, 0, subedge1 (1, 0), ((twist(1)>=0)?0:1), subedge1 (0, 0), ((twist(0)>=0)?1:0)) ;
  innerface_t * f7 = new innerface_t (l, e0, 0, subedge1 (1, 2), ((twist(1)>=0)?1:0), subedge1 (2, 0), ((twist(2)>=0)?0:1)) ;
  assert(f0 && f1 && f2 && f3 && f4 && f5 && f6 && f7) ;
  f0->append(f1) ;
  f1->append(f2) ;
  f2->append(f3) ;
  f3->append(f4) ;
  f4->append(f5) ;
  f5->append(f6) ;
  f6->append(f7) ;

  // we divide by 8 means we divide the volume by 8
  const double childVolume = calculateChildVolume( 0.125 * _volume );
  
  // pointer `this' is the pointer to the father element 
  innertetra_t * h0 = new innertetra_t (l, f0, -1, subface3(1, 0), twist(1), subface3(2, 0), twist(2), subface3(3, 0), twist(3), this, 0 , childVolume) ;
  innertetra_t * h1 = new innertetra_t (l, subface3(0, 0), twist(0), f1, -3, subface3(2, 2), twist(2), subface3(3, 1), twist(3), this, 1 , childVolume) ;
  innertetra_t * h2 = new innertetra_t (l, subface3(0, 2), twist(0), subface3(1, 1), twist(1), f2, -1, subface3(3, 2), twist(3), this, 2 , childVolume) ;
  innertetra_t * h3 = new innertetra_t (l, subface3(0, 1), twist(0), subface3(1, 2), twist(1), subface3(2, 1), twist(2), f3, 0,  this, 3 , childVolume) ;
  innertetra_t * h4 = new innertetra_t (l, f7, -3, subface3(2, 3), ((twist(2)>=0) ? ((twist(2)+2)%3) : twist(2)) , f4, 2, f0, 0, this, 4 , childVolume) ;  
  innertetra_t * h5 = new innertetra_t (l, f4, -3, f1, 0, f5, 2, subface3(3, 3), ((twist(3)>=0) ? (twist(3)+1)%3 : (twist(3)-1)%3-1), this, 5 , childVolume) ;
  innertetra_t * h6 = new innertetra_t (l, f3, -1, f6, -3, subface3(1, 3), ((twist(1)>=0) ? twist(1) : twist(1)%3-1), f7, 1, this, 6 , childVolume) ;
  innertetra_t * h7 = new innertetra_t (l, subface3(0, 3), ((twist(0)>=0) ? (twist(0)+1)%3 : (twist(0)-1)%3-1), f5, -3, f2, 0, f6, 1, this, 7 , childVolume) ;
  assert(h0 && h1 && h2 && h3 && h4 && h5 && h6 && h7) ;
  h0->append(h1) ;
  h1->append(h2) ;
  h2->append(h3) ;
  h3->append(h4) ;
  h4->append(h5) ;
  h5->append(h6) ;
  h6->append(h7) ;
  _inner = new inner_t( h0, f0, e0 ); 
  assert( _inner );
  _rule = myrule_t :: iso8 ;
  
  this->detachleafs();
  return ;
}

template < class A > TetraTop < A > :: 
BisectionInfo :: BisectionInfo ( myrule_t r ) : _caller( 0 )
{
  switch(r) 
  {
    case myrule_t :: e01 :
      _faces[ 0 ] = 2;   _faceRules[ 0 ] = face3rule_t :: e20;
      _faces[ 1 ] = 3;   _faceRules[ 1 ] = face3rule_t :: e01;
      _vertices[ 0 ] = 0; _vertices[ 1 ] = 1;
      _caller = new CallSplitImpl< myrule_t :: e01 > ();
      break ;
    case myrule_t :: e12 :
      _faces[ 0 ] = 0;   _faceRules[ 0 ] = face3rule_t :: e20;
      _faces[ 1 ] = 3;   _faceRules[ 1 ] = face3rule_t :: e12;
      _vertices[ 0 ] = 1; _vertices[ 1 ] = 2;
      _caller = new CallSplitImpl< myrule_t :: e12 > ();
      break ;
    case myrule_t :: e20 :
      _faces[ 0 ] = 1;   _faceRules[ 0 ] = face3rule_t :: e01;
      _faces[ 1 ] = 3;   _faceRules[ 1 ] = face3rule_t :: e20;
      _vertices[ 0 ] = 0; _vertices[ 1 ] = 2;
      _caller = new CallSplitImpl< myrule_t :: e20 > ();
      break ;
    case myrule_t :: e23 :
      _faces[ 0 ] = 0;   _faceRules[ 0 ] = face3rule_t :: e12;
      _faces[ 1 ] = 1;   _faceRules[ 1 ] = face3rule_t :: e12;
      _vertices[ 0 ] = 2; _vertices[ 1 ] = 3;
      _caller = new CallSplitImpl< myrule_t :: e23 > ();
      break ;
    case myrule_t :: e30 :
      _faces[ 0 ] = 1;   _faceRules[ 0 ] = face3rule_t :: e20;
      _faces[ 1 ] = 2;   _faceRules[ 1 ] = face3rule_t :: e01;
      _vertices[ 0 ] = 3; _vertices[ 1 ] = 0;
      _caller = new CallSplitImpl< myrule_t :: e30 > ();
      break ;
    case myrule_t :: e31 :
      _faces[ 0 ] = 0;   _faceRules[ 0 ] = face3rule_t :: e01;
      _faces[ 1 ] = 2;   _faceRules[ 1 ] = face3rule_t :: e12;
      _vertices[ 0 ] = 3; _vertices[ 1 ] = 1;
      _caller = new CallSplitImpl< myrule_t :: e31 > ();
      break ;
    default :
      cerr << "**FEHLER (FATAL) beim unbedingten Verfeinern mit unbekannter Regel: " ;
      cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << endl ;
      abort () ;
      break ;
  }
}

// --refineImmediate 
template < class A >  void TetraTop < A > :: refineImmediate (myrule_t r) 
{
  assert (getrule () == myrule_t :: nosplit) ;
  typedef typename myhface3_t :: myrule_t face3rule_t;

  if( r == myrule_t :: iso8 ) 
  {
    // Das refineImmediate (..) auf allen Fl"achen wird vom tetra :: refine (..)
    // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
    // "uberall hface3 :: refine (..) true geliefert hat, wohl aber z.B. von
    // restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
    // erzwingen m"ussen und d"urfen.
      
    {
      for (int i = 0 ; i < 4 ; ++i)
        myhface3 (i)->refineImmediate (face3rule_t (myhface3_t :: myrule_t :: iso4).rotate (twist (i))) ; 
    }
    splitISO8 () ;
  }
  else if( r == myrule_t :: bisect )
  {
    // call refinement with appropriate rule 
    // given by suggestRule 
    // std::cout << "--- refineimmediately ---" << std::endl;
    // for( int qq = 0; qq < 4; ++qq )
    //   std::cout << myvertex(qq) << std::endl;

    BisectionInfo :: splitEdge( this, suggestRule() );
  }
  else 
  {
    // it is assured that r is one out of e01 ... e32 
    // call refinement directly 
    BisectionInfo :: splitEdge( this, r );
  }

  // call post refinement procedure 
  this->postRefinement () ;
  return ;
}


// --refine 
template < class A >  bool TetraTop < A > :: refine () 
{
  myrule_t r = _req ;
  if (r != myrule_t :: crs && r != myrule_t :: nosplit) 
  {
    if (r != getrule ()) 
    {
      assert (getrule () == myrule_t :: nosplit) ;
      _req = myrule_t :: nosplit ;
      switch (r) 
      {
        case myrule_t :: crs :
        case myrule_t :: nosplit :
          return true ;
        case myrule_t :: iso8 :
          {
            for (int i = 0 ; i < 4 ; ++i )
              if (!myhface3 (i)->refine (face3rule_t (face3rule_t :: iso4).rotate (twist (i)), twist (i))) return false ; 
          }
          break ;
        case myrule_t :: e01 :
        case myrule_t :: e12 :
        case myrule_t :: e20 :
        case myrule_t :: e23 :
        case myrule_t :: e30 :
        case myrule_t :: e31 :
          if( ! BisectionInfo :: refineFaces( this, r ) ) return false ;
          break ;
        default :
          cerr << "**WARNUNG (FEHLER IGNORIERT) falsche Verfeinerungsregel [" << int(getrule ()) ;
          cerr << "] (ignoriert) in " << __FILE__ << " " << __LINE__ << endl ;
          assert( false );
          return false ;
      }
      
  // Vorsicht: Im Fall eines konformen Verfeinerers mu"s hier die entstandene Verfeinerung
  // untersucht werden und dann erst das Element danach verfeinert werden.
      
      refineImmediate (r) ;
      return true ;
    }
  }
  return true ;
}

template < class A >  bool TetraTop < A > :: refineBalance (balrule_t r, int fce) 
{
  /*
  if ( r != balrule_t :: iso4 || r != balrule_t :: bisect ) 
  {
    //cerr << "**WARNUNG (IGNORIERT) in TetraTop < A > :: refineBalance (..) nachschauen, Datei " 
    //   << __FILE__ << " Zeile " << __LINE__ << endl ;
   
  // Bisher kann die Balancierung nur die isotrope Achtelung handhaben,
  // falls mehr gew"unscht wird muss es hier eingebaut werden. Im Moment wird
  // die Balancierung einfach verweigert, d.h. die Verfeinerung des anfordernden
  // Elements f"allt flach.
    return false ;
  }
  */
  /*
  if ( r != balrule_t :: iso4 || r!= balrule_t :: e01 )
  {
    //cerr << "**WARNUNG (IGNORIERT) in TetraTop < A > :: refineBalance (..) nachschauen, Datei " 
    //   << __FILE__ << " Zeile " << __LINE__ << endl ;
   
  // Bisher kann die Balancierung nur die isotrope Achtelung handhaben,
  // falls mehr gew"unscht wird muss es hier eingebaut werden. Im Moment wird
  // die Balancierung einfach verweigert, d.h. die Verfeinerung des anfordernden
  // Elements f"allt flach.
    return false ;
  }
  */

  // if status is still non-refined 
  if (getrule () == myrule_t :: nosplit) 
  {
    if( r == balrule_t :: iso4 )
    {
      // if face is a leaf face 
      if (! myhface3 (fce)->leaf ()) 
      {
        for (int i = 0 ; i < 4 ; ++i)
        {  
          if (i != fce)
            if ( ! myhface3 (i)->refine (balrule_t ( balrule_t :: iso4 ).rotate (twist (i)), twist (i)) ) 
              return false ;
        }
        _req = myrule_t :: nosplit ;
        refineImmediate (myrule_t :: iso8) ;
      }
    }
    else 
    {
      // if face is a leaf face 
      if (! myhface3 (fce)->leaf ()) 
      {
        /*
        for (int i = 0 ; i < 4 ; ++i)
        {  
          if (i != fce)
            if ( ! myhface3 (i)->refine (balrule_t ( r ).rotate (twist (i)), twist (i)) ) 
              return false ;
        }
        */
        _req = myrule_t :: nosplit ;
        if (! BisectionInfo :: refineFaces( this, suggestRule() ) ) return false ;
        refineImmediate ( myrule_t :: bisect ) ;
      }
    }
  }
  return true ;
}

template < class A >  bool TetraTop < A > :: coarse () 
{
  if (this->leaf ()) 
  {
    assert (_req == myrule_t :: nosplit || _req == myrule_t :: crs) ;
    myrule_t w = _req ;
    _req = myrule_t :: nosplit ;
    // end recursion if rule is not croarsen 
    if (w != myrule_t :: crs) 
    {
      return false ;
    }
   
    // if I have faces that are not leaf, we cannot coarsen 
    for (int i = 0 ; i < 4 ; ++i) 
    {
      if ( ! myhface3 (i)->leaf ()) return false ;
    }
    // else coarsen 
    return true ;
  } 
  else 
  {
    assert (_req == myrule_t :: nosplit) ;
    bool x = true ;
    {
      for (innertetra_t * h = dwnPtr() ; h ; h = h->next ()) x &= h->coarse () ; 
    }

    // if x is true, then all children are marked for coarsening and have
    // not faces that are not leaf 
    if (x) 
    {
      this->preCoarsening () ;
      this->attachleafs();

      delete _inner ; 
      _inner = 0 ;

      // for bisection refinement we have to again 
      // set the face neighbours, since they have been overwritten   
      // by the refined element  
      if( _rule != myrule_t :: iso8 )
      {
        for (int i = 0 ; i < 4 ; ++i )
        {
          // if face number is negative, it's not a real object 
          // in this case we have to set the neighbour again
          if( this->myneighbour( i ).second < 0 ) 
          {
            typedef pair < Gitter :: Geometric :: hface3 :: myconnect_t*, int > myconnectpair_t;
            assert( ! this->myneighbour( i ).first->isRealObject() );
            myhface3( i )->attachElementAgain( 
                  myconnectpair_t( Gitter :: Geometric :: InternalHasFace3()(this), i ),   
                  twist( i ) 
                );
          }
        }
      }

      // reset refinement rule 
      _rule = myrule_t :: nosplit ;
      {
        for (int i = 0 ; i < 4 ; ++i ) 
        {
          this->myneighbour (i).first->bndNotifyCoarsen () ;
          myhface3 (i)->coarse () ;
        }
      }
      return false ;
    }
  }
  return false ;
}

// buckupIndex of tetra  
template < class A > void TetraTop < A > :: backupIndex (ostream & os) const 
{
  this->doBackupIndex( os );

  // write children 
  {
    for (const innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->backupIndex (os) ; 
  }
  return;
}

// buckupIndex of tetra  
template < class A > void TetraTop < A > :: backupIndex (ObjectStream& os) const 
{
  this->doBackupIndex( os );

  // write children 
  {
    for (const innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->backupIndex (os) ; 
  }
  return;
}

// buckupTetra 
template < class A > void TetraTop < A > :: backup (ostream & os) const 
{
  doBackup( os );
}
template < class A > void TetraTop < A > :: backup (ObjectStream& os) const 
{
  doBackup( os );
}

template < class A > template <class OutStream_t> 
void TetraTop < A > :: doBackup (OutStream_t & os) const 
{
  os.put ((char) getrule ()) ;
  {for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ; }
  {for (const innerface_t * f = innerHface () ; f ; f = f->next ()) f->backup (os) ; }
  {for (const innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->backup (os) ; }
  
  return ;
}

// overloaded restoreIndex Method 
template < class A > 
template < class istream_t>
void TetraTop < A > :: 
restoreIndexImpl (istream_t & is, RestoreInfo& restoreInfo ) 
{
  // mark this element a non hole 
  typedef typename Gitter :: Geometric :: BuilderIF BuilderIF; 

  // restore index 
  this->doRestoreIndex( is, restoreInfo, BuilderIF :: IM_Elements );

  // TODO 
  // restore other indices 
  
  {
    for (innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->restoreIndex (is, restoreInfo ) ; 
  }
  return;
}

// overloaded restoreIndex Method 
template < class A > void TetraTop < A > :: 
restoreIndex (istream & is, RestoreInfo& restoreInfo ) 
{
  restoreIndexImpl( is, restoreInfo );
}

// overloaded restoreIndex Method 
template < class A > void TetraTop < A > :: 
restoreIndex (ObjectStream& is, RestoreInfo& restoreInfo ) 
{
  restoreIndexImpl( is, restoreInfo );
}

// restoreTetra
template < class A > void TetraTop < A > :: restore (istream & is) 
{
  doRestore( is );
}

template < class A > void TetraTop < A > :: restore (ObjectStream & is) 
{
  doRestore( is );
}

template < class A > template <class InStream_t>
void TetraTop < A > :: doRestore (InStream_t & is) 
{
  // restore () stellt den Elementbaum aus der Verfeinerungs-
  // geschichte wieder her. Es ruft refine () auf und testet
  // auf den korrekten Vollzug der Verfeinerung. Danach werden
  // die inneren Gitterteile restore'd.
 
  myrule_t r ((char) is.get ()) ;
  assert(getrule() == myrule_t :: nosplit) ;
  if (r == myrule_t :: nosplit) 
  {
    // Vorsicht: beim restore m"ussen sich sowohl Element als auch
    // Randelement um die Korrektheit der Nachbarschaft k"ummern,
    // und zwar dann wenn sie "on the top" sind (= die gelesene
    // Verfeinerungsregel ist nosplit). (s.a. beim Randelement)
    // Die nachfolgende L"osung ist weit davon entfernt, sch"on
    // zu sein - leider. Eventuell wird mit der Verbesserung der
    // Behandlung der nichtkonf. Situationen mal eine "Anderung
    // n"otig.
  
    for (int i = 0 ; i < 4 ; ++i) 
    {
      myhface3_t & f (*(myhface3 (i))) ;
      if (!f.leaf ()) 
      {
        switch (f.getrule ()) 
        {
          case balrule_t :: e01 :
          case balrule_t :: e12 :
          case balrule_t :: e20 :
            { for (int j = 0 ; j < 2 ; j ++) f.subface3 (j)->nb.complete (f.nb) ;}
            break ;
          case balrule_t :: iso4 :
            { for (int j = 0 ; j < 4 ; j ++) f.subface3 (j)->nb.complete (f.nb) ; }
            break ;
          default :
            abort () ;
            break ;
        }
      }
    }
  } 
  else 
  {
    // Auf dem Element gibt es kein refine (myrule_t) deshalb mu"s erst
    // request (myrule_t) und dann refine () durchgef"uhrt werden.
  
    // request read rule 
    request (r) ;
    // refine tetra 
    refine() ;
    
    assert (getrule() == r) ;

    // call restore on inner items 
    { for (inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->restore (is) ; }
    { for (innerface_t * f = innerHface () ; f ; f = f->next ()) f->restore (is) ; }
    
    // call restore on children 
    { 
      for (innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->restore (is) ; 
    }
  }
  
  return ;
}

// ######                                                           #####  #######
// #     #  ######  #####      #     ####   #####      #     ####  #     #   #      ####   #####
// #     #  #       #    #     #    #    #  #    #     #    #    #       #   #     #    #  #    #
// ######   #####   #    #     #    #    #  #    #     #    #       #####    #     #    #  #    #
// #        #       #####      #    #    #  #    #     #    #            #   #     #    #  #####
// #        #       #   #      #    #    #  #    #     #    #    # #     #   #     #    #  #
// #        ######  #    #     #     ####   #####      #     ####   #####    #      ####   #
   
template < class A > typename Periodic3Top < A > :: myhedge1_t * Periodic3Top < A > :: subedge1 (int i, int j) {
  switch (myhface3(i)->getrule()) {
    case myhface3_t :: myrule_t :: e01 :
    case myhface3_t :: myrule_t :: e12 :
    case myhface3_t :: myrule_t :: e20 :
      assert( j == 0 );
      return myhface3 (i)->subedge1 (0) ;
    case myhface3_t :: myrule_t :: iso4 :
      assert( j < 3 );
      return ((twist (i) < 0) ? myhface3 (i)->subedge1 ((8 - j + twist (i)) % 3) : myhface3 (i)->subedge1 ((j + twist (i)) % 3)) ;
    case myhface3_t :: myrule_t :: nosplit :
      cerr << "**FEHLER (FATAL): subedge1 () auf nicht in verfeinerter Fl\"ache aufgerufen. In " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
      return 0 ;
  }
  return 0 ;
}

template < class A > const typename Periodic3Top < A > :: myhedge1_t * Periodic3Top < A > :: subedge1 (int i, int j) const {
  return ((Periodic3Top < A > *)this)->subedge1 (i,j) ;
}

template < class A > typename Periodic3Top < A > ::  myhface3_t * Periodic3Top < A > :: subface3 (int i, int j) {
  switch (myhface3 (i)->getrule ()) {
  case myhface3_t :: myrule_t :: e01 :
    assert( j < 2 );
    if ( twist(i) == 0 ||  twist(i) == 1  ||  twist(i) == -1 )
      return myhface3(i)->subface3(j) ;
    if ( twist(i) == 2 ||  twist(i) == -2 ||  twist(i) == -3 )
      return myhface3(i)->subface3(!j) ;
      cerr << __FILE__ << " " << __LINE__ << "myhface3(i)->subface3()" << endl;
      return 0;
  case myhface3_t :: myrule_t :: e12 :
    assert( j < 2 );
    if ( twist(i) == 0  ||  twist(i) == 2 ||  twist(i) == -3 )
      return myhface3(i)->subface3(j) ;
    if ( twist(i) == -1 ||  twist(i) == 1 ||  twist(i) == -2 )
      return myhface3(i)->subface3(!j) ;
    cerr << __FILE__ << " " << __LINE__ << "myhface3(i)->subface3()" << endl;
    return 0;
  case myhface3_t :: myrule_t :: e20 :
    assert( j < 2 );
    if ( twist(i) == 1 ||  twist(i) == 2 ||  twist(i) == -2 )
      return myhface3(i)->subface3(j) ;
    if ( twist(i) == 0 ||  twist(i) == -1 || twist(i) == -3 )
      return myhface3(i)->subface3(!j) ;
    cerr << __FILE__ << " " << __LINE__ << "myhface3(i)->subface3()" << endl;
    return 0;
  case myhface3_t :: myrule_t :: iso4 :
    assert( j < 4 );
    if ( j == 3 )
      return myhface3(i)->subface3 (3) ;
    if ( j < 3 )
      return myhface3 (i)->subface3 (twist(i) < 0 ? (7 - j + twist(i)) % 3 : (j + twist(i)) % 3) ;
  case myhface3_t :: myrule_t :: nosplit :
    cerr << "**FEHLER (FATAL): subface3 () auf nicht verfeinerter Fl\"ache aufgerufen. In " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
    return 0 ;
  default:
    cerr << "**FEHLER (FATAL): Falsche Verfeinerungsregel [" << myhface3(i)->getrule() << "] in " << __FILE__ << " " << __LINE__ << endl ;
    abort() ;
  } 
  return 0 ;
}

template < class A > const typename Periodic3Top < A > ::  myhface3_t * Periodic3Top < A > :: subface3 (int i, int j) const {
  return ((Periodic3Top < A > *)this)->subface3 (i,j) ;
}

template < class A > void Periodic3Top < A > :: split_iso4 () 
{
  const int l = 1 + this->level () ;
  innerperiodic3_t * p0 = new innerperiodic3_t (l, subface3 (0,0), twist (0), subface3 (1,0), twist (1), this , 0) ;
  innerperiodic3_t * p1 = new innerperiodic3_t (l, subface3 (0,1), twist (0), subface3 (1,2), twist (1), this , 1) ;
  innerperiodic3_t * p2 = new innerperiodic3_t (l, subface3 (0,2), twist (0), subface3 (1,1), twist (1), this , 2) ;
  
  // Mir ist nicht ganz klar, warum der Twist auf diese seltsame Art umzurechnen ist,
  // die Zeile (bzw. die Formel) habe ich aus Mario's Tetradeder Split Iso-8 "uber-
  // nommen, ohne im einzelnen nachzupr"ufen, ob die Regel richtig ist. (BS)
  
  innerperiodic3_t * p3 = new innerperiodic3_t (l, subface3 (0,3), (twist(0) >= 0 ? (twist(0)+1)%3 : (twist(0)-1)%3-1), subface3 (1,3), (twist(1)>=0 ? (twist(1)+1)%3 : (twist(1)-1)%3-1) , this , 3) ;
  assert (p0 && p1 && p2 && p3) ;
  p0->append(p1) ;
  p1->append(p2) ;
  p2->append(p3) ;
  _dwn = p0 ;
  _rule = myrule_t :: iso4 ;
  p0->_up = p1->_up = p2->_up = p3->_up = this; //us
  return ;
}

template < class A > void Periodic3Top < A > :: refineImmediate (myrule_t r) {

  // Die Methode wird nur vom restore () und vom refineBalance () auf-
  // gerufen und geht davon aus, dass das betroffene Element noch nicht
  // verfeinert ist -> ist ein Blatt der Hierarchie.

  assert (this->leaf()) ;
  switch(r) {
    case myrule_t :: iso4 :
    
      // Das refineImmediate (..) auf allen Fl"achen wird vom periodic3 :: refine (..)
      // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
      // "uberall hface3 :: refine (..) true geliefert hat, wohl aber z.B. von
      // restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
      // erzwingen m"ussen und d"urfen.
   
      typedef typename myhface3_t :: myrule_t face3rule_t;
      myhface3 (0)->refineImmediate (face3rule_t (r).rotate (twist (0))) ;
      myhface3 (1)->refineImmediate (face3rule_t (r).rotate (twist (1))) ;
      split_iso4 () ;
      break ;

    case myrule_t :: e01 :
    case myrule_t :: e12 :
    case myrule_t :: e20 :
    
      // Mit den drei anisotropen Regeln k"onnen wir leider noch nichts anfangen.
    
      abort () ;
    default :
      cerr << "**FEHLER (FATAL) beim unbedingten Verfeinern mit unbekannter Regel: " ;
      cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << endl ;
      abort () ;
      break ;
  }
  this->postRefinement () ;
  return ;
}

template < class A > bool Periodic3Top < A > :: refine () {

  // Das refine () reagiert nicht auf die Elementaktivierung zur Verfeinerung
  // in der globalen Schleife, weil das perioodische Randelement sich nur auf
  // Anforderung zur Balancierung aus einem anliegenden Element direkt verfeinert.

  return true ;
}

template < class A > bool Periodic3Top < A > :: refineBalance (balrule_t r, int fce) 
{
  if (r != balrule_t :: iso4) 
  {
    cerr << "**WARNUNG (IGNORIERT) in Periodic3Top < A > :: refineBalance (..) nachschauen, Datei " 
       << __FILE__ << " Zeile " << __LINE__ << endl ;
   
  // Bisher kann die Balancierung nur die isotrope Achtelung handhaben,
  // falls mehr gew"unscht wird, muss es hier eingebaut werden. Im Moment wird
  // die Balancierung einfach verweigert, d.h. die Verfeinerung des anfordernden
  // Elements f"allt flach.
  
    return false ;
  } 
  else 
  {
    // Der nachfolgende Aufruf nutzt aus, dass die Regel der periodischen R"ander
    // sich direkt auf die Balancierungsregel des entsprechenden Polygonverbinders
    // projezieren l"asst (n"amlich 1:1). Deshalb unterscheidet der Aufruf nicht nach
    // der angeforderten Regel in einer 'case' Anweisung.
    
    // take opposite face 
    const int opp = 1 - fce ;
    if (myhface3 (opp)->refine (typename myhface3_t :: myrule_t (r).rotate (twist (opp)), twist (opp))) 
    {
      refineImmediate (r) ;
      return true ;
    } 
    else 
    {
      return false ;
    }
  }
}

template < class A > bool Periodic3Top < A > :: coarse () {

  // Das Vergr"obern geschieht auch passiv, sobald ein anliegendes Element
  // vergr"obert wird durch den Aufruf von "bndNotifyCoarsen ()" s.u.

  bndNotifyCoarsen () ;
  return false ;
}

template < class A > bool Periodic3Top < A > :: bndNotifyCoarsen () {

  // Wie beim Randelement auch: Die Vergr"oberung eines anliegenden Elements
  // l"ost einen Vorgang aus, der feststellt ob das periodische RE ebenfalls
  // vergr"obert werden soll.
  
  innerperiodic3_t * p = down () ;
  if (!p) return false ;
  bool x = true ;
  do {

  // Falls p kein Blatt der Hierarchie ist, 
  // die Vergr"oberungsm"oglichkeit weitergeben.

    if (!p->leaf ()) p->coarse () ;

  // F"ur die hintere und vordere Fl"ache feststellen, ob
  // der Referenzenz"ahler mehr als einen Eintrag ergibt.

    if (p->myhface3 (0)->ref > 1) (x = false) ;
    if (p->myhface3 (1)->ref > 1) (x = false) ;

  } while ( (p = p->next ()) ) ;
  if (x) {
  
  // Falls keine Fl"achen anliegen, die auf Kinder oder Kindes- 
  // mit mehr als einer Referenz f"uhren, ist sicher, dass das
  // Bezugsrandelement zwischen zwei 'relativ groben' Elementen
  // liegt. Somit kann es vergr"obert werden.
  
    this->preCoarsening () ;
    
    delete _dwn ;
    _dwn = 0 ;
    _rule = myrule_t :: nosplit ;
    myhface3 (0)->coarse () ;
    myhface3 (1)->coarse () ;
  }
  return x ;
}

template < class A > void Periodic3Top < A > :: backup (ostream & os) const 
{
  doBackup( os );
}
template < class A > void Periodic3Top < A > :: backup (ObjectStream& os) const 
{
  doBackup( os );
}

template < class A > template <class OutStream_t>
void Periodic3Top < A > :: doBackup (OutStream_t& os) const 
{
  os.put ((char) getrule ()) ;
  { for (const innerperiodic3_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > void Periodic3Top < A > :: restore (istream & is) 
{
  doRestore( is );
}
template < class A > void Periodic3Top < A > :: restore (ObjectStream& is) 
{
  doRestore( is );
}

template < class A > template <class InStream_t> 
void Periodic3Top < A > :: doRestore (InStream_t & is) 
{
  myrule_t r ((char) is.get ()) ;
  assert(getrule () == myrule_t :: nosplit) ; // Testen auf unverfeinerten Zustand
  if (r == myrule_t :: nosplit) {
    for (int i = 0 ; i < 2 ; i ++) {
      myhface3_t & f (*(myhface3 (i))) ;
      if (!f.leaf ()) {
        switch (f.getrule ()) {
    case balrule_t :: iso4 :
            {for (int j = 0 ; j < 4 ; j ++) f.subface3 (j)->nb.complete (f.nb) ;}
      break ;
    default :
      cerr << "**FEHLER (FATAL) beim restore mit unbekannter Balancierungsregel: "
                 << "[" << r << "]. In " << __FILE__ << __LINE__ << endl ;
      abort () ;
      break ;
  }
      }
    }
  } 
  else {
    refineImmediate (r) ;
    assert (getrule() == r) ;
    {for (innerperiodic3_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
  }
  return ;
}


// template instantiation 
#ifndef GITTER_TETRA_TOP_PLL_H_INCLUDED
template class Hface3Top< GitterBasis :: Objects :: Hface3Empty > ;
template class Hbnd3Top< GitterBasis :: Objects :: Hbnd3Default > ;
template class TetraTop < GitterBasis :: Objects :: TetraEmpty > ;
template class Periodic3Top < GitterBasis :: Objects :: Periodic3Empty > ;
#endif
