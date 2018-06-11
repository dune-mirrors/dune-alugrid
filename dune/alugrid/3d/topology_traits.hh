#ifndef DUNE_ALU3DGRIDTOPOLOGYTRAITS_HH
#define DUNE_ALU3DGRIDTOPOLOGYTRAITS_HH

namespace Dune {
  // types of the elementes,
  // i.e . tetra or hexa, mixed is not implemeneted
  enum ALU3dGridElementType { tetra = 4, hexa = 7, mixed, error };

  template <ALU3dGridElementType type>
  struct EntityCount {};

  template <>
  struct EntityCount<tetra> {
    enum {numFaces = 4};
    enum {numVertices = 4};
    enum {numEdges = 6};
    enum {numVerticesPerFace = 3};
    enum {numEdgesPerFace = 3};
  };

  template <>
  struct EntityCount<hexa> {
    enum {numFaces = 6};
    enum {numVertices = 8};
    enum {numEdges = 12};
    enum {numVerticesPerFace = 4};
    enum {numEdgesPerFace = 4};
  };

  template <ALU3dGridElementType type>
  struct ElementTopologyTraits {};

  template <>
  struct ElementTopologyTraits<tetra> : EntityCount<tetra> {
    static constexpr int dune2aluFace[numFaces] = {3, 2, 1, 0};
    // see gittergeo.cc in the ALUGrid code for this mapping
    static constexpr int dune2aluEdge[numEdges] = {0, 1, 3, 2, 4, 5};
    // and inverse mapping
    static constexpr int alu2duneEdge[numEdges] = {0, 1, 3, 2, 4, 5} ;

    static constexpr int alu2duneFace[numFaces] = {3, 2, 1, 0};

    static constexpr int dune2aluVertex[numVertices] = {0, 1, 2, 3};
    static constexpr int alu2duneVertex[numVertices] = {0, 1, 2, 3};

    static constexpr int generic2aluFace[ numFaces ] = {3, 2, 1, 0};
    static constexpr int alu2genericFace[ numFaces ] = {3, 2, 1, 0};

    static constexpr int generic2aluVertex[ numVertices ] = {0, 1, 2, 3};
    static constexpr int alu2genericVertex[ numVertices ] = {0, 1, 2, 3};

    // actually, the orientation changes on each face, but this is compensated
    // by the change in orientation of the reference face
    static constexpr int faceOrientation[numFaces] = {-1, 1, -1, 1};

    static constexpr int dune2aluFaceVertex[numFaces][numVerticesPerFace] = {{0, 1, 2},
                                                                              {0, 2, 1},
                                                                              {0, 1, 2},
                                                                              {0, 2, 1}};
    static constexpr int alu2duneFaceVertex[numFaces][numVerticesPerFace] = {{0, 2, 1},
                                                                              {0, 1, 2},
                                                                              {0, 2, 1},
                                                                              {0, 1, 2}};

    static constexpr int duneFaceTwist[ numFaces ] = { 0, -1, 0, -1 };
    static constexpr int duneEdgeMap[ numEdges ][ 2 ]
      = { { 3, 0 }, { 1, 0 }, { 3, 1 }, { 2, 0 }, { 0, 0 }, { 1, 1 } };

    static constexpr int faceVertex[ numFaces ][ numVerticesPerFace ]
      = { {1, 2, 3}, {0, 3, 2}, {0, 1, 3}, {0, 2, 1} };
  };

  template <>
  struct ElementTopologyTraits<hexa> : EntityCount<hexa> {
    // which face in the ALUGrid hexa is the face in the dune reference hexa
    static constexpr int dune2aluFace[numFaces] = {5, 3, 2, 4, 0, 1};

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
    // maps edges in the ALUGrid reference hexa to edges in the Dune Hexa
    static constexpr int dune2aluEdge[numEdges] = {2, 4, 7, 6, 1, 3,
                                                  0, 5, 9, 10, 8, 11};
    // inverse mapping of the above dune2aluEdge for hexas
    static constexpr int alu2duneEdge[numEdges] = {6, 4, 0, 5, 1, 7,
                                                  3, 2, 10, 8, 9, 11};

    //////////////////////////////////////////////////////////////////////

    // inverse mapping  of the dune2aluFace mapping
    static constexpr int alu2duneFace[numFaces] = {4, 5, 2, 1, 3, 0};

    // map the hexa vertices to the dune hexahedron vertices
    static constexpr int dune2aluVertex[numVertices] = {0, 1, 3, 2, 4, 5, 7, 6};

    // the same as dune2aluVertex for hexas
    static constexpr int alu2duneVertex[numVertices] = {0, 1, 3, 2, 4, 5, 7, 6};

    static constexpr int generic2aluFace[ numFaces ]
      = {5, 3, 2, 4, 0, 1};
    static constexpr int alu2genericFace[ numFaces ]
      = {4, 5, 2, 1, 3, 0};

    static constexpr int generic2aluVertex[ numVertices ]
      = {0, 1, 3, 2, 4, 5, 7, 6};
    static constexpr int alu2genericVertex[ numVertices ]
      = {0, 1, 3, 2, 4, 5, 7, 6};

    static constexpr int faceOrientation[numFaces] = {-1, 1, 1, -1, -1, 1};

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
    // coming up
    // parameter is local dune face number and vertex number
    static constexpr int dune2aluFaceVertex[numFaces][numVerticesPerFace] = {{0, 3, 1, 2}, // ok
                                                         {0, 1, 3, 2}, // ok
                                                         {0, 1, 3, 2}, // ok
                                                         {1, 0, 2, 3}, // ok
                                                         {0, 3, 1, 2}, // ok
                                                         {0, 1, 3, 2}};// ok

    // the inverse mapping to the above dune2aluFaceVertex
    // for hexa (see docu above)
    static constexpr int alu2duneFaceVertex[numFaces][numVerticesPerFace] = {{0, 2, 3, 1},  // ok
                                                         {0, 1, 3, 2},  // ok
                                                         {0, 1, 3, 2},  // ok
                                                         {0, 1, 3, 2},  // ok
                                                         {1, 0, 2, 3},  // ok
                                                         {0, 2, 3, 1}}; // ok

    static constexpr int duneFaceTwist[ numFaces ] = { -1, 0, 0, -4, -1, 0 };

    static constexpr int duneEdgeMap[ numEdges ][ 2 ]
      = { { 5, 0 }, { 2, 1 }, { 4, 1 }, { 3, 1 }, { 0, 0 }, { 3, 0 },
          { 2, 0 }, { 0, 1 }, { 5, 1 }, { 1, 1 }, { 1, 0 }, { 4, 2 } };
    static constexpr int faceVertex[ numFaces ][ numVerticesPerFace ]
      = { {0, 1, 2, 3}, {5, 4, 7, 6}, {0, 4, 5, 1}, {1, 5, 6, 2}, {3, 2, 6, 7}, {0, 3, 7, 4} };
  };


  template <ALU3dGridElementType type>
  struct FaceTopologyTraits {};

  template <>
  struct FaceTopologyTraits<tetra> : EntityCount<tetra> {
    static constexpr int twist(int index, int faceTwist) {
      return (faceTwist < 0) ?
        (7 - index + faceTwist)%3 : (faceTwist + index)%3 ;
    }

    static constexpr int invTwist(int index, int faceTwist) {
      return (faceTwist < 0) ?
        (7 - index + faceTwist)%3 : (3 + index - faceTwist)%3;
    }

    // alu triangle face are oriented just the other way then dune faces
    // therefore vertex 1 and 2 are swapped because
    // ALUGrid tetra face are oriented just the other way compared to Dune
    // tetra faces, see also gittergeo.cc of the ALUGrid code
    static constexpr int dune2aluVertex[numVerticesPerFace] = {0, 2, 1};

    // mapping of twists from alu 2 dune
    static constexpr int alu2duneTwist[ 2 * numVerticesPerFace] = { -2, -3, -1, 0, 2, 1 };

    // mapping of twists from alu 2 dune
    static constexpr int aluTwistMap[ 2 * numVerticesPerFace] = { 1, 2, 0, -1, -2, -3 };

    // alu triangle face are oriented just the other way then dune faces
    // therefore vertex 1 and 2 are swaped
    static constexpr int alu2duneVertex[numVerticesPerFace] = {0, 2, 1};

    static constexpr int dune2aluEdge[numEdgesPerFace] = {1, 2, 0};

    static constexpr int alu2duneEdge[numEdgesPerFace] = {2, 0, 1};
  };

  template <>
  struct FaceTopologyTraits<hexa> : EntityCount<hexa> {

    static constexpr int twist(int index, int faceTwist) {
      return (faceTwist < 0) ?
        (9 - index + faceTwist)%4 : (faceTwist + index)%4 ;
    }

    static constexpr int invTwist(int index, int faceTwist) {
      return (faceTwist < 0) ?
        (9 - index + faceTwist)%4 : (4 + index - faceTwist)%4;
    }

    // mapping of twists from alu 2 dune
    static constexpr int aluTwistMap[ 2 * numVerticesPerFace] = { -2, -3, -4, -1, 0, 1, 2, 3 } ;

    // the mapping of vertices in the reference quad
    // this is used for hexa face during intersection iterator build
    // and to calculate the intersectionSelfLocal and
    // intersectionSelfNeighbor geometries.
    static constexpr int dune2aluVertex[numVerticesPerFace] = {0, 3, 1, 2};

    static constexpr int alu2duneVertex[numVerticesPerFace] = {0, 2, 3, 1};
    static constexpr int dune2aluEdge[numEdgesPerFace] = {0, 2, 3, 1};

    static constexpr int alu2duneEdge[numEdgesPerFace] = {0, 3, 1, 2};
  };

} // end namespace Dune
#endif
