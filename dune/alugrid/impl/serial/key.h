// (c) bernhard schupp 1997 - 1998
#ifndef KEY_H_INCLUDED
#define KEY_H_INCLUDED

namespace ALUGrid
{

  template < class A > class Key3
  {
  protected :
    A _a, _b, _c ;
  public :
    Key3 () ;
    Key3 (const A &,const A &,const A &) ;
    Key3 (const Key3 < A > &) ;
    const Key3 < A > & operator = (const Key3 < A > &) ;
    bool operator < (const Key3 < A > &) const ;
  } ;

  template < class A > class Key4
  {
    protected :
      A _a, _b, _c, _d ;
    public :
      Key4 () ;
      Key4 (const A &, const A &, const A &, const A &) ;
      Key4 (const Key4 < A > &) ;
      const Key4 < A > & operator = (const Key4 < A > &) ;
      bool operator < (const Key4 < A > &) const ;
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

} // namespace ALUGrid

#endif // #ifndef KEY_H_INCLUDED
