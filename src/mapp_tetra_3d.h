	// (c) mario ohlberger, 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.2  2004/10/28 16:26:06  robertk
 * minor changes.
 *
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
 * Revision 1.3  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef MAPP_TETRA_3D_H_INCLUDED
#define MAPP_TETRA_3D_H_INCLUDED

#include <math.h>
#include <stdlib.h>

#include "gitter_sti.h"

static volatile char RCSId_mapp_tetra_3d_h [] = "$Id$" ;

class LinearMapping {
  private :
    const double (&p0)[3], (&p1)[3], (&p2)[3], (&p3)[3];
    double a[4][3] ;
    double Df[3][3] ;
    double Dfi[3][3] ;
    double DetDf ;
    void inverse () ;
  public :
    LinearMapping (const double (&)[3], const double (&)[3], const double (&)[3], const double (&)[3]) ;
    LinearMapping (const LinearMapping &) ;
   ~LinearMapping () {}
    double det () const { return DetDf ; }
    void map2world (const double (&)[4], double (&)[3]) const ;
    void map2world (const double , const double , const double, const double, double (&)[3]) const ;
    void world2map (const double (&)[3], double (&)[4]) ;
} ;

class quadraturTetra3Dbasis {
  protected :
    static const double _p1 [4] ;
    static const double _w2 [4] ;
    static const double _p2 [4][4] ;
    static const double _w7 [64] ;
    static const double _p7 [64][4] ;
} ;

template < class A > class quadraturTetra3D : private quadraturTetra3Dbasis {
  private :
    LinearMapping _map ;
  public :
    typedef typename A :: val_t val_t;
    typedef typename A :: arg_t arg_t;
    quadraturTetra3D (const LinearMapping & m) : _map (m) {}
   ~quadraturTetra3D () {}
    inline val_t integrate1 (val_t, const arg_t & = arg_t ()) ;
    inline val_t integrate2 (val_t, const arg_t & = arg_t ()) ;
    inline val_t integrate7 (val_t, const arg_t & = arg_t ()) ;
} ;

class FunctionWrapper {
  public :
    struct arg {
	double (*f)(const double (&)[4], LinearMapping &, void *) ;
	void *user ;
	arg () { abort() ; } ;
	arg ( double (*p)(const double (&)[4], LinearMapping &, void * ), void *a ) : f(p), user(a) {} ;
    };
    typedef double val_t ;
    typedef arg arg_t ;
  public :
    inline val_t operator () (const double (&)[4], LinearMapping &, const arg_t & ) ;
} ;


class LinearSurfaceMapping {
  const double (&_p0)[3], (&_p1)[3], (&_p2)[3] ;
  double _b [3][3] ;
protected:
  double _n [3] ;
public :
    inline LinearSurfaceMapping (const double (&)[3], const double (&)[3], const double (&)[3]) ;
    inline LinearSurfaceMapping (const LinearSurfaceMapping &) ;
   ~LinearSurfaceMapping() { }
    inline void map2world(const double (&)[3], double (&)[3]) const ;
    inline void map2world(double x, double y, double z, double (&w)[3]) const ;
    inline void normal(double (&)[3]) const;
} ;

class quadraturTriang2Dbasis {
  protected :
    static const double _p1 [3] ;
    static const double _w3 [7] ;
    static const double _p3 [7][3] ;
    static const double _w5 [7] ;
    static const double _p5 [7][3] ;
    static const double _w7 [16] ;
    static const double _p7 [16][3] ;
} ;

template < class A > class quadraturTriang2D : private quadraturTriang2Dbasis {
  LinearSurfaceMapping _map ;
  public:
    typedef typename A :: val_t val_t;
    typedef typename A :: arg_t arg_t;
    quadraturTriang2D(const LinearSurfaceMapping & m) : _map (m) {}
   ~quadraturTriang2D() { }
    inline val_t integrate1 (val_t, const arg_t & = arg_t()) ;
    inline val_t integrate3 (val_t, const arg_t & = arg_t()) ;
    inline val_t integrate5 (val_t, const arg_t & = arg_t()) ;
    inline val_t integrate7 (val_t, const arg_t & = arg_t()) ;
} ;

template < class A > class quadraturTriang2D_1 : private quadraturTriang2Dbasis {
  LinearSurfaceMapping _map ;
  public:
    typedef typename A :: val_t val_t;
    typedef typename A :: arg_t arg_t;
    quadraturTriang2D_1(const LinearSurfaceMapping & m) : _map (m) {}
   ~quadraturTriang2D_1() { }
    inline val_t integrate1 (val_t, const arg_t & = arg_t()) ;
    inline val_t integrate3 (val_t, const arg_t & = arg_t()) ;
    inline val_t integrate5 (val_t, const arg_t & = arg_t()) ;
    inline val_t integrate7 (val_t, const arg_t & = arg_t()) ;
} ;
/*********** INLINES ********************************************************/

inline LinearMapping :: LinearMapping(const double (&x0)[3], const double (&x1)[3], const double (&x2)[3], const double (&x3)[3]) 
  : p0(x0), p1(x1), p2(x2), p3(x3) {
  a[0][0] = p3[0] ;
  a[0][1] = p3[1] ;
  a[0][2] = p3[2] ;
  Df [0][0] = a[1][0] = p0[0] - p3[0] ;
  Df [0][1] = a[1][1] = p0[1] - p3[1] ;
  Df [0][2] = a[1][2] = p0[2] - p3[2] ;
  Df [1][0] = a[2][0] = p1[0] - p3[0] ;
  Df [1][1] = a[2][1] = p1[1] - p3[1] ;
  Df [1][2] = a[2][2] = p1[2] - p3[2] ;
  Df [2][0] = a[3][0] = p2[0] - p3[0] ;
  Df [2][1] = a[3][1] = p2[1] - p3[1] ;
  Df [2][2] = a[3][2] = p2[2] - p3[2] ;
  DetDf = - (Df[0][0] * Df[1][1] * Df[2][2] - Df[0][0] * Df[1][2] * Df[2][1] - 
	     Df[1][0] * Df[0][1] * Df[2][2] + Df[1][0] * Df[0][2] * Df[2][1] + 
	     Df[2][0] * Df[0][1] * Df[1][2] - Df[2][0] * Df[0][2] * Df[1][1]) ;
  return ;
}

inline LinearMapping ::LinearMapping (const LinearMapping & map)
  : p0 (map.p0), p1 (map.p1), p2 (map.p2), p3 (map.p3), DetDf (map.DetDf) { 
  memcpy (a, map.a, sizeof(double[4][3])) ;
  memcpy (Df, map.Df, sizeof (double [3][3])) ;
  return ;
}

inline void LinearMapping :: map2world (const double (&p)[4], double (&world)[3]) const {
  world[0] = a[0][0] + a[1][0] * p[0] + a[2][0] * p[1] + a[3][0] * p[2] ;
  world[1] = a[0][1] + a[1][1] * p[0] + a[2][1] * p[1] + a[3][1] * p[2] ;
  world[2] = a[0][2] + a[1][2] * p[0] + a[2][2] * p[1] + a[3][2] * p[2] ;
  return ;
}

inline void LinearMapping::map2world (const double x1, const double x2, const double x3, const double x4, double (&world)[3]) const {
  double map [4] ;
  map[0] = x1 ;
  map[1] = x2 ;
  map[2] = x3 ;
  map[3] = x4 ;
  map2world (map, world) ;
  return ;
}

template < class A > inline quadraturTetra3D < A > :: val_t quadraturTetra3D < A > :: integrate1 (val_t base, const arg_t & x) {								
  val_t t = A()( _p1 , _map, x) ;
  base += (t *= ( _map.det () / 6.0)) ;
  return base ;
}

template < class A > inline quadraturTetra3D < A > :: val_t quadraturTetra3D < A > :: integrate2 (val_t base, const arg_t & x) {		
  for(int i = 0 ; i < 4 ; i ++) {					
    val_t t = A()( _p2 [i], _map, x) ;
    base += (t *= ( _w2 [i] * _map.det ())) ;
  }						
  return base ;
}

template < class A > inline quadraturTetra3D < A > :: val_t quadraturTetra3D < A > :: integrate7 (val_t base, const arg_t & x) {		
  for(int i = 0 ; i < 64 ; i ++) {
    val_t t = A()( _p7 [i], _map, x) ;
    base += (t *= ( _w7 [i] * _map.det ())) ;
  }						
  return base ;
}

inline FunctionWrapper :: val_t FunctionWrapper :: operator () (const double (&coord)[4], LinearMapping &map, const arg_t &func ) {
  return (*(func.f))(coord, map, func.user) ; 
}

inline LinearSurfaceMapping :: LinearSurfaceMapping (const double (&x0)[3], 
    const double (&x1)[3], const double (&x2)[3]) 
  : _p0 (x0), _p1 (x1), _p2 (x2) {
  _b[0][0] = _p0[0] ;
  _b[0][1] = _p0[1] ;
  _b[0][2] = _p0[2] ;
  _b[1][0] = _p1[0] ;
  _b[1][1] = _p1[1] ;
  _b[1][2] = _p1[2] ;
  _b[2][0] = _p2[0] ;
  _b[2][1] = _p2[1] ;
  _b[2][2] = _p2[2] ;

	// Vorsicht: Im Unterschied zu der Originalversion von Mario ist
	// die Dreiecksfl"achennormale hier mit -1/2 skaliert, wobei
	// das Vorzeichen auf die widerspr"uchlichen Konventionen bei
	// Dreiecks- und Vierecksfl"achen zur"uckgeht.

  _n[0] = -0.5 * ((_p1[1]-_p0[1]) *(_p2[2]-_p1[2]) - (_p2[1]-_p1[1]) *(_p1[2]-_p0[2])) ;
  _n[1] = -0.5 * ((_p1[2]-_p0[2]) *(_p2[0]-_p1[0]) - (_p2[2]-_p1[2]) *(_p1[0]-_p0[0])) ;
  _n[2] = -0.5 * ((_p1[0]-_p0[0]) *(_p2[1]-_p1[1]) - (_p2[0]-_p1[0]) *(_p1[1]-_p0[1])) ;
  
  return ;
}

inline LinearSurfaceMapping :: LinearSurfaceMapping (const LinearSurfaceMapping & m) : _p0(m._p0), _p1(m._p1), _p2(m._p2) {
  memcpy(_b, m._b, sizeof(double [3][3])) ;
  memcpy(_n, m._n, sizeof(double [3])) ;
  return ;
}

inline void LinearSurfaceMapping :: map2world (const double (&map)[3], double (&wld)[3]) const {
  double x = map [0] ;
  double y = map [1] ;
  double z = map [2] ;
  wld[0] =  x * _b[0][0] + y * _b[1][0] + z * _b[2][0] ;
  wld[1] =  x * _b[0][1] + y * _b[1][1] + z * _b[2][1] ;
  wld[2] =  x * _b[0][2] + y * _b[1][2] + z * _b[2][2] ;
  return ;
}

inline void LinearSurfaceMapping :: map2world(double x, double y, double z, double (&w)[3]) const {
  double p [3] ;
  p[0] = x ;
  p[1] = y ;
  p[2] = z ;
  map2world (p,w) ;
  return ;
}

inline void LinearSurfaceMapping :: normal (double (&normal)[3]) const {
  normal[0] = _n[0] ;
  normal[1] = _n[1] ;
  normal[2] = _n[2] ;
  return ;
}

template < class A > inline quadraturTriang2D < A > :: val_t quadraturTriang2D < A > :: integrate1 (val_t base, const arg_t & x) {
  double n [3] ;
  _map.normal (n) ;
  return base + A ()(_p1, n, x)  ;
}

template < class A > inline quadraturTriang2D < A > :: val_t quadraturTriang2D < A > :: integrate3 (val_t base, const arg_t & x) {
  double n [3] ;
  _map.normal (n) ;
  for (int i = 0 ; i < 7 ; i++) {
    val_t t = A ()(_p3 [i], n, x) ;
    base += (t *= _w3 [i]) ;
  }
  return base  ;
}

template < class A > inline quadraturTriang2D < A > :: val_t quadraturTriang2D < A > :: integrate5 (val_t base, const arg_t & x) {
  double n [3] ;
  _map.normal (n) ;
  for (int i = 0 ; i < 7 ; i++) {
    val_t t = A ()(_p5 [i], n, x) ;
    base += (t *= _w5 [i]) ;
  }
  return base  ;
}

template < class A > inline quadraturTriang2D < A > :: val_t quadraturTriang2D < A > :: integrate7 (val_t base, const arg_t & x) {
  double n [3] ;
  _map.normal (n) ;
  for (int i = 0 ; i < 16 ; i++) {
    val_t t = A ()(_p7 [i], n, x) ;
    base += (t *= _w7 [i]) ;
  }
  return base  ;
}

template < class A > inline quadraturTriang2D_1 < A > :: val_t quadraturTriang2D_1 < A > :: integrate1 (val_t base, const arg_t & x) {
  double n [3] ;
  return base + A ()(_p1, _map, x)  ;
}

template < class A > inline quadraturTriang2D_1 < A > :: val_t quadraturTriang2D_1 < A > :: integrate3 (val_t base, const arg_t & x) {
  double n [3] ;
  for (int i = 0 ; i < 7 ; i++) {
    val_t t = A ()(_p3 [i], _map, x) ;
    base += (t *= _w3 [i]) ;
  }
  return base  ;
}

template < class A > inline quadraturTriang2D_1 < A > :: val_t quadraturTriang2D_1 < A > :: integrate5 (val_t base, const arg_t & x) {
  double n [3] ;
  for (int i = 0 ; i < 7 ; i++) {
    val_t t = A ()(_p5 [i], _map, x) ;
    base += (t *= _w5 [i]) ;
  }
  return base  ;
}

template < class A > inline quadraturTriang2D_1 < A > :: val_t quadraturTriang2D_1 < A > :: integrate7 (val_t base, const arg_t & x) {
  double n [3] ;
  for (int i = 0 ; i < 16 ; i++) {
    val_t t = A ()(_p7 [i], _map, x) ;
    base += (t *= _w7 [i]) ;
  }
  return base  ;
}
#include "mapp_tetra_3d_ext.h"

#endif	// MAPP_TETRA_3D_H_INCLUDED
