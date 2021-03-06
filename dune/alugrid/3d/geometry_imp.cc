#ifndef DUNE_ALUGRID_GEOMETRY_IMP_CC
#define DUNE_ALUGRID_GEOMETRY_IMP_CC

#include "grid.hh"
#include "mappings.hh"
#include "geometry.hh"
#include <dune/alugrid/common/twists.hh>


namespace Dune {
// --Geometry

template< int mydim, int cdim, class GridImp>
inline GeometryType
ALU3dGridGeometry< mydim, cdim, GridImp > :: type () const
{
  return GeometryType( (elementType == tetra) ?
      Impl :: SimplexTopology< mydim > :: type :: id :
      Impl :: CubeTopology   < mydim > :: type :: id,
      mydim );
}

template< int mydim, int cdim, class GridImp>
inline int
ALU3dGridGeometry<mydim, cdim, GridImp >::corners() const
{
  return corners_;
}

template< int mydim, int cdim, class GridImp>
inline typename ALU3dGridGeometry<mydim, cdim, GridImp >::GlobalCoordinate
ALU3dGridGeometry<mydim, cdim, GridImp >::
corner (int i) const
{
  return geoImpl()[ i ];
}


template< int mydim, int cdim, class GridImp>
inline typename ALU3dGridGeometry<mydim, cdim, GridImp >::GlobalCoordinate
ALU3dGridGeometry<mydim, cdim, GridImp >::
global (const LocalCoordinate& local) const
{
  GlobalCoordinate global;
  geoImpl().mapping().map2world(local, global);
  return global;
}

template< int mydim, int cdim, class GridImp >
inline typename ALU3dGridGeometry<mydim, cdim, GridImp >::LocalCoordinate
ALU3dGridGeometry<mydim, cdim, GridImp >::
local (const GlobalCoordinate& global) const
{
  LocalCoordinate local;
  geoImpl().mapping().world2map(global, local);
  return local;
}

template< int mydim, int cdim, class GridImp>
inline typename ALU3dGridGeometry<mydim, cdim, GridImp >::ctype
ALU3dGridGeometry<mydim, cdim, GridImp >::
integrationElement (const LocalCoordinate& local) const
{
  // this is the only case we need to specialize
  if( mydim == 3 && elementType == tetra )
  {
    alugrid_assert ( geoImpl().valid() );
    return 6.0 * geoImpl().volume();
  }
  else
    return geoImpl().mapping().det( local );
}

template<int mydim, int cdim, class GridImp>
inline typename ALU3dGridGeometry<mydim, cdim, GridImp >::ctype
ALU3dGridGeometry<mydim, cdim, GridImp >::
volume () const
{
  if( mydim == 3 )
  {
    alugrid_assert ( geoImpl().valid() );
    return geoImpl().volume() ;
  }
  else if ( mydim == 2 && elementType == tetra )
  {
    enum { factor = Factorial<mydim>::factorial };
    // local vector does not affect the result
    const LocalCoordinate dummy(0);
    return integrationElement( dummy ) / ((ctype) factor);
  }
  else
  {
    return integrationElement(LocalCoordinate(0.5));
  }
}

template< int mydim, int cdim, class GridImp>
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
affine() const
{
  return geoImpl().mapping().affine();
}

template< int mydim, int cdim, class GridImp>
inline const typename ALU3dGridGeometry<mydim, cdim, GridImp >::JacobianInverseTransposed&
ALU3dGridGeometry<mydim, cdim, GridImp >::
jacobianInverseTransposed (const LocalCoordinate& local) const
{
  return geoImpl().mapping().jacobianInverseTransposed( local );
}

template< int mydim, int cdim, class GridImp>
inline const typename ALU3dGridGeometry<mydim, cdim, GridImp >::JacobianTransposed&
ALU3dGridGeometry<mydim, cdim, GridImp >::
jacobianTransposed (const LocalCoordinate& local) const
{
  return geoImpl().mapping().jacobianTransposed( local );
}

template <int mydim, int cdim, class GridImp>
inline void
ALU3dGridGeometry<mydim, cdim, GridImp >::
print (std::ostream& ss) const
{
  const char* charElType = (elementType == tetra) ? "tetra" : "hexa";
  ss << "ALU3dGridGeometry<" << mydim << ", " << cdim << ", " << charElType << "> = {\n";
  for(int i=0; i<corners(); ++i)
  {
    ss << " corner " << i << " ";
    ss << "{" << corner(i) << "}"; ss << std::endl;
  }
  ss << "} \n";
}

// built Geometry
template <int mydim, int cdim, class GridImp>
template <class GeometryType>
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeomInFather(const GeometryType &fatherGeom , const GeometryType & myGeom)
{
  // update geo impl
  geoImpl().updateInFather( fatherGeom, myGeom );

  // my volume is a part of 1 for hexas, for tetra adjust with factor
  double volume = myGeom.volume() / fatherGeom.volume() ;
  if( elementType == tetra && mydim == 3 )
  {
    volume /= 6.0; // ???
    geoImpl().setVolume( volume );
#ifdef ALUGRIDDEBUG
    LocalCoordinate local( 0.0 );
    alugrid_assert ( std::abs( 6.0 * geoImpl().volume() - integrationElement( local ) ) < 1e-12 );
#endif
  }
  else
    geoImpl().setVolume( volume );

  return true;
}

//--hexaBuildGeom
template <int mydim, int cdim, class GridImp>
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const IMPLElementType& item)
{
  alugrid_assert( int(mydim) == 2 || int(mydim) == 3 );

  if ( elementType == hexa )
  {
    // if this assertion is thrown, use ElementTopo::dune2aluVertex instead
    // of number when calling myvertex
    alugrid_assert ( ElementTopo::dune2aluVertex(0) == 0 );
    alugrid_assert ( ElementTopo::dune2aluVertex(1) == 1 );
    alugrid_assert ( ElementTopo::dune2aluVertex(2) == 3 );
    alugrid_assert ( ElementTopo::dune2aluVertex(3) == 2 );
    alugrid_assert ( ElementTopo::dune2aluVertex(4) == 4 );
    alugrid_assert ( ElementTopo::dune2aluVertex(5) == 5 );
    alugrid_assert ( ElementTopo::dune2aluVertex(6) == 7 );
    alugrid_assert ( ElementTopo::dune2aluVertex(7) == 6 );

    if( mydim == 3 ) // hexahedron
    {
      // update geo impl
      geoImpl().update( item.myvertex(0)->Point(),
                        item.myvertex(1)->Point(),
                        item.myvertex(3)->Point(),
                        item.myvertex(2)->Point(),
                        item.myvertex(4)->Point(),
                        item.myvertex(5)->Point(),
                        item.myvertex(7)->Point(),
                        item.myvertex(6)->Point() );
    }
    else if ( mydim == 2 ) // quadrilateral
    {
      // update geo impl (drop vertex 4,5,6,7)
      geoImpl().update( item.myvertex(0)->Point(),
                        item.myvertex(1)->Point(),
                        item.myvertex(3)->Point(),
                        item.myvertex(2)->Point() );
    }
  }
  else if( elementType == tetra )
  {
    // if this assertion is thrown, use ElementTopo::dune2aluVertex instead
    // of number when calling myvertex
    alugrid_assert ( ElementTopo::dune2aluVertex(0) == 0 );
    alugrid_assert ( ElementTopo::dune2aluVertex(1) == 1 );
    alugrid_assert ( ElementTopo::dune2aluVertex(2) == 2 );
    alugrid_assert ( ElementTopo::dune2aluVertex(3) == 3 );

    if( mydim == 3 ) // tetrahedron
    {
      // update geo impl
      geoImpl().update( item.myvertex(0)->Point(),
                        item.myvertex(1)->Point(),
                        item.myvertex(2)->Point(),
                        item.myvertex(3)->Point() );
    }
    else if( mydim == 2 ) // triangle
    {
      // update geo impl (drop vertex 0)
      geoImpl().update( item.myvertex(1)->Point(),
                        item.myvertex(2)->Point(),
                        item.myvertex(3)->Point() );
    }
  }

  if( mydim == 3 )
  {
    // get volume of element
    geoImpl().setVolume( item.volume() );
  }

  return true;
}

// buildFaceGeom
template <int mydim, int cdim, class GridImp>
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const HFaceType & item, int t)
{
  // get geo face
  const GEOFaceType& face = static_cast<const GEOFaceType&> (item);

  const int numVertices = ElementTopo::numVerticesPerFace;
  typedef ALUTwist< numVertices, 2 > Twist;
  const Twist twist( t );

  // for all vertices of this face get rotatedIndex
  int rotatedALUIndex[ 4 ];
  for( int i = 0; i < numVertices; ++i )
    rotatedALUIndex[ i ] = (elementType == tetra ? twist( i ) : twist( i ) ^ (twist( i ) >> 1));

  if( elementType == hexa )
  {
    if( mydim  == 2 ) //quadrilateral
    {
      // update geometry implementation
      geoImpl().update( face.myvertex(rotatedALUIndex[0])->Point(),
                        face.myvertex(rotatedALUIndex[1])->Point(),
                        face.myvertex(rotatedALUIndex[2])->Point(),
                        face.myvertex(rotatedALUIndex[3])->Point() );
    }
    else if ( mydim == 1) //edge
    {
      //update geometry implementation
      //we cannot use the rotatedALUIndex here, because for the codimiterator we get the wrong twist
      geoImpl().update( face.myvertex(t < 0 ? 0 : 3)->Point(),
                        face.myvertex(t < 0 ? 3 : 0)->Point() );
    }
  }
  else if ( elementType == tetra )
  {
    if ( mydim == 2)  //triangle
    {
      // update geometry implementation
      geoImpl().update( face.myvertex(rotatedALUIndex[0])->Point(),
                        face.myvertex(rotatedALUIndex[1])->Point(),
                        face.myvertex(rotatedALUIndex[2])->Point());
    }
    else if ( mydim == 1 )  //edge
    {
      //update geometry implementation (drop index 0)
      geoImpl().update( face.myvertex( rotatedALUIndex[1])->Point(),
                        face.myvertex( rotatedALUIndex[2])->Point() );
    }
  }

  return true;
}

// --buildFaceGeom
template <int mydim, int cdim, class GridImp>
template <class coord_t>
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const coord_t& p0,
          const coord_t& p1,
          const coord_t& p2,
          const coord_t& p3)
{
  // update geometry implementation
  geoImpl().update( p0, p1, p2, p3 );
  return true;
}

// --buildFaceGeom
template <int mydim, int cdim, class GridImp>
template <class coord_t>
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const coord_t& p0,
          const coord_t& p1,
          const coord_t& p2)
{
  // update geometry implementation
  geoImpl().update( p0, p1, p2 );
  return true;
}


// --buildFaceGeom for edges
template <int mydim, int cdim, class GridImp>
template <class coord_t>
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const coord_t& p0,
          const coord_t& p1)
{
  alugrid_assert (mydim == 1 );
  // update geometry implementation
  geoImpl().update( p0, p1 );
  return true;
}


template <int mydim, int cdim, class GridImp> // for faces
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const FaceCoordinatesType& coords)
{
  if ( elementType == hexa )
  {
    if ( mydim == 2)
      return buildGeom( coords[0], coords[1], coords[2], coords[3] );
    else if ( mydim == 1 )
      return buildGeom( coords[0], coords[1] );
  }
  else
  {
    alugrid_assert ( elementType == tetra );
    if (mydim == 2 )
      return buildGeom( coords[0], coords[1], coords[2] );
    else if ( mydim == 1 )
      return buildGeom( coords[0], coords[1] );
  }
}

template <int mydim, int cdim, class GridImp> // for edges
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const HEdgeType & item, int twist)
{
  const GEOEdgeType & edge = static_cast<const GEOEdgeType &> (item);

  if (mydim == 1) // edge
  {
     // update geometry implementation
    geoImpl().update( edge.myvertex((twist)  %2)->Point(),
                      edge.myvertex((1+twist)%2)->Point() );
  }
  else if ( mydim == 0) // point
  {
    if (elementType == hexa)
    {
      // update geometry implementation (drop vertex 1 as it has higher global index)
      geoImpl().update( edge.myvertex(0)->Point() );
    }
    else if ( elementType == tetra)
    {
      // update geometry implementation (drop vertex 0)
      geoImpl().update( edge.myvertex(1)->Point() );
    }
  }
  return true;
}

template <int mydim, int cdim, class GridImp> // for Vertices ,i.e. Points
inline bool
ALU3dGridGeometry<mydim, cdim, GridImp >::
buildGeom(const VertexType & item, int twist)
{
  // update geometry implementation
  geoImpl().update( static_cast<const GEOVertexType &> (item).Point() );
  return true;
}

} // end namespace Dune
#endif // end DUNE_ALUGRID_GEOMETRY_IMP_CC
