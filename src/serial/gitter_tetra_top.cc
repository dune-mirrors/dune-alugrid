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


template < class A > void Hface3Top < A > :: split_e01 () 
{
  // NOTE: edge numbering is not opposite vertex !!!
  // see gitter_geo.cc 
  // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})
 
  assert( _inner == 0 );
  const int newLevel = 1 + level () ;
  myvertex_t * ev0 = myhedge1(0)->subvertex (0) ;

  //myvertex_t * ev1 = myhedge1(1)->subvertex (2) ;
  //myvertex_t * ev2 = myhedge1(2)->subvertex (2) ;
  assert(ev0) ;
  inneredge_t * e0 = new inneredge_t (newLevel, ev0, this->myvertex (2) ) ;
  assert( e0 ) ;
  innerface_t * f0 = new innerface_t (newLevel, 
                                      this->subedge1(0,0), twist(0), 
                                      e0, 0, 
                                      myhedge1(2), twist(2), 
                                      0) ; // child number 
  innerface_t * f1 = new innerface_t (newLevel, 
                                      myhedge1(1), twist(1), 
                                      e0, 1, 
                                      this->subedge1(0,1), twist(0), 
                                      1) ; // child number 

  std::cout << "split_e01 " << ev0 << endl;
  cout << "New subface 0" << f0 << endl;
  cout << "New subface 1" << f1 << endl;

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
  /* 
    3 (in tetra)
    |\
    |1\       face 1 in tetra ref element, split edge is 2--3 
    |  \
    |   \ (1) edge in father face
    |2  0\
    |     \
    |      \
    |2  1  0\
   4|........\ 2 (in tetra)
    |2  1  1 / 
 (2)|       /
    |      /
    |2   0/ (0), edge in father face
    |    / 
    |   /
    |  /
    |0/
    |/
    0 (in tetra)
  */
  
  assert( _inner == 0 );
  const int newLevel= 1 + level () ;
  myvertex_t * ev0 = myhedge1(1)->subvertex (0) ;

  std::cout << "Vx in split_e12 = " << ev0 << endl;
  //myvertex_t * ev1 = myhedge1(1)->subvertex (0) ;
  //myvertex_t * ev2 = myhedge1(2)->subvertex (0) ;
  assert(ev0) ;
  // create new inner edge 
  inneredge_t * e0 = new inneredge_t (newLevel, ev0, this->myvertex(0) ) ;
  assert( e0 ) ;
  innerface_t * f0 = new innerface_t (newLevel, // level 
                                      this->subedge1(1,0), twist(1), // edge 0, twist 
                                      e0, 0,                         // edge 1, twist 
                                      myhedge1(0), twist(0),         // edge 2, twist 
                                      0 ) ; // child number 

  innerface_t * f1 = new innerface_t (newLevel, // level 
                                      myhedge1(2), twist(2),         // edge 0, twist
                                      e0, 1,                         // edge 1, twist 
                                      this->subedge1(1,1), twist(1), // edge 2, twist 
                                      1 ) ; // child number 
  assert (f0 && f1 ) ;
  f0->append(f1) ;
  _inner = new inner_t( f0 , e0 );
  _rule = myrule_t :: e12 ;
  return ;
}

template < class A >  void Hface3Top < A > :: split_e20 () 
{
  // NOTE: edge numbering is not opposite vertex !!!
  // see gitter_geo.cc 
  // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})

  assert( _inner == 0 );
  const int newLevel= 1 + level () ;
  myvertex_t * ev0 = myhedge1(2)->subvertex (0) ;

  /* 
    3 (in tetra)
    |\
    |1\       face 1 in tetra ref element, split edge is 3--0 
    |  \
    |   \ (1) edge in father face
    |2  0\
    |     \
    |      \
    |2  1  0\
   4|........\ 2 (in tetra)
    |2  1  1 / 
 (2)|       /
    |      /
    |2   0/ (0), edge in father face
    |    / 
    |   /
    |  /
    |0/
    |/
    0 (in tetra)
  */

  assert(ev0) ;
  inneredge_t * e0 = new inneredge_t (newLevel, ev0, this->myvertex(1) ) ;
  assert( e0 ) ;
  innerface_t * f0 = new innerface_t (newLevel, // level 
                                      this->subedge1(2,0), twist(2), // edge 0, twist 
                                      e0, 0,                         // edge 1, twist 
                                      myhedge1(1), twist(1),         // edge 2, twist 
                                      0 ) ; // child number 

  innerface_t * f1 = new innerface_t (newLevel, // level 
                                      myhedge1(0), twist(0),         // edge 0, twist
                                      e0, 1,                         // edge 1, twist 
                                      this->subedge1(2,1), twist(2), // edge 2, twist 
                                      1 ) ; // child number 
  assert (f0 && f1 ) ;

  std::cout << "split_e20 " << ev0 << endl;
  cout << "New subface 0" << f0 << endl;
  cout << "New subface 1" << f1 << endl;

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
  if (r != getrule ()) {
    assert (getrule () == myrule_t :: nosplit) ;
    switch(r) {
      typedef typename myhedge1_t :: myrule_t myhedge1rule_t;
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
        cout << "Got rule = " << int(r) << endl;
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
  
      assert (this->getrule () == myrule_t :: nosplit) ;
      switch (r) {
      case balrule_t :: e01 :
        //cout << "refLikeEl: e01 " << endl;
        if (!myhface3 (0)->refine (balrule_t (balrule_t :: e01).rotate (twist (0)), twist (0))) return false ;
        split_e01 () ;
        break;
      case balrule_t :: e12 :
        //cout << "refLikeEl: e12 " << endl;
        if (!myhface3 (0)->refine (balrule_t (balrule_t :: e12).rotate (twist (0)), twist (0))) return false ;
        split_e12 () ;
        break;
      case balrule_t :: e20 :
        //cout << "refLikeEl: e20 " << endl;
        if (!myhface3 (0)->refine (balrule_t (balrule_t :: e20).rotate (twist (0)), twist (0))) return false ;
        split_e20 () ;
        break;
      case balrule_t :: iso4 :
        if (!myhface3 (0)->refine (balrule_t (balrule_t :: iso4).rotate (twist (0)), twist (0))) return false ;
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
                LinearMapping ( this->myvertex(0)->Point(), 
                                this->myvertex(1)->Point(),
                                this->myvertex(2)->Point(), 
                                this->myvertex(3)->Point())).integrate1 (0.0) 
            : vol )
  , _lvl (l) 
  , _nChild(nChild)
  , _rule (myrule_t :: nosplit)
  , _type( (_up->_type + 1) % 3 ) // my type is (t+1)%d where t is the fathers type 
{
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
      LinearMapping ( this->myvertex(0)->Point(), 
                      this->myvertex(1)->Point(),
                      this->myvertex(2)->Point(), 
                      this->myvertex(3)->Point())).integrate1 (0.0) );
  if( std::abs( calculatedVolume - _volume ) >1e-10 ) 
    cout << "Determinant of Tetra[" << this->getIndex() << "] is wrong" << endl;
  //assert( std::abs( calculatedVolume - _volume ) / _volume  < 1e-10 ); 
#endif

  return ;
}

// constrcutor mit IndexManager uebergabe
// this is the macro element constructor 
template < class A > TetraTop < A > :: 
TetraTop (int l, myhface3_t * f0, int t0,
          myhface3_t * f1, int t1, myhface3_t * f2, int t2, 
          myhface3_t * f3, int t3) 
  : A (f0, t0, f1, t1, f2, t2, f3, t3)
  , _bbb (0), _up(0)
  , _inner( 0 )  
  , _volume( quadraturTetra3D < VolumeCalc > 
    (LinearMapping ( this->myvertex(0)->Point(), this->myvertex(1)->Point(),
                     this->myvertex(2)->Point(), this->myvertex(3)->Point())).integrate1 (0.0) )
  , _lvl (l) 
  , _nChild(0)  // we are macro ==> nChild 0 
  , _rule (myrule_t :: nosplit) 
  , _type( 0 ) // type of macro elements should be zero 
{ 
  assert( this->level() == l );

  // _up wird im Constructor uebergeben
  this->setIndex( indexManager().getIndex() );

  //print();
  return ;
}

template < class A > TetraTop < A > :: ~TetraTop () 
{
  this->freeIndex( indexManager() );
  // attachleafs is called in constructor of TetraEmpty
  // if delete is called on macro we only call this method on leaf
  if (! _inner ) this->detachleafs();
  if (_bbb) delete _bbb ;
  if (_inner) delete _inner ;
  return ;
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


template < class A >  typename TetraTop < A > ::  myhface3_t * TetraTop < A > :: subface3 (int i, int j) {
  switch (myhface3(i)->getrule()) {
  case myhface3_t :: myrule_t :: e01 :
    assert( j < 2 );
    if ( twist(i) == 0 ||  twist(i) == 1 ||  twist(i) == -1 )
      return myhface3(i)->subface3(j) ;
    if ( twist(i) == 2 ||  twist(i) == -2 || twist(i) == -3 )
      return myhface3(i)->subface3(!j) ;
      cerr << __FILE__ << " " << __LINE__ << "myhface3(i)->subface3()" << endl;
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
    cerr << "**FEHLER (FATAL): Falsche Verfeinerungsregel [" << myhface3(i)->getrule() << "] in "__FILE__ << " " << __LINE__ << endl ;
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
  const int l = 1 + this->level () ;
  
  myhedge1_t* edge1 = this->myhedge1( 5 );
  cout << "Got no split edge = " << edge1->myvertex( 0 ) << " " << edge1->myvertex( 1 ) << endl ;
  cout << "Got split edge  0 = " << this->subedge1 (3, 0)->myvertex( 0 ) << " " << this->subedge1 (3, 0)->myvertex( 1 ) << endl ;
  cout << "Got split edge  1 = " << this->subedge1 (2, 0)->myvertex( 0 ) << " " << this->subedge1 (2, 0)->myvertex( 1 ) << endl ;
  int edgeTwist = ( edge1->myvertex( 0 ) == this->myvertex( 2 ) ) ? 0 : 1 ;
  innerface_t* f0 = new innerface_t (l, this->subedge1 (3, 0), 0, 
                                        edge1 , edgeTwist , 
                                        //this->myhedge1( 5 ), 0,// myface0->twist( 0 ), 
                                        this->subedge1 (2, 0), 1 ) ;
  assert(f0) ;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );

  cout << twist( 2 ) << " twist 2 " << endl;
  
  //innertetra_t * h0 = new innertetra_t (l, subface3(2, 0), twist (2), f0, 1, myhface3(1), twist (1), subface3(3, 0), twist (3), this, 0, childVolume) ;
  //innertetra_t * h1 = new innertetra_t (l, subface3(3, 1), twist (3), myhface3(0), twist (0), f0, 0, subface3(2, 1), twist (2), this, 1, childVolume) ;
  innertetra_t * h0 = new innertetra_t (l, myhface3(1), twist (1), f0, -1, subface3(3, 0), twist (3), subface3(2, 0), twist (2), this, 0, childVolume) ;
  innertetra_t * h1 = new innertetra_t (l, myhface3(0), twist (0), f0, 0, subface3(2, 1), twist (2), subface3(3, 1), twist (3), this, 1, childVolume) ;
  assert(h0 && h1) ;
  h0->append(h1) ;
  _inner = new inner_t( h0, f0 ); 
  assert( _inner );
  this->detachleafs();
  return ;
}

template < class A >  void TetraTop < A > :: split_e12 () 
{
  abort();
  assert( _inner == 0 );
  const int l = 1 + this->level () ;
  
  innerface_t * f0 = new innerface_t (l, this->subedge1 (3, 3), 1, this->subedge1 (0, 3), 0, this->subedge1 (2, 2), 0 ) ;
  assert(f0 ) ;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  innertetra_t * h0 = new innertetra_t (l, subface3(0, 0), twist (0), f0, 0, myhface3(2), twist (2), subface3(3, 0), twist (3), this, 0, childVolume) ;
  innertetra_t * h1 = new innertetra_t (l, subface3(0, 1), twist (0), myhface3(1), twist (1), f0, 1, subface3(3, 1), twist (3), this, 1, childVolume) ;
  assert(h0 && h1) ;
  h0->append(h1) ;
  _inner = new inner_t( h0, f0 ); 
  assert( _inner );
  _rule = myrule_t :: e12 ;
  this->detachleafs();
  return ;
}

template < class A >  void TetraTop < A > :: split_e20 () 
{
  abort();
  assert( _inner == 0 );
  const int l = 1 + this->level () ;
  
  innerface_t * f0 = new innerface_t (l, this->subedge1 (3, 3), 1, this->subedge1 (0, 3), 0, this->subedge1 (2, 2), 0 ) ;
  assert(f0) ;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  innertetra_t * h0 = new innertetra_t (l, subface3(0, 0), twist (0), f0, 0, myhface3(2), twist (2), subface3(3, 0), twist (3), this, 0, childVolume) ;
  innertetra_t * h1 = new innertetra_t (l, subface3(0, 1), twist (0), myhface3(1), twist (1), f0, 1, subface3(3, 1), twist (3), this, 1, childVolume) ;
  assert(h0 && h1) ;
  h0->append(h1) ;
  _inner = new inner_t( h0, f0 ); 
  assert( _inner );
  _rule = myrule_t :: e20 ;
  this->detachleafs();
  return ;
}

template < class A >  void TetraTop < A > :: split_e23 () 
{
  abort();
  assert( _inner == 0 );
  const int l = 1 + this->level () ;
  
  innerface_t * f0 = new innerface_t (l, this->subedge1 (3, 3), 1, this->subedge1 (0, 3), 0, this->subedge1 (2, 2), 0 ) ;
  assert(f0) ;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  innertetra_t * h0 = new innertetra_t (l, subface3(0, 0), twist (0), f0, 0, myhface3(2), twist (2), subface3(3, 0), twist (3), this, 0, childVolume) ;
  innertetra_t * h1 = new innertetra_t (l, subface3(0, 1), twist (0), myhface3(1), twist (1), f0, 1, subface3(3, 1), twist (3), this, 1, childVolume) ;
  assert(h0 && h1) ;
  h0->append(h1) ;
  _inner = new inner_t( h0, f0 ); 
  assert( _inner );
  _rule = myrule_t :: e23 ;
  this->detachleafs();
  return ;
}

template < class A >  void TetraTop < A > :: split_e30 () 
{
  abort();
  assert( _inner == 0 );
  const int l = 1 + this->level () ;
  
  innerface_t * f0 = new innerface_t (l, this->subedge1 (3, 3), 1, this->subedge1 (0, 3), 0, this->subedge1 (2, 2), 0 ) ;
  assert(f0) ;
  double childVolume = 0.5 * _volume;
  innertetra_t * h0 = new innertetra_t (l, subface3(0, 0), twist (0), f0, 0, myhface3(2), twist (2), subface3(3, 0), twist (3), this, 0, childVolume) ;
  innertetra_t * h1 = new innertetra_t (l, subface3(0, 1), twist (0), myhface3(1), twist (1), f0, 1, subface3(3, 1), twist (3), this, 1, childVolume) ;
  assert(h0 && h1) ;
  h0->append(h1) ;
  _inner = new inner_t( h0, f0 ); 
  assert( _inner );
  _rule = myrule_t :: e30 ;
  this->detachleafs();
  return ;
}

template < class A >  void TetraTop < A > :: split_e31 () 
{
  abort();
  assert( _inner == 0 );
  const int l = 1 + this->level () ;
  
  innerface_t * f0 = new innerface_t (l, this->subedge1 (3, 3), 1, this->subedge1 (0, 3), 0, this->subedge1 (2, 2), 0) ;
  assert(f0) ;

  // we divide by 2 means we divide the volume by 2
  const double childVolume = calculateChildVolume( 0.5 * _volume );
  
  innertetra_t * h0 = new innertetra_t (l, subface3(0, 0), twist (0), f0, 0, myhface3(2), twist (2), subface3(3, 0), twist (3), this, 0, childVolume) ;
  innertetra_t * h1 = new innertetra_t (l, subface3(0, 1), twist (0), myhface3(1), twist (1), f0, 1, subface3(3, 1), twist (3), this, 1, childVolume) ;
  assert(h0 && h1) ;
  h0->append(h1) ;
  _inner = new inner_t( h0, f0 ); 
  assert( _inner );
  _rule = myrule_t :: e31 ;
  this->detachleafs();
  return ;
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

  //cout << "Valid twist not found!!!" << endl;
  //return 0;

  // we should not get here 
  assert( false );
  abort();
  return -5;
}

template < class A > bool 
TetraTop < A > :: checkTetra( const innertetra_t *tetra, const int nChild ) const 
{
  // make sure face twists are ok 
  bool twistOk = true ;
  for(int fce=0; fce<4; ++fce ) 
  {
    cout << "Check face " << fce << " of tetra " << tetra->getIndex() << " , type = " << int(tetra->_type) << " with twist " << tetra->twist( fce ) << endl;
    for(int i=0; i<3; ++i ) 
    {
      //const bool type2ch1 = ( nChild == 1 ) && (tetra->_type == 2);
      //const int mapVx[4]  = { 0, 1, 2 , 3} ;//(type2ch1) ? 3:2 , (type2ch1) ? 2:3 };

      // use proto type to check face twists 
      if( tetra->myvertex( Gitter :: Geometric :: Tetra :: prototype[ fce ][ i ] )->getIndex() != 
              tetra->myvertex( fce, i )->getIndex() )
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

    if( ! tetra->myneighbour( fce ).first->isRealObject()  ) 
    {
      cout << "Neighbour " << fce << " of Tetra " << tetra->getIndex()  << " is wrong " << endl;
      cout << "Check face " << tetra->myhface3( fce )->getIndex() << endl;
    }
    // make sure neighbor is something meaningful 
    //assert( tetra->myneighbour( fce ).first->isRealObject() );
  }
  
  return true;
  //cout << endl;
  return twistOk;
}

// --bisect
template < class A >  void TetraTop < A > :: bisect () 
{
  // this refinement follows Stevenson, 2008 
  // and Nochetto, Siebert, Veser, 2009 

  assert( _inner == 0 );
  const int newLevel = 1 + this->level();

  // type of element, we assume that on level all elements have type 0 
  const unsigned char elType = elementType() ;
  assert( elType == _type );

  const int face2 = ( elType == 2 && _nChild == 1 ) ? 1 : 0;
  myhedge1_t* subEdge2 = this->subedge1 (2+face2, 0);

  cout << "Refine tetra " << this->getIndex() << " with type " << int(elType) << " and chNr " << int(_nChild) << endl;


  cout << "sub 1 " << this->subedge1 (1, 0)->myvertex( 0 ) << " " << this->subedge1 (1, 0)->myvertex( 1 ) << endl;
  cout << "sub 2 " << subEdge2->myvertex( 0 ) << " " << subEdge2->myvertex( 1 ) << endl;

  myhedge1_t* subEdge = this->subedge1 (1, 0);
  myhedge1_t* orgEdge = this->myhedge1( 3+face2 ) ;
  //(_type == 1 ) && (_nChild == 1) ? 4 : 3 );

  //cout << "org " << orgEdge << endl;
  // calculate twist of last edge 
 
  const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

  // new inner face 
  innerface_t * f0 = 
    new innerface_t (newLevel, 
                     subEdge, 0, // from face 1 get subedge 0
                     orgEdge, edgeTwst, 
                     subEdge2, 1 // from face 2 get subedge 0  
                    ) ;
  assert(f0) ;

  cout << "New inner face is " << f0 << endl;

  const double childVolume = 0.5 * _volume;


  //cout << "Check sub face " << subface3(1, 1) << " with vertex " << this->myvertex( 1+offset )->getIndex() << endl;
  myhface3_t* subFace20 = ( face2 ) ? subface3(3, 0) : subface3(1, 0); 
  const int twist20 = ( face2 ) ? twist( 3 ) : twist( 1 );

  const int vx10[ 2 ] = { this->myvertex( 0 )->getIndex(),
                          this->myvertex( 3 )->getIndex() };

  myhface3_t* subFace10 = myhface3( 3-face2 );
  const int twst10 = ( face2 ) ?  
                         calculateFace3Twist( vx10, subFace10, 2) :  
                         twist( 3-face2 );

  const int twst20 = (elType == 1) && (_nChild == 1) ? 
                       calculateFace2Twist( this->myvertex( 2 )->getIndex(), subFace20 ) : 
                       twist20;

  const int twstCheck = ( face2 ) ? twist( 1 ) >= 0 ? 1 : 0 : 0;
  myhface3_t* subFace30 = ( face2 ) ? subface3(1, twstCheck ) : subface3(2,0);
  //cout << "twist of subface3(1,1) is " << twist( 1 ) << endl;

  const int vx30[ 2 ] = { this->myvertex( 0 )->getIndex(),
                          this->myvertex( 3 )->getIndex() };

  //const int twst30 = ( face2 ) ? calculateFace3Twist( vx30, subFace30, 2) : twist( 2 );
  const int twst30 = ( face2 ) ? calculateFace3Twist( vx30, subFace30, 2) : twist( 2 );

  innertetra_t * h0 = new innertetra_t (newLevel, 
                                        f0, 0-face2 ,  // face 0 and twist
                                        subFace10, twst10,    // face 2 and twist
                                        subFace20, twst20,    // face 1 and twist  
                                        subFace30, twst30,  // face 3 and twist
                                        this, 0, childVolume) ;    // father, childNo, volume

  myhface3_t* subFace21 = face2 ? subface3(1, 1-twstCheck) : subface3(2, 1); 
  //cout << "Check sub face " << subFace21 << " with vertex " << this->myvertex( 1+offset )->getIndex() << endl;

  const int twst21 = twist( 2+face2 );//(elType == 2) ? calculateFace2Twist( h0->myvertex( 1 )->getIndex(), subFace21 ): twist(2);

  // calculate twist of face 1 
  const int vx11[2] = { this->myvertex( 3-face2 )->getIndex(), 
                        this->myvertex( 2-face2 )->getIndex() };

  const int twst11 = calculateFace3Twist( vx11, myhface3( 0 ), 1 );

  int twst[ 3 ] = { -1, -1 , -1 };

  const int vx31[2] = { this->myvertex( 3-face2 )->getIndex(), 
                        h0->myvertex( 1 )->getIndex() };

  myhface3_t* subFace31 = face2 ? subface3(3,1) : subface3(1, 1 );
  const int twst31 = ( elType > 0 ) ? calculateFace3Twist( vx31, subFace31, 1 ) : twist(1);

  innertetra_t * h1 = new innertetra_t (newLevel, f0, twst[ elType ] + face2 , // face 0, twist is -1 or 0
                                        myhface3( 0 ) , twst11, //twst11 , // face 1
                                        subFace21, twst21, //Face21, twst21, //Face21, twst21,       // face 2
                                        subFace31, twst31, //twst31,       // face 3
                                        this, 1, childVolume) ;  // father, childNo, volume


  cout << "New tetra " << h0 << endl;
  assert( checkTetra( h0, 0 ) );

  cout << "New tetra " << h1 << endl;

  // do dheckTetra here, otherwise check for neighbors fails 
  assert( checkTetra( h1, 1 ) );
  cout << endl;

  // check vertices 
  // v0 of first child is always v0 of father 
  assert( h0->myvertex( 0 )->getIndex() == this->myvertex( 0 )->getIndex() );
  // v0 of second child is always v3 of father 
  assert( h1->myvertex( 0 )->getIndex() == this->myvertex( 3-face2 )->getIndex() );
  
  // v1 of first child is also v1 of second child 
  assert( h0->myvertex( 1 )->getIndex() == h1->myvertex( 1 )->getIndex() );

  // v2 of first child is always v1 of father 
  assert( h0->myvertex( 2+face2 )->getIndex() == this->myvertex( 1 )->getIndex() );
  // v3 of first child is always v2 of father 
  assert( h0->myvertex( 3-face2 )->getIndex() == this->myvertex( 2+face2 )->getIndex() );
  

  // v2 of second child is always v1 of father 
  assert( h1->myvertex( 2+face2 )->getIndex() == this->myvertex( 2+face2 )->getIndex() );
  // v3 of second child is always v2 of father 
  assert( h1->myvertex( 3-face2 )->getIndex() == this->myvertex( 1 )->getIndex() );

  assert(h0 && h1) ;
  h0->append(h1) ;
  _inner = new inner_t( h0, f0 ); 
  assert( _inner );
  _rule = myrule_t :: bisect ;
  this->detachleafs();
  return ;
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
  innerface_t * f0 = new innerface_t (l, this->subedge1 (3, 2), ((twist(3)>=0)?1:0), this->subedge1 (1, 2), ((twist(1)>=0)?1:0), this->subedge1 (2, 2), ((twist(2)>=0)?1:0)) ;
  innerface_t * f1 = new innerface_t (l, this->subedge1 (3, 0), ((twist(3)>=0)?1:0), this->subedge1 (2, 1), ((twist(2)>=0)?1:0), this->subedge1 (0, 2), ((twist(0)>=0)?1:0)) ;
  innerface_t * f2 = new innerface_t (l, this->subedge1 (3, 1), ((twist(3)>=0)?1:0), this->subedge1 (0, 1), ((twist(0)>=0)?1:0), this->subedge1 (1, 0), ((twist(1)>=0)?1:0)) ;
  innerface_t * f3 = new innerface_t (l, this->subedge1 (2, 0), ((twist(2)>=0)?0:1), this->subedge1 (0, 0), ((twist(0)>=0)?0:1), this->subedge1 (1, 1), ((twist(1)>=0)?0:1)) ;
  innerface_t * f4 = new innerface_t (l, e0, 0, this->subedge1 (3, 2), ((twist(3)>=0)?0:1), this->subedge1 (2, 1), ((twist(2)>=0)?1:0)) ;
  innerface_t * f5 = new innerface_t (l, e0, 0, this->subedge1 (3, 1), ((twist(3)>=0)?1:0), this->subedge1 (0, 2), ((twist(0)>=0)?0:1)) ; 
  innerface_t * f6 = new innerface_t (l, e0, 0, this->subedge1 (1, 0), ((twist(1)>=0)?0:1), this->subedge1 (0, 0), ((twist(0)>=0)?1:0)) ;
  innerface_t * f7 = new innerface_t (l, e0, 0, this->subedge1 (1, 2), ((twist(1)>=0)?1:0), this->subedge1 (2, 0), ((twist(2)>=0)?0:1)) ;
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

// --refineImmediate 
template < class A >  void TetraTop < A > :: refineImmediate (myrule_t r) 
{
  assert (getrule () == myrule_t :: nosplit) ;
  typedef typename myhface3_t :: myrule_t myhface3rule_t;

  switch(r) 
  {
    // non-conforming spliting into 8 children 
    case myrule_t :: iso8 :
        
      // Das refineImmediate (..) auf allen Fl"achen wird vom tetra :: refine (..)
      // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
      // "uberall hface3 :: refine (..) true geliefert hat, wohl aber z.B. von
      // restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
      // erzwingen m"ussen und d"urfen.
        
      {
        for (int i = 0 ; i < 4 ; ++i)
          myhface3 (i)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: iso4).rotate (twist (i))) ; 
      }
      splitISO8 () ;
      break ;

    // conforming bisection   
    // --bisect 
    case myrule_t :: bisect :
      {
        // refine faces first 
        //std::cout << "Refine child " << childNr << endl;
        //std::cout << "Split face " << myhface3 (1) << endl;
        cout << "refineImmediate Tetra " << this->getIndex() << endl;
        
        // for the second child with type 2 we need to spilt 
        // the edge between vertex 0 and 2 
        if( elementType () == 2 && _nChild == 1 ) 
        { 
          std::cout << "Split special " << endl;
          // split face 1 and 2 with appropriate rules 
          // this corresponds to splitting of the edge between vertex 0 and 3 
          myhface3 (1)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1))) ;
          std::cout << "Split face 2 " << endl;
          myhface3 (3)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e20).rotate (twist (3))) ;
        }
        else 
        {
          // split face 1 and 2 with appropriate rules 
          // this corresponds to splitting of the edge between vertex 0 and 3 
          myhface3 (1)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e20).rotate (twist (1))) ;
          myhface3 (2)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (2))) ;
        }

        bisect() ;
        break ;
      }
    case myrule_t :: e01 :
      myhface3 (2)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e20).rotate (twist (2))) ;
      myhface3 (3)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (3))) ;
      split_e01 () ;
      break ;
    case myrule_t :: e12 :
      myhface3 (0)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (0))) ;
      myhface3 (3)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (3))) ;
      split_e12 () ;
      break ;
    case myrule_t :: e20 :
      myhface3 (1)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1))) ;
      myhface3 (3)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (3))) ;
      split_e20 () ;
      break ;
    case myrule_t :: e23 :
      myhface3 (0)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (0))) ;
      myhface3 (1)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1))) ;
      split_e23 () ;
      break ;
    case myrule_t :: e30 :
      // refine faces first 
      myhface3 (1)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1))) ;
      myhface3 (2)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (2))) ;
      split_e30 () ;
      break ;
    case myrule_t :: e31 :
      cout << "Split faces e31 " << endl;
      myhface3 (0)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (0))) ;
      myhface3 (2)->refineImmediate (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (2))) ;
      split_e31 () ;
      break ;
    default :
      cerr << "**FEHLER (FATAL) beim unbedingten Verfeinern mit unbekannter Regel: " ;
      cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << endl ;
      abort () ;
      break ;
  }

  this->postRefinement () ;
  return ;
}

template < class A >  bool TetraTop < A > :: refine () 
{
  myrule_t r = _req ;
  if (r != myrule_t :: crs && r != myrule_t :: nosplit) 
  {
    if (r != getrule ()) 
    {
      assert (getrule () == myrule_t :: nosplit) ;
      _req = myrule_t :: nosplit ;
      switch (r) {
        typedef typename myhface3_t :: myrule_t  myhface3rule_t;
        case myrule_t :: crs :
        case myrule_t :: nosplit :
          return true ;
        case myrule_t :: iso8 :
          {
            for (int i = 0 ; i < 4 ; ++i )
              if (!myhface3 (i)->refine (myhface3rule_t (myhface3_t :: myrule_t :: iso4).rotate (twist (i)), twist (i))) return false ; 
          }
          break ;
        case myrule_t :: bisect : // refined edge e30
          if( elementType() == 2 && _nChild == 1 ) 
          {
            if (!myhface3 (1)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1)), twist (1))) return false ;
            if (!myhface3 (3)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e20).rotate (twist (3)), twist (3))) return false ;
          }
          else 
          {
            if (!myhface3 (1)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e20).rotate (twist (1)), twist (1))) return false ;
            if (!myhface3 (2)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (2)), twist (2))) return false ;
          }
          break ;
        case myrule_t :: e01 :
          if (!myhface3 (2)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e20).rotate (twist (2)), twist (2))) return false ;
          if (!myhface3 (3)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (3)), twist (3))) return false ;
          break ;
        case myrule_t :: e12 :
          if (!myhface3 (0)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (0)), twist (0))) return false ;
          if (!myhface3 (3)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (3)), twist (3))) return false ;
          break ;
        case myrule_t :: e20 :
          if (!myhface3 (1)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1)), twist (1))) return false ;
          if (!myhface3 (3)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (3)), twist (3))) return false ;
          break ;
  case myrule_t :: e23 :
          if (!myhface3 (0)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (0)), twist (0))) return false ;
          if (!myhface3 (1)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1)), twist (1))) return false ;
          break ;
  case myrule_t :: e30 :
          if (!myhface3 (1)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (1)), twist (1))) return false ;
          if (!myhface3 (2)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (2)), twist (2))) return false ;
          break ;
  case myrule_t :: e31 :
          if (!myhface3 (0)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (0)), twist (0))) return false ;
          if (!myhface3 (2)->refine (myhface3rule_t (myhface3_t :: myrule_t :: e01).rotate (twist (2)), twist (2))) return false ;      
          break ;
        default :
          cerr << "**WARNUNG (FEHLER IGNORIERT) falsche Verfeinerungsregel [" << getrule () ;
          cerr << "] (ignoriert) in " << __FILE__ << " " << __LINE__ << endl ;
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
        for (int i = 0 ; i < 4 ; ++i)
        {  
          if (i != fce)
            if ( ! myhface3 (i)->refine (balrule_t ( r ).rotate (twist (i)), twist (i)) ) 
              return false ;
        }
        _req = myrule_t :: nosplit ;
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

template < class A > void TetraTop < A > :: backupCMode (ostream & os) const {

  // Das backup im alten Stil, d.h. levelweise die Verfeinerungsregeln
  // vom Gitter runterschreiben. Diese Technik wird nur f"ur das backup
  // noch unterst"utzt, um die Daten mit "alteren Konstruktionen visual.
  // zu k"onnen.

  os << getrule () << " " ;
  return ;
}

// buckupTetra 
template < class A > void TetraTop < A > :: backupIndex (ostream & os) const 
{
  // write my index 
  os.write( ((const char *) & this->_idx ), sizeof(int) ) ;

  /*
  // write interior indices 
  {
    const vertex_STI * vx = this->innerVertex(); 
    if( vx ) vx->backupIndex( os );

    for(const hedge_STI * e = this->innerHedge () ; e ; e = e->next ())
    {
      e->backupIndex( os ); 
    }

    for(const hface_STI * f = this->innerHface () ; f ; f = f->next ())
    {
      f->backupIndex( os ); 
    }
  }
  */

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
template < class A > void TetraTop < A > :: 
restoreIndex (istream & is, vector<bool> (& isHole) [4] ) 
{
  // free index from constructor
  is.read ( ((char *) &(this->_idx) ), sizeof(int) );

  // mark this element a non hole 
  typedef typename Gitter :: Geometric :: BuilderIF BuilderIF; 

  // make sure sizes match 
  assert( this->getIndex() < (int) isHole[BuilderIF::IM_Elements].size() );
  // set entry to false, because this is not a hole  
  isHole[BuilderIF :: IM_Elements][this->getIndex()] = false;

  // TODO 
  // restore other indices 
  
  /*
  // write interior indices 
  {
    const vertex_STI * vx = this->innerVertex(); 
    if( vx ) vx->restoreIndex( is );

    for(const hedge_STI * e = this->innerHedge () ; e ; e = e->next ())
    {
      e->restoreIndex( is ); 
    }

    for(const hface_STI * f = this->innerHface () ; f ; f = f->next ())
    {
      f->restoreIndex( is ); 
    }
  }
  */

  {
    for (innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->restoreIndex (is, isHole ) ; 
  }
  return;
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
    cerr << "**FEHLER (FATAL): Falsche Verfeinerungsregel [" << myhface3(i)->getrule() << "] in "__FILE__ << " " << __LINE__ << endl ;
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
   
      typedef typename myhface3_t :: myrule_t myhface3rule_t;
      myhface3 (0)->refineImmediate (myhface3rule_t (r).rotate (twist (0))) ;
      myhface3 (1)->refineImmediate (myhface3rule_t (r).rotate (twist (1))) ;
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

template < class A > void Periodic3Top < A > :: backupCMode (ostream & os) const {

  // Das backup im alten Stil, d.h. levelweise die Verfeinerungsregeln
  // vom Gitter runterschreiben. Diese Technik wird nur f"ur das backup
  // noch unterst"utzt, um die Daten mit "alteren Konstruktionen visual.
  // zu k"onnen.
  
  os << getrule () << " " ;
  return ;
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
