#include <config.h>
#include "faceutility.hh"

namespace Dune
{

  // Implementation of ALU3dGridSurfaceMappingFactory
  // ------------------------------------------------

  template< int actualDim, int actualDimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, tetra, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
    return new SurfaceMappingType( fieldVector2alu3d_ctype(coords[0]) , 
                                   fieldVector2alu3d_ctype(coords[1]) , 
                                   fieldVector2alu3d_ctype(coords[2]) );
  }


  template< int actualDim, int actualDimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, hexa, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
    return new SurfaceMappingType( coords[0], coords[1], coords[2], coords[3] );
  }


  template< int actualDim, int actualDimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, tetra, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
  {
    return new SurfaceMappingType( face.myvertex(0)->Point(), face.myvertex(1)->Point(), face.myvertex(2)->Point() );
  }


  template< int actualDim, int actualDimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< actualDim, actualDimw, hexa, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
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
  template< int actualDim, int actualDimw, ALU3dGridElementType type, class Comm >
  void ALU3dGridGeometricFaceInfoBase< actualDim, actualDimw, type, Comm >
    ::referenceElementCoordinatesUnrefined ( SideIdentifier side, CoordinateType &result ) const
  {
    // get the parent's face coordinates on the reference element (Dune reference element)
    CoordinateType cornerCoords;
    referenceElementCoordinatesRefined ( side, cornerCoords );

    typename Base::SurfaceMappingType *referenceElementMapping = Base::buildSurfaceMapping( cornerCoords );

    NonConformingMappingType faceMapper( connector_.face().parentRule(), connector_.face().nChild() );

    const ReferenceFaceType& refFace = getReferenceFace();
    // do the mappings
    const int numCorners = refFace.size( 2 );
    for( int i = 0; i < numCorners; ++i )
    {
      const FieldVector< alu3d_ctype, 2 > &childLocal = refFace.position( i, 2 );
      alu3dMap2World( *referenceElementMapping, faceMapper.child2parent( childLocal ), result[ i ] );
    }

    delete referenceElementMapping;
  }
  
  template<  int actualDimw, ALU3dGridElementType type, class Comm >
  void ALU3dGridGeometricFaceInfoBase< 2, actualDimw, type, Comm >
    ::referenceElementCoordinatesUnrefined ( SideIdentifier side, LocalCoordinateType &result ) const
  {
    //TODO use connector.face.nChild and (maybe twist)    referenceElementCoordinatesRefined ( side, cornerCoords )
    
    // get the parent's face coordinates on the reference element (Dune reference element)
    LocalCoordinateType cornerCoords;
    referenceElementCoordinatesRefined ( side, cornerCoords );
    
    // this is a dune face index
    const int faceIndex = 
      (side == INNER ? 
       ElementTopo::alu2duneFace(connector_.innerALUFaceIndex()) :
       ElementTopo::alu2duneFace(connector_.outerALUFaceIndex()));
    const int faceTwist = 
      (side == INNER ?
       connector_.innerTwist() :
       connector_.outerTwist());
    
    std::cout << "happy ever after..." << faceTwist<< std::endl;
    
    if(connector_.face().nChild() == (faceTwist < 0 ? 1 : 0)){
      result[0] = cornerCoords[0];
      result[1] =  ( cornerCoords[1] + cornerCoords[0] );
      result[1] *=0.5;
    }
    else if(connector_.face().nChild() == (faceTwist < 0 ? 0 : 1))
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
