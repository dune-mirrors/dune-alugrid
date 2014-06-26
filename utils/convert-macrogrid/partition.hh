#ifndef ALUGRID_PARTITION_MACROGRID_HH
#define ALUGRID_PARTITION_MACROGRID_HH

#include <dune/common/fvector.hh>
#include <dune/alugrid/common/hsfc.hh>
#include <dune/alugrid/impl/parallel/gitter_pll_ldb.cc>

template< ElementRawID rawId >
void orderElementHSFC(const std::vector< Vertex > &vertices,
                      std::vector< Element< rawId > > &elements )
{
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

  const size_t nElements = elements.size();
  std::vector< Element< rawId > > orderedElements( nElements );

  typedef std::map< double, int > hsfc_t;
  hsfc_t hsfc;

  for( size_t el = 0; el<nElements; ++el )
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
}

template< ElementRawID rawId >
void fillNeighbors(const std::vector< Vertex > &vertices,
                   std::vector< Element< rawId > > &elements )
{
  typedef ALUGrid::LinkedObject::Identifier FaceKeyType;
  typedef std::pair<int,int> pair_t ;
  typedef std::map< FaceKeyType, std::vector< pair_t > > FaceMapType ;

  FaceMapType faceMap;
  const size_t nElements = elements.size();
  for( size_t el = 0; el<nElements; ++el ) 
  {
    for( int fce=0; fce<Element< rawId >::numFaces; ++fce ) 
    {
      int vx[ 4 ] = {-1,-1,-1,-1};
      // get face vertex numbers 
      for( int j=0; j<Element< rawId >::numVerticesPerFace; ++ j )
      {
        vx[ j ] = elements[ el ].vertices[ Element< rawId >::prototype( fce, j ) ];
      }

      FaceKeyType key( vx[0], vx[1], vx[2], vx[3] );
      faceMap[ key].push_back( pair_t( el, fce ) );

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
}

// we assume that the elements have been ordered by using the above ordering method
template< ElementRawID rawId >
void partition(const std::vector< Vertex >     &vertices,
               std::vector< Element< rawId > > &elements,
               const int nPartitions,
               const int partMethod )
{
  typedef ALUGrid::LoadBalancer LoadBalancerType;
  typedef typename LoadBalancerType :: DataBase DataBaseType;
   
  const DataBaseType :: method mth = DataBaseType :: method ( partMethod );
  // load balancing data base 
  DataBaseType db ;

  // order elements using the Hilbert space filling curve
  orderElementHSFC( vertices, elements );

  // fill neighbor information (needed for process border detection)
  fillNeighbors( vertices, elements );

  const int weight = 1 ;
  const size_t nElements = elements.size();
  for( size_t el = 0; el<nElements; ++el )
  {
    db.vertexUpdate( typename LoadBalancerType::GraphVertex( el, weight ) );
  }

  // if graph partitioning is used 
  if( mth > DataBaseType :: ALUGRID_SpaceFillingCurveSerial ) 
  {
    for( size_t el = 0; el<nElements; ++el )
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
  for( size_t el = 0; el<nElements; ++el )
  {
    elements[ el ].rank = partition[ el ];
  }
}

#endif // ALUGRID_PARTITION_MACROGRID_HH
