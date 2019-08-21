#include "topology.hh"

namespace Dune
{


  //////////////////////////////////////////////////////////////////////
  //
  //              alu hexa                                   dune hexa
  //
  //                       y                                          y
  //   faces              /                                          /
  //                     /                                          /
  //              7---------6                                 6---------7
  //          z  /.        /|                             z  /.        /|
  //          | / .  1    / |                             | / .  5    / |
  //          |/  .      /  |                             |/  .      /  |
  //          4---------5   | <-- 4 (rear)                4---------5   | <-- 3 (rear)
  //    5 --> |   .     | 3 |                       0 --> |   .     | 1 |
  //  (left)  |   3.....|...2                     (left)  |   2.....|...3
  //          |  .      |  /                              |  .      |  /
  //          | .   2   | / <-- 0 (below)                 | .   2   | / <-- 4 (below)
  //          |.        |/                                |.        |/
  //          0---------1 ---x                            0---------1 ---x
  //
  //                       y                                       y
  //             alu      /                           dune        /
  //  edges              /                                       /
  //              7---11----6                             6---11----7
  //          z  /.        /|                         z  /.        /|
  //          | 9 .      10 |                         | 8 .       9 |
  //          |/  7      /  6                         |/  2      /  3
  //          4-----8---5   |                         4----10---5   |
  //          |   .     |   |                         |   .     |   |
  //          |   3...5.|...2                         |   2...7.|...3
  //          2  .      4  /                          0  .      1  /
  //          | 1       | 3                           | 4       | 5
  //          |.        |/                            |.        |/
  //          0-----0---1 ---x                        0-----6---1 ---x
  //
  //
  ////////////////////////////////////////////////////////////////////

  template<>
  const int ElementTopologyMapping< tetra >::generic2aluFace_[ numFaces ]
    = {3, 2, 1, 0};
  template<>
  const int ElementTopologyMapping< tetra >::alu2genericFace_[ numFaces ]
    = {3, 2, 1, 0};

  template<>
  const int ElementTopologyMapping< hexa >::generic2aluFace_[ numFaces ]
    = {5, 3, 2, 4, 0, 1};
  template<>
  const int ElementTopologyMapping< hexa >::alu2genericFace_[ numFaces ]
    = {4, 5, 2, 1, 3, 0};

  template<>
  const int ElementTopologyMapping< tetra >::generic2aluVertex_[ numVertices ]
    = {0, 1, 2, 3};
  template<>
  const int ElementTopologyMapping< tetra >::alu2genericVertex_[ numVertices ]
    = {0, 1, 2, 3};

  template<>
  const int ElementTopologyMapping< hexa >::generic2aluVertex_[ numVertices ]
    = {0, 1, 3, 2, 4, 5, 7, 6};
  template<>
  const int ElementTopologyMapping< hexa >::alu2genericVertex_[ numVertices ]
    = {0, 1, 3, 2, 4, 5, 7, 6};

  // actually, the orientation changes on each face, but this is compensated
  // by the change in orientation of the reference face
  template <>
  const int ElementTopologyMapping<tetra>::
  faceOrientation_[EntityCount<tetra>::numFaces] = {-1, 1, -1, 1};

  template <>
  const int ElementTopologyMapping<hexa>::
  faceOrientation_[EntityCount<hexa>::numFaces] = {-1, 1, 1, -1, -1, 1};

  template<>
  const int ElementTopologyMapping< tetra >::duneEdgeMap_[ numEdges ][ 2 ]
    = { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 2 } };


  //********************************************************************
  //
  //  local(inside) and global(outside) numbering of the face of a hexa
  //
  //           aluFace 0       ==         duneFace 4
  //
  //    z   3           2                2         3
  //    |    ----------                  ----------
  //    |   /1       2/ alu --> dune    /2       3/  dune --> alu
  //    |  /         / {0,2,3,1}       /         /   {0,3,1,2}
  //    | /         /                 /         /
  //    |/0       3/                 /0       1/
  //     ----------                  ----------
  //    0         1                 0         1
  //
  ////////////////////////////////////////////////////////////////////
  //
  //           aluFace 1       ==         duneFace 5
  //
  //        7           6                6         7
  //         ----------                  ----------
  //        /3       2/ alu --> dune    /2       3/  dune --> alu
  //       /         / {0,1,3,2}       /         /   {0,1,3,2}
  //      /         /                 /         /
  //     /0       1/                 /0       1/
  //   4 ---------- 5              4 ---------- 5
  //    |
  //    |
  //    |
  //    |z
  //
  ////////////////////////////////////////////////////////////////////
  //
  //           aluFace 2       ==         duneFace 2
  //
  //     4          5                    4          5
  //      ----------                      ----------
  //      |3      2|  alu --> dune        |2      3|  dune --> alu
  //      |        |  {0,1,3,2}           |        |  {0,1,3,2}
  //      |        |                      |        |
  //      |0      1|                      |0      1|
  //      ----------                      ----------
  //     0          1                    0          1
  //
  ////////////////////////////////////////////////////////////////////
  //
  //           aluFace 4       ==         duneFace 3
  //
  //     7          6                    4          5
  //      ----------                      ----------
  //      |2      3|  alu --> dune        |2      3|  dune --> alu
  //      |        |  {1,0,2,3}           |        |  {1,0,2,3}
  //      |        |                      |        |
  //      |1      0|                      |0      1|
  //      ----------                      ----------
  //     3          2                    0          1
  //
  ///////////////////////////////////////////////////////////////////
  //
  //           aluFace 3       ==         duneFace 1
  //
  //           6                                 7
  //          /|                                /|
  //         /2|   alu --> dune                /3|    dune --> alu
  //        /  |   {0,1,3,2}                  /  |    {0,1,3,2}
  //       /   |                             /   |
  //     5/3  1|2                          5/2  1|3
  //      |   /                             |   /
  //      |  /                              |  /
  //      |0/                               |0/
  //      |/                                |/
  //      /--------x                        /
  //     1                                 1
  //
  ///////////////////////////////////////////////////////////////////
  //
  //           aluFace 5       ==         duneFace 0
  //
  //           7                                 6
  //          /|                                /|
  //         /2|   alu --> dune                /3|    dune --> alu
  //        /  |   {0,2,3,1}                  /  |    {0,3,1,2}
  //       /   |                             /   |
  //     4/1  3|3                          4/2  1|2
  //      |   /                             |   /
  //      |  /                              |  /
  //      |0/                               |0/
  //      |/                                |/
  // x----/                                 /
  //     0                                 0
  //
  // maps for each face the local number so that the right dune face is

  template<>
  const int ElementTopologyMapping< hexa >::duneEdgeMap_[ numEdges ][ 2 ]
    = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 4, 0 }, { 4, 1 },
        { 4, 2 }, { 4, 3 }, { 5, 0 }, { 5, 1 }, { 5, 2 }, { 5, 3 } };

  template<>
  const int ElementTopologyMapping< tetra >::faceVertex_[ numFaces ][ numVerticesPerFace ]
    = { {0,1,2}, {0, 1, 3}, {0, 2, 3}, {1, 2, 3} };
  template<>
  const int ElementTopologyMapping< hexa >::faceVertex_[ numFaces ][ numVerticesPerFace ]
    = { {0, 2, 4, 6}, {1, 3, 5, 7}, {0, 1, 4, 5}, {2, 3, 6, 7}, {0, 1, 2, 3}, {4, 5, 6, 7} };

  //- class FaceTopologyMapping
  template <>
  int FaceTopologyMapping<tetra>::
  twist(int index, int faceTwist) {
    return  0;
  }

  template <>
  int FaceTopologyMapping<hexa>::
  twist(int index, int faceTwist) {
    return 0;
  }

  template <>
  int FaceTopologyMapping<tetra>::
  invTwist(int index, int faceTwist)
  {
    return 0;
  }

  template <>
  int FaceTopologyMapping<hexa>::
  invTwist(int index, int faceTwist) {
    return 0;
  }


  // mapping of twists from alu 2 dune
  template <>
  const int FaceTopologyMapping<tetra>::
  aluTwistMap_[ 2 * EntityCount<tetra>::numVerticesPerFace] = { 1, 2, 0, -1, -2, -3 };

  // mapping of twists from alu 2 dune
  template <>
  const int FaceTopologyMapping<hexa>::
  aluTwistMap_[ 2 * EntityCount<hexa>::numVerticesPerFace] = { -2, -3, -4, -1, 0, 1, 2, 3 } ;



  template< ALU3dGridElementType type >
  inline int ElementTopologyMapping< type >::generic2aluFace ( const int index )
  {
    alugrid_assert ( (index >= 0) && (index < numFaces) );
    return generic2aluFace_[ index ];
  }

  template< ALU3dGridElementType type >
  inline int ElementTopologyMapping< type >::alu2genericFace ( const int index )
  {
    alugrid_assert ( (index >= 0) && (index < numFaces) );
    return alu2genericFace_[ index ];
  }

  template< ALU3dGridElementType type >
  inline int ElementTopologyMapping< type >::generic2aluVertex ( const int index )
  {
    alugrid_assert ( (index >= 0) && (index < numVertices) );
    return generic2aluVertex_[ index ];
  }

  template< ALU3dGridElementType type >
  inline int ElementTopologyMapping< type >::alu2genericVertex ( const int index )
  {
    alugrid_assert ( (index >= 0) && (index < numVertices) );
    return alu2genericVertex_[ index ];
  }

  template <ALU3dGridElementType type>
  inline int ElementTopologyMapping<type>::faceOrientation(int index) {
    alugrid_assert (index >= 0 && index < numVertices);
    return faceOrientation_[index];
  }


  template< ALU3dGridElementType type >
  inline std::pair< int, int > ElementTopologyMapping< type >::duneEdgeMap ( int edge )
  {
    alugrid_assert( (edge >= 0) && (edge < numEdges ) );
    return std::make_pair( duneEdgeMap_[ edge ][ 0 ], duneEdgeMap_[ edge ][ 1 ] );
  }

  template< ALU3dGridElementType type >
  inline int ElementTopologyMapping< type >::faceVertex ( int face, int local )
  {
    alugrid_assert ( (face >= 0) && (face < numFaces) );
    alugrid_assert ( (local >= 0) && (local < numVerticesPerFace) );
    return faceVertex_[ face ][ local ];
  }

  template <ALU3dGridElementType type>
  inline int FaceTopologyMapping<type>::
  aluTwistMap(const int aluTwist)
  {
    // this map has been calculated by grid/test/checktwists.cc
    // and the dune-fem twist calculator
    // this should be revised after the release 2.1
    return aluTwistMap_[ aluTwist + ((type == tetra) ? 3 : 4) ];
  }

  template class ElementTopologyMapping<tetra>;
  template class ElementTopologyMapping<hexa>;

  template class FaceTopologyMapping<tetra>;
  template class FaceTopologyMapping<hexa>;
} // end namespace Dune
