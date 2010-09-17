// (c) Bernhard Schupp 1997 - 1998
// modifications for dune interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef WALK_H_INCLUDED
#define WALK_H_INCLUDED

#include "myalloc.h"
#include "gitter_sti.h"

// always returns 1 for walk over all elements   
template < class A > class is_def_true {
  public :
    typedef A val_t ;
    int operator () (const A * x) const { return 1 ; }
    int operator () (const A & x) const { return 1 ; }
} ;

// for leaf iterator , return true is elements is leaf 
template < class A > class is_leaf {
  public :
    typedef A val_t ;
    int operator () (const A * x) const { return (*this)( *x ); }
    int operator () (const A & x) const { return x.leaf () ; }
} ;

// for leaf iterator , return true is elements is not leaf 
template < class A > class is_not_leaf {
  public :
    typedef A val_t ;
    int operator () (const A * x) const { return (*this)( *x ); }
    int operator () (const A & x) const { return ! x.leaf () ; }
} ;


// returns true if given element has the level 
template < class A > class any_has_level {
  int lvl ;
  public :
    typedef A val_t ;
    any_has_level (int i = 0) : lvl (i) { }
    any_has_level ( const any_has_level & org ) : lvl (org.lvl) { }
    int operator () (const A * x) const { return (*this)( *x ); }
    int operator () (const A & x) const { return x.level () == lvl ? 1 : 0 ; }

    // return given level 
    int level () const { return lvl; }
} ;

// returns true if given element has the level 
template < class A > class any_has_level_periodic {
  int lvl ;
  public :
    typedef A val_t ;
    any_has_level_periodic (int i = 0) : lvl (i) { }
    any_has_level_periodic ( const any_has_level_periodic & org ) : lvl (org.lvl) { }

    int operator () (const A * x) const { return (*this)( *x ); }
    int operator () (const A & x) const { return (x.level () == lvl) && ( ! x.leaf()  || x.isLeafEntity() ); }
    // return given level 
    int level () const { return lvl; }
} ;
template < class A > class leaf_has_level {
  int lvl ;
  public :
    typedef A val_t ;
    leaf_has_level (int i = 0) : lvl (i) { }
    int operator () (const A * x) const { return (*this)( *x ); }
    int operator () (const A & x) const { return x.level () == lvl ? x.leaf () : 0 ; }
} ;

template < class A > class has_int_vertex {
  public :
    int operator () (const A * x) const { return (*this)( *x ); }
    int operator () (const A & x) const { return x.innerVertex () ? 1 : 0 ; }
} ;

template < class A > class has_int_edge {
  public :
    int operator () (const A * x) const { return (*this)( *x ); }
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

//! new rule for Dune LeafIterator 
//! all entities with hav either the level or are leaf entities with one
//! level <= the desired level 
template < class A > class leaf_or_has_level
{
  int lvl_;
  public :
    //! Constructor storing the level 
    leaf_or_has_level (int i = 0) : lvl_ (i) { }

    //! check if go next
    int operator () (const A * x) const
    {
      int level = x->level () ;
      return ((level == lvl_) || (x->leaf () && (level <= lvl_) )) ? 1 : 0 ;
    }

    //! check if go next
    int operator () (const A & x) const
    {
      int level = x.level () ;
      return ((level == lvl_) || (x.leaf () && (level <= lvl_) )) ? 1 : 0 ;
    }
};

//! new rule for Dune LeafIterator minus one 
//! all entities with are either leaf entities or thier children are leaf entities 
template < class A > class leaf_or_father_of_leaf
{
  public :
    typedef A val_t ;

    //! Constructor storing the level 
    leaf_or_father_of_leaf () {}

    //! check if go next
    int operator () (const A * x) const
    {
      return ( x->leaf() || ( (!x->leaf()) && (x->down()->leaf()) ) ) ? 1 : 0 ;
    }

    //! check if go next
    int operator () (const A & x) const
    {
      return ( x.leaf() || ( (!x.leaf()) && (x.down()->leaf() ) ) ) ? 1 : 0 ;
    }
};

//! new rule for Dune LeafIterator minus one 
//! all entities with are either leaf entities or thier children are leaf entities 
template < class A > class is_leaf_entity
{
  public :
    typedef A val_t ;

    //! Constructor storing the level 
    is_leaf_entity () {}

    //! check if go next
    int operator () (const A * x) const
    {
      return ( x->isLeafEntity() ) ? 1 : 0; 
    }

    //! check if go next
    int operator () (const A & x) const
    {
      return ( x.isLeafEntity() ) ? 1 : 0; 
    }
};

//! new rule for Dune LeafIterator minus one 
//! all entities with are either leaf entities or thier children are leaf entities 
template < class A > class is_interior_leaf_entity
{
  public :
    typedef A val_t ;

    //! Constructor storing the level 
    is_interior_leaf_entity () {}

    //! check if go next
    int operator () (const A * x) const
    {
      return ( x->isLeafEntity() && x->isInterior() ) ? 1 : 0; 
    }

    //! check if go next
    int operator () (const A & x) const
    {
      return ( x.isLeafEntity() && x.isInterior() ) ? 1 : 0; 
    }
};

//**********************************************************************************
//**********************************************************************************
//**********************************************************************************

template < class A > class ListIterator : public IteratorSTI < A >, public MyAlloc 
{
  typedef list < A * > ListType;
  ListType& _list ;
  typename ListType :: iterator _curr ;
  public :
    // createing ListIterator, default status is done 
    inline ListIterator (const ListType &) ;
    // create ListIterator by making a copy  
    inline ListIterator (const ListIterator < A > &) ;
    inline ~ListIterator () ;
    inline void first () ;
    inline void next () ;
    inline int done () const ;
    inline int size () ;
    inline A & item () const ;
    // make a copy of the iterator 
    inline virtual IteratorSTI < A > * clone () const;
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
    inline virtual IteratorSTI < A > * clone () const;
  private :
    // maximal depth of a grid hierarchy, not possible an a 64bit system yet.
    // this is a secure upper bound 
    enum { maxStackDepth = 16 } ;

    A * _seed ;
    A * _stack [ maxStackDepth ] ;
    B _cmp ;
    int _pos ;
    int _cnt ;
    inline int pushdown () ;
    inline int pullup () ;
    inline int count () const ;
    // make a copy of given iterator 
    inline void assignIterator (const TreeIterator < A, B > &) ;
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
    inline virtual IteratorSTI < val_t > * clone () const ;
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
    inline IteratorSTI < C > * clone () const;
  private:  
} ;

template < class A > class VectorAlign : public IteratorSTI < A >, public MyAlloc {
  typedef IteratorSTI < A > * pointer_t ;
  vector < pointer_t > _it ;
  typename vector < pointer_t > :: const_iterator _curr, _ahead ;
  int _cnt ;
  public :
    inline VectorAlign (const vector < pointer_t > &) ;
    inline VectorAlign (const VectorAlign < A > &) ;
    inline ~VectorAlign () ;
    inline void first () ;
    inline void next () ;
    inline int size () ;
    inline int done () const ;
    inline A & item () const ;
    inline virtual IteratorSTI < A > * clone () const;
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
    inline virtual IteratorSTI < val_t > * clone () const;
  private :
    void removeObj(); 
    // count items 
    int count () const; 

    A _outer ;
    B * _inner ;
    int _cnt ;
    comp_t _cmp ;
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

template < class A > inline ListIterator < A > :: ListIterator (const ListType& l) 
  : _list ((ListType &)l), _curr (_list.end()) {
  return ;
}

template < class A > inline ListIterator < A > :: 
ListIterator (const ListIterator < A > & w) : _list(w._list) , _curr(w._curr)
{
  return ;
}

template < class A > inline IteratorSTI< A > * ListIterator < A > :: 
clone () const
{
  return new ListIterator < A > (*this);
}

template < class A > inline ListIterator < A > :: ~ListIterator () {
  return ;
}

template < class A > inline void ListIterator < A > :: first () {
  _curr = _list.begin () ;
  return ;
}

template < class A > inline void ListIterator < A > :: next ()  {
  ++ _curr ;
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

/////////////////////////////////////////////////////////////////////////////////////
//
//  --TreeIterator 
//
/////////////////////////////////////////////////////////////////////////////////////

template < class A, class B > inline int TreeIterator < A, B > :: pushdown () {
  A * e = _stack [_pos] ;
  for( ; e ? ! _cmp (e) : 0 ; _stack [ ++ _pos] = (e = e->down ())) ;
  return e ? 1 : (-- _pos, 0) ;
}

template < class A, class B > inline int TreeIterator < A, B > :: pullup () {
  for( ; _pos >= 0 ; _pos -- ) if ( (_stack [_pos] = _stack [_pos]->next () ) ) break ;
  return _pos < 0 ? 0 : 1 ; 
}

template < class A, class B > inline int TreeIterator < A, B > :: count () const
{
  // we cannot use our own iterator, because counting will cahnge the
  // status of the iterator 
  TreeIterator < A, B > counter(*this); 
  int i = 0 ;
  for (counter.first () ; ! counter.done () ; counter.next ()) ++ i ;
  return i ;
}

template < class A, class B > inline TreeIterator < A, B > :: TreeIterator (A & s, const B & c) 
  : _seed (& s), _cmp (c), _pos (0), _cnt (-1) {
  _stack [0] = 0 ;
  return ;
}

template < class A, class B > inline TreeIterator < A, B > :: TreeIterator (A * s, const B & c) 
  : _seed (s), _cnt (-1), _pos (0), _cmp (c) {
  _stack [0] = 0 ;
  return ;
}

template < class A, class B > inline TreeIterator < A, B > :: TreeIterator ( const TreeIterator < A, B > & w) 
{
  assignIterator(w);
  return ;
}

template < class A, class B > inline IteratorSTI < A > *  TreeIterator < A, B > :: 
clone () const
{
  return new TreeIterator < A, B > (*this);
}

template < class A, class B > inline TreeIterator < A, B > :: ~TreeIterator () {
  return ;
}

template < class A, class B > inline void TreeIterator < A, B > :: 
assignIterator (const TreeIterator < A, B > & w) 
{
  // make a copy of iterator 
  _seed = w._seed ;
  _cnt  = w._cnt ;
  _pos  = w._pos ;
  _cmp  = w._cmp ;

  assert( _pos < maxStackDepth );
  for (int i = 0 ; i <= _pos ; ++i) _stack [i] = w._stack [i] ;
}

template < class A, class B > inline const TreeIterator < A, B > & TreeIterator < A, B > :: operator = (const TreeIterator < A, B > & w) 
{
  assignIterator(w);
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
  return  (_cnt == -1) ? (_cnt = count()) : _cnt ;
}

template < class A, class B > inline int TreeIterator < A , B > :: done () const {
  assert( _pos >= 0 );
  assert( _pos < maxStackDepth );
  return ! _stack [_pos] ;
}

template < class A, class B > inline A & TreeIterator < A , B > :: item () const {
  assert (! done ())  ;
  return * _stack [_pos] ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//  --Wrapper 
//
////////////////////////////////////////////////////////////////////////////////////////

template < class A, class B > inline Wrapper < A, B > :: Wrapper (const A & w) : _walk (w) {
  return ;
}

template < class A, class B > inline Wrapper < A, B > :: Wrapper (const Wrapper < A, B > & w) : _walk (w._walk) {
  return ;
}

template < class A, class B > inline IteratorSTI< typename B::val_t > * 
Wrapper < A, B > :: clone () const
{
  return new Wrapper < A, B > (*this);
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

////////////////////////////////////////////////////////////////////////////
//
//  --AlignIterator 
//
////////////////////////////////////////////////////////////////////////////

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

template < class A, class B, class C > 
inline IteratorSTI < C > * AlignIterator < A, B, C > :: 
clone () const
{
  return new AlignIterator < A, B, C > (*this);
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

////////////////////////////////////////////////////////////////////////////////
//
//  --VectorAlign 
//
////////////////////////////////////////////////////////////////////////////////

template < class A > inline VectorAlign < A > :: 
VectorAlign (const vector < pointer_t > & l) 
  : _it (l), _curr (_it.end()), _ahead (_it.end()), _cnt (-1) {
  return ;
}

template < class A > inline VectorAlign < A > :: 
VectorAlign (const VectorAlign < A > & org ) 
  : _it(org._it.size(),0) , _curr(_it.end()) , _ahead(_it.end()) , _cnt(org._cnt) 
{
  // _it is a vector with pointers of iterators, we have to clone all
  // pointers of the vector, therefore iterate over vector and call clone  
  typedef typename vector < pointer_t > :: iterator iterator_t ; 
  typedef typename vector < pointer_t > :: const_iterator constiterator_t ; 
  constiterator_t orgit = org._it.begin();
  iterator_t pend   = _it.end (); 
  for (iterator_t p = _it.begin (); p != pend ; ++ p , ++ orgit ) 
  {
    (*p) = (*orgit)->clone();
    if( org._curr  == orgit ) _curr  = p;
    if( org._ahead == orgit ) _ahead = p;
  }

  assert( (! org.done()) ? (_curr  != _it.end()) : 1 );
  assert( (! org.done()) ? ((org._ahead != org._it.end()) ? (_ahead != _it.end()) : 1) : 1 );
  assert( (! org.done()) ? (!done()) : 1); 
}

template < class A > inline IteratorSTI < A > * 
VectorAlign < A > :: clone () const 
{
  return new VectorAlign < A > (*this);
}

template < class A > VectorAlign < A > :: ~VectorAlign () {
  typedef typename vector < pointer_t > :: iterator iterator_t ; 
  const iterator_t pend = _it.end (); 
  for (iterator_t p = _it.begin (); p != pend ; ++p ) 
  {
    delete *p;
  }
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

////////////////////////////////////////////////////////////////////////////
//
//  --Insert 
//
////////////////////////////////////////////////////////////////////////////

template < class  A, class B > Insert < A, B > :: Insert (const A & w, comp_t c)
  : _outer (w), _inner (0), _cnt (-1), _cmp (c) 
{ 
  return ;
}

template < class  A, class B > inline Insert < A, B > :: Insert (const Insert < A, B > & w) 
  : _outer (w._outer)
  // if w_inner exsists, call copy constructor 
  , _inner ( (w._inner) ? new B (* (w._inner) ) : 0 )
  , _cnt (w._cnt), _cmp (w._cmp) 
{ 
  return ;
}

template < class  A, class B > inline IteratorSTI< typename B :: val_t > * 
Insert < A, B > :: clone () const
{
  return new Insert < A, B > (*this); 
}

template < class  A, class B > Insert < A, B > :: ~Insert () 
{ 
  removeObj(); 
}

template < class  A, class B > inline void Insert < A, B > :: removeObj () 
{ 
  if (_inner) 
  { 
    delete _inner ;
    _inner = 0;
  }
  return ;
}

template < class  A, class B > inline void Insert < A, B > :: first () 
{
  // deletes _inner 
  removeObj(); 
  
  for (_outer.A::first () ; ! _outer.A::done () ; _outer.A::next ()) 
  {
    _inner = new B (_outer.A::item (), _cmp) ;
    _inner->B::first () ;
    if(!_inner->B::done ()) break ;
    else { 
      removeObj();
    }
  }
  return ;
}

template < class  A, class B > inline void Insert < A, B > :: next () {
  assert(_inner) ;
  _inner->B::next () ;
  if(_inner->B::done ()) 
  {
    removeObj();
    
    for(_outer.A::next () ; ! _outer.A::done () ; _outer.A::next ()) 
    {
      _inner = new B(_outer.A::item (), _cmp) ;
      _inner->B::first () ;
      if(!_inner->B::done ()) 
        break ;
      else 
        removeObj();
    }
  }
  return ;
}
    
template < class  A, class B > inline int Insert < A, B > :: done () const {
  return _outer.A::done () ? 1 : _inner ? _inner->B::done () : 1 ;
}

template < class  A, class B > inline int Insert < A, B > :: size () 
{
  return (_cnt == -1) ? (_cnt = count()) : _cnt;
}

template < class  A, class B > inline typename Insert < A, B > :: val_t & 
Insert < A, B > :: item () const 
{
  assert (! done ()) ; 
  return _inner->B::item () ;
}

template < class  A, class B > inline int Insert < A, B > :: count () const
{
  Insert < A, B > counter(*this);
  int i = 0;
  for(counter.first(); !counter.done(); counter.next()) ++ i;
  return i;
}

#endif  // WALK_H_INCLUDED
