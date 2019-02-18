#ifndef DUNE_ALU3DGRIDTOPOLOGY_HH
#define DUNE_ALU3DGRIDTOPOLOGY_HH

#include <utility>

#include <dune/alugrid/common/alugrid_assert.hh>

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


  //! Maps indices of the Dune reference element onto the indices of the
  //! ALU3dGrid reference element and vice-versa.
  template <ALU3dGridElementType type>
  class ElementTopologyMapping
  {
  public:
    enum { numFaces = EntityCount<type>::numFaces };
    enum { numVertices = EntityCount<type>::numVertices };
    enum { numEdges = EntityCount<type>::numEdges };
    enum { numVerticesPerFace = EntityCount<type>::numVerticesPerFace };

    static int generic2aluFace ( const int index );
    static int alu2genericFace ( const int index );

    static int generic2aluVertex ( const int index );
    static int alu2genericVertex ( const int index );

    //! Return 1 if faces in ALU3dGrid and Dune reference element
    //! have the same orientation (edge 0->1 is taken as reference as
    //! they are the same in both reference elements), -1 otherwise.
    //! The index is a Dune face index
    static int faceOrientation(int index);


    static std::pair< int, int > duneEdgeMap ( int edge );

    /** \brief Maps a local vertex on a face onto a global vertex
     *
     *  \param[in]  face   index of the face (with respect to ALU reference
     *                     element)
     *  \param[in]  local  local index of vertex on the face
     *  \returns global index of vertex in ALU reference element
     */
    static int faceVertex ( int face, int local );

  private:

    static const int generic2aluFace_[ numFaces ];
    static const int alu2genericFace_[ numFaces ];

    static const int generic2aluVertex_[ numVertices ];
    static const int alu2genericVertex_[ numVertices ];

    const static int faceOrientation_[numFaces];

    static const int duneEdgeMap_[ numEdges ][ 2 ];

    static const int faceVertex_[ numFaces ][ numVerticesPerFace ];
  };

  //! Maps indices of the Dune reference face onto the indices of the
  //! ALU3dGrid reference face and vice-versa.
  template <ALU3dGridElementType type>
  class FaceTopologyMapping {
  public:
    //  private:
    static int twist(int index, int faceTwist);
    static int invTwist(int index, int faceTwist);

    // for each aluTwist apply additional mapping
    static int aluTwistMap(const int aluTwist);
  private:

    const static int alu2duneTwist_[ 2 * EntityCount<type>::numVerticesPerFace ];
    const static int aluTwistMap_[ 2 * EntityCount<type>::numVerticesPerFace ];
  };

} // end namespace Dune
#endif
