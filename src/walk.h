	// (c) bernhard schupp 1997 - 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.5  2004/12/21 17:36:45  robertk
 * removed some warnings.
 *
 * Revision 1.4  2004/12/20 13:55:25  robertk
 * gcc compileable.
 *
 * Revision 1.3  2004/11/29 18:00:21  robertk
 * 1 instead of true in is_def_true.
 *
 * Revision 1.2  2004/11/16 19:26:09  robertk
 * Added walk option "for all elements" .
 *
 * Revision 1.1  2004/10/25 16:39:54  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.1  2004/10/15 09:48:38  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.4  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.3  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef WALK_H_INCLUDED
#define WALK_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <functional>
  #include <list>
  #include <vector>
#else
  #include <function.h>
  #include <list.h>
  #include <vector.h>
#endif

#include "myalloc.h"
#include "gitter_sti.h"

static volatile char RCSId_walk_h [] = "$Id$" ;

template < class A > class is_def_true {
  public :
    typedef A val_t ;
    int operator () (const A * x) const { return 1 ; }
    int operator () (const A & x) const { return 1 ; }
} ;

template < class A > class is_leaf {
  public :
    typedef A val_t ;
    int operator () (const A * x) const { return x->leaf() ; }
    int operator () (const A & x) const { return x.leaf () ; }
} ;

template < class A > class is_not_leaf {
  public :
    typedef A val_t ;
    int operator () (const A * x) const { return ! x->leaf () ; }
    int operator () (const A & x) const { return ! x.leaf () ; }
} ;

template < class A > class any_has_level {
  int lvl ;
  public :
    any_has_level (int i = 0) : lvl (i) { }
    int operator () (const A * x) const { return x->level () == lvl ? 1 : 0 ; }
    int operator () (const A & x) const { return x.level () == lvl ? 1 : 0 ; }
} ;

template < class A > class leaf_has_level {
  int lvl ;
  public :
    leaf_has_level (int i = 0) : lvl (i) { }
    int operator () (const A * x) const { return x->level () == lvl ? x->leaf () : 0 ; }
    int operator () (const A & x) const { return x.level () == lvl ? x.leaf () : 0 ; }
} ;

template < class A > class has_int_vertex {
  public :
    int operator () (const A * x) const { return x->innerVertex () ? 1 : 0 ; }
    int operator () (const A & x) const { return x.innerVertex () ? 1 : 0 ; }
} ;

template < class A > class has_int_edge {
  public :
    int operator () (const A * x) const { return x->innerHedge () ? 1 : 0 ; }
    int operator () (const A & x) const { return x.innerHedge () ? 1 : 0 ; }
} ;

template < class A > class has_int_face {
  public :
    int operator () (const A * x) const { return x->innerHface () ? 1 : 0 ; }
    int operator () (const A & x) const { return x.innerHface () ? 1 : 0 ; }
} ;

template < class A > class unary_not {
  public :
    typedef typename A :: val_t val_t ;
    int operator () (val_t * x) const { return ! A()(x) ; }
    int operator () (val_t & x) const { return ! A()(x) ; }
} ;

template < class A > class childs_are_leafs {
  public :
    int operator () (const A *) const ;
    int operator () (const A &) const ;
} ;

template < class A > class ListIterator : public IteratorSTI < A >, public MyAlloc {
  list < A * > & _list ;
  typename list < A * > :: iterator _curr ;
  public :
    inline ListIterator (const ListIterator < A > &) ;
    inline ListIterator (const list < A * > &) ;
    inline ~ListIterator () ;
    inline void first () ;
    inline void next () ;
    inline int done () const ;
    inline int size () ;
    inline A & item () const ;
} ;

template < class A, class B > class TreeIterator : public IteratorSTI < A >, public MyAlloc {
  public :
    typedef B comp_t ;
    typedef A val_t ;
    inline TreeIterator (A &, const B & = B()) ;
    inline TreeIterator (A *, const B & = B()) ;
    inline TreeIterator (const TreeIterator < A, B > &) ;
    inline ~TreeIterator () ;
    inline const TreeIterator < A, B > & operator = (const TreeIterator < A, B > &) ;
    inline void first () ;
    inline void next () ;
    inline int size () ;
    inline int done () const ;
    inline val_t & item () const ;
  private :
    enum { max = 100 } ;
    A * _seed ;
    A * _stack [max] ;
    B _cmp ;
    int _pos ;
    int _cnt ;
    inline int pushdown () ;
    inline int pullup () ;
    inline int count () ;
} ;

template < class A, class B > class Wrapper : public IteratorSTI < typename B :: val_t >, public MyAlloc {
  A _walk ;
  public :
    typedef typename B :: val_t val_t ;
    inline Wrapper (const A &) ;
    inline Wrapper (const Wrapper < A, B > &) ;
    inline ~Wrapper () ;
    inline void first () ;
    inline void next () ;
    inline int size () ;
    inline int done () const ;
    inline val_t & item () const ;
} ;

template < class A, class B, class C > class AlignIterator : public IteratorSTI < C >, public MyAlloc {
  A _walk1 ;
  B _walk2 ;
  int _curr, _cnt ;
  public :
    inline AlignIterator (const A &, const B &) ;
    inline AlignIterator (const AlignIterator < A, B, C > &) ;
    inline ~AlignIterator () ;
    inline void first() ;
    inline void next () ;
    inline int size () ;
    inline int done () const ;
    inline C & item () const ;
} ;

template < class A > class VectorAlign : public IteratorSTI < A >, public MyAlloc {
  typedef IteratorSTI < A > * pointer_t ;
  vector < pointer_t > _it ;
  typename vector < pointer_t > :: const_iterator _curr, _ahead ;
  int _cnt ;
  VectorAlign (const VectorAlign < A > &) ;
  public :
    inline VectorAlign (const vector < pointer_t > &) ;
    inline ~VectorAlign () ;
    inline void first () ;
    inline void next () ;
    inline int size () ;
    inline int done () const ;
    inline A & item () const ;
} ;

template < class  A, class B > class Insert : public IteratorSTI < typename B :: val_t >, public MyAlloc {
  public :
    typedef typename B :: comp_t comp_t ;
    typedef typename B :: val_t  val_t ;
    Insert (const A &, comp_t = comp_t ()) ;
    inline Insert (const Insert < A, B > &) ;
   ~Insert () ;
    void first () ;
    void next () ;
    int done () const ;
    int size () ;
    val_t & item () const ;
  private :
    comp_t _cmp ;
    A _outer ;
    B * _inner ;
    int _cnt ;
} ;


	//
	//    #    #    #  #          #    #    #  ######
	//    #    ##   #  #          #    ##   #  #
	//    #    # #  #  #          #    # #  #  #####
	//    #    #  # #  #          #    #  # #  #
	//    #    #   ##  #          #    #   ##  #
	//    #    #    #  ######     #    #    #  ######
	//


template < class A > int childs_are_leafs < A > :: operator () (const A * x) const {
  if (x->leaf ()) { 
    return 0 ; 
  } else { 
    for (const A * y = x->down () ; y ; y = y->next ()) if (!y->leaf ()) return 0 ;
  }
  return 1 ;
}
  
template < class A > int childs_are_leafs < A > :: operator () (const A & x) const {
  if (x.leaf ()) {
    return 0 ;
  } else {
    for (const A * y = x.down () ; y ; y = y->next ()) if (!y->leaf ()) return 0 ;
  }
  return 1 ;
}

template < class A > inline ListIterator < A > :: ListIterator (const list < A * > & l) : _list ((list < A * > &)l), _curr () {
  return ;
}

template < class A > inline ListIterator < A > :: ListIterator (const ListIterator < A > & w) : _list (w._list), _curr (w._curr) {
  return ;
}

template < class A > inline ListIterator < A > :: ~ListIterator () {
  return ;
}

template < class A > inline void ListIterator < A > :: first () {
  _curr = _list.begin () ;
  return ;
}

template < class A > inline void ListIterator < A > :: next ()  {
  _curr ++ ;
  return ;
}

template < class A > inline int ListIterator < A > :: done () const {
  return _curr == _list.end () ;
}

template < class A > inline int ListIterator < A > :: size () {
  return _list.size () ;
}

template < class A > inline A & ListIterator < A > :: item () const {
  return ** _curr ;
}

template < class A, class B > inline int TreeIterator < A, B > :: pushdown () {
  A * e = _stack [_pos] ;
  for( ; e ? ! _cmp (e) : 0 ; _stack [ ++ _pos] = (e = e->down ())) ;
  return e ? 1 : (-- _pos, 0) ;
}

template < class A, class B > inline int TreeIterator < A, B > :: pullup () {
  for( ; _pos >= 0 ; _pos -- ) if (_stack [_pos] = _stack [_pos]->next ()) break ;
  return _pos < 0 ? 0 : 1 ; 
}

template < class A, class B > inline int TreeIterator < A, B > :: count () {
  int i = 0 ;
  for (first () ; ! done () ; next ()) i ++ ;
  return i ;
}

template < class A, class B > inline TreeIterator < A, B > :: TreeIterator (A & s, const B & c) 
  : _seed (& s), _cnt (-1), _pos (0), _cmp (c) {
  _stack [0] = 0 ;
  _cnt = count () ;
  return ;
}

template < class A, class B > inline TreeIterator < A, B > :: TreeIterator (A * s, const B & c) 
  : _seed (s), _cnt (-1), _pos (0), _cmp (c) {
  _stack [0] = 0 ;
  _cnt = count () ;
  return ;
}

template < class A, class B > inline TreeIterator < A, B > :: TreeIterator ( const TreeIterator < A, B > & w) 
  : _seed (w._seed), _cnt (w._cnt), _pos (w._pos), _cmp (w._cmp) {
  for(int i = 0 ; i <= _pos ; i ++) _stack [i] = w._stack [i] ;
  return ;
}

template < class A, class B > inline TreeIterator < A, B > :: ~TreeIterator () {
  return ;
}

template < class A, class B > inline const TreeIterator < A, B > & TreeIterator < A, B > :: operator = (const TreeIterator < A, B > & w) {
  _seed = w._seed ;
  _cnt = w._cnt ;
  _pos = w._pos ;
  _cmp = w._cmp ;
  for (int i = 0 ; i < _pos + 1 ; i ++) _stack [i] = w._stack [i] ;
  return w ;
}

template < class A, class B > inline void TreeIterator < A , B > :: first () {
  if (_seed) {
    * _stack = _seed ;
    _pos = 0 ;
    do {
      if(pushdown ()) return ;
    } while(pullup ()) ;
  }
  _pos = 0 ;
  * _stack = 0 ;
  return ;
}

template < class A, class B > inline void TreeIterator < A , B > :: next () {
  A * e = _stack [_pos] ;
  A * d = e->down () ;
  if (d) {
    _stack[++_pos] = d ;
    if(pushdown ()) return ;
  }
  while (pullup ()) {
    if (pushdown ()) return ;
  }
  _pos = 0 ;
  * _stack = 0 ;
  return ;
}

template < class A, class B > inline int TreeIterator < A , B > :: size () {
  assert (_cnt != -1) ;
  return  _cnt ;
}

template < class A, class B > inline int TreeIterator < A , B > :: done () const {
  return ! _stack [_pos] ;
}

template < class A, class B > inline A & TreeIterator < A , B > :: item () const {
  assert (! done ())  ;
  return * _stack [_pos] ;
}

template < class A, class B > inline Wrapper < A, B > :: Wrapper (const A & w) : _walk (w) {
  return ;
}

template < class A, class B > inline Wrapper < A, B > :: Wrapper (const Wrapper < A, B > & w) : _walk (w._walk) {
  return ;
}

template < class A, class B > inline Wrapper < A, B > :: ~Wrapper () {
  return ;
}

template < class A, class B > inline void Wrapper < A, B > :: first () {
  _walk.A :: first () ;
  return ;
}

template < class A, class B > inline void Wrapper < A, B > :: next () {
  _walk.A :: next () ;
  return ;
}

template < class A, class B > inline int Wrapper < A, B > :: size () {
  return _walk.A :: size () ;
}

template < class A, class B > inline int Wrapper < A, B > :: done () const {
  return _walk.A :: done () ;
}

template < class A, class B > inline typename Wrapper < A, B > :: val_t & 
Wrapper < A, B > :: item () const {
  assert (! done ()) ;
  return B ()(_walk.A :: item ()) ;
}

template < class A, class B, class C > 
inline AlignIterator < A, B, C > :: 
AlignIterator (const A & a, const B & b) 
  : _walk1 (a), _walk2 (b), _curr (0), _cnt (-1) {
  return ;
}

template < class A, class B, class C > 
inline AlignIterator < A, B, C > :: AlignIterator (const AlignIterator < A, B, C > & a) 
  : _walk1 (a._walk1), _walk2 (a._walk2), _curr (a._curr), _cnt (-1) {
  return ;
}

template < class A, class B, class C > inline AlignIterator < A, B, C > :: ~AlignIterator () {
  return ;
}

template < class A, class B, class C > inline void AlignIterator < A, B, C > :: first() { 
  _curr = 0 ;
  _walk1.A :: first() ;
  if(_walk1.A :: done()) {
    _curr = 1 ;
    _walk2.B :: first() ;
  }
  return ;
}

template < class A, class B, class C > inline void AlignIterator < A, B, C > :: next () {
  !_curr ? (_walk1.A::next(), (_walk1.A::done() ? (_walk2.B::first(), _curr = 1) : 0)) : (_walk2.B::next(), 0) ;
  return ;
}

template < class A, class B, class C > inline int AlignIterator < A, B, C > :: size () {
  return (_cnt == -1) ? (_cnt = _walk1.A::size () + _walk2.B::size ()) : _cnt ;
}

template < class A, class B, class C > inline int AlignIterator < A, B, C > :: done () const {
  return _curr ? _walk2.B::done () : 0 ;
}

template < class A, class B, class C > inline C & AlignIterator < A, B, C > :: item () const {
  assert (! done ()) ;
  if (_curr) return (C &) _walk2.B::item () ; 
  else return (C &) _walk1.A::item () ;
}

template < class A > inline VectorAlign < A > :: VectorAlign (const vector < pointer_t > & l) : _it (l), _curr (), _ahead (), _cnt (-1) {
  return ;
}

template < class A > VectorAlign < A > :: ~VectorAlign () {
  for (typename vector < pointer_t > :: iterator p = _it.begin () 
      ; p != _it.end () ; delete (*p ++)) ;
  return ;
}

template < class A > void VectorAlign < A > :: first () {
  for (_curr = _it.begin () ; (_curr == _it.end () ? 0 : ((*_curr)->first (), (*_curr)->done ())) ; _curr ++) ;
  _ahead = _curr;
  if (_ahead != _it.end ())
    _ahead++;
  for ( ; (_ahead == _it.end () ? 0 : ((*_ahead)->first (), (*_ahead)->done ())) ; _ahead ++) ;
  return ;
}

template < class A > void VectorAlign < A > :: next () {
  (*_curr)->next () ;
  if ((*_curr)->done ())
  {
    if (_ahead != _it.end ())
      (_curr = _ahead, _ahead ++) ;
    for ( ; (_ahead == _it.end () ? 0 : ((*_ahead)->first (), (*_ahead)->done ())) ; _ahead ++) ;
  }
  return ;
}

template < class A > int VectorAlign < A > :: size () {
  if (_cnt == -1) {
    _cnt = 0 ;
    for (typename vector < pointer_t > :: iterator p = _it.begin () ; 
        p != _it.end () ; _cnt += (*p++)->size ()) ;
  }
  return _cnt ;
}

template < class A > inline int VectorAlign < A > :: done () const {
   return ((_ahead == _it.end ()) ? ((_curr == _it.end ()) ? 1 : (*_curr)->done ()) : 0);
}

template < class A > inline A & VectorAlign < A > :: item () const {
  assert (! done ()) ;
  return (*_curr)->item () ;
}

template < class  A, class B > Insert < A, B > :: Insert (const A & w, comp_t c)
  : _outer (w), _inner (0), _cnt (0), _cmp (c) { 
  for( first () ; ! done () ; next ()) _cnt ++ ;
  return ;
}

template < class  A, class B > inline Insert < A, B > :: Insert (const Insert < A, B > & w) 
  : _outer (w._outer), _inner (0), _cnt (w._cnt), _cmp (w._cmp) { 
  _inner = w._inner ? new B (* w._inner) : 0 ;
  return ;
}

template < class  A, class B > Insert < A, B > :: ~Insert () { 
  if (_inner) delete _inner ;
  return ;
}

template < class  A, class B > void Insert < A, B > :: first () {
  if (_inner) { 
    delete _inner ;
    _inner = 0 ;
  }
  for (_outer.A::first () ; ! _outer.A::done () ; _outer.A::next ()) {
    _inner = new B (_outer.A::item (), _cmp) ;
    _inner->B::first () ;
    if(!_inner->B::done ()) break ;
    else { 
      delete _inner ;
      _inner = 0 ;
    }
  }
  return ;
}

template < class  A, class B > void Insert < A, B > :: next () {
  assert(_inner) ;
  _inner->B::next () ;
  if(_inner->B::done ()) {
    delete _inner ;
    _inner = 0 ;
    for(_outer.A::next () ; ! _outer.A::done () ; _outer.A::next ()) {
      _inner = new B(_outer.A::item (), _cmp) ;
      _inner->B::first () ;
      if(!_inner->B::done ()) break ;
      else { 
        delete _inner ;
        _inner = 0 ;
      }
    }
  }
  return ;
}
    
template < class  A, class B > int Insert < A, B > :: done () const {
  return _outer.A::done () ? 1 : _inner ? _inner->B::done () : 1 ;
}

template < class  A, class B > int Insert < A, B > :: size () {
  return _cnt ;
}

template < class  A, class B > typename Insert < A, B > :: val_t & 
Insert < A, B > :: item () const {
  assert (! done ()) ; 
  return _inner->B::item () ;
}

#endif  // WALK_H_INCLUDED
