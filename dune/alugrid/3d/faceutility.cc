#include <config.h>
#include "faceutility.hh"

namespace Dune
{

  // Implementation of ALU3dGridSurfaceMappingFactory
  // ------------------------------------------------

  template<  class Comm >
  typename ALU3dGridSurfaceMappingFactory< 3, 3, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 3, 3, tetra, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
    return new SurfaceMappingType( fieldVector2alu3d_ctype(coords[0]) , 
                                   fieldVector2alu3d_ctype(coords[1]) , 
                                   fieldVector2alu3d_ctype(coords[2]) );
  }


  template<  class Comm >
  typename ALU3dGridSurfaceMappingFactory< 3, 3, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 3, 3, hexa, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
    return new SurfaceMappingType( coords[0], coords[1], coords[2], coords[3] );
  }


  template< class Comm >
  typename ALU3dGridSurfaceMappingFactory< 3, 3, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 3, 3, tetra, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
  {
    return new SurfaceMappingType( face.myvertex(0)->Point(), face.myvertex(1)->Point(), face.myvertex(2)->Point() );
  }


  template<  class Comm >
  typename ALU3dGridSurfaceMappingFactory< 3, 3, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 3, 3, hexa, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
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
  
  //specialisation for 2d
   template< int dimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< 2, dimw, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 2, dimw, tetra, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
   static  SurfaceMappingType map;
    map.buildMapping( coords[0], coords[1] );
    return &map;
  }


  template<  int dimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< 2, dimw, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 2, dimw, hexa, Comm >::buildSurfaceMapping ( const CoordinateType &coords ) const
  {
    static SurfaceMappingType map;
    map.buildMapping( coords[0], coords[1] );
    return &map;
  }


  template< int dimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< 2, dimw, tetra, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 2, dimw, tetra, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
  {
    FieldVector<alu3d_ctype, dimw> coords1,coords2;
    coords1[0] = face.myvertex(1)->Point()[0];
    coords1[1] = face.myvertex(1)->Point()[1];
    coords2[0] = face.myvertex(2)->Point()[0];   
    coords2[1] = face.myvertex(2)->Point()[1];
    if(dimw ==3 )
    {
      coords1[2] = face.myvertex(1)->Point()[2];
      coords2[2] = face.myvertex(2)->Point()[2];        
    }             
    static SurfaceMappingType map;
    map.buildMapping( coords1, coords2 );
    return &map;
  }


  template<  int dimw, class Comm >
  typename ALU3dGridSurfaceMappingFactory< 2, dimw, hexa, Comm >::SurfaceMappingType *
  ALU3dGridSurfaceMappingFactory< 2, dimw, hexa, Comm >::buildSurfaceMapping ( const GEOFaceType &face ) const
  {
    // this is the new implementation using FieldVector 
    // see mappings.hh 
    FieldVector<alu3d_ctype, dimw> coords1,coords2;
    coords1[0] = face.myvertex(0)->Point()[0];
    coords1[1] = face.myvertex(0)->Point()[1];
    coords2[0] = face.myvertex(3)->Point()[0];   
    coords2[1] = face.myvertex(3)->Point()[1];
    if(dimw ==3 )
    {
      coords1[2] = face.myvertex(0)->Point()[2];
      coords2[2] = face.myvertex(3)->Point()[2];        
    }             
    static SurfaceMappingType map;
    map.buildMapping( coords1, coords2 );
    return &map;
  }



  // Helper Functions
  // ----------------

  template<  int m, int n >
  inline void
  alu3dMap2World ( const LinearMapping< n, m > &mapping,
                   const FieldVector< alu3d_ctype, m > &x,
                   FieldVector< alu3d_ctype, n > &y )
  {
    mapping.map2world( x, y );
  }

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
  template< int dim, int dimw, ALU3dGridElementType type, class Comm >
  void ALU3dGridGeometricFaceInfoBase< dim, dimw, type, Comm >
    ::referenceElementCoordinatesUnrefined ( SideIdentifier side, CoordinateType &result ) const
  {
  
      // get the parent's face coordinates on the reference element (Dune reference element)
      CoordinateType cornerCoords;
      referenceElementCoordinatesRefined ( side, cornerCoords );

      typename Base::SurfaceMappingType *referenceElementMapping = Base::buildSurfaceMapping( cornerCoords );

      NonConformingMappingType faceMapper( connector_.face().parentRule(), connector_.face().nChild() );

      const ReferenceFaceType& refFace = getReferenceFace();
      // do the mappings
      const int numCorners = refFace.size( dim-1 );
      for( int i = 0; i < numCorners; ++i )
      {
        const FieldVector< alu3d_ctype, dim-1 > &childLocal = refFace.position( i, dim-1 );
        alu3dMap2World( *referenceElementMapping, faceMapper.child2parent( childLocal ), result[ i ] );
      }

      delete referenceElementMapping;
  }
  



  // Explicit Template Instatiation
  // ------------------------------

  template struct ALU3dGridSurfaceMappingFactory< 2, 2, tetra, ALUGridNoComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2, 2, hexa, ALUGridNoComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2, 2, tetra, ALUGridNoComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2, 2, hexa, ALUGridNoComm >;

  template struct ALU3dGridSurfaceMappingFactory< 2, 2, tetra, ALUGridMPIComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2, 2, hexa, ALUGridMPIComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2, 2, tetra, ALUGridMPIComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2, 2, hexa, ALUGridMPIComm >;
  
  template struct ALU3dGridSurfaceMappingFactory< 2, 3, tetra, ALUGridNoComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2, 3, hexa, ALUGridNoComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2, 3, tetra, ALUGridNoComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2, 3, hexa, ALUGridNoComm >;

  template struct ALU3dGridSurfaceMappingFactory< 2, 3, tetra, ALUGridMPIComm >;
  template struct ALU3dGridSurfaceMappingFactory< 2, 3, hexa, ALUGridMPIComm >;

  template class ALU3dGridGeometricFaceInfoBase< 2, 3, tetra, ALUGridMPIComm >;
  template class ALU3dGridGeometricFaceInfoBase< 2, 3, hexa, ALUGridMPIComm >;
  
  template struct ALU3dGridSurfaceMappingFactory< 3, 3, tetra, ALUGridNoComm >;
  template struct ALU3dGridSurfaceMappingFactory< 3, 3, hexa, ALUGridNoComm >;

  template class ALU3dGridGeometricFaceInfoBase< 3, 3, tetra, ALUGridNoComm >;
  template class ALU3dGridGeometricFaceInfoBase< 3, 3, hexa, ALUGridNoComm >;

  template struct ALU3dGridSurfaceMappingFactory< 3, 3, tetra, ALUGridMPIComm >;
  template struct ALU3dGridSurfaceMappingFactory< 3, 3, hexa, ALUGridMPIComm >;

  template class ALU3dGridGeometricFaceInfoBase< 3, 3, tetra, ALUGridMPIComm >;
  template class ALU3dGridGeometricFaceInfoBase< 3, 3, hexa, ALUGridMPIComm >;
 
} // end namespace Dune
