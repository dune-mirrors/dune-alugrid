// (c) bernhard schupp 1997 - 1998
#ifndef MAPP_CUBE_3D_H_INCLUDED
#define MAPP_CUBE_3D_H_INCLUDED

#include <cmath>

#include "gitter_sti.h"

namespace ALUGrid
{

  class LinearMapping;	// in mapp_tetra_3d.h

  class TrilinearMapping
  {
    static const alucoord_t _epsilon;

    const alucoord_t (&p0)[ 3 ], (&p1)[ 3 ], (&p2)[ 3 ], (&p3)[ 3 ];
    const alucoord_t (&p4)[ 3 ], (&p5)[ 3 ], (&p6)[ 3 ], (&p7)[ 3 ];
    alucoord_t a [ 8 ][ 3 ];
    alucoord_t Df [ 3 ][ 3 ];
    alucoord_t Dfi [ 3 ][ 3 ];
    alucoord_t DetDf;
    void linear (const alucoord_t (&)[3]);
    void inverse (const alucoord_t (&)[3]);
    public :
      inline TrilinearMapping (const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3],
                               const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3]);
      inline TrilinearMapping (const TrilinearMapping &);
     ~TrilinearMapping () {}
      alucoord_t det (const alucoord_t (&)[3]);
      inline void map2world (const alucoord_t (&)[3], alucoord_t (&)[3]) const;
      inline void map2world (const alucoord_t , const alucoord_t , const alucoord_t , alucoord_t (&)[3]) const;
      void world2map (const alucoord_t (&)[3], alucoord_t (&)[3]);

      static inline void barycenter(const alucoord_t (&p0)[3], const alucoord_t (&p1)[3], const alucoord_t (&p2)[3], const alucoord_t (&p3)[3],
                                    const alucoord_t (&p4)[3], const alucoord_t (&p5)[3], const alucoord_t (&p6)[3], const alucoord_t (&p7)[3],
                                    alucoord_t (&barycenter)[3])
      {
        barycenter[0] = 0.125 * (p0[0] + p1[0] + p2[0] + p3[0] + p4[0] + p5[0] + p6[0] + p7[0]);
        barycenter[1] = 0.125 * (p0[1] + p1[1] + p2[1] + p3[1] + p4[1] + p5[1] + p6[1] + p7[1]);
        barycenter[2] = 0.125 * (p0[2] + p1[2] + p2[2] + p3[2] + p4[2] + p5[2] + p6[2] + p7[2]);

  #ifdef ALUGRIDDEBUG
        {
          TrilinearMapping map(p0,p1,p2,p3,p4,p5,p6,p7);
          alucoord_t p[3];
          map.map2world(.0, .0, .0, p);
          for(int j=0; j<3; ++j)
          {
            alugrid_assert ( fabs(barycenter[j] - p[j]) < 1e-8 );
          }
        }
  #endif
      }

      // returns true if mapping is affine
      inline bool affine() const;
  };

  class QuadraturCube3Dbasis
  {
  protected:
    static const alucoord_t _p2 [4][3];
    static const alucoord_t _p3 [8][3];
  };

  struct VolumeCalc
  {
    typedef alucoord_t val_t;
    typedef int arg_t;

    val_t operator() ( const alucoord_t (&)[ 3 ], const arg_t & );
    val_t operator() ( const alucoord_t (&)[ 4 ], const LinearMapping &, const arg_t & );
  };

  struct SurfaceCalc
  {
  public:
    typedef alucoord_t val_t;
    typedef int arg_t;
    val_t operator() ( const alucoord_t (&)[ 2 ], const alucoord_t (&)[ 3 ], const arg_t & );
    val_t operator() ( const alucoord_t (&)[ 3 ], const alucoord_t (&)[ 3 ], const arg_t & );
  };

  template< class A > class QuadraturCube3D : private QuadraturCube3Dbasis {
    private :
      TrilinearMapping _map;
    public :
      QuadraturCube3D (const TrilinearMapping & m) : _map (m) {}
     ~QuadraturCube3D () {}
      typedef typename A::val_t val_t;
      typedef typename A::arg_t arg_t;
      inline val_t integrate2 (val_t, const arg_t & = arg_t ());
      inline val_t integrate3 (val_t, const arg_t & = arg_t ());
  };

  template< class A > class QuadraturCube3D_1 : private QuadraturCube3Dbasis {
    private :
      TrilinearMapping _map;
    public :
      QuadraturCube3D_1 (const TrilinearMapping & m) : _map (m) {}
     ~QuadraturCube3D_1 () {}
      typedef typename A::val_t val_t;
      typedef typename A::arg_t arg_t;
      inline val_t integrate2 (val_t, const arg_t & = arg_t ());
      inline val_t integrate3 (val_t, const arg_t & = arg_t ());
  };

  class BilinearSurfaceMapping {
    const alucoord_t (&_p0)[3], (&_p1)[3], (&_p2)[3], (&_p3)[3];
    alucoord_t _b [4][3];
    alucoord_t _n [3][3];
    public :
      inline BilinearSurfaceMapping (const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3]);
      inline BilinearSurfaceMapping (const BilinearSurfaceMapping &);
     ~BilinearSurfaceMapping () {}
      inline void map2world(const alucoord_t (&)[2], alucoord_t (&)[3]) const;
      inline void map2world(alucoord_t x, alucoord_t y, alucoord_t (&w)[3]) const;
      inline void normal(const alucoord_t (&)[2], alucoord_t (&)[3]) const;

      static inline void barycenter(const alucoord_t (&p0)[3], const alucoord_t (&p1)[3], const alucoord_t (&p2)[3], const alucoord_t (&p3)[3],
                                    alucoord_t (&barycenter)[3])
      {
        barycenter[0] = 0.25 * (p0[0] + p1[0] + p2[0] + p3[0]);
        barycenter[1] = 0.25 * (p0[1] + p1[1] + p2[1] + p3[1]);
        barycenter[2] = 0.25 * (p0[2] + p1[2] + p2[2] + p3[2]);

  #ifdef ALUGRIDDEBUG
        {
          BilinearSurfaceMapping map(p0,p1,p2,p3);
          alucoord_t p[3];
          map.map2world(.0, .0, p);
          for(int j=0; j<3; ++j)
          {
            alugrid_assert ( fabs(barycenter[j] - p[j]) < 1e-8 );
          }
        }
  #endif
      }
  };

  class QuadraturCube2Dbasis {
    protected :
      static const alucoord_t _p1 [2];
      static const alucoord_t _p3 [4][2];
  };

  template< class A > class QuadraturCube2D : private QuadraturCube2Dbasis {
    BilinearSurfaceMapping _map;
    public:
      QuadraturCube2D(const BilinearSurfaceMapping & m) : _map (m) {}
     ~QuadraturCube2D() {}
      typedef typename A::val_t val_t;
      typedef typename A::arg_t arg_t;
      inline val_t integrate1 (val_t, const arg_t & = arg_t());
      inline val_t integrate3 (val_t, const arg_t & = arg_t());
      inline val_t integrate  (val_t, const arg_t & = arg_t(), int = 1);
  };

  template< class A > class QuadraturCube2D_1 : private QuadraturCube2Dbasis {
    BilinearSurfaceMapping _map;
    public:
      QuadraturCube2D_1(const BilinearSurfaceMapping & m) : _map (m) {}
     ~QuadraturCube2D_1() {}
      typedef typename A::val_t val_t;
      typedef typename A::arg_t arg_t;
      inline val_t integrate1 (val_t, const arg_t & = arg_t());
      inline val_t integrate3 (val_t, const arg_t & = arg_t());
      inline val_t integrate  (val_t, const arg_t & = arg_t(), int = 1);
  };

          //
          //    #    #    #  #          #    #    #  ######
          //    #    ##   #  #          #    ##   #  #
          //    #    # #  #  #          #    # #  #  #####
          //    #    #  # #  #          #    #  # #  #
          //    #    #   ##  #          #    #   ##  #
          //    #    #    #  ######     #    #    #  ######
          //


  inline TrilinearMapping::TrilinearMapping (const alucoord_t (&x0)[3], const alucoord_t (&x1)[3],
                        const alucoord_t (&x2)[3], const alucoord_t (&x3)[3], const alucoord_t (&x4)[3],
                        const alucoord_t (&x5)[3], const alucoord_t (&x6)[3], const alucoord_t (&x7)[3])
    : p0(x0), p1(x1), p2(x2), p3(x3), p4(x4), p5(x5), p6(x6), p7(x7) {
    a [0][0] = p3 [0];
    a [0][1] = p3 [1];
    a [0][2] = p3 [2];
    a [1][0] = p0 [0] - p3 [0];
    a [1][1] = p0 [1] - p3 [1];
    a [1][2] = p0 [2] - p3 [2];
    a [2][0] = p2 [0] - p3 [0];
    a [2][1] = p2 [1] - p3 [1];
    a [2][2] = p2 [2] - p3 [2];
    a [3][0] = p7 [0] - p3 [0];
    a [3][1] = p7 [1] - p3 [1];
    a [3][2] = p7 [2] - p3 [2];
    a [4][0] = p1 [0] - p2 [0] - a [1][0];
    a [4][1] = p1 [1] - p2 [1] - a [1][1];
    a [4][2] = p1 [2] - p2 [2] - a [1][2];
    a [5][0] = p6 [0] - p7 [0] - a [2][0];
    a [5][1] = p6 [1] - p7 [1] - a [2][1];
    a [5][2] = p6 [2] - p7 [2] - a [2][2];
    a [6][0] = p4 [0] - p0 [0] - a [3][0];
    a [6][1] = p4 [1] - p0 [1] - a [3][1];
    a [6][2] = p4 [2] - p0 [2] - a [3][2];
    a [7][0] = p5 [0] - p4 [0] + p7 [0] - p6 [0] - p1 [0] + p0 [0] + a [2][0];
    a [7][1] = p5 [1] - p4 [1] + p7 [1] - p6 [1] - p1 [1] + p0 [1] + a [2][1];
    a [7][2] = p5 [2] - p4 [2] + p7 [2] - p6 [2] - p1 [2] + p0 [2] + a [2][2];
    return;
  }

  inline TrilinearMapping::TrilinearMapping (const TrilinearMapping & map)
    : p0(map.p0), p1(map.p1), p2(map.p2), p3(map.p3), p4(map.p4), p5(map.p5), p6(map.p6), p7(map.p7) {
    for (int i = 0; i < 8; i ++)
      for (int j = 0; j < 3; j ++)
        a [i][j] = map.a [i][j];
    return;
  }

  inline void TrilinearMapping::map2world(const alucoord_t (&p)[3], alucoord_t (&world)[3]) const {
    alucoord_t x = .5 * (p [0] + 1.);
    alucoord_t y = .5 * (p [1] + 1.);
    alucoord_t z = .5 * (p [2] + 1.);
    alucoord_t t3 = y * z;
    alucoord_t t8 = x * z;
    alucoord_t t13 = x * y;
    alucoord_t t123 = x * t3;
    world [0] = a [0][0] + a [1][0] * x + a [2][0] * y + a [3][0] * z + a [4][0] * t13 + a [5][0] * t3 + a [6][0] * t8 + a [7][0] * t123;
    world [1] = a [0][1] + a [1][1] * x + a [2][1] * y + a [3][1] * z + a [4][1] * t13 + a [5][1] * t3 + a [6][1] * t8 + a [7][1] * t123;
    world [2] = a [0][2] + a [1][2] * x + a [2][2] * y + a [3][2] * z + a [4][2] * t13 + a [5][2] * t3 + a [6][2] * t8 + a [7][2] * t123;
    return;
  }

  inline void TrilinearMapping::map2world(const alucoord_t x1, const alucoord_t x2, const alucoord_t x3, alucoord_t (&world)[3]) const {
    alucoord_t map [3];
    map [0] = x1;
    map [1] = x2;
    map [2] = x3;
    map2world (map, world);
    return;
  }

  inline bool TrilinearMapping::affine () const
  {
    alucoord_t sum = 0.0;
    // summ all factor from non-linaer terms
    for(int i=4; i<8; ++i)
    {
      for(int j=0; j<3; ++j)
      {
        sum += fabs(a[i][j]);
      }
    }

    // mapping is affine when all higher terms are zero
    return (sum < _epsilon);
  }

  template< class A > inline typename QuadraturCube3D < A >::val_t QuadraturCube3D < A >::integrate2 (val_t base, const arg_t & x) {

          // Exakt f"ur Polynome vom Grad <= 2
          // auf dem W"urfel [-1,1]x[-1,1]x[-1,1], V([-1,1]^3) = 8.0

    for(int i = 0; i < 4; i ++) {
      val_t t = A()( _p2 [i], x);
      base += (t *= ( _map.det ( _p2 [i]) * 2.0));
    }
    return base;
  }

  template< class A > inline typename QuadraturCube3D < A >::val_t QuadraturCube3D < A >::integrate3 (val_t base, const arg_t & x) {

          // exakt f"ur Polynome vom Grad <= 3

    for(int i = 0; i < 8; i ++ ) {
      val_t t = A()( _p3 [i], x);
      base += (t *= _map.det ( _p3 [i]));
    }
    return base;
  }

  template< class A > inline typename QuadraturCube3D_1 < A >::val_t QuadraturCube3D_1 < A >::integrate2 (val_t base, const arg_t & x) {

          // Exakt f"ur Polynome vom Grad <= 2
          // auf dem W"urfel [-1,1]x[-1,1]x[-1,1], V([-1,1]^3) = 8.0

    for(int i = 0; i < 4; i ++) {
      val_t t = A()( _p2 [i], _map, x);
      base += (t *= ( _map.det ( _p2 [i]) * 2.0));
    }
    return base;
  }

  template< class A > inline typename QuadraturCube3D_1 < A >::val_t QuadraturCube3D_1 < A >::integrate3 (val_t base, const arg_t & x) {

          // exakt f"ur Polynome vom Grad <= 3

    for(int i = 0; i < 8; i ++ ) {
      val_t t = A()( _p3 [i], _map, x);
      base += (t *= _map.det ( _p3 [i]));
    }
    return base;
  }

  inline VolumeCalc::val_t VolumeCalc::operator () (const alucoord_t (&)[3], const arg_t &) {
    return 1.0;
  }

  inline VolumeCalc::val_t VolumeCalc::operator () (const alucoord_t (&)[4], const LinearMapping &, const arg_t &) {
    return 1.0;
  }

  inline BilinearSurfaceMapping::BilinearSurfaceMapping (const alucoord_t (&x0)[3], const alucoord_t (&x1)[3], const alucoord_t (&x2)[3], const alucoord_t (&x3)[3])
    : _p0 (x0), _p1 (x1), _p2 (x2), _p3 (x3) {
    _b [0][0] = _p0 [0];
    _b [0][1] = _p0 [1];
    _b [0][2] = _p0 [2];
    _b [1][0] = _p3 [0] - _p0 [0];
    _b [1][1] = _p3 [1] - _p0 [1];
    _b [1][2] = _p3 [2] - _p0 [2];
    _b [2][0] = _p1 [0] - _p0 [0];
    _b [2][1] = _p1 [1] - _p0 [1];
    _b [2][2] = _p1 [2] - _p0 [2];
    _b [3][0] = _p2 [0] - _p1 [0] - _b [1][0];
    _b [3][1] = _p2 [1] - _p1 [1] - _b [1][1];
    _b [3][2] = _p2 [2] - _p1 [2] - _b [1][2];
    _n [0][0] = _b [1][1] * _b [2][2] - _b [1][2] * _b [2][1];
    _n [0][1] = _b [1][2] * _b [2][0] - _b [1][0] * _b [2][2];
    _n [0][2] = _b [1][0] * _b [2][1] - _b [1][1] * _b [2][0];
    _n [1][0] = _b [1][1] * _b [3][2] - _b [1][2] * _b [3][1];
    _n [1][1] = _b [1][2] * _b [3][0] - _b [1][0] * _b [3][2];
    _n [1][2] = _b [1][0] * _b [3][1] - _b [1][1] * _b [3][0];
    _n [2][0] = _b [3][1] * _b [2][2] - _b [3][2] * _b [2][1];
    _n [2][1] = _b [3][2] * _b [2][0] - _b [3][0] * _b [2][2];
    _n [2][2] = _b [3][0] * _b [2][1] - _b [3][1] * _b [2][0];
    return;
  }

  inline BilinearSurfaceMapping::BilinearSurfaceMapping (const BilinearSurfaceMapping & m)
          : _p0(m._p0), _p1(m._p1), _p2(m._p2), _p3(m._p3) {
    {for (int i = 0; i < 4; i ++)
      for (int j = 0; j < 3; j ++ )
        _b [i][j] = m._b [i][j];
    }
    {for (int i = 0; i < 3; i ++)
      for (int j = 0; j < 3; j ++ )
        _n [i][j] = m._n [i][j];
    }
    return;
  }

  inline void BilinearSurfaceMapping::map2world (const alucoord_t (&map)[2], alucoord_t (&wld)[3]) const {
    alucoord_t x = .5 * (map [0] + 1.0);
    alucoord_t y = .5 * (map [1] + 1.0);
    alucoord_t xy = x * y;
    wld[0] = _b [0][0] + x * _b [1][0] + y * _b [2][0] + xy * _b [3][0];
    wld[1] = _b [0][1] + x * _b [1][1] + y * _b [2][1] + xy * _b [3][1];
    wld[2] = _b [0][2] + x * _b [1][2] + y * _b [2][2] + xy * _b [3][2];
    return;
  }

  inline void BilinearSurfaceMapping::map2world (alucoord_t x, alucoord_t y, alucoord_t (&w)[3]) const {
    alucoord_t p [2];
    p [0] = x;
    p [1] = y;
    map2world (p,w);
    return;
  }

  inline void BilinearSurfaceMapping::normal (const alucoord_t (&map)[2], alucoord_t (&normal)[3]) const {
    alucoord_t x = .5 * (map [0] + 1.0);
    alucoord_t y = .5 * (map [1] + 1.0);
    normal [0] = -( _n [0][0] + _n [1][0] * x + _n [2][0] * y);
    normal [1] = -( _n [0][1] + _n [1][1] * x + _n [2][1] * y);
    normal [2] = -( _n [0][2] + _n [1][2] * x + _n [2][2] * y);
    return;
  }

  template< class A > typename QuadraturCube2D < A >::val_t QuadraturCube2D < A >::integrate1 (val_t base, const arg_t & x) {
    alucoord_t n [3];
    _map.normal (_p1,n);
    return base + A () (_p1,n,x);
  }

  template< class A > typename QuadraturCube2D < A >::val_t QuadraturCube2D < A >::integrate3 (val_t base, const arg_t & x) {
    const alucoord_t quarter = 1.0/4.0;
    val_t res;
    for(int i = 0; i < 4; i ++) {
      alucoord_t n [3];
      _map.normal (_p3 [i],n);
      val_t tmp = A () ( _p3 [i],n,x);
      base += (tmp *= quarter);
    }
    return base;
  }

  template< class A > typename QuadraturCube2D < A >::val_t QuadraturCube2D < A >::integrate(val_t base, const arg_t & x, int resolution) {
    alucoord_t w = 1.0/alucoord_t (resolution * resolution);
    alucoord_t delta = 2.0/alucoord_t (resolution);
    for(int i = 0; i < resolution; i ++) {
      alucoord_t p [2];
      p [0] = delta * (alucoord_t (i) + .5) - 1.0;
      for(int j = 0; j < resolution; j ++) {
        alucoord_t n [3];
        p [1] = delta * (alucoord_t (j) + .5) - 1.0;
        _map.normal (p,n);
        val_t tmp = A () (p,n,x);
        tmp *= w;
        base += tmp;
      }
    }
    return base;
  }

  template< class A > typename QuadraturCube2D_1 < A >::val_t QuadraturCube2D_1< A >::integrate1 ( val_t base, const arg_t &x )
  {
    return base + A()( _p1,_map, x );
  }

  template< class A >
  typename QuadraturCube2D_1 < A >::val_t QuadraturCube2D_1< A >::integrate3 ( val_t base, const arg_t & x )
  {
    const alucoord_t quarter = 1.0/4.0;
    val_t res;
    for(int i = 0; i < 4; i ++) {
      val_t tmp = A () ( _p3 [i],_map,x);
      base += (tmp *= quarter);
    }
    return base;
  }

  template< class A >
  typename QuadraturCube2D_1 < A >::val_t QuadraturCube2D_1 < A >::integrate(val_t base, const arg_t & x, int resolution)
  {
    alucoord_t w = 1.0/alucoord_t (resolution * resolution);
    alucoord_t delta = 2.0/alucoord_t (resolution);
    for(int i = 0; i < resolution; i ++) {
      alucoord_t p [2];
      p [0] = delta * (alucoord_t (i) + .5) - 1.0;
      for(int j = 0; j < resolution; j ++) {
        p [1] = delta * (alucoord_t (j) + .5) - 1.0;
        val_t tmp = A () (p,_map,x);
        tmp *= w;
        base += tmp;
      }
    }
    return base;
  }

  inline SurfaceCalc::val_t SurfaceCalc::operator()(const alucoord_t (&)[2], const alucoord_t (&n)[3], const arg_t &)
  {
    return sqrt (n [0] * n [0] + n [1] * n [1] + n [2] * n [2]);
  }

  inline SurfaceCalc::val_t SurfaceCalc::operator()(const alucoord_t (&)[3], const alucoord_t (&n)[3], const arg_t &)
  {
    return sqrt (n [0] * n [0] + n [1] * n [1] + n [2] * n [2]);
  }

} // namespace ALUGrid

#endif // #ifndef MAPP_CUBE_3D_H_INCLUDED
