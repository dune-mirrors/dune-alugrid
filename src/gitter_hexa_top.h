	// (c) bernhard schupp 1997 - 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.5  2005/01/19 18:26:24  robertk
 * removed warnings.
 *
 * Revision 1.4  2005/01/19 17:45:25  robertk
 * removed warnings.
 *
 * Revision 1.3  2004/12/20 13:55:08  robertk
 * gcc compileable.
 *
 * Revision 1.2  2004/11/16 19:33:48  robertk
 * Added methods up for Hbnd4Top elements.
 *
 * Revision 1.1  2004/10/25 16:39:52  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.3  2004/10/19 16:28:36  robertk
 * backup and restore method for VertexGeo implemented. not runing at the
 * moment.
 *
 * Revision 1.2  2004/10/19 13:25:11  robertk
 * Adapted constructors of VertexGeo for IndexManagement.
 *
 * Revision 1.1  2004/10/15 09:48:37  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.6  2002/05/24 09:05:31  dedner
 * Vorl"aufig syntaktisch korrekte, d.h. kompilierbare Version
 *
 * Revision 1.5  2002/05/23 16:37:41  dedner
 * Test nach Einbau der Periodischen 4-Raender
 *
 * Revision 1.4  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.3  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef GITTER_HEXA_TOP_H_INCLUDED
#define GITTER_HEXA_TOP_H_INCLUDED

#include "mapp_cube_3d.h"

static volatile char RCSId_gitter_hexa_top_h [] = "$Id$" ;

template < class A > class Hedge1Top : public A {
  protected :
    typedef Hedge1Top < A >      	inneredge_t ;
    typedef typename A :: innervertex_t	innervertex_t ;
    typedef typename A :: myvertex_t	myvertex_t ;
    typedef typename A :: myrule_t	myrule_t ;
  private :
    int _lvl ;
    inneredge_t * _dwn, * _bbb ;
    innervertex_t * _cv ;
    myrule_t _rule ;
  public :
    inline Hedge1Top (int,myvertex_t *,myvertex_t *) ;
    virtual ~Hedge1Top () ;
    inneredge_t * subedge1 (int) ;
    const inneredge_t * subedge1 (int) const ;
    inneredge_t * down () ;
    const inneredge_t * down () const ;
    inneredge_t * next () ;
    const inneredge_t * next () const ;
    int level () const ;
    inline void append (inneredge_t *) ;
    innervertex_t * innerVertex () ;
    const innervertex_t * innerVertex () const ;
    innervertex_t * subvertex (int) ;
    const innervertex_t * subvertex (int) const ;
  public :
    virtual void backup (ostream &) const ;
    virtual void restore (istream &) ;
  public :
    virtual myrule_t getrule () const ;
    virtual void refineImmediate (myrule_t) ;
    virtual bool coarse () ;
} ;

template < class A > class Hface4Top : public A {
  protected :
    typedef Hface4Top < A >      innerface_t ;
    typedef typename A :: inneredge_t      inneredge_t ;
    typedef typename A :: innervertex_t    innervertex_t ;
    typedef typename A :: myhedge1_t       myhedge1_t ;
    typedef typename A :: myvertex_t       myvertex_t ;
    typedef typename A :: myrule_t         myrule_t ;
  private :
    innerface_t * _dwn, * _bbb ;
    innervertex_t * _cv ;
    inneredge_t   * _ed ;
    int _lvl ;
    myrule_t _rule ;
    inline myhedge1_t * subedge1 (int,int) ;
    inline const myhedge1_t * subedge1 (int,int) const ;
    void splitISO4 () ;
  public:
    inline Hface4Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
    virtual ~Hface4Top () ;
    innervertex_t * subvertex (int) ;
    const innervertex_t * subvertex (int) const ;
    inneredge_t * subedge1 (int) ;
    const inneredge_t * subedge1 (int) const ;
    innerface_t * subface4 (int) ;
    const innerface_t * subface4 (int) const ;
    int level () const ;
    innervertex_t * innerVertex () ;
    const innervertex_t * innerVertex () const ;
    inneredge_t * innerHedge () ;
    const inneredge_t * innerHedge () const ;
    innerface_t * down () ;
    const innerface_t * down () const ;
    innerface_t * next () ;
    const innerface_t * next () const ;
    void append (innerface_t * f) ;
  public :
    virtual myrule_t getrule () const ;
    virtual bool refine (myrule_t,int) ;
    virtual void refineImmediate (myrule_t) ;
    virtual bool coarse () ;
  public :
    virtual void backup (ostream &) const ;
    virtual void restore (istream &) ;
} ;

template < class A > class Hbnd4Top : public A {
  protected :
    typedef Hbnd4Top < A >        	innerbndseg_t ;
    typedef typename A :: myhface4_t  	myhface4_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    void splitISO4 () ;
    bool refineLikeElement (balrule_t) ;
  private :
    innerbndseg_t * _bbb, * _dwn, * _up ;
    int _lvl ;
    inline bool coarse () ;
    inline void append (innerbndseg_t *) ;
  public :
    inline Hbnd4Top (int,myhface4_t *,int,ProjectVertex *, innerbndseg_t * ) ;
    virtual ~Hbnd4Top () ;
    bool refineBalance (balrule_t,int) ;
    bool bndNotifyCoarsen () ;
    void restoreFollowFace () ;
    int level () const ;
    innerbndseg_t * next () ;
    innerbndseg_t * down () ;
    const innerbndseg_t * next () const ;
    const innerbndseg_t * down () const ;
    
    // for dune 
    innerbndseg_t * up () ;
    const innerbndseg_t * up () const ;
} ;

template < class A > class HexaTop : public A {
  protected :
    typedef HexaTop < A >         	innerhexa_t ;
    typedef typename A :: innerface_t	innerface_t ;
    typedef typename A :: inneredge_t	inneredge_t ;
    typedef typename A :: innervertex_t	innervertex_t ;
    typedef typename A :: myhface4_t  	myhface4_t ;
    typedef typename A :: myhedge1_t	myhedge1_t ;
    typedef typename A :: myvertex_t	myvertex_t ;
    typedef typename A :: myrule_t	myrule_t ;
    typedef typename A :: balrule_t 	balrule_t ;
    inline void refineImmediate (myrule_t) ;
    inline void append (innerhexa_t * h) ;
  private :
    innerhexa_t * _bbb, * _dwn, * _up ; //us
    innerface_t * _fc ;
    inneredge_t * _ed ;
    innervertex_t * _cv ;
    int _lvl ;
    myrule_t _rule, _req ;
    void splitISO8 () ;
    inline myhedge1_t * subedge1 (int,int) ;
    inline const myhedge1_t * subedge1 (int,int) const ;
    inline myhface4_t * subface4 (int,int) ;
    inline const myhface4_t * subface4 (int,int) const ;
  public:
    inline HexaTop (int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
		     myhface4_t *,int,myhface4_t *,int,myhface4_t *,int) ;
    virtual ~HexaTop () ;
    //testweise us
    inline innerhexa_t * up () ;
    inline const innerhexa_t * up () const;
    //testweise us
    inline innerhexa_t * down () ;
    inline const innerhexa_t * down () const ;
    inline innerhexa_t * next () ;
    inline const innerhexa_t * next () const ;
    inline innervertex_t * innerVertex () ;
    inline const innervertex_t * innerVertex () const ;
    inline inneredge_t * innerHedge () ;
    inline const inneredge_t * innerHedge () const ;
    inline innerface_t * innerHface () ;
    inline const innerface_t * innerHface () const ;
    int level () const ;
  public :
    myrule_t getrule () const ;
    bool refine () ;
    void request (myrule_t) ;
    bool refineBalance (balrule_t,int) ;
    bool coarse () ;
    bool bndNotifyCoarsen () ;
    void backupCMode (ostream &) const ;
    void backup (ostream &) const ;
    void restore (istream &) ;
} ;

// Anfang - Neu am 23.5.02 (BS)

template < class A > class Periodic4Top : public A {
  protected :
    typedef Periodic4Top < A > 		innerperiodic4_t  ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: inneredge_t 	inneredge_t ;
    typedef typename A :: innerface_t 	innerface_t ;
    typedef typename A :: myhedge1_t 	myhedge1_t ;
    typedef typename A :: myhface4_t 	myhface4_t ;
    typedef typename A :: myrule_t 	myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    inline void refineImmediate (myrule_t) ;
    inline void append (innerperiodic4_t * h) ;
  private :
    innerperiodic4_t * _dwn, * _bbb, * _up ; //us
    int _lvl ;
    myrule_t _rule ;
  private :
    void splitISO4 () ;
  protected :
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    myhface4_t * subface4 (int,int) ;
    const myhface4_t * subface4 (int i, int j) const ;
  public:
    inline Periodic4Top (int,myhface4_t *,int,myhface4_t *,int) ;
    virtual inline ~Periodic4Top () ;
    //testweise us
    inline innerperiodic4_t * up () ;
    inline const innerperiodic4_t * up () const;
    //testweise us
    inline innerperiodic4_t * down () ;
    inline const innerperiodic4_t * down () const ;
    inline innerperiodic4_t * next () ;
    inline const innerperiodic4_t * next () const ;
    inline innervertex_t * innerVertex () ;
    inline const innervertex_t * innerVertex () const ;
    inline inneredge_t * innerHedge () ;
    inline const inneredge_t * innerHedge () const ;
    inline innerface_t * innerHface () ;
    inline const innerface_t * innerHface () const ;
    inline int level () const ;
  public :
    myrule_t getrule () const ;
    bool refine () ;
    void request (myrule_t) ;
    bool refineBalance (balrule_t,int) ;
    bool coarse () ;
    bool bndNotifyCoarsen () ;
    void backupCMode (ostream &) const ;
    void backup (ostream &) const ;
    void restore (istream &) ;
};

// Ende - Neu am 23.5.02 (BS)

	//
	//    #    #    #  #          #    #    #  ######
	//    #    ##   #  #          #    ##   #  #
	//    #    # #  #  #          #    # #  #  #####
	//    #    #  # #  #          #    #  # #  #
	//    #    #   ##  #          #    #   ##  #
	//    #    #    #  ######     #    #    #  ######
	//

// #     #                                    #    #######
// #     #  ######  #####    ####   ######   ##       #      ####   #####
// #     #  #       #    #  #    #  #       # #       #     #    #  #    #
// #######  #####   #    #  #       #####     #       #     #    #  #    #
// #     #  #       #    #  #  ###  #         #       #     #    #  #####
// #     #  #       #    #  #    #  #         #       #     #    #  #
// #     #  ######  #####    ####   ######  #####     #      ####   #


template < class A > inline Hedge1Top < A > :: Hedge1Top (int l, myvertex_t * a, myvertex_t * b) 
  : A (a,b), _lvl (l), _dwn (0), _bbb (0), _cv (0), _rule (myrule_t :: nosplit) {
  return ;
}

template < class A > Hedge1Top < A > :: ~Hedge1Top () {
  //_bbb ? (delete _bbb, 0) : 0 ;
  //_dwn ? (delete _dwn, 0) : 0 ;
  //_cv  ? (delete _cv,  0) : 0 ;
  if(_bbb) delete _bbb;
  if(_dwn) delete _dwn;
  if(_cv)  delete _cv;
  return ;
}

template < class A > int Hedge1Top < A > :: level () const {
  return _lvl ;
}

template < class A > Hedge1Top < A > * Hedge1Top < A > :: down () {
  return _dwn ;
}

template < class A > const Hedge1Top < A > * Hedge1Top < A > :: down () const {
  return _dwn ;
}

template < class A > Hedge1Top < A > * Hedge1Top < A > :: next () {
  return _bbb ;
}

template < class A > const Hedge1Top < A > * Hedge1Top < A > :: next () const {
  return _bbb ;
}

template < class A > void Hedge1Top < A > :: backup (ostream & os) const {
  os.put ((char) getrule ()) ;
  //myvertex(0)->backup( os );
  //myvertex(1)->backup( os );
  {for (const inneredge_t * d = down () ; d ; d = d->next ()) d->backup (os) ; }
  return ;
}

template < class A > void Hedge1Top < A > :: restore (istream & is) {
  char r = (char) is.get () ;
  //myvertex(0)->restore( is );
  //myvertex(1)->restore( is );
  refineImmediate (myrule_t (r)) ;
  {for (inneredge_t * d = down () ; d ; d = d->next ()) d->restore (is) ; }
  return ;
}

template < class A >  inline void Hedge1Top < A > :: append (inneredge_t * e) {
  assert (!_bbb && e) ; 
  _bbb = e ;
  return ;
}

template < class A > typename Hedge1Top < A > :: myrule_t 
Hedge1Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > void Hedge1Top < A > :: refineImmediate (myrule_t r) {
  if (r != getrule ()) {
    assert (getrule () == myrule_t :: nosplit) ;
    switch (r) {
      case myrule_t :: iso2 :
        {
          int l = 1 + level () ;
          assert (_cv == 0 && _dwn == 0) ;
          // the last myvertex(0) is submitted for the indexmanager reference, rk
          _cv = new innervertex_t (l, .5 * (this->myvertex(0)->Point()[0] + this->myvertex(1)->Point()[0]),
			      .5 * (this->myvertex(0)->Point()[1] + this->myvertex(1)->Point()[1]),  
			      .5 * (this->myvertex(0)->Point()[2] + this->myvertex(1)->Point()[2]) , *(this->myvertex(0)) ) ;
          assert (_cv) ;
          inneredge_t * e0 = new inneredge_t (l, this->myvertex(0), _cv) ;
          inneredge_t * e1 = new inneredge_t (l, _cv, this->myvertex(1)) ;
          assert (e0 && e1) ;
          (_dwn = e0)->append (e1) ;
          _rule = myrule_t :: iso2 ;
          break ;
        }
      default :
        cerr << "**FEHLER (FATAL) falsche Verfeinerungsregel [" << r ;
        cerr << "] in " << __FILE__ << " " << __LINE__ << endl ;
        abort () ;
        break ;
    }
    this->postRefinement () ;
  }
  return ;
}

template < class A > bool Hedge1Top < A > :: coarse () {
  if (this->leaf ()) return false ;
  bool x = true ;

	// Der Wert von x bleibt 'true' falls alle Kinder der Kante
	// Bl"atter sind und zudem keine Referenzen auf diese Kanten
	// gesetzt sind. Andernfalls liegt kein vergr"oberungsf"ahiger
	// Knoten vor.
	// Vorsicht: Im parallelen Gitter bleiben auch Kanten ohne
	// Refcount stehen, um konsistente "Uberg"ange zu erhalten.

  for (inneredge_t * f = this->down () ; f ; f = f->next ()) {
    if (f->leaf ()) {
      x &= ! f->ref ;
    } else {
      x = false ;
      f->coarse () ;
    }
  }
  if (x) {

	// Falls lockedAgainstCoarsening () aufgerufen 'true' liefert
	// soll die Operation des Vergr"oberns nicht sofort ausgef"uhrt
	// sondern (pending) zur"uckgestellt werden.

    if (!this->lockedAgainstCoarsening ()) {
      delete _dwn ; 
      _dwn = 0 ;
      delete _cv ;
      _cv = 0 ;
      _rule = myrule_t :: nosplit ;
    }
  }
  return x ;
}

template < class A > Hedge1Top < A > * Hedge1Top < A > :: subedge1 (int n) {
  assert (n == 0 || n == 1) ;
  assert (n ? this->down ()->next () : this->down ()) ;
  return n ? this->down ()->next () : this->down () ;
}

template < class A > const Hedge1Top < A > * Hedge1Top < A > :: subedge1 (int n) const {
  assert (n == 0 || n == 1) ;
  assert (n ? this->down ()->next () : this->down ()) ;
  return n ? this->down ()->next () : this->down () ;
}

template < class A > inline typename Hedge1Top < A > :: innervertex_t * 
Hedge1Top < A > :: innerVertex () {
  return _cv ;
}

template < class A > inline const typename Hedge1Top < A > :: innervertex_t * Hedge1Top < A > :: innerVertex () const {
  return _cv ;
}

template < class A > inline typename Hedge1Top < A > :: innervertex_t * Hedge1Top < A > :: subvertex (int) {
  return _cv ;
}

template < class A > inline const typename Hedge1Top < A > :: innervertex_t * Hedge1Top < A > :: subvertex (int) const {
  return _cv ;
}

// #     #                                 #       #######
// #     #  ######    ##     ####   ###### #    #     #      ####   #####
// #     #  #        #  #   #    #  #      #    #     #     #    #  #    #
// #######  #####   #    #  #       #####  #    #     #     #    #  #    #
// #     #  #       ######  #       #      #######    #     #    #  #####
// #     #  #       #    #  #    #  #           #     #     #    #  #
// #     #  #       #    #   ####   ######      #     #      ####   #


template < class A > typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: down () {
  return _dwn ;
}

template < class A > const typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: down () const {
  return _dwn ;
}

template < class A > typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: next () {
  return _bbb ;
}

template < class A > const typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: next () const {
  return _bbb ;
}

template < class A > int Hface4Top < A > :: level () const {
  return _lvl ;
}

template < class A > typename Hface4Top < A > :: myhedge1_t * 
Hface4Top < A > :: subedge1 (int i,int j) {
  assert(j == 0 || j == 1) ;
  return this->myhedge1 (i)->subedge1 (j ? 1 - this->twist(i) : this->twist(i)) ;
}

template < class A > const typename Hface4Top < A > :: myhedge1_t * 
Hface4Top < A > :: subedge1 (int i,int j) const {
  assert(j == 0 || j == 1) ;
  return this->myhedge1 (i)->subedge1 (j ? 1 - this->twist(i) : this->twist(i)) ;
}

template < class A > typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: subvertex (int) {
  assert (getrule() == myrule_t :: iso4) ;
  return _cv ;
}

template < class A > const typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: subvertex (int) const {
  assert (getrule() == myrule_t :: iso4) ;
  return _cv ;
}

template < class A > typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: subedge1 (int n) {
  inneredge_t * e = _ed ;
  for (int i = 0 ; i < n ; i ++ ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > const typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: subedge1 (int n) const {
  const inneredge_t * e = _ed ;
  for (int i = 0 ; i < n ; i ++ ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > typename Hface4Top < A > :: innerface_t * 
Hface4Top < A > :: subface4 (int n) {
  innerface_t * f = this->down () ;
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > const typename Hface4Top < A > :: innerface_t * 
Hface4Top < A > :: subface4 (int n) const {
  const innerface_t * f = this->down () ;
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > inline Hface4Top < A > :: Hface4Top (int l, myhedge1_t * e0, int t0, myhedge1_t * e1, int t1, 
  myhedge1_t * e2, int t2, myhedge1_t * e3, int t3) : A (e0, t0, e1, t1, e2, t2, e3, t3), 
  _dwn (0), _bbb (0), _cv (0), _ed (0), _lvl (l), _rule (myrule_t :: nosplit) {
  return ;
}

template < class A > Hface4Top < A > :: ~Hface4Top () {
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  if (_ed) delete _ed ;
  if (_cv) delete _cv ;
  return ;
}

template < class A > typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: innerVertex () {
  return _cv ;
}

template < class A > const typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: innerVertex () const {
  return _cv ;
}

template < class A > typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: innerHedge () {
  return _ed ;
}

template < class A > const typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: innerHedge () const {
  return _ed ;
}

template < class A > inline void Hface4Top < A > :: append (innerface_t * f) {
  assert (_bbb == 0) ;
  _bbb = f ;
  return ;
}

template < class A > typename Hface4Top < A > :: myrule_t 
Hface4Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > void Hface4Top < A > :: splitISO4 () {
  int l = 1 + level () ;
  assert (_cv == 0 && _ed == 0 && _dwn == 0) ;
  {
    BilinearSurfaceMapping 
      map(this->myvertex (0)->Point(),
          this->myvertex (1)->Point(), 
          this->myvertex (2)->Point(), 
          this->myvertex (3)->Point()) ;
    double p [3] ;
    map.map2world( .0, .0, p) ;
    // myvertex(0) is submitted for the indexmanager reference 
    _cv = new innervertex_t (l, p[0], p[1], p[2], *(this->myvertex(0))) ;
    assert (_cv) ;
  }
  myvertex_t * ev0 = this->myhedge1(0)->subvertex (0) ;
  myvertex_t * ev1 = this->myhedge1(1)->subvertex (0) ;
  myvertex_t * ev2 = this->myhedge1(2)->subvertex (0) ;
  myvertex_t * ev3 = this->myhedge1(3)->subvertex (0) ;
  assert(ev0 && ev1 && ev2 && ev3) ;
  inneredge_t * e0 = new inneredge_t (l, ev0, _cv) ;
  inneredge_t * e1 = new inneredge_t (l, ev1, _cv) ;
  inneredge_t * e2 = new inneredge_t (l, ev2, _cv) ;
  inneredge_t * e3 = new inneredge_t (l, ev3, _cv) ;
  assert( e0 && e1 && e2 && e3) ;
  e0->append(e1) ;
  e1->append(e2) ;
  e2->append(e3) ;
  innerface_t * f0 = new innerface_t (l, this->subedge1(0,0), this->twist(0), e0, 0, e3, 1, this->subedge1(3,1), this->twist(3)) ;
  innerface_t * f1 = new innerface_t (l, this->subedge1(0,1), this->twist(0), this->subedge1(1,0), this->twist(1), e1, 0, e0, 1) ;
  innerface_t * f2 = new innerface_t (l, e1, 1, this->subedge1(1,1), this->twist(1), this->subedge1(2,0), this->twist(2), e2, 0) ;
  innerface_t * f3 = new innerface_t (l, e3, 0, e2, 1, this->subedge1(2,1), this->twist(2), this->subedge1(3,0), this->twist(3)) ;
  assert (f0 && f1 && f2 && f3) ;  
  f0->append(f1) ;
  f1->append(f2) ;
  f2->append(f3) ;
  _ed  = e0 ;
  _dwn = f0 ;
  _rule = myrule_t :: iso4 ;
  return ;
}

template < class A > void Hface4Top < A > :: refineImmediate (myrule_t r) {
  if (r != getrule ()) {
    assert (getrule () == myrule_t :: nosplit) ;
    switch(r) {
      typedef typename myhedge1_t :: myrule_t myhedge1rule_t;
      case myrule_t :: iso4 :
      this->myhedge1 (0)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (this->twist (0))) ;
      this->myhedge1 (1)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (this->twist (1))) ;
      this->myhedge1 (2)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (this->twist (2))) ;
      this->myhedge1 (3)->refineImmediate (myhedge1rule_t (myhedge1_t :: myrule_t :: iso2).rotate (this->twist (3))) ;
      splitISO4 () ;
      break ;
    default :
      cerr << "**FEHLER (FATAL) falsche Verfeinerungsregel [" << r ;
      cerr << "] in " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
      break ;
    }
// ??	// Die nichtkonforme Nachbarschaft noch erg"anzen und auf der
// ??	// Fl"ache die Situation nach der Verfeinerung vervollst"andigen.
// ??	
// ??   {for (innerface_t * f = down () ; f ; f = f->next ()) f->nb = nb ; }
    this->postRefinement () ;
  }
  return ;
}

template < class A > bool Hface4Top < A > :: refine (myrule_t r, int twist) {
  if (r != getrule ()) {
    assert (getrule () == myrule_t :: nosplit ? 1 : 
    	(cerr << "**FEHLER beim Verfeinern mit Regel " << r << " auf " << getrule () << endl, 0)) ;
    switch(r) {
      case myrule_t :: iso4 :
      {
	
	bool a = twist < 0 ? this->nb.front ().first->refineBalance (r,this->nb.front ().second)
                           : this->nb.rear ().first->refineBalance  (r,this->nb.rear  ().second) ;
	
	if (a) {	
	  if (getrule () == myrule_t :: nosplit) {
	    refineImmediate (r) ;
	    {for (innerface_t * f = down () ; f ; f = f->next ()) f->nb = this->nb ; }
	  } else {
	    assert (getrule () == myrule_t :: iso4) ;
	  }
	  return true ;
	} else {
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

template < class A > bool Hface4Top < A > :: coarse () {
  innerface_t * f = down() ;
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
  } while (f = f->next()) ;
  if (x) {
    
	// Hier wird tats"achlich vergr"obert, d.h. alle Kinder 
	// werden beseitigt, und das Bezugsobjekt wird zum neuen
	// Blatt im Baum.
    
    delete _dwn ; 
    _dwn = 0 ;
    delete _ed ;
    _ed = 0 ;
    delete _cv ;
    _cv = 0 ;
    _rule = myrule_t :: nosplit ;
    {for (int i = 0 ; i < 4 ; i ++ ) this->myhedge1 (i)->coarse () ; }
  }
  return x ;
}

template < class A > void Hface4Top < A > :: backup (ostream & os) const {
  os.put ((char) getrule ()) ;
  {for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ; }
  {for (const innerface_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > void Hface4Top < A > :: restore (istream & is) {
  refineImmediate (myrule_t ((char) is.get ())) ;
  {for (inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->restore (is) ; }
  {for (innerface_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
  return ;
}

// #     #                         #       #######
// #     #  #####   #    #  #####  #    #     #      ####   #####
// #     #  #    #  ##   #  #    # #    #     #     #    #  #    #
// #######  #####   # #  #  #    # #    #     #     #    #  #    #
// #     #  #    #  #  # #  #    # #######    #     #    #  #####
// #     #  #    #  #   ##  #    #      #     #     #    #  #
// #     #  #####   #    #  #####       #     #      ####   #

template < class A > inline Hbnd4Top < A > :: Hbnd4Top (int l, myhface4_t * f, int i, ProjectVertex *ppv, innerbndseg_t * up)
  : A (f, i,ppv), _bbb (0), _dwn (0), _up(up) , _lvl (l) {
  return ;
}

template < class A > Hbnd4Top < A > :: ~Hbnd4Top () {
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  return ;
}

template < class A > int Hbnd4Top < A > :: level () const {
  return _lvl ;
}

template < class A > typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: next () { 
  return _bbb ;
}

template < class A > const typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: next () const { 
  return _bbb ;
}

template < class A > typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: down () { 
  return _dwn ;
}

template < class A > const typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: down () const { 
  return _dwn ;
}

template < class A > typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: up () { 
  return _up ;
}

template < class A > const typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > ::up () const { 
  return _up ;
}

template < class A > inline void Hbnd4Top < A > :: append (innerbndseg_t * b) {
  assert (_bbb == 0) ;
  _bbb = b ;
  return ;
}

template < class A > bool Hbnd4Top < A > :: coarse () {
  innerbndseg_t * b = down () ;
  if (!b) return false ;
  bool x = true ;
  do {
    if(b->myhface4(0)->ref > 1) (b->coarse (), x = false) ;
  } while (b = b->next()) ;
  if (x) {
    if (! this->lockedAgainstCoarsening ()) {
      this->preCoarsening () ;
      delete _dwn ;
      _dwn = 0 ;
      this->myhface4 (0)->coarse () ;
    }
  }
  return x ;
}

template < class A > inline bool Hbnd4Top < A > :: bndNotifyCoarsen () {
  return coarse () ;
}

template < class A > inline void Hbnd4Top < A > :: splitISO4 () {
  int l = 1 + level () ;
  assert (_dwn == 0) ;
  innerbndseg_t * b0 = new innerbndseg_t (l, this->subface4 (0,0), this->twist (0), this->projection, this) ;
  innerbndseg_t * b1 = new innerbndseg_t (l, this->subface4 (0,1), this->twist (0), this->projection, this) ;
  innerbndseg_t * b2 = new innerbndseg_t (l, this->subface4 (0,2), this->twist (0), this->projection, this) ;
  innerbndseg_t * b3 = new innerbndseg_t (l, this->subface4 (0,3), this->twist (0), this->projection, this) ;
  assert (b0 && b1 && b2 && b3) ;
  b0->append(b1) ;
  b1->append(b2) ;
  b2->append(b3) ;
  _dwn = b0 ;
  return ;
}

template < class A > inline bool Hbnd4Top < A > :: refineBalance (balrule_t r, int b) {

	// Die Methode refineBalance () f"uhrt auf dem Randabschluss entweder
	// unbedingt die Verfeinerung durch, da im Verlauf der Verfeinerung keine
	// weiteren Anforerungen mehr an den Randabschluss  gerichtet werden 
	// ODER gibt die Verfeinerung als nicht erf"ullt zur"uck: Dann liegt
	// es am Aufrufer die Verfeinerung nochmals anzuforern.

  assert (b == 0) ;
  assert (this->leaf ()) ;
  if (! bndNotifyBalance (r,b)) {
  
  	// Hier kann der innere Rand [parallel] die Verfeinerung
	// verhindern, damit z.B. das Durchverfeinern im anisotropen
	// Fall erstmal nicht stattfindet, wenn nicht klar ist, wie die
	// weitere Rekursion aussieht. Dazu muss auf dem Niveau der Klasse
	// des Template-Arguments die Methode bndNotifyBalance () "uber-
	// schrieben werden. Die Defaultmethode liefert immer 'true'.
  
    return false ;
  } else {
    if(r == myrule_t :: iso4) {
    
    	// Der Rand verfeinert unbedingt die anliegende Fl"ache und dann
	// sich selbst, weil die Anforderung durch die Fl"ache kam, und
	// dahinter keine Balancierung stattfinden muss.
    
      this->myhface4 (0)->refineImmediate (r) ;
      splitISO4 () ;
    } else {
      cerr << "**FEHLER (FATAL, weil nicht vorgesehen) beim Verfeinern am " ;
      cerr << "Randst\"uck mit der Regel [" << r << "] in " ;
      cerr << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    
    	// postRefinement () gibt die M"oglichkeit auf dem Niveau des
	// Template-Arguments eine Methode aufzurufen, um eventuelle
	// Operationen auf dem verfeinerten Randst"uck aufzurufen.
    
    this->postRefinement () ;
    return true ;
  }
}

template < class A > inline bool Hbnd4Top < A > :: refineLikeElement (balrule_t r) {

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
	// der Fl"ache, da getrule () auf myhface4 (0)->getrule () umgeleitet
	// ist.
	
      assert (this->getrule () == myrule_t :: nosplit) ;
      switch (r) {
      case balrule_t :: iso4 :
        if (! this->myhface4 (0)->refine(balrule_t (balrule_t :: iso4).rotate (this->twist (0)), this->twist (0))) return false ;
        splitISO4 () ;
        return true ;
      default :
        cerr << "**WARNUNG (FEHLER IGNORIERT) falsche Verfeinerungsregel [" << this->getrule () ;
        cerr << "] (ignoriert) in " << __FILE__ << " " << __LINE__ << endl ;
        return false ;
      }
    }
  }
}

template < class A > void Hbnd4Top < A > :: restoreFollowFace () {

	// retoreFollowFace () veranlasst das Randelement sich am 
	// bestehenden Fl"achenbaum wiederherzustellen durch die
	// entsprechende Verfeinerung.
	
  myhface4_t & f (*(this->myhface4 (0))) ;
  if (!f.leaf ()) {
    balrule_t r = f.getrule () ;
    switch (r) {
      case myrule_t :: iso4 :
        splitISO4 () ;
	break ;
      default :
        cerr << "**FEHLER (FATAL, weil nicht vorgesehen) beim Verfeinern am " ;
        cerr << "Randst\"uck mit der Regel [" << r << "] in " ;
        cerr << __FILE__ << " " << __LINE__ << endl ;
        abort () ;
	break ;
    }
    this->postRefinement () ;
    {for (innerbndseg_t * b = down () ; b ; b = b->next ()) b->restoreFollowFace () ; }
  }
  return ;
}

// #     #                         #######
// #     #  ######  #    #    ##      #      ####   #####
// #     #  #        #  #    #  #     #     #    #  #    #
// #######  #####     ##    #    #    #     #    #  #    #
// #     #  #         ##    ######    #     #    #  #####
// #     #  #        #  #   #    #    #     #    #  #
// #     #  ######  #    #  #    #    #      ####   #

template < class A > inline typename HexaTop < A > :: myhedge1_t * HexaTop < A > :: subedge1 (int i, int j) {
  return (j < 4) ? ((this->twist (i) < 0) ? this->myhface4 (i)->myhedge1 ((8 - j + this->twist (i)) % 4) : 
    this->myhface4 (i)->myhedge1 ((j + this->twist (i)) % 4)) : 
    ((this->twist (i) < 0) ? this->myhface4 (i)->subedge1 ((12 - j + this->twist (i)) % 4) : 
    this->myhface4 (i)->subedge1 ((j + this->twist (i)) % 4)) ;
}
    
template < class A > inline const typename HexaTop < A > :: myhedge1_t * HexaTop < A > :: subedge1 (int i, int j) const {
  return (j < 4) ? ((this->twist (i) < 0) ? this->myhface4 (i)->myhedge1 ((8 - j + this->twist (i)) % 4) : 
      this->myhface4 (i)->myhedge1 ((j + this->twist (i)) % 4)) : 
	((this->twist (i) < 0) ? this->myhface4 (i)->subedge1 ((12 - j + this->twist (i)) % 4) : 
	this->myhface4 (i)->subedge1 ((j + this->twist (i)) % 4)) ;
}
    
template < class A > inline typename HexaTop < A > :: myhface4_t * HexaTop < A > :: subface4 (int i, int j) {
  return (this->myhface4(i)->getrule() == myhface4_t :: myrule_t :: iso4) ? 
	this->myhface4(i)->subface4(this->twist(i) < 0 ? (9 - j + this->twist(i)) % 4 : (j + this->twist(i)) % 4) :
	(abort (), (myhface4_t *)0) ;
}
    
template < class A > inline const typename HexaTop < A > :: myhface4_t * HexaTop < A > :: subface4 (int i, int j) const {
  return (this->myhface4(i)->getrule() == myhface4_t :: myrule_t :: iso4) ? 
    this->myhface4(i)->subface4(this->twist(i) < 0 ? (9 - j + this->twist(i)) % 4 : (j + this->twist(i)) % 4) :
    (abort (), (const myhface4_t *)0) ;
}

template < class A > inline HexaTop < A > :: HexaTop (int l, myhface4_t * f0, int t0, myhface4_t * f1, int t1, 
	myhface4_t * f2, int t2, myhface4_t * f3, int t3, myhface4_t * f4, int t4, myhface4_t * f5, int t5) 
  : A (f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5)
  , _bbb (0), _dwn (0), _up(0), _fc (0), _ed (0), _cv (0), _lvl (l),
   _rule (myrule_t :: nosplit), _req (myrule_t :: nosplit) { //_up eingef. us
  return ;
}

template < class A > HexaTop < A > :: ~HexaTop () {
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  if (_fc) delete _fc ;
  if (_ed) delete _ed ;
  if (_cv) delete _cv ;
  return ;
}

//us eingefuegt
template < class A > inline typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: up () {
  return _up ;
} 

template < class A > inline const typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: up () const {
   return _up ; 
}
//ende us

template < class A > inline typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: down () {
  return _dwn ;
}

template < class A > inline const typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: down () const {
  return _dwn ;
}

template < class A > inline typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: next () {
  return _bbb ;
}

template < class A > inline const typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: next () const {
  return _bbb ;
}

template < class A > inline typename HexaTop < A > :: innervertex_t * HexaTop < A > :: innerVertex () {
  return _cv ;
}

template < class A > inline const typename HexaTop < A > :: innervertex_t * HexaTop < A > :: innerVertex () const {
  return _cv ;
}

template < class A > inline typename HexaTop < A > :: inneredge_t * HexaTop < A > :: innerHedge () {
  return _ed ;
}

template < class A > inline const typename HexaTop < A > :: inneredge_t * HexaTop < A > :: innerHedge () const {
  return _ed ;
}

template < class A > inline typename HexaTop < A > :: innerface_t * HexaTop < A > :: innerHface () {
  return _fc ;
}

template < class A > inline const typename HexaTop < A > :: innerface_t * HexaTop < A > :: innerHface () const {
  return _fc ;
}

template < class A > inline void HexaTop < A > :: append (HexaTop < A > * h) {
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > int HexaTop < A > :: level () const {
  return _lvl ;
}

template < class A > void HexaTop < A > :: splitISO8 () {
  int l = 1 + level () ;
  assert (_dwn == 0 && _fc == 0 && _ed == 0 && _cv == 0) ;  
  {
    TrilinearMapping map(
        this->myvertex(0)->Point(), this->myvertex(1)->Point(),
	this->myvertex(2)->Point(), this->myvertex(3)->Point(), this->myvertex(4)->Point(),
	this->myvertex(5)->Point(), this->myvertex(6)->Point(), this->myvertex(7)->Point()) ;
    double p[3] ;
    map.map2world(.0, .0, .0, p) ;
    _cv = new innervertex_t (l, p[0], p[1], p[2], *(this->myvertex(0)) ) ;
    assert (_cv) ;
  }
  myvertex_t * fv0 = this->myhface4 (0)->subvertex (0) ;
  myvertex_t * fv1 = this->myhface4 (1)->subvertex (0) ;
  myvertex_t * fv2 = this->myhface4 (2)->subvertex (0) ;
  myvertex_t * fv3 = this->myhface4 (3)->subvertex (0) ;
  myvertex_t * fv4 = this->myhface4 (4)->subvertex (0) ;
  myvertex_t * fv5 = this->myhface4 (5)->subvertex (0) ;
  assert(fv0 && fv1 && fv2 && fv3 && fv4 && fv5) ;
  inneredge_t * e0 = new inneredge_t (l, fv0, _cv) ;
  inneredge_t * e1 = new inneredge_t (l, fv1, _cv) ;
  inneredge_t * e2 = new inneredge_t (l, fv2, _cv) ;
  inneredge_t * e3 = new inneredge_t (l, fv3, _cv) ;
  inneredge_t * e4 = new inneredge_t (l, fv4, _cv) ;
  inneredge_t * e5 = new inneredge_t (l, fv5, _cv) ;
  assert(e0 && e1 && e2 && e3 && e4 && e5) ;
  e0->append(e1) ;
  e1->append(e2) ;
  e2->append(e3) ;
  e3->append(e4) ;
  e4->append(e5) ;
  innerface_t * f0 = new innerface_t (l, this->subedge1 (2, 7), 0, e2, 0, e5, 1, this->subedge1 (5, 4), 1) ;
  innerface_t * f1 = new innerface_t (l, this->subedge1(2, 5), 1, this->subedge1 (3, 7), 0, e3, 0, e2, 1) ;
  innerface_t * f2 = new innerface_t (l, e3, 1, this->subedge1 (3, 5), 1, this->subedge1 (4, 7), 0, e4, 0) ;
  innerface_t * f3 = new innerface_t (l, e5, 0, e4, 1, this->subedge1 (4, 5), 1, this->subedge1 (5, 6), 0) ;
  innerface_t * f4 = new innerface_t (l, this->subedge1 (0, 7), 0, e0, 0, e2, 1, this->subedge1 (2, 4), 1) ;
  innerface_t * f5 = new innerface_t (l, this->subedge1 (0, 5), 1, this->subedge1 (4, 4), 0, e4, 0, e0, 1) ; 
  innerface_t * f6 = new innerface_t (l, e4, 1, this->subedge1 (4, 6), 1, this->subedge1 (1, 6), 0, e1, 0) ;
  innerface_t * f7 = new innerface_t (l, e2, 0, e1, 1, this->subedge1 (1, 4), 1, this->subedge1 (2, 6), 0) ;
  innerface_t * f8 = new innerface_t (l, this->subedge1 (0, 4), 0, e0, 0, e5, 1, this->subedge1 (5, 7), 1) ;
  innerface_t * f9 = new innerface_t (l, this->subedge1 (0, 6), 1, this->subedge1 (3, 4), 0, e3, 0, e0, 1) ;
  innerface_t * f10 = new innerface_t (l, e3, 1, this->subedge1 (3, 6), 1, this->subedge1 (1, 5), 0, e1, 0) ;
  innerface_t * f11 = new innerface_t (l, e5, 0, e1, 1, this->subedge1 (1, 7), 1, this->subedge1 (5, 5), 0) ;
  assert(f0 && f1 && f2 && f3 && f4 && f5 && f6 && f7 && f8 && f9 && f10 && f11) ;
  f0->append(f1) ;
  f1->append(f2) ;
  f2->append(f3) ;
  f3->append(f4) ;
  f4->append(f5) ;
  f5->append(f6) ;
  f6->append(f7) ;
  f7->append(f8) ;
  f8->append(f9) ;
  f9->append(f10) ;
  f10->append(f11) ;
  innerhexa_t * h0 = new innerhexa_t (l, this->subface4 (0, 0), this->twist (0), f0, 0, this->subface4 (2, 0), this->twist (2), f4, 0, f8, -4, this->subface4 (5, 0), this->twist (5)) ;
  innerhexa_t * h1 = new innerhexa_t (l, this->subface4 (0, 3), this->twist (0), f1, 0, this->subface4 (2, 1), this->twist (2), this->subface4 (3, 0), this->twist (3), f9, -4, f4, -1) ;
  innerhexa_t * h2 = new innerhexa_t (l, this->subface4 (0, 2), this->twist (0), f2, 0,f9, 0, subface4 (3, 1), this->twist (3), this->subface4 (4, 0), this->twist (4), f5, -1) ;
  innerhexa_t * h3 = new innerhexa_t (l, this->subface4 (0, 1), this->twist (0), f3, 0, f8, 0, f5, 0, this->subface4(4, 1), this->twist (4), this->subface4(5, 3), this->twist (5)) ;
  innerhexa_t * h4 = new innerhexa_t (l, f0, -1, this->subface4(1, 0), this->twist (1), this->subface4(2, 3), this->twist (2), f7, 0, f11, -4, this->subface4(5, 1), this->twist (5)) ;
  innerhexa_t * h5 = new innerhexa_t (l, f1, -1, this->subface4(1, 1), this->twist (1), this->subface4(2, 2), this->twist (2), this->subface4(3, 3), this->twist (3), f10, -4, f7, -1) ;
  innerhexa_t * h6 = new innerhexa_t (l, f2, -1, this->subface4(1, 2), this->twist (1), f10, 0, this->subface4(3, 2), this->twist (3), this->subface4(4, 3), this->twist (4), f6, -1) ;
  innerhexa_t * h7 = new innerhexa_t (l, f3, -1, this->subface4(1, 3), this->twist (1), f11, 0, f6, 0, this->subface4(4, 2), this->twist (4), this->subface4(5, 2), this->twist (5)) ;
  assert(h0 && h1 && h2 && h3 && h4 && h5 && h6 && h7) ;
  h0->append(h1) ;
  h1->append(h2) ;
  h2->append(h3) ;
  h3->append(h4) ;
  h4->append(h5) ;
  h5->append(h6) ;
  h6->append(h7) ;
  _ed = e0 ;
  _fc = f0 ;
  _dwn = h0 ;
  _rule = myrule_t :: iso8 ;
  return ;
}

template < class A > typename HexaTop < A > :: myrule_t HexaTop < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > void HexaTop < A > :: request (myrule_t r) {
  assert (r.isValid ()) ;
  _req = r ;
  return ;
}

template < class A > void HexaTop < A > :: refineImmediate (myrule_t r) {
  assert (getrule () == myrule_t :: nosplit) ;
  switch(r) {
    case myrule_t :: iso8 :
    
	// Das refineImmediate (..) auf allen Fl"achen wird vom Hexa :: refine (..)
	// zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind wenn
	// "uberall hface4 :: refine (..) true geliefert hat, wohl aber z.B. von
	// restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
	// erzwingen m"ussen und d"urfen.
    
      {
        typedef typename myhface4_t :: myrule_t myhface4rule_t;
        for (int i = 0 ; i < 6 ; i ++)
        this->myhface4 (i)->refineImmediate (myhface4rule_t (myhface4_t :: myrule_t :: iso4).rotate (this->twist (i))) ; }
      splitISO8 () ;
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

template < class A > bool HexaTop < A > :: refine () {
  myrule_t r = _req ;
  if (r != myrule_t :: crs && r != myrule_t :: nosplit) {
    if (r != getrule ()) {
      assert (getrule () == myrule_t :: nosplit) ;
      _req = myrule_t :: nosplit ;
      switch (r) {
        case myrule_t :: crs :
        case myrule_t :: nosplit :
          return true ;
        case myrule_t :: iso8 :
	  {
      typedef typename myhface4_t :: myrule_t myhface4rule_t;
      for (int i = 0 ; i < 6 ; i ++ )
            if (!this->myhface4 (i)->refine (myhface4rule_t (myhface4_t :: myrule_t :: iso4)
	  					.rotate (this->twist (i)), this->twist (i))) return false ;
          }
          refineImmediate (r) ;
          return true ;
        default :
          cerr << "**WARNUNG (FEHLER IGNORIERT) falsche Verfeinerungsregel [" << getrule () ;
          cerr << "] (ignoriert) in " << __FILE__ << " " << __LINE__ << endl ;
          return false ;
      }
    }
  }
  return true ;
}

template < class A > bool HexaTop < A > :: refineBalance (balrule_t r, int fce) {
  assert (r == balrule_t :: iso4) ;
  if (getrule () == myrule_t :: nosplit) {
    if (! this->myhface4 (fce)->leaf ()) {
      for (int i = 0 ; i < 6 ; i ++)
        if (i != fce)
          if (!this->myhface4 (i)->refine (balrule_t (balrule_t :: iso4).rotate (this->twist (i)), this->twist (i))) 
	    return false ;
      _req = myrule_t :: nosplit ;
      refineImmediate (myrule_t :: iso8) ;
    }
  }
  return true ;
}

template < class A > bool HexaTop < A > :: coarse () {
  if (this->leaf ()) {
    assert (_req == myrule_t :: nosplit || _req == myrule_t :: crs) ;
    myrule_t w = _req ;
    _req = myrule_t :: nosplit ;
    if (w != myrule_t :: crs) return false ;
    for (int i = 0 ; i < 6 ; i ++) if (!this->myhface4 (i)->leaf ()) return false ;
    return true ;
  } else {
    assert (_req == myrule_t :: nosplit) ;
    bool x = true ;
    {for (innerhexa_t * h = down () ; h ; h = h->next ()) x &= h->coarse () ; }
    if (x) {
      this->preCoarsening () ;
      delete _dwn ; 
      _dwn = 0 ;
      delete _fc ;
      _fc = 0 ;
      delete _ed ;
      _ed = 0 ;
      delete _cv ; 
      _cv = 0 ;
      _rule = myrule_t :: nosplit ;
      {
        for (int i = 0 ; i < 6 ; i ++ ) {
      	  this->myneighbour (i).first->bndNotifyCoarsen () ;
          this->myhface4 (i)->coarse () ;
	}
      }
      return false ;
    }
  }
  return false ;
}

template < class A > bool HexaTop < A > :: bndNotifyCoarsen () {
  return true ;
}

template < class A > void HexaTop < A > :: backupCMode (ostream & os) const {

	// Das backup im alten Stil, d.h. levelweise die Verfeinerungsregeln
	// vom Gitter runterschreiben. Diese Technik wird nur f"ur das backup
	// noch unterst"utzt, um die Daten mit "alteren Konstruktionen visual.
	// zu k"onnen.

  os << getrule () << " " ;
  return ;
}

template < class A > void HexaTop < A > :: backup (ostream & os) const {
  os.put ((char) getrule ()) ;
  {for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ; }
  {for (const innerface_t * f = innerHface () ; f ; f = f->next ()) f->backup (os) ; }
  {for (const innerhexa_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > void HexaTop < A > :: restore (istream & is) {

	// restore () stellt den Elmentbaum aus der Verfeinerungs
	// geschichte wieder her. Es ruft refine () auf und testet
	// auf den korrekten Vollzug der Verfeinerung. Danach werden
	// die inneren Gitterteile restore'd.

  myrule_t r ((char) is.get ()) ;
  assert(getrule() == myrule_t :: nosplit) ;
  if (r == myrule_t :: nosplit) {
  
	// Vorsicht: beim restore m"ussen sich sowohl Element als auch
	// Randelement um die Korrektheit der Nachbarschaft k"ummern,
	// und zwar dann wenn sie "on the top" sind (= die gelesene
	// Verfeinerungsregel ist nosplit). (s.a. beim Randelement)
  
    for (int i = 0 ; i < 6 ; i ++) {
      myhface4_t & f (*(this->myhface4 (i))) ;
      if (!f.leaf ()) {
        for (int j = 0 ; j < 4 ; j ++) f.subface4 (j)->nb.complete (f.nb) ;
      }
    }
  } else {
    request (r) ;
    refine () ;
    assert (getrule() == r) ;
    {for (inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->restore (is) ; }
    {for (innerface_t * f = innerHface () ; f ; f = f->next ()) f->restore (is) ; }
    {for (innerhexa_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
  }
  return ;
}

// Anfang - Neu am 23.5.02 (BS)

// ######                                                          #       #######
// #     #  ######  #####      #     ####   #####      #     ####  #    #     #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #     #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #     #
// #        #       #####      #    #    #  #    #     #    #      #######    #
// #        #       #   #      #    #    #  #    #     #    #    #      #     #
// #        ######  #    #     #     ####   #####      #     ####       #     #

   
template < class A > inline Periodic4Top < A > :: Periodic4Top (int l, myhface4_t * f0, int t0,
	myhface4_t * f1, int t1) : A (f0, t0, f1, t1), _dwn (0), _bbb (0), _up(0), _lvl (l), 
	_rule (myrule_t :: nosplit) {	// ok
  return ;
}

template < class A > inline Periodic4Top < A > :: ~Periodic4Top () {	// ok
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  return ;
}

template < class A > inline int Periodic4Top < A > :: level () const {	// ok
  return _lvl ;
}

//us eingefuegt
template < class A > inline typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: up () {
  return _up ;
}

template < class A > inline const typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: up () const {
   return _up ; 
}
//ende us

template < class A > inline typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: down () { // ok
  return _dwn ;
}

template < class A > inline const typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: down () const { // ok
  return _dwn ;
}

template < class A > inline typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: next () { // ok
  return _bbb ;
}

template < class A > inline const typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: next () const { // ok
  return _bbb ;
}

template < class A > inline typename Periodic4Top < A > :: innervertex_t * Periodic4Top < A > :: innerVertex () { // ok
  return 0 ;
}

template < class A > inline const typename Periodic4Top < A > :: innervertex_t * Periodic4Top < A > :: innerVertex () const { // ok
  return 0 ;
}

template < class A > inline typename Periodic4Top < A > :: inneredge_t * Periodic4Top < A > :: innerHedge () { // ok
  return 0 ;
}

template < class A > inline const typename Periodic4Top < A > :: inneredge_t * Periodic4Top < A > :: innerHedge () const { // ok
  return 0 ;
}

template < class A > inline typename Periodic4Top < A > :: innerface_t * Periodic4Top < A > :: innerHface () { // ok
  return 0 ;
}

template < class A > inline const typename Periodic4Top < A > :: innerface_t * Periodic4Top < A > :: innerHface () const { // ok
  return 0 ;
}

template < class A > inline void Periodic4Top < A > :: append (Periodic4Top < A > * h) { // ok
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > typename Periodic4Top < A > :: myhedge1_t * Periodic4Top < A > :: subedge1 (int i, int j) {	// ??
  assert (getrule () == myrule_t :: iso4) ;
  return (j < 4) ? ((this->twist (i) < 0) ? this->myhface4 (i)->myhedge1 ((8 - j + this->twist (i)) % 4) : 	// aus dem Hexaeder
    this->myhface4 (i)->myhedge1 ((j + this->twist (i)) % 4)) : 
    ((this->twist (i) < 0) ? this->myhface4 (i)->subedge1 ((12 - j + this->twist (i)) % 4) : 
    this->myhface4 (i)->subedge1 ((j + this->twist (i)) % 4)) ;
}

template < class A > const typename Periodic4Top < A > :: myhedge1_t * Periodic4Top < A > :: subedge1 (int i, int j) const { // ok
  return ((Periodic4Top < A > *)this)->subedge1 (i,j) ;
}

template < class A > typename Periodic4Top < A > ::  myhface4_t * Periodic4Top < A > :: subface4 (int i, int j) { // ok
  return (this->myhface4(i)->getrule() == myhface4_t :: myrule_t :: iso4) ? 
	this->myhface4(i)->subface4(this->twist(i) < 0 ? (9 - j + this->twist(i)) % 4 : (j + this->twist(i)) % 4) :	// Zeile aus dem Hexaeder
	(abort (), (myhface4_t *)0) ;
}

template < class A > const typename Periodic4Top < A > ::  myhface4_t * Periodic4Top < A > :: subface4 (int i, int j) const {
  return ((Periodic4Top < A > *)this)->subface4 (i,j) ;
}

template < class A > typename Periodic4Top < A > :: myrule_t Periodic4Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > void Periodic4Top < A > :: request (myrule_t) {

	// Einen Request zur Verfeinerung zu setzen, ist vorl"aufig inhaltlich nicht
	// vorgesehen und wird deshalb ignoriert (leise).

  return ;
}

template < class A > void Periodic4Top < A > :: splitISO4 () {	// ok
  int l = 1 + level () ;
  innerperiodic4_t * p0 = new innerperiodic4_t (l, this->subface4 (0,0), this->twist (0), this->subface4 (1,0), this->twist (1)) ;
  innerperiodic4_t * p1 = new innerperiodic4_t (l, this->subface4 (0,1), this->twist (0), this->subface4 (1,3), this->twist (1)) ;
  innerperiodic4_t * p2 = new innerperiodic4_t (l, this->subface4 (0,2), this->twist (0), this->subface4 (1,2), this->twist (1)) ;
  innerperiodic4_t * p3 = new innerperiodic4_t (l, this->subface4 (0,3), this->twist (0), this->subface4 (1,1), this->twist (1)) ;
  assert (p0 && p1 && p2 && p3) ;
  p0->append(p1) ;
  p1->append(p2) ;
  p2->append(p3) ;
  _dwn = p0 ;
  _rule = myrule_t :: iso4 ;
  return ;
}

template < class A > void Periodic4Top < A > :: refineImmediate (myrule_t r) {

	// Die Methode wird nur vom restore () und vom refineBalance () auf-
	// gerufen und geht davon aus, dass das betroffene Element noch nicht
	// verfeinert ist -> ist ein Blatt der Hierarchie.

  assert (this->leaf()) ;
  switch (r) {
    case myrule_t :: iso4 :
    
	// Das refineImmediate (..) auf allen Fl"achen wird vom periodic4 :: refine (..)
	// zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
	// "uberall hface4 :: refine (..) true geliefert hat, wohl aber z.B. von
	// restore () oder abgeleiteten Funktionen, die eine direkte Verfeinerung
	// erzwingen m"ussen und d"urfen.
    
      typedef typename myhface4_t :: myrule_t myhface4rule_t;
      this->myhface4 (0)->refineImmediate (myhface4rule_t (r).rotate (this->twist (0))) ;
      this->myhface4 (1)->refineImmediate (myhface4rule_t (r).rotate (this->twist (1))) ;
      splitISO4 () ;
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

template < class A > bool Periodic4Top < A > :: refine () { // ok

	// Das refine () reagiert nicht auf die Elementaktivierung zur Verfeinerung
	// in der globalen Schleife, weil das perioodische Randelement sich nur auf
	// Anforderung zur Balancierung aus einem anliegenden Element direkt verfeinert.

  return true ;
}

template < class A > bool Periodic4Top < A > :: refineBalance (balrule_t r, int fce) {  // ok
  if (r != balrule_t :: iso4) {
    cerr << "**WARNUNG (IGNORIERT) in Periodic4Top < A > :: refineBalance (..) nachschauen, Datei " 
    	 << __FILE__ << " Zeile " << __LINE__ << endl ;
	 
	// Bisher kann die Balancierung nur die isotrope Achtelung handhaben,
	// falls mehr gew"unscht wird, muss es hier eingebaut werden. Im Moment wird
	// die Balancierung einfach verweigert, d.h. die Verfeinerung des anfordernden
	// Elements f"allt flach.
	
    return false ;
  } else {
  
	// Der nachfolgende Aufruf nutzt aus, dass die Regel der periodischen R"ander
	// sich direkt auf die Balancierungsregel des entsprechenden Polygonverbinders
	// projezieren l"asst (n"amlich 1:1). Deshalb unterscheidet der Aufruf nicht nach
	// der angeforderten Regel in einer 'case' Anweisung.
	
    typedef typename myhface4_t :: myrule_t myhface4rule_t;
    int opp = fce == 0 ? 1 : 0 ;
    if (this->myhface4 (opp)->refine (myhface4rule_t (r).rotate (this->twist (opp)), this->twist (opp))) {
      refineImmediate (r) ;
      return true ;
    } else {
      return false ;
    }
  }
}

template < class A > bool Periodic4Top < A > :: coarse () { // ok

	// Das Vergr"obern geschieht auch passiv, sobald ein anliegendes Element
	// vergr"obert wird durch den Aufruf von "bndNotifyCoarsen ()" s.u.

  bndNotifyCoarsen () ;
  return false ;
}

template < class A > bool Periodic4Top < A > :: bndNotifyCoarsen () {

	// Wie beim Randelement auch: Die Vergr"oberung eines anliegenden Elements
	// l"ost einen Vorgang aus, der feststellt ob das periodische RE ebenfalls
	// vergr"obert werden soll.
  
  innerperiodic4_t * p = down () ;
  if (!p) return false ;
  bool x = true ;
  do {

	// Falls p kein Blatt der Hierarchie ist, 
	// die Vergr"oberungsm"oglichkeit weitergeben.

    if (!p->leaf ()) p->coarse () ;

	// F"ur die hintere und vordere Fl"ache feststellen, ob
	// der Referenzenz"ahler mehr als einen Eintrag ergibt.

    if (p->myhface4 (0)->ref > 1) (x = false) ;
    if (p->myhface4 (1)->ref > 1) (x = false) ;

  } while (p = p->next ()) ;
  if (x) {
  
	// Falls keine Fl"achen anliegen, die auf Kinder oder Kindes- 
	// mit mehr als einer Referenz f"uhren, ist sicher, dass das
	// Bezugsrandelement zwischen zwei 'relativ groben' Elementen
	// liegt. Somit kann es vergr"obert werden.
  
    this->preCoarsening () ;
    delete _dwn ;
    _dwn = 0 ;
    _rule = myrule_t :: nosplit ;
    this->myhface4 (0)->coarse () ;
    this->myhface4 (1)->coarse () ;
  }
  return x ;
}

template < class A > void Periodic4Top < A > :: backupCMode (ostream & os) const {

	// Das backup im alten Stil, d.h. levelweise die Verfeinerungsregeln
	// vom Gitter runterschreiben. Diese Technik wird nur f"ur das backup
	// noch unterst"utzt, um die Daten mit "alteren Konstruktionen visual.
	// zu k"onnen.
	
  os << getrule () << " " ;
  return ;
}

template < class A > void Periodic4Top < A > :: backup (ostream & os) const {
  os.put ((char) getrule ()) ;
  {for (const innerperiodic4_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > void Periodic4Top < A > :: restore (istream & is) {
  myrule_t r ((char) is.get ()) ;
  assert(getrule () == myrule_t :: nosplit) ;	// Testen auf unverfeinerten Zustand
  if (r == myrule_t :: nosplit) {
    for (int i = 0 ; i < 2 ; i ++) {
      myhface4_t & f (*(this->myhface4 (i))) ;
      if (!f.leaf ()) {
        switch (f.getrule ()) {
	  case balrule_t :: iso4 :
            {for (int j = 0 ; j < 4 ; j ++) f.subface4 (j)->nb.complete (f.nb) ;}
	    break ;
	  default :
	    cerr << "**FEHLER (FATAL) beim restore mit unbekannter Balancierungsregel: "
                 << "[" << r << "]. In " << __FILE__ << __LINE__ << endl ;
	    abort () ;
	    break ;
	}
      }
    }
  } else {
    refineImmediate (r) ;
    assert (getrule() == r) ;
    {for (innerperiodic4_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
  }
  return ;
}

// Ende - Neu am 23.5.02 (BS)

#endif	//	GITTER_HEXA_TOP_H_INCLUDED
