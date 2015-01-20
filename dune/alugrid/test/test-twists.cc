#include <config.h>

// the twist need the experimental features
#ifndef DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS
#define DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS 1
#endif

#include <dune/alugrid/3d/topology.hh>
#include <dune/alugrid/common/twists.hh>
#include <dune/alugrid/test/checktwists.hh>

template< class Topo, class Twists >
bool checkReferenceTwists ( const Topo &topo, const Twists &twists )
{
  using Dune::permutation;

  typedef typename Twists::Twist Twist;

  bool success = true;

  for( int face = 0; face < topo.numFaces; ++face )
  {
    Twist twist( topo.duneFaceTwist( face ) );

    std::vector< int > v1( topo.numVerticesPerFace );
    std::vector< int > v2( topo.numVerticesPerFace );

    for( int i = 0; i < topo.numVerticesPerFace; ++i )
      v1[ i ] = (topo.numVerticesPerFace == 3 ? twist( i ) : twist( i ) ^ (twist( i ) >> 1));
    for( int i = 0; i < topo.numVerticesPerFace; ++i )
      v2[ i ] = topo.dune2aluFaceVertex( face, i );

    if( !std::equal( v1.begin(), v1.end(), v2.begin() ) )
    {
      std::cerr << "Error: Wrong reference twist for DUNE face " << face << ": " << permutation( twist, topo.numVerticesPerFace ) << std::endl;
      std::cerr << "       Application yields ";
      char delim = '[';
      for( int i = 0; i < topo.numVerticesPerFace; ++i, delim = ',' )
        std::cerr << delim << ' ' << v1[ i ];
      std::cerr << " ] (should be ";
      delim = '[';
      for( int i = 0; i < topo.numVerticesPerFace; ++i, delim = ',' )
        std::cerr << delim << ' ' << v2[ i ];
      std::cerr << " ])." << std::endl;
      success = false;
    }
  }

  return success;
}


int main ( int argc, char **argv )
{
  bool success = true;

  Dune::ALUTwists< 2, 1 > lineTwists;
  success &= Dune::checkTwists( lineTwists );

  Dune::ALUTwists< 3, 2 > triangleTwists;
  success &= Dune::checkTwists( triangleTwists );

  Dune::ALUTwists< 4, 2 > quadrilateralTwists;
  success &= Dune::checkTwists( quadrilateralTwists );

  Dune::ElementTopologyMapping< Dune::tetra > tetraTopo;
  success &= checkReferenceTwists( tetraTopo, triangleTwists );

  Dune::ElementTopologyMapping< Dune::hexa > hexaTopo;
  success &= checkReferenceTwists( hexaTopo, quadrilateralTwists );

  return success ? 0 : 1;
}
