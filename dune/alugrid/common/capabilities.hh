#ifndef DUNE_ALUGRID_CAPABILITIES_HH
#define DUNE_ALUGRID_CAPABILITIES_HH

#include <dune/common/version.hh>
#include <dune/geometry/type.hh>
#include <dune/grid/common/capabilities.hh>
#include <dune/alugrid/common/declaration.hh>

/** @file
 *  @author Robert Kloefkorn
 *  @brief Capabilities for ALUGrid
 */

namespace Dune
{

  namespace Capabilities
  {

    // Capabilities for ALUGrid
    // ------------------------

    /** \brief ALUGrid has only one geometry type for codim 0 entities
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
    struct hasSingleGeometryType< ALUGrid< dim, dimworld, eltype, refinementtype, Comm > >
    {
      static const bool v = true;
      static const unsigned int topologyId = (eltype == cube) ?
        GeometryTypes::cube(dim).id() : GeometryTypes::simplex(dim).id();
    };

    /** \brief ALUGrid has entities for all codimension
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm, int cdim >
    struct hasEntity< ALUGrid< dim, dimworld, eltype, refinementtype, Comm >, cdim >
    {
      static const bool v = true;
    };

    /** \brief ALUGrid has entities for all codimension
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm, int cdim >
    struct hasEntityIterator< ALUGrid< dim, dimworld, eltype, refinementtype, Comm >, cdim >
      : public hasEntity< ALUGrid< dim, dimworld, eltype, refinementtype, Comm >, cdim >
    {
    };

    /** \brief ALUGrid can communicate when Comm == ALUGridMPIComm
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, int codim >
    struct canCommunicate< ALUGrid< dim, dimworld, eltype, refinementtype, ALUGridNoComm >, codim >
    {
      static const bool v = false;
    };

    /** \brief ALUGrid can communicate
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, int codim >
    struct canCommunicate< ALUGrid< dim, dimworld, eltype, refinementtype, ALUGridMPIComm >, codim >
    {
      static const bool v = true;
    };

    /** \brief ALUGrid has potentially nonconforming level grids
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
    struct isLevelwiseConforming< ALUGrid< dim, dimworld, eltype, refinementtype, Comm > >
    {
      static const bool v = false;
    };

    /** \brief ALUGrid has potentially nonconforming leaf grids (unless refinementtype is conforming)
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
    struct isLeafwiseConforming< ALUGrid< dim, dimworld, eltype, refinementtype, Comm > >
    {
      static const bool v = refinementtype == conforming ;
    };

    /** \brief ALUGrid has backup and restore facilities
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
    struct hasBackupRestoreFacilities< ALUGrid< dim, dimworld, eltype, refinementtype, Comm > >
    {
      static const bool v = true;
    };


    /** \brief ALUGrid is not generally thread safe (only grid iteration is)
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
    struct threadSafe< ALUGrid< dim, dimworld, eltype, refinementtype, Comm > > {
      static const bool v = false;
    };

    /** \brief ALUGrid is view thread safe (i.e. things that don't modify the grid)
    \ingroup ALUGrid
    */
    template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
    struct viewThreadSafe< ALUGrid< dim, dimworld, eltype, refinementtype, Comm > > {
      static const bool v = true;
    };


  } // end namespace Capabilities

} //end  namespace Dune

#endif // #ifdef DUNE_ALUGRID_CAPABILITIES_HH
