	// (c) bernhard schupp 1997 - 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:39:53  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.1  2004/10/15 09:48:37  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.2  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef KEY_H_INCLUDED
#define KEY_H_INCLUDED

static volatile char RCSId_key_h [] = "$Id$" ;

template < class A > class Key3 {
  protected :
    A _a, _b, _c ;
  public :
    inline Key3 () ;
    inline Key3 (const A &,const A &,const A &) ;
    inline Key3 (const Key3 < A > &) ;
    inline const Key3 < A > & operator = (const Key3 < A > &) ;
    inline bool operator < (const Key3 < A > &) const ;
} ;

template < class A > class Key4 {
  protected :
    A _a, _b, _c, _d ;
  public :
    inline Key4 () ;
    inline Key4 (const A &, const A &, const A &, const A &) ;
    inline Key4 (const Key4 < A > &) ;
    inline const Key4 < A > & operator = (const Key4 < A > &) ;
    inline bool operator < (const Key4 < A > &) const ;
} ;

template < class A > inline Key3 < A > :: Key3 () : _a (-1), _b (-1), _c (-1) {
  return ;
}

template < class A > inline Key3 < A > :: Key3 (const A & a, const A & b, const A & c) : _a (a), _b (b), _c (c) {
  return ;
}

template < class A > inline Key3 < A > :: Key3 (const Key3 < A > & k) : _a (k._a), _b (k._b), _c (k._c) {
  return ;
}

template < class A > inline const Key3 < A > & Key3 < A > :: operator = (const Key3 < A > & k) {
  _a = k._a ;
  _b = k._b ;
  _c = k._c ;
  return * this ;
}

template < class A > inline bool Key3 < A > :: operator < (const Key3 < A > & k) const {
  return _a < k._a ? true : (_a == k._a ? (_b < k._b ? true : (_b == k._b ? (_c < k._c ? true : false) : false)) : false) ;
}

template < class A > inline Key4 < A > :: Key4 () : _a (), _b (), _c (), _d () {
  return ;
}

template < class A > inline Key4 < A > :: Key4 (const A & a, const A & b, const A & c, const A & d) : _a(a), _b (b), _c (c), _d (d) {
  return ;
}

template < class A > inline Key4 < A > :: Key4 (const Key4 < A > & k) : _a (k._a), _b (k._b), _c(k._c), _d (k._d) {
  return ;
}

template < class A > inline const Key4 < A > & Key4 < A > :: operator = (const Key4 < A > & k) {
  _a = k._a ;
  _b = k._b ;
  _c = k._c ;
  _d = k._d ;
  return * this ;
}

template < class A > inline bool Key4 < A >  :: operator < (const Key4 < A > & k) const {
  return _a < k._a ? true : (_a == k._a ? (_b < k._b ? true : (_b == k._b ? (_c < k._c ? true :
  (_c == k._c ? (_d < k._d ? true : false) : false)) : false)) : false) ;
}

#endif
