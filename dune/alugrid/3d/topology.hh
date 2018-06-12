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

    //! Maps face index from Dune onto ALU3dGrid reference element
    static int dune2aluFace(int index);
    //! Maps face index from ALU3dGrid onto Dune reference element
    static int alu2duneFace(int index);

    //! Maps edge index from Dune onto ALU3dGrid reference element
    static int dune2aluEdge(int index);
    //! Maps edge index from ALU3dGrid onto Dune reference element
    static int alu2duneEdge(int index);

    //! Maps vertex index from Dune onto ALU3dGrid reference element
    static int dune2aluVertex(int index);
    //! Maps vertex index from ALU3dGrid onto Dune reference element
    static int alu2duneVertex(int index);

    static int generic2aluFace ( const int index );
    static int alu2genericFace ( const int index );

    static int generic2aluVertex ( const int index );
    static int alu2genericVertex ( const int index );

    //! Return 1 if faces in ALU3dGrid and Dune reference element
    //! have the same orientation (edge 0->1 is taken as reference as
    //! they are the same in both reference elements), -1 otherwise.
    //! The index is a Dune face index
    static int faceOrientation(int index);

    //! Maps local vertex index of a face onto a global vertex index
    //! (Dune->ALU3dGrid)
    //! \param face Face index (Dune reference element)
    //! \param localVertex Local vertex index on face <i>face</i> (Dune reference
    //! element)
    //! \return global vertex index in ALU3dGrid reference element
    static int dune2aluFaceVertex(int face, int localVertex);
    //! Maps local vertex index of a face onto a global vertex index
    //! (ALU3dGrid->Dune)
    //! \param face Face index (ALU3dGrid reference element)
    //! \param localVertex Local vertex index on face <i>face</i>
    //! (ALU3dGrid reference element)
    //! \return global vertex index in Dune reference element
    static int alu2duneFaceVertex(int face, int localVertex);

    /**
     * \brief obtain twist of ALU reference face with respect to DUNE reference face
     *
     * Applying this twist to the DUNE reference vertices, i.e.,
     * - convert vertex number to ALU numbering,
     * - apply returned twist,
     *
     * equals the result of dune2aluFaceVertex
     *
     * The inverse of this twist, applied to ALU reference vertices, i.e.,
     * - apply inverse twist
     * - convert to DUNE numbering
     * .
     * yields alu2duneFaceVertex.
     *
     * \param[in]  face  face index (in DUNE reference element)
     * \returns reference face twist
     */
    static int duneFaceTwist ( int face );

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
    const static int dune2aluFace_[numFaces];
    const static int alu2duneFace_[numFaces];

    const static int dune2aluEdge_[numEdges];
    const static int alu2duneEdge_[numEdges];

    const static int dune2aluVertex_[numVertices];
    const static int alu2duneVertex_[numVertices];

    static const int generic2aluFace_[ numFaces ];
    static const int alu2genericFace_[ numFaces ];

    static const int generic2aluVertex_[ numVertices ];
    static const int alu2genericVertex_[ numVertices ];

    const static int faceOrientation_[numFaces];

    const static int dune2aluFaceVertex_[numFaces][numVerticesPerFace];
    const static int alu2duneFaceVertex_[numFaces][numVerticesPerFace];

    static const int duneFaceTwist_[ numFaces ];

    static const int duneEdgeMap_[ numEdges ][ 2 ];

    static const int faceVertex_[ numFaces ][ numVerticesPerFace ];
  };

  //! Maps indices of the Dune reference face onto the indices of the
  //! ALU3dGrid reference face and vice-versa.
  template <ALU3dGridElementType type>
  class FaceTopologyMapping {
  public:
    //! Maps vertex index from Dune onto ALU3dGrid reference face
    static int dune2aluVertex(int index);
    //! Maps vertex index from Dune onto ALU3dGrid reference face, where the
    //! face in the ALU3dGrid has the twist <i>twist</i> compared to the orientation
    //! of the respective face in the reference element
    //! \param index local Dune vertex index on the particular face (i.e. the
    //! face which has a twist <i>twist</i> compared to the reference element's face
    //! \param twist twist of the face in consideration
    //! \return local ALU3dGrid vertex index on reference element face
    static int dune2aluVertex(int index, int twist);
    //! Maps vertex index from ALU3dGrid onto Dune reference face
    static int alu2duneVertex(int index);
    //! Maps vertex index from ALU3dGrid onto Dune reference face, where the
    //! face in the ALU3dGrid has the twist <i>twist</i> compared to the orientation
    //! of the respective face in the reference element
    //! \param index local ALU3dGrid vertex index on the particular face (i.e.
    //! the face which has a twist <i>twist</i> compared to the reference element's
    //! face
    //! \param twist twist of the face in consideration
    //! \return local Dune vertex index on reference element face
    static int alu2duneVertex(int index, int twist);
    //! Maps edge index from Dune onto ALU3dGrid reference face
    static int dune2aluEdge(int index);
    //! Maps edge index from ALU3dGrid onto Dune reference face
    static int alu2duneEdge(int index);
    //  private:
    static int twist(int index, int faceTwist);
    static int invTwist(int index, int faceTwist);

    static int twistedDuneIndex( const int idx, const int twist );

    // for each aluTwist apply additional mapping
    static int aluTwistMap(const int aluTwist);
  private:
    const static int dune2aluVertex_[EntityCount<type>::numVerticesPerFace];
    const static int alu2duneVertex_[EntityCount<type>::numVerticesPerFace];

    const static int dune2aluEdge_[EntityCount<type>::numEdgesPerFace];
    const static int alu2duneEdge_[EntityCount<type>::numEdgesPerFace];

    const static int alu2duneTwist_[ 2 * EntityCount<type>::numVerticesPerFace ];
    const static int aluTwistMap_[ 2 * EntityCount<type>::numVerticesPerFace ];
  };

} // end namespace Dune
#endif
