#ifndef DUNE_ALU3DGRID_FACTORY_CC
#define DUNE_ALU3DGRID_FACTORY_CC

// config.h is included via cmd line argument

#include <vector>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>

// provided by consistent-edge-orientation module
#if HAVE_MESH_CONSISTENCY
#include <mesh-consistency.hh>
#else
#warning "mesh-consistency is disabled for cube meshes. Install manually or set cmake flag ALUGRID_DOWNLOAD_MESH_CONSISTENCY to TRUE."
#endif

namespace Dune
{
  namespace detail
  {
    bool correctCubeOrientationAvailable()
    {
#if HAVE_MESH_CONSISTENCY
      return true;
#else
      return false ;
#endif
    }

    bool correctCubeOrientation(const int dimension,
                                const std::vector< Dune::FieldVector<double,3> >& vertices,
                                std::vector<std::vector<unsigned int> >& elements)
    {
      bool result = false;
#if HAVE_MESH_CONSISTENCY
      if(dimension == 3)
      {
        result = MeshConsistency::orient_consistently(vertices, elements, MeshConsistency::hexahedronType);
      }
      else if ( dimension == 2 )
      {
        result = MeshConsistency::orient_consistently(vertices, elements, MeshConsistency::quadrilateralType);
      }
      else
#endif
      {
        DUNE_THROW(Exception,"mesh-consistency package not found, cannot re-orient cube elements!");
      }
      return result;
    }

  } // end namespace detail

}// end namespace Dune
#endif
