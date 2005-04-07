#ifndef __MAPP_TETRA_3D_EXT_HH__
#define __MAPP_TETRA_3D_EXT_HH__

// BSGridVecType is defined in BSGrid interface to dune or in gitter_sti.hh 
class BSGridLinearSurfaceMapping
{
protected:
  double _n [3] ;
  const double (&_p0)[3], (&_p1)[3], (&_p2)[3] ;
public: 
  inline BSGridLinearSurfaceMapping 
    (const double (&)[3], const double (&)[3], const double (&)[3]);
    
  // same as method normal of LinearSurfaceMapping, just for Dune Vecs 
  inline void normal(double * normal) const;
};


inline BSGridLinearSurfaceMapping :: BSGridLinearSurfaceMapping (const double (&x0)[3], 
const double (&x1)[3], const double (&x2)[3])
  //: LinearSurfaceMapping (x0,x1,x2)
  : _p0 (x0), _p1 (x1), _p2 (x2)
{
  // copied from LinearSurfaceMapping, ist a bit faster 
  _n[0] = -0.5 * ((_p1[1]-_p0[1]) *(_p2[2]-_p1[2]) - (_p2[1]-_p1[1]) *(_p1[2]-_p0[2])) ;
  _n[1] = -0.5 * ((_p1[2]-_p0[2]) *(_p2[0]-_p1[0]) - (_p2[2]-_p1[2]) *(_p1[0]-_p0[0])) ;
  _n[2] = -0.5 * ((_p1[0]-_p0[0]) *(_p2[1]-_p1[1]) - (_p2[0]-_p1[0]) *(_p1[1]-_p0[1])) ;
}

inline void BSGridLinearSurfaceMapping :: normal (double * normal) const
{
  normal[0] = this->_n[0];
  normal[1] = this->_n[1];
  normal[2] = this->_n[2];
  return ;
}
#endif
