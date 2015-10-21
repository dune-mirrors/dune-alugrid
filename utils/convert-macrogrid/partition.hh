#ifndef ALUGRID_PARTITION_MACROGRID_HH
#define ALUGRID_PARTITION_MACROGRID_HH

#include <dune/common/fvector.hh>
#include <dune/alugrid/common/hsfc.hh>
#include <dune/alugrid/impl/parallel/gitter_pll_ldb.h>

#include <dune/alugrid/impl/binaryio.hh>
#include <dune/alugrid/impl/byteorder.hh>
#include <dune/alugrid/impl/macrofileheader.hh>
#include <dune/alugrid/impl/serial/serialize.h>
#include <dune/alugrid/impl/serial/gitter_sti.h>
#include <dune/alugrid/impl/serial/gitter_mgb.h>



using ALUGrid::MacroFileHeader;

// ElementRawID
// ------------

enum ElementRawID { TETRA_RAW = ALUGrid :: MacroGridBuilder :: TETRA_RAW,  // = 4
                    HEXA_RAW  = ALUGrid :: MacroGridBuilder :: HEXA_RAW    // = 8
                  };
static const int closure = ALUGrid :: Gitter :: hbndseg_STI :: closure ; // = 211


// Vertex
// ------

struct Vertex
{
  typedef Dune::FieldVector< double, 3 > Coordinate ;
  int id;
  Coordinate x ;

  std::set< int > linkage ;
  std::set< int > elements ;

  Vertex () : id( -1 ), x( 0 ), linkage(), elements() {}

  void fillLinkage( std::vector< int >& links, const int rank ) const
  {
    links.clear();
    const size_t lSize = linkage.size() ;
    if( lSize > 0 )
    {
      typedef std::set<int> :: iterator iterator ;
      // convert links to vector
      links.reserve( lSize );
      iterator lend = linkage.end();
      for( iterator link = linkage.begin(); link != lend; ++link )
      {
        // insert all ranks except mine
        if( *link != rank )
          links.push_back( *link );
      }
    }
  }
};



// Element
// -------

template< ElementRawID >
struct Element;

template<>
struct Element< TETRA_RAW >
{
  static const int numVertices = 4;
  static const int numFaces = 4 ;
  static const int numVerticesPerFace = 3 ;
  int vertices[ numVertices ];
  int neighbor[ numFaces ];
  int rank ;

  Element () : rank( 0 )
  {
    for( int i=0; i<numVertices; ++ i ) vertices[ i ] = -1;
    for( int i=0; i<numFaces; ++ i )    neighbor[ i ] = -1;
  }

  static int prototype( const int face, const int vx )
  {
    return ALUGrid::Gitter::Geometric::Tetra::prototype[ face ][ vx ];
  }
};

template<>
struct Element< HEXA_RAW >
{
  static const int numVertices = 8;
  static const int numFaces = 6 ;
  static const int numVerticesPerFace = 4 ;
  int vertices[ numVertices ];
  int neighbor[ numFaces ];
  int rank ;

  Element () : rank( 0 )
  {
    for( int i=0; i<numVertices; ++ i ) vertices[ i ] = -1;
    for( int i=0; i<numFaces; ++ i )    neighbor[ i ] = -1;
  }
  static int prototype( const int face, const int vx )
  {
    return ALUGrid::Gitter::Geometric::Hexa::prototype[ face ][ vx ];
  }
};



// BndSeg
// ------

template< ElementRawID >
struct BndSeg;

template<>
struct BndSeg< TETRA_RAW >
{
  static const int numVertices = 3;
  int bndid;
  int element; // number of element this segment belongs to
  int vertices[ numVertices ];

  BndSeg() : element( -1 ) {}
};

template<>
struct BndSeg< HEXA_RAW >
{
  static const int numVertices = 4;
  int bndid;
  int element; // number of element this segment belongs to
  int vertices[ numVertices ];

  BndSeg() : element( -1 ) {}
};



// Periodic
// --------

template< ElementRawID >
struct Periodic;

template<>
struct Periodic< TETRA_RAW >
{
  static const int numVertices = 6;
  int bndid;
  int element[ 2 ]; // number of element this segment belongs to
  int vertices[ numVertices ];

  Periodic() { element[ 0 ] = element[ 1 ] = -1; }
};

template<>
struct Periodic< HEXA_RAW >
{
  static const int numVertices = 8;
  int bndid;
  int element[ 2 ]; // number of element this segment belongs to
  int vertices[ numVertices ];

  Periodic() { element[ 0 ] = element[ 1 ] = -1; }
};


template< ElementRawID rawId >
void orderElementHSFC(const std::vector< Vertex > &vertices,
                      std::vector< Element< rawId > > &elements )
{
#ifdef USE_ZOLTAN_HSFC_ORDERING
  typedef typename Vertex::Coordinate  CoordinateType ;

  CoordinateType maxCoord;
  CoordinateType minCoord;
  const size_t vertexSize = vertices.size();
  if( vertexSize > 0 )
  {
    maxCoord = vertices[ 0 ].x;
    minCoord = vertices[ 0 ].x;
  }

  for( size_t i=0; i<vertexSize; ++i )
  {
    const CoordinateType& vx = vertices[ i ].x;
    for( int d=0; d<3; ++d )
    {
      maxCoord[ d ] = std::max( maxCoord[ d ], vx[ d ] );
      minCoord[ d ] = std::min( minCoord[ d ], vx[ d ] );
    }
  }

  // get element's center to hilbert index mapping
  Dune::SpaceFillingCurveOrdering< CoordinateType > sfc( minCoord, maxCoord );

  const int nElements = elements.size();
  std::vector< Element< rawId > > orderedElements( nElements );

  typedef std::map< double, int > hsfc_t;
  hsfc_t hsfc;

  for( int el = 0; el<nElements; ++el )
  {
    CoordinateType center( 0 );
    for( int i=0; i<rawId; ++i )
    {
      center += vertices[ elements[ el ].vertices[ i ] ].x;
    }
    center /= double( rawId );
    // generate hilbert index from element's center and store index
    hsfc[ sfc.hilbertIndex( center ) ] = el;
  }

  typedef typename hsfc_t :: iterator iterator;
  const iterator end = hsfc.end();
  size_t idx = 0;
  for( iterator it = hsfc.begin(); it != end; ++it, ++idx )
  {
    orderedElements[ idx ] = elements[ (*it).second ];
  }

  // store newly ordered elements in element vector
  elements.swap( orderedElements );
#else
  std::cerr << "Zoltan not found, no Hilbert space filling curve available." << std::endl;
#endif
}

template< ElementRawID rawId >
void fillNeighbors(const std::vector< Vertex > &vertices,
                   std::vector< Element< rawId > > &elements,
                   std::vector< BndSeg< rawId > >& bndSegs,
                   std::vector< Periodic< rawId > > &periodics )
{
  typedef ALUGrid::LinkedObject::Identifier FaceKeyType;
  typedef std::pair<int,int> pair_t ;
  typedef std::map< FaceKeyType, std::vector< pair_t > > FaceMapType ;

  FaceMapType faceMap;
  const int nElements = elements.size();
  for( int el = 0; el<nElements; ++el )
  {
    for( int fce=0; fce<Element< rawId >::numFaces; ++fce )
    {
      int vx[ 4 ] = {-1,-1,-1,-1};
      // get face vertex numbers
      for( int j=0; j<Element< rawId >::numVerticesPerFace; ++ j )
      {
        vx[ j ] = elements[ el ].vertices[ Element< rawId >::prototype( fce, j ) ];
      }

      std::sort( vx, vx+Element< rawId >::numVerticesPerFace );
      FaceKeyType key( vx[0], vx[1], vx[2], vx[3] );
      faceMap[ key ].push_back( pair_t( el, fce ) );

      // reset neighbor information (-1 is boundary)
      elements[ el ].neighbor[ fce ] = -1;
    }
  }

  typedef typename FaceMapType :: iterator iterator ;
  const iterator end = faceMap.end();
  for( iterator it = faceMap.begin(); it != end; ++it )
  {
    std::vector< pair_t >& nbs = (*it).second ;
    // size should be either 2 (interior) or 1 (boundary)
    assert( nbs.size() == 2 || nbs.size() == 1 );
    if( nbs.size() == 2 )
    {
      // set neighbor information
      for( int i=0; i<2; ++i )
        elements[ nbs[ i ].first ].neighbor[ nbs[ i ].second ] = nbs[ 1-i ].first;
    }
  }

  const int bndSegSize = bndSegs.size();
  for( int i=0; i<bndSegSize; ++i )
  {
    int vx[ 4 ] = {-1,-1,-1,-1};
    // get face vertex numbers
    for( int j=0; j<BndSeg< rawId >::numVertices; ++ j )
    {
      vx[ j ] = bndSegs[ i ].vertices[ j ];
    }

    std::sort( vx, vx+BndSeg< rawId >::numVertices );
    FaceKeyType key( vx[0], vx[1], vx[2], vx[3] );
    std::vector< pair_t >& nbs = faceMap[ key ];
    assert( nbs.size() == 1 );
    // set element number for boundary segment
    bndSegs[ i ].element = nbs[ 0 ].first ;
  }

  const int periodicsSize = periodics.size();
  for( int i=0; i<periodicsSize; ++i )
  {
    int vx[ 4 ] = {-1,-1,-1,-1};
    const int vxSize = Periodic< rawId >::numVertices/2;
    int fceVx = 0;
    for( int fce=0; fce<2; ++fce )
    {
      // get face vertex numbers
      for( int j=0; j<vxSize; ++ j, ++fceVx )
      {
        vx[ j ] = periodics[ i ].vertices[ fceVx ];
      }

      std::sort( vx, vx+vxSize );
      FaceKeyType key( vx[0], vx[1], vx[2], vx[3] );
      std::vector< pair_t >& nbs = faceMap[ key ];
      assert( nbs.size() == 1 );
      // set element number for boundary segment
      periodics[ i ].element[ fce ] = nbs[ 0 ].first ;
    }
  }
}

template < ElementRawID rawId >
void computeLinkage( std::vector< Vertex >& vertices,
                     const std::vector< Element< rawId > > &elements )
{
  // store element-vertex linkage in case this option was selected
  const int elementListSize = elements.size();
  // the position in the element list is also the element id
  for( int i = 0; i < elementListSize; ++i )
  {
    for( int j = 0; j < Element< rawId >::numVertices; ++j )
    {
      // currently this only works if the vertex id is the same as the position
      assert( vertices[ elements[ i ].vertices[ j ] ].id == elements[ i ].vertices[ j ] );
      Vertex& vertex = vertices[ elements[ i ].vertices[ j ] ];
      vertex.elements.insert( i );
      vertex.linkage.insert( elements[ i ].rank );
    }
  }

}

// we assume that the elements have been ordered by using the above ordering method
template< ElementRawID rawId >
void partition(const std::vector< Vertex >     &vertices,
               std::vector< Element< rawId > > &elements,
               std::vector< BndSeg< rawId > >& bndSegs,
               std::vector< Periodic< rawId > > &periodics,
               const int nPartitions,
               const int partMethod )
{
  typedef ALUGrid::LoadBalancer LoadBalancerType;
  typedef typename LoadBalancerType :: DataBase DataBaseType;

  const DataBaseType :: method mth = DataBaseType :: method ( partMethod );

  if( mth < DataBaseType :: ALUGRID_SpaceFillingCurveSerial ||
      mth > DataBaseType :: METIS_PartGraphRecursive )
  {
    std::cerr << "Invalid partitioning method, valid are: " <<
      DataBaseType :: methodToString(DataBaseType::ALUGRID_SpaceFillingCurveSerial) << ", " <<
      DataBaseType :: methodToString(DataBaseType::METIS_PartGraphKway) << ", and " <<
      DataBaseType :: methodToString(DataBaseType::METIS_PartGraphRecursive) << std::endl;
    std::abort();
  }

  // load balancing data base
  DataBaseType db ;

  // order elements using the Hilbert space filling curve
  orderElementHSFC( vertices, elements );

  // fill neighbor information (needed for process border detection)
  fillNeighbors( vertices, elements, bndSegs, periodics );

  const int weight = 1 ;
  const int nElements = elements.size();
  for( int el = 0; el<nElements; ++el )
  {
    db.vertexUpdate( typename LoadBalancerType::GraphVertex( el, weight ) );
  }

  // if graph partitioning is used
  if( mth > DataBaseType :: ALUGRID_SpaceFillingCurveSerial )
  {
    for( int el = 0; el<nElements; ++el )
    {
      for( int fce=0; fce<Element< rawId >::numFaces; ++fce )
      {
        const int nbIdx = elements[ el ].neighbor[ fce ];
        const int elIdx = el ;
        if( elIdx < nbIdx ) // this automatically excludes bnd
        {
          db.edgeUpdate( typename LoadBalancerType::GraphEdge( elIdx, nbIdx, weight, 0, 0) );
        }
      }
    }
  }

  // serial mp access
  ALUGrid :: MpAccessSerial mpa ;

  // obtain partition vector using ALUGrid's serial sfc partitioning
  std::vector< int > partition = db.repartition( mpa, mth, nPartitions );

  // set rank information
  for( int el = 0; el<nElements; ++el )
  {
    elements[ el ].rank = partition[ el ];
  }

  // elements on periodic boundary need to be on one process
  const int periodicsSize = periodics.size();
  for( int i=0; i<periodicsSize; ++i )
  {
    // make element have the same rank, element 0 sets rank of element 1
    elements[ periodics[ i ].element[ 1 ] ].rank = elements[ periodics[ i ].element[ 0 ] ].rank;
  }

  bndSegs.reserve( bndSegs.size() * 2 );

  //insert internal boundaries
  for( int el = 0; el<nElements; ++el )
  {
    Element< rawId >& element = elements[ el ];
    for( int fce=0; fce<Element< rawId >::numFaces; ++fce )
    {
      const int nb = element.neighbor[ fce ];
      if( nb != -1 && el < nb ) // if neighbor is element and my number is smaller
      {
        // if rank of neighboring element differ we have to insert interior boundary
        if( element.rank != elements[ nb ].rank )
        {
          BndSeg< rawId > bndSeg;
          bndSeg.bndid = closure ; // 211

          // get vertex numbers
          for( int vx=0; vx<BndSeg< rawId >::numVertices; ++vx )
          {
            bndSeg.vertices[ vx ] = element.vertices[ Element< rawId >::prototype( fce, vx ) ];
          }

          // insert interior face for neighbor (orientation is ok, look from element)
          bndSeg.element = nb ;
          bndSegs.push_back( bndSeg );

          // switch orientation for element (look from neighbor)
          std::swap( bndSeg.vertices[ 1 ], bndSeg.vertices[ Element< rawId >::numVerticesPerFace-1 ] );

          // insert interior face for element
          bndSeg.element = el ;
          bndSegs.push_back( bndSeg );
        }
      }
    }
  }
}

#endif // ALUGRID_PARTITION_MACROGRID_HH
