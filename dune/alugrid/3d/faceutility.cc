#include <config.h>
#include "faceutility.hh"

namespace Dune
{

  // Implementation of ALU3dGridSurfaceMappingFactory
  // ------------------------------------------------

  template< int dim, int dimworld, class Comm >
  typename ALU3dGridSurfaceMappingFactory< dim, dimworld, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< dim, dimworld, tetra, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
    return new SurfaceMappingType( fieldVector2alu3d_ctype(coords[0]) ,
                                   fieldVector2alu3d_ctype(coords[1]) ,
                                   fieldVector2alu3d_ctype(coords[2]) );
  }


  template< int dim, int dimworld, class Comm >
  typename ALU3dGridSurfaceMappingFactory< dim, dimworld, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< dim, dimworld, hexa, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
    return new SurfaceMappingType( coords[0], coords[1], coords[2], coords[3] );
  }


  template< int dim, int dimworld, class Comm >
  typename ALU3dGridSurfaceMappingFactory< dim, dimworld, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< dim, dimworld, tetra, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
  {
    return new SurfaceMappingType( face.myvertex(0)->Point(), face.myvertex(1)->Point(), face.myvertex(2)->Point() );
  }


  template< int dim, int dimworld, class Comm >
  typename ALU3dGridSurfaceMappingFactory< dim, dimworld, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< dim, dimworld, hexa, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
  {
    typedef FaceTopologyMapping< hexa > FaceTopo;
    // this is the new implementation using FieldVector
    // see mappings.hh
    // we have to swap the vertices, because
    // local face numbering in Dune is different to ALUGrid (see topology.cc)
    return new SurfaceMappingType(
        face.myvertex( FaceTopo::dune2aluVertex(0) )->Point(),
        face.myvertex( FaceTopo::dune2aluVertex(1) )->Point(),
        face.myvertex( FaceTopo::dune2aluVertex(2) )->Point(),
        face.myvertex( FaceTopo::dune2aluVertex(3) )->Point() );
  }



  // Helper Functions
  // ----------------

  template< int m, int n >
  inline void
  alu3dMap2World ( const ALU3DSPACE LinearSurfaceMapping &mapping,
                   const FieldVector< alu3d_ctype, m > &x,
                   FieldVector< alu3d_ctype, n > &y )
  {
    mapping.map2world( fieldVector2alu3d_ctype( x ), fieldVector2alu3d_ctype( y ) );
  }

  template< int m, int n >
  inline void
  alu3dMap2World ( const BilinearSurfaceMapping &mapping,
                   const FieldVector< alu3d_ctype, m > &x,
                   FieldVector< alu3d_ctype, n > &y )
  {
    mapping.map2world( x, y );
  }



  //- class ALU3dGridGeometricFaceInfoBase
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  void ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >
    ::referenceElementCoordinatesUnrefined ( SideIdentifier side, CoordinateType &result ) const
  {
    // get the parent's face coordinates on the reference element (Dune reference element)
    CoordinateType cornerCoords;
    referenceElementCoordinatesRefined ( side, cornerCoords );

    std::unique_ptr< typename Base::SurfaceMappingType > referenceElementMapping( Base::buildSurfaceMapping( cornerCoords ) );

    NonConformingMappingType faceMapper( connector_.face().parentRule(), connector_.face().nChild() );

    // do the mappings
    const int numCorners = childLocal_.size();
    for( int i = 0; i < numCorners; ++i )
    {
      alu3dMap2World( *referenceElementMapping, faceMapper.child2parent( childLocal_[ i ] ), result[ i ] );
    }
  }

  template<  int dimworld, ALU3dGridElementType type, class Comm >
  void ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >
    ::referenceElementCoordinatesUnrefined ( SideIdentifier side, LocalCoordinateType &result ) const
  {
    //TODO use connector.face.nChild and (maybe twist)    referenceElementCoordinatesRefined ( side, cornerCoords )

    // get the parent's face coordinates on the reference element (Dune reference element)
    LocalCoordinateType cornerCoords;
    referenceElementCoordinatesRefined ( side, cornerCoords );


    //for some reason the behavior for tetra and hexa is opposite
    if(connector_.face().nChild() == ( type == tetra) ? 1 : 0){
      result[0] = cornerCoords[0];
      result[1] =  ( cornerCoords[1] + cornerCoords[0] );
      result[1] *=0.5;
    }
    else if(connector_.face().nChild() == ( type == tetra ) ? 0 : 1)
    {
      result[0] = ( cornerCoords[1] + cornerCoords[0] );
      result[0] *= 0.5;
      result[1] = cornerCoords[1];
    }
    else
      std::cerr << "Trying to access more than two children on one face" << std::endl;

  }



  // Explicit Template Instatiation
  // ------------------------------

  template struct ALU3dGridSurfaceMappingFactory< 2,2,tetra, ALUGridNoComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2,2,hexa, ALUGridNoComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2,2,tetra, ALUGridNoComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2,2,hexa, ALUGridNoComm >;

  template struct ALU3dGridSurfaceMappingFactory< 2,2,tetra, ALUGridMPIComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2,2,hexa, ALUGridMPIComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2,2,tetra, ALUGridMPIComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2,2,hexa, ALUGridMPIComm >;

    template struct ALU3dGridSurfaceMappingFactory< 2,3,tetra, ALUGridNoComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2,3,hexa, ALUGridNoComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2,3,tetra, ALUGridNoComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2,3,hexa, ALUGridNoComm >;

  template struct ALU3dGridSurfaceMappingFactory< 2,3,tetra, ALUGridMPIComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2,3,hexa, ALUGridMPIComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2,3,tetra, ALUGridMPIComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2,3,hexa, ALUGridMPIComm >;

    template struct ALU3dGridSurfaceMappingFactory< 3,3,tetra, ALUGridNoComm >;
  template struct ALU3dGridSurfaceMappingFactory< 3,3,hexa, ALUGridNoComm >;

  template class ALU3dGridGeometricFaceInfoBase< 3,3,tetra, ALUGridNoComm >;
  template class ALU3dGridGeometricFaceInfoBase< 3,3,hexa, ALUGridNoComm >;

  template struct ALU3dGridSurfaceMappingFactory< 3,3,tetra, ALUGridMPIComm >;
  template struct ALU3dGridSurfaceMappingFactory< 3,3,hexa, ALUGridMPIComm >;

  template class ALU3dGridGeometricFaceInfoBase< 3,3,tetra, ALUGridMPIComm >;
  template class ALU3dGridGeometricFaceInfoBase< 3,3,hexa, ALUGridMPIComm >;

} // end namespace Dune
