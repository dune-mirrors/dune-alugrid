#ifndef __DUNE_MAPP_TETRA_3D_EXT_HH__
#define __DUNE_MAPP_TETRA_3D_EXT_HH__


// BSGridVecType is defined in BSGrid interface to dune or in gitter_sti.hh 
class BSGridLinearSurfaceMapping : public LinearSurfaceMapping 
{
public: 
  inline BSGridLinearSurfaceMapping 
    (const double (&)[3], const double (&)[3], const double (&)[3]);
    
  // same as method normal of LinearSurfaceMapping, just for Dune Vecs 
  inline void normal(BSGridVecType &normal) const;
};


inline BSGridLinearSurfaceMapping :: BSGridLinearSurfaceMapping (const double (&x0)[3], 
const double (&x1)[3], const double (&x2)[3])
  : LinearSurfaceMapping (x0,x1,x2)
{
}

inline void BSGridLinearSurfaceMapping :: normal (BSGridVecType & normal) const
{
  normal[0] = this->_n[0];
  normal[1] = this->_n[1];
  normal[2] = this->_n[2];
  return ;
}
#endif
