#ifndef DUNE_ALU3DGRIDTOPOLOGY_HH
#define DUNE_ALU3DGRIDTOPOLOGY_HH

#include <utility>

#include <dune/alugrid/common/alugrid_assert.hh>

#include "topology_traits.hh"

namespace Dune {
  //! Maps indices of the Dune reference element onto the indices of the
  //! ALU3dGrid reference element and vice-versa.
  template <ALU3dGridElementType type>
  class ElementTopologyMapping : public ElementTopologyTraits<type>
  {
    using Base = ElementTopologyTraits<type>;
  public:
    enum { numFaces = EntityCount<type>::numFaces };
    enum { numVertices = EntityCount<type>::numVertices };
    enum { numEdges = EntityCount<type>::numEdges };
    enum { numVerticesPerFace = EntityCount<type>::numVerticesPerFace };

    //! Maps face index from Dune onto ALU3dGrid reference element
    static inline int dune2aluFace(int index) {
      alugrid_assert (index >= 0 && index < numFaces);
      return Base::dune2aluFace[index];
    }
    //! Maps face index from ALU3dGrid onto Dune reference element
    static inline int alu2duneFace(int index) {
      alugrid_assert (index >= 0 && index < numFaces);
      return Base::alu2duneFace[index];
    }

    //! Maps edge index from Dune onto ALU3dGrid reference element
    static inline int dune2aluEdge(int index) {
      alugrid_assert (index >= 0 && index < numEdges);
      return Base::dune2aluEdge[index];
    }
    //! Maps edge index from ALU3dGrid onto Dune reference element
    static int alu2duneEdge(int index) {
      alugrid_assert (index >= 0 && index < numEdges);
      return Base::alu2duneEdge[index];
    }

    //! Maps vertex index from Dune onto ALU3dGrid reference element
    static inline int dune2aluVertex(int index) {
      alugrid_assert (index >= 0 && index < numVertices);
      return Base::dune2aluVertex[index];
    }
    //! Maps vertex index from ALU3dGrid onto Dune reference element
    static inline int alu2duneVertex(int index) {
      alugrid_assert (index >= 0 && index < numVertices);
      return Base::alu2duneVertex[index];
    }

    static inline int generic2aluFace ( const int index ) {
      alugrid_assert ( (index >= 0) && (index < numFaces) );
      return Base::generic2aluFace[ index ];
    }
    static inline int alu2genericFace ( const int index ) {
      alugrid_assert ( (index >= 0) && (index < numFaces) );
      return Base::alu2genericFace[ index ];
    }

    static inline int generic2aluVertex ( const int index ) {
      alugrid_assert ( (index >= 0) && (index < numVertices) );
      return Base::generic2aluVertex[ index ];
    }
    static inline int alu2genericVertex ( const int index ) {
      alugrid_assert ( (index >= 0) && (index < numVertices) );
      return Base::alu2genericVertex[ index ];
    }

    //! Return 1 if faces in ALU3dGrid and Dune reference element
    //! have the same orientation (edge 0->1 is taken as reference as
    //! they are the same in both reference elements), -1 otherwise.
    //! The index is a Dune face index
    static inline int faceOrientation(int index) {
      alugrid_assert (index >= 0 && index < numVertices);
      return Base::faceOrientation[index];
    }

    //! Maps local vertex index of a face onto a global vertex index
    //! (Dune->ALU3dGrid)
    //! \param face Face index (Dune reference element)
    //! \param localVertex Local vertex index on face <i>face</i> (Dune reference
    //! element)
    //! \return global vertex index in ALU3dGrid reference element
    static inline int dune2aluFaceVertex(int face, int localVertex) {
      alugrid_assert (face >= 0 && face < numFaces);
      alugrid_assert (localVertex >= 0 && localVertex < numVerticesPerFace);
      return Base::dune2aluFaceVertex[face][localVertex];
    }

    //! Maps local vertex index of a face onto a global vertex index
    //! (ALU3dGrid->Dune)
    //! \param face Face index (ALU3dGrid reference element)
    //! \param localVertex Local vertex index on face <i>face</i>
    //! (ALU3dGrid reference element)
    //! \return global vertex index in Dune reference element
    static inline int alu2duneFaceVertex(int face, int localVertex) {
      alugrid_assert (face >= 0 && face < numFaces);
      alugrid_assert (localVertex >= 0 && localVertex < numVerticesPerFace);
      return Base::alu2duneFaceVertex[face][localVertex];
    }

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
    static inline int duneFaceTwist ( int face ) {
      alugrid_assert( (face >= 0) && (face < numFaces) );
      return Base::duneFaceTwist[ face ];
    }

    static inline std::pair< int, int > duneEdgeMap ( int edge ) {
      alugrid_assert( (edge >= 0) && (edge < numEdges ) );
      return std::make_pair( Base::duneEdgeMap[ edge ][ 0 ], Base::duneEdgeMap[ edge ][ 1 ] );
    }

    /** \brief Maps a local vertex on a face onto a global vertex
     *
     *  \param[in]  face   index of the face (with respect to ALU reference
     *                     element)
     *  \param[in]  local  local index of vertex on the face
     *  \returns global index of vertex in ALU reference element
     */
    static inline int faceVertex ( int face, int local ) {
      alugrid_assert ( (face >= 0) && (face < numFaces) );
      alugrid_assert ( (local >= 0) && (local < numVerticesPerFace) );
      return Base::faceVertex[ face ][ local ];
    }
  };

  //! Maps indices of the Dune reference face onto the indices of the
  //! ALU3dGrid reference face and vice-versa.
  template <ALU3dGridElementType type>
  class FaceTopologyMapping : public FaceTopologyTraits<type> {
    using Base = FaceTopologyTraits<type>;
  public:
    //! Maps vertex index from Dune onto ALU3dGrid reference face
    static inline int dune2aluVertex(int index) {
      alugrid_assert (index >= 0 && index < EntityCount<type>::numVerticesPerFace);
      return Base::dune2aluVertex[index];
    }
    //! Maps vertex index from Dune onto ALU3dGrid reference face, where the
    //! face in the ALU3dGrid has the twist <i>twist</i> compared to the orientation
    //! of the respective face in the reference element
    //! \param index local Dune vertex index on the particular face (i.e. the
    //! face which has a twist <i>twist</i> compared to the reference element's face
    //! \param twist twist of the face in consideration
    //! \return local ALU3dGrid vertex index on reference element face
    static inline int dune2aluVertex(int index, int twist) {
      alugrid_assert (index >= 0 && index < EntityCount<type>::numVerticesPerFace);
      return Base::invTwist(Base::dune2aluVertex[index], twist);
    }
    //! Maps vertex index from ALU3dGrid onto Dune reference face
    static inline int alu2duneVertex(int index) {
      alugrid_assert (index >= 0 && index < EntityCount<type>::numVerticesPerFace);
      return Base::alu2duneVertex[index];
    }
    //! Maps vertex index from ALU3dGrid onto Dune reference face, where the
    //! face in the ALU3dGrid has the twist <i>twist</i> compared to the orientation
    //! of the respective face in the reference element
    //! \param index local ALU3dGrid vertex index on the particular face (i.e.
    //! the face which has a twist <i>twist</i> compared to the reference element's
    //! face
    //! \param twist twist of the face in consideration
    //! \return local Dune vertex index on reference element face
    static inline int alu2duneVertex(int index, int twist) {
      alugrid_assert (index >= 0 && index < EntityCount<type>::numVerticesPerFace);
      return Base::alu2duneVertex[Base::invTwist(index, twist)];
    }
    //! Maps edge index from Dune onto ALU3dGrid reference face
    static inline int dune2aluEdge(int index) {
      alugrid_assert (index >= 0 && index < EntityCount<type>::numEdgesPerFace);
      return Base::dune2aluEdge[index];
    }
    //! Maps edge index from ALU3dGrid onto Dune reference face
    static inline int alu2duneEdge(int index) {
      alugrid_assert (index >= 0 && index < EntityCount<type>::numEdgesPerFace);
      return Base::alu2duneEdge[index];
    }
    //  private:
    /*
    static int twist(int index, int faceTwist);
    static int invTwist(int index, int faceTwist);
    */

    static inline int twistedDuneIndex(const int duneIdx, const int aluTwist) {
      return alu2duneVertex( dune2aluVertex(duneIdx) , aluTwist );
    }

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
