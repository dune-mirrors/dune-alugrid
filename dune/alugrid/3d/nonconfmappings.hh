#ifndef DUNE_ALU3DGRID_NONCONFORMING_MAPPINGS_HH
#define DUNE_ALU3DGRID_NONCONFORMING_MAPPINGS_HH

// System includes
#include <limits>
#include <cmath>

// Dune includes
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/exceptions.hh>

// Local includes
#include "alu3dinclude.hh"

#include "../impl/serial/mappings.hh"

namespace Dune {

  ///////////////////////////////////////////////////////////////////
  //
  // NonConforming Mappings
  //
  ///////////////////////////////////////////////////////////////////


  //! General form of non-conforming face mapping
  //! This class is empty and needs to be specialised
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  class NonConformingFaceMapping;

  //! Non-conforming face mappings for tetrahedra
  template< int dim, int dimworld, class Comm >
  class NonConformingFaceMapping< dim, dimworld, tetra, Comm >
  {
  public:
    typedef FieldVector< alu3d_ctype, 3 > CoordinateType;
    typedef typename ALU3dImplTraits< tetra, Comm >::HfaceRuleType RefinementRuleType;

    NonConformingFaceMapping ( RefinementRuleType rule, int nChild )
    : rule_( rule ), nChild_( nChild )
    {}

    void child2parent ( const CoordinateType &childCoordinates,
                        CoordinateType &parentCoordinates) const;

    CoordinateType child2parent ( const FieldVector< alu3d_ctype, 2 > &childCoordinates ) const;

  private:
    void child2parentNosplit(const CoordinateType& childCoordinates,
                             CoordinateType& parentCoordinates) const;
    void child2parentE01(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentE12(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentE20(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentIso4(const CoordinateType& childCoordinates,
                          CoordinateType& parentCoordinates) const;

    RefinementRuleType rule_;
    int nChild_;
  };

  //! Non-conforming face mappings for hexahedra
  template< int dim, int dimworld, class Comm >
  class NonConformingFaceMapping< dim, dimworld, hexa, Comm >
  {
  public:
    typedef FieldVector< alu3d_ctype, 2 > CoordinateType;
    typedef typename ALU3dImplTraits< hexa, Comm >::HfaceRuleType RefinementRuleType;

    NonConformingFaceMapping ( RefinementRuleType rule, int nChild )
    : rule_( rule ), nChild_( nChild )
    {}

    void child2parent ( const CoordinateType &childCoordinates,
                        CoordinateType &parentCoordinates) const;

    CoordinateType child2parent ( const FieldVector< alu3d_ctype, 2 > &childCoordinates ) const;

  private:
    void child2parentNosplit(const CoordinateType& childCoordinates,
                             CoordinateType& parentCoordinates) const;
    void child2parentIso4(const CoordinateType& childCoordinates,
                          CoordinateType& parentCoordinates) const;

    RefinementRuleType rule_;
    int nChild_;
  };

} // end namespace Dune

#endif
