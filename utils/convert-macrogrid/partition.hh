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
      center += vertices[ elements[ i ] ].x; 
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
void partition(const std::vector< Vertex > &vertices,
               const std::vector< Element< rawId > > &elements,
               const int nPartitions, 
               std::vector< int >& partition )
{
  // enforce space filling curve ordering
  orderElementsHSFC( vertices, elements );

  typedef ALUGrid::LoadBalancer LoadBalancerType;
  typedef typename LoadBalancerType :: DataBase DataBaseType;
   
  DataBaseType db ;

  const int weight = 1 ;
  const size_t nElements = elements.size();
  for( size_t el = 0; el<nElements; ++el )
  {
    db.vertexUpdate( typename LoadBalancerType::GraphVertex( el, weight ) );
  }

  // serial mp access 
  ALUGrid :: MpAccessSerial mpa ;

  // obtain partition vector using ALUGrid's serial sfc partitioning 
  partition = db.repartition( mpa, DataBaseType :: ALUGRID_SpaceFillingCurveSerial, nPartitions );
}

#endif // ALUGRID_PARTITION_MACROGRID_HH
