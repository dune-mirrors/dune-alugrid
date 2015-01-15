// (c) bernhard schupp 1997 - 1998
#include <config.h>

#include <dune/alugrid/common/alugrid_assert.hh>
#include <cmath>
#include <cstdlib>

#include "mapp_cube_3d.h"

namespace ALUGrid
{

  const alucoord_t TrilinearMapping :: _epsilon = 1.0e-8 ;
  const alucoord_t QuadraturCube3Dbasis :: _p2 [4][3] = { { .816496580927726, .0,  .5773502691896258},
                                                      { .0, .816496580927726, -.5773502691896258},
                                                      { -.816496580927726, .0, .5773502691896258},
                                                      { .0, -.816496580927726, -.5773502691896258} } ;
  const alucoord_t QuadraturCube3Dbasis :: _p3 [8][3] = { {  .5773502691896258,  .5773502691896258,  .5773502691896258},
                                                      { -.5773502691896258,  .5773502691896258,  .5773502691896258},
                                                      {  .5773502691896258, -.5773502691896258,  .5773502691896258},
                                                      { -.5773502691896258, -.5773502691896258,  .5773502691896258},
                                                      {  .5773502691896258,  .5773502691896258, -.5773502691896258},
                                                      { -.5773502691896258,  .5773502691896258, -.5773502691896258},
                                                      {  .5773502691896258, -.5773502691896258, -.5773502691896258},
                                                      { -.5773502691896258, -.5773502691896258, -.5773502691896258} } ;

  const alucoord_t QuadraturCube2Dbasis :: _p1 [2] = { .0, .0 } ;
  const alucoord_t QuadraturCube2Dbasis :: _p3 [4][2] = { { .5773502691896258,  .5773502691896258 },
                                                      {-.5773502691896258,  .5773502691896258 },
                                                      { .5773502691896258, -.5773502691896258 },
                                                      {-.5773502691896258, -.5773502691896258 } } ;


  void TrilinearMapping :: linear(const alucoord_t (&p)[3]) {
    alucoord_t x = .5 * (p[0] + 1.) ;
    alucoord_t y = .5 * (p[1] + 1.) ;
    alucoord_t z = .5 * (p[2] + 1.) ;
    alucoord_t t0 = .5 ;
    alucoord_t t3 = y * z ;
    alucoord_t t8 = x * z ;
    alucoord_t t13 = x * y ;
    Df[2][0] = t0 * ( a[1][2] + y * a[4][2] + z * a[6][2] + t3 * a[7][2] ) ;
    Df[2][1] = t0 * ( a[2][2] + x * a[4][2] + z * a[5][2] + t8 * a[7][2] ) ;
    Df[1][2] = t0 * ( a[3][1] + y * a[5][1] + x * a[6][1] + t13 * a[7][1] ) ;
    Df[2][2] = t0 * ( a[3][2] + y * a[5][2] + x * a[6][2] + t13 * a[7][2] ) ;
    Df[0][0] = t0 * ( a[1][0] + y * a[4][0] + z * a[6][0] + t3 * a[7][0] ) ;
    Df[0][2] = t0 * ( a[3][0] + y * a[5][0] + x * a[6][0] + t13 * a[7][0] ) ;
    Df[1][0] = t0 * ( a[1][1] + y * a[4][1] + z * a[6][1] + t3 * a[7][1] ) ;
    Df[0][1] = t0 * ( a[2][0] + x * a[4][0] + z * a[5][0] + t8 * a[7][0] ) ;
    Df[1][1] = t0 * ( a[2][1] + x * a[4][1] + z * a[5][1] + t8 * a[7][1] ) ;

  }

  alucoord_t TrilinearMapping :: det(const alucoord_t (&point)[3]) {
          //  Determinante der Abbildung f:[-1,1]^3 -> Hexaeder im Punkt point.
    linear (point) ;
    return (DetDf = Df[0][0] * Df[1][1] * Df[2][2] - Df[0][0] * Df[1][2] * Df[2][1] -
                    Df[1][0] * Df[0][1] * Df[2][2] + Df[1][0] * Df[0][2] * Df[2][1] +
                    Df[2][0] * Df[0][1] * Df[1][2] - Df[2][0] * Df[0][2] * Df[1][1]) ;
  }

  void TrilinearMapping :: inverse(const alucoord_t (&p)[3]) {
          //  Kramer - Regel, det() rechnet Df und DetDf neu aus.
    alucoord_t val = 1.0 / det(p) ;
    Dfi[0][0] = ( Df[1][1] * Df[2][2] - Df[1][2] * Df[2][1] ) * val ;
    Dfi[0][1] = ( Df[0][2] * Df[2][1] - Df[0][1] * Df[2][2] ) * val ;
    Dfi[0][2] = ( Df[0][1] * Df[1][2] - Df[0][2] * Df[1][1] ) * val ;
    Dfi[1][0] = ( Df[1][2] * Df[2][0] - Df[1][0] * Df[2][2] ) * val ;
    Dfi[1][1] = ( Df[0][0] * Df[2][2] - Df[0][2] * Df[2][0] ) * val ;
    Dfi[1][2] = ( Df[0][2] * Df[1][0] - Df[0][0] * Df[1][2] ) * val ;
    Dfi[2][0] = ( Df[1][0] * Df[2][1] - Df[1][1] * Df[2][0] ) * val ;
    Dfi[2][1] = ( Df[0][1] * Df[2][0] - Df[0][0] * Df[2][1] ) * val ;
    Dfi[2][2] = ( Df[0][0] * Df[1][1] - Df[0][1] * Df[1][0] ) * val ;
    return ;
  }

  void TrilinearMapping :: world2map (const alucoord_t (&wld)[3], alucoord_t (&map)[3]) {
          //  Newton - Iteration zum Invertieren der Abbildung f.
    alucoord_t err = 10.0 * _epsilon ;
#ifdef ALUGRIDDEBUG
    int count = 0 ;
#endif
    map [0] = map [1] = map [2] = .0 ;
    do {
      alucoord_t upd [3] ;
      map2world (map, upd) ;
      inverse (map) ;
      alucoord_t u0 = upd [0] - wld [0] ;
      alucoord_t u1 = upd [1] - wld [1] ;
      alucoord_t u2 = upd [2] - wld [2] ;
      alucoord_t c0 = Dfi [0][0] * u0 + Dfi [0][1] * u1 + Dfi [0][2] * u2 ;
      alucoord_t c1 = Dfi [1][0] * u0 + Dfi [1][1] * u1 + Dfi [1][2] * u2 ;
      alucoord_t c2 = Dfi [2][0] * u0 + Dfi [2][1] * u1 + Dfi [2][2] * u2 ;
      map [0] -= c0 ;
      map [1] -= c1 ;
      map [2] -= c2 ;
      err = fabs (c0) + fabs (c1) + fabs (c2) ;
      alugrid_assert (count ++ < 1000) ;
    } while (err > _epsilon) ;
  }

} // namespace ALUGrid
