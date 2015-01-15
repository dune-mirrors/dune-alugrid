// (c) mario ohlberger, 1998
#ifndef MAPP_TETRA_3D_H_INCLUDED
#define MAPP_TETRA_3D_H_INCLUDED

#include <math.h>
#include <stdlib.h>

#include "gitter_sti.h"
#include "mapp_cube_3d.h"

namespace ALUGrid
{

  class LinearMapping
  {
    private :
      const alucoord_t (&p0)[3], (&p1)[3], (&p2)[3], (&p3)[3];
      alucoord_t a[4][3] ;
      alucoord_t Df[3][3] ;
      alucoord_t Dfi[3][3] ;
      alucoord_t DetDf ;
      void inverse () ;
    public :
      LinearMapping (const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3]) ;
      LinearMapping (const LinearMapping &) ;
     ~LinearMapping () {}
      alucoord_t det () const { return DetDf ; }
      void map2world (const alucoord_t (&)[4], alucoord_t (&)[3]) const ;
      void map2world (const alucoord_t , const alucoord_t , const alucoord_t, const alucoord_t, alucoord_t (&)[3]) const ;
      void world2map (const alucoord_t (&)[3], alucoord_t (&)[4]) ;

      static inline void barycenter(const alucoord_t (&p0)[3], const alucoord_t (&p1)[3],
                                    const alucoord_t (&p2)[3], const alucoord_t (&p3)[3],
                                    alucoord_t (&barycenter)[3])
      {
        barycenter[0] = 0.25 * (p0[0] + p1[0] + p2[0] + p3[0]);
        barycenter[1] = 0.25 * (p0[1] + p1[1] + p2[1] + p3[1]);
        barycenter[2] = 0.25 * (p0[2] + p1[2] + p2[2] + p3[2]);
  #ifdef ALUGRIDDEBUG
        LinearMapping map(p0,p1,p2,p3);
        alucoord_t p[3] ;
        map.map2world(.25, .25, .25, .25, p) ;
        for(int j=0; j<3; ++j)
        {
          alugrid_assert ( fabs(barycenter[j] - p[j]) < 1e-8 );
        }
  #endif
      }
  } ;

  class quadraturTetra3Dbasis {
    protected :
      static const alucoord_t _p1 [4] ;
      static const alucoord_t _w2 [4] ;
      static const alucoord_t _p2 [4][4] ;
      static const alucoord_t _w7 [64] ;
      static const alucoord_t _p7 [64][4] ;
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
        alucoord_t (*f)(const alucoord_t (&)[4], LinearMapping &, void *) ;
        void *user ;
        arg () { abort() ; }
        arg ( alucoord_t (*p)(const alucoord_t (&)[4], LinearMapping &, void * ), void *a ) : f(p), user(a) {}
      };
      typedef alucoord_t val_t ;
      typedef arg arg_t ;
    public :
      inline val_t operator () (const alucoord_t (&)[4], LinearMapping &, const arg_t & ) ;
  } ;


  class LinearSurfaceMapping {
    const alucoord_t (&_p0)[3], (&_p1)[3], (&_p2)[3] ;
    alucoord_t _b [3][3] ;
  protected:
    alucoord_t _n [3] ;
  public :
      inline LinearSurfaceMapping (const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3]) ;
      inline LinearSurfaceMapping (const LinearSurfaceMapping &) ;
     ~LinearSurfaceMapping() { }
      inline void map2world(const alucoord_t (&)[3], alucoord_t (&)[3]) const ;
      inline void map2world(alucoord_t x, alucoord_t y, alucoord_t z, alucoord_t (&w)[3]) const ;
      inline void normal(alucoord_t (&)[3]) const;
  } ;

  class quadraturTriang2Dbasis {
    protected :
      static const alucoord_t _p1 [3] ;
      static const alucoord_t _w3 [7] ;
      static const alucoord_t _p3 [7][3] ;
      static const alucoord_t _w5 [7] ;
      static const alucoord_t _p5 [7][3] ;
      static const alucoord_t _w7 [16] ;
      static const alucoord_t _p7 [16][3] ;
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

  inline LinearMapping :: LinearMapping(const alucoord_t (&x0)[3], const alucoord_t (&x1)[3], const alucoord_t (&x2)[3], const alucoord_t (&x3)[3])
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
    memcpy (a, map.a, sizeof(alucoord_t[4][3])) ;
    memcpy (Df, map.Df, sizeof (alucoord_t [3][3])) ;
    return ;
  }

  inline void LinearMapping :: map2world (const alucoord_t (&p)[4], alucoord_t (&world)[3]) const {
    world[0] = a[0][0] + a[1][0] * p[0] + a[2][0] * p[1] + a[3][0] * p[2] ;
    world[1] = a[0][1] + a[1][1] * p[0] + a[2][1] * p[1] + a[3][1] * p[2] ;
    world[2] = a[0][2] + a[1][2] * p[0] + a[2][2] * p[1] + a[3][2] * p[2] ;
    return ;
  }

  inline void LinearMapping::map2world (const alucoord_t x1, const alucoord_t x2, const alucoord_t x3, const alucoord_t x4, alucoord_t (&world)[3]) const {
    alucoord_t map [4] ;
    map[0] = x1 ;
    map[1] = x2 ;
    map[2] = x3 ;
    map[3] = x4 ;
    map2world (map, world) ;
    return ;
  }

  template < class A > inline typename quadraturTetra3D < A > :: val_t quadraturTetra3D < A > :: integrate1 (val_t base, const arg_t & x) {
    val_t t = A()( _p1 , _map, x) ;
    base += (t *= ( _map.det () / 6.0)) ;
    return base ;
  }

  template < class A > inline typename quadraturTetra3D < A > :: val_t quadraturTetra3D < A > :: integrate2 (val_t base, const arg_t & x) {
    for(int i = 0 ; i < 4 ; i ++) {
      val_t t = A()( _p2 [i], _map, x) ;
      base += (t *= ( _w2 [i] * _map.det ())) ;
    }
    return base ;
  }

  template < class A > inline typename quadraturTetra3D < A > :: val_t quadraturTetra3D < A > :: integrate7 (val_t base, const arg_t & x) {
    for(int i = 0 ; i < 64 ; i ++) {
      val_t t = A()( _p7 [i], _map, x) ;
      base += (t *= ( _w7 [i] * _map.det ())) ;
    }
    return base ;
  }

  inline FunctionWrapper :: val_t FunctionWrapper :: operator () (const alucoord_t (&coord)[4], LinearMapping &map, const arg_t &func ) {
    return (*(func.f))(coord, map, func.user) ;
  }

  inline LinearSurfaceMapping :: LinearSurfaceMapping (const alucoord_t (&x0)[3],
      const alucoord_t (&x1)[3], const alucoord_t (&x2)[3])
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
    memcpy(_b, m._b, sizeof(alucoord_t [3][3])) ;
    memcpy(_n, m._n, sizeof(alucoord_t [3])) ;
    return ;
  }

  inline void LinearSurfaceMapping :: map2world (const alucoord_t (&map)[3], alucoord_t (&wld)[3]) const {
    alucoord_t x = map [0] ;
    alucoord_t y = map [1] ;
    alucoord_t z = map [2] ;
    wld[0] =  x * _b[0][0] + y * _b[1][0] + z * _b[2][0] ;
    wld[1] =  x * _b[0][1] + y * _b[1][1] + z * _b[2][1] ;
    wld[2] =  x * _b[0][2] + y * _b[1][2] + z * _b[2][2] ;
    return ;
  }

  inline void LinearSurfaceMapping :: map2world(alucoord_t x, alucoord_t y, alucoord_t z, alucoord_t (&w)[3]) const {
    alucoord_t p [3] ;
    p[0] = x ;
    p[1] = y ;
    p[2] = z ;
    map2world (p,w) ;
    return ;
  }

  inline void LinearSurfaceMapping :: normal (alucoord_t (&normal)[3]) const {
    normal[0] = _n[0] ;
    normal[1] = _n[1] ;
    normal[2] = _n[2] ;
    return ;
  }

  template < class A > inline typename quadraturTriang2D < A > :: val_t
  quadraturTriang2D < A > :: integrate1 (val_t base, const arg_t & x) {
    alucoord_t n [3] ;
    _map.normal (n) ;
    return base + A ()(_p1, n, x)  ;
  }

  template < class A > inline typename quadraturTriang2D < A > :: val_t
  quadraturTriang2D < A > :: integrate3 (val_t base, const arg_t & x) {
    alucoord_t n [3] ;
    _map.normal (n) ;
    for (int i = 0 ; i < 7 ; i++) {
      val_t t = A ()(_p3 [i], n, x) ;
      base += (t *= _w3 [i]) ;
    }
    return base  ;
  }

  template < class A > inline typename quadraturTriang2D < A > :: val_t
  quadraturTriang2D < A > :: integrate5 (val_t base, const arg_t & x) {
    alucoord_t n [3] ;
    _map.normal (n) ;
    for (int i = 0 ; i < 7 ; i++) {
      val_t t = A ()(_p5 [i], n, x) ;
      base += (t *= _w5 [i]) ;
    }
    return base  ;
  }

  template < class A > inline typename quadraturTriang2D < A > :: val_t
  quadraturTriang2D < A > :: integrate7 (val_t base, const arg_t & x) {
    alucoord_t n [3] ;
    _map.normal (n) ;
    for (int i = 0 ; i < 16 ; i++) {
      val_t t = A ()(_p7 [i], n, x) ;
      base += (t *= _w7 [i]) ;
    }
    return base  ;
  }

  template < class A > inline typename quadraturTriang2D_1 < A > :: val_t
  quadraturTriang2D_1 < A > :: integrate1 (val_t base, const arg_t & x) {
    return base + A ()(_p1, _map, x)  ;
  }

  template < class A > inline typename quadraturTriang2D_1 < A > :: val_t
  quadraturTriang2D_1 < A > :: integrate3 (val_t base, const arg_t & x) {
    for (int i = 0 ; i < 7 ; i++) {
      val_t t = A ()(_p3 [i], _map, x) ;
      base += (t *= _w3 [i]) ;
    }
    return base  ;
  }

  template < class A > inline typename quadraturTriang2D_1 < A > :: val_t
  quadraturTriang2D_1 < A > :: integrate5 (val_t base, const arg_t & x) {
    for (int i = 0 ; i < 7 ; i++) {
      val_t t = A ()(_p5 [i], _map, x) ;
      base += (t *= _w5 [i]) ;
    }
    return base  ;
  }

  template < class A > inline typename quadraturTriang2D_1 < A > :: val_t
  quadraturTriang2D_1 < A > :: integrate7 (val_t base, const arg_t & x) {
    for (int i = 0 ; i < 16 ; i++) {
      val_t t = A ()(_p7 [i], _map, x) ;
      base += (t *= _w7 [i]) ;
    }
    return base  ;
  }

} // namespace ALUGrid

//#include "mapp_tetra_3d_ext.h"

#endif // #ifndef MAPP_TETRA_3D_H_INCLUDED
