#ifndef ALU2D_VTXPROJECTION_H
#define ALU2D_VTXPROJECTION_H

#include "grid.h"

#define ALUGRID_VERTEX_PROJECTION

namespace ALU2DGrid
{

  template <int N, int NV>
  struct VtxProjection
  {
    typedef Hier< Bndel < N, NV > > hbndel_t;
    typedef Hier< Element < N, NV > > helement_t;

    enum { ncoord = helement_t::ncoord };

    virtual ~VtxProjection()
    {}

    virtual int operator()(const hbndel_t *, const double, double (&) [ncoord]) const
    {
      return 1;
    }

    virtual int operator()(const helement_t *, const double (&) [2], double (&) [ncoord]) const
    {
      return 1;
    }
  };

} // namespace ALU2DGrid

#endif // #ifndef ALU2D_VTPROJECTION_H_INCLUDED
