#ifndef DUNE_ALU3DGRIDFACEUTILITY_HH
#define DUNE_ALU3DGRIDFACEUTILITY_HH

#include <type_traits>
#include <utility>

#include <dune/geometry/referenceelements.hh>

#include "mappings.hh"
#include "alu3dinclude.hh"
#include "topology.hh"

namespace Dune
{

  // convert FieldVectors to alu3dtypes
  // only used for calculating the normals because the method of the
  // mapping classes want double (&)[3] and we have FieldVectors which store an
  // double [3] this is why we can cast here
  // plz say notin' Adrian
  template< int dim >
  inline alu3d_ctype (&fieldVector2alu3d_ctype ( FieldVector< alu3d_ctype, dim > &val ))[ dim ]
  {
    return ((alu3d_ctype (&)[dim]) (*( &(val[0])) ));
  }

  // convert const FieldVectors to const alu3dtypes
  template< int dim >
  inline const alu3d_ctype (&fieldVector2alu3d_ctype ( const FieldVector< alu3d_ctype, dim > &val ))[ dim ]
  {
    return ((const alu3d_ctype (&)[dim]) (*( &(val[0])) ) );
  }


  // * Note: reconsider lazy evaluation of coordinates

  //- class ALU3dGridFaceInfo
  /* \brief Stores face and adjoining elements of the underlying ALU3dGrid
     The class has the same notion of inner and outer element as the
     intersection iterator.
  */
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  class ALU3dGridFaceInfo
  {
    typedef ALU3dImplTraits< type, Comm >  ImplTraits;
    //- private typedefs
    typedef typename ImplTraits::HasFaceType HasFaceType;
  public:
    enum ConformanceState {CONFORMING, REFINED_INNER, REFINED_OUTER, UNDEFINED };
    //- typedefs
    typedef typename ImplTraits::GEOFaceType GEOFaceType;
    typedef typename ImplTraits::GEOElementType GEOElementType;
    typedef typename ImplTraits::GEOPeriodicType GEOPeriodicType;
    typedef typename ImplTraits::IMPLElementType IMPLElementType;
    typedef typename ImplTraits::GhostPairType GhostPairType;
    typedef typename ImplTraits::BNDFaceType BNDFaceType;

  public:
    //! constructor creating empty face info
    ALU3dGridFaceInfo( const bool levelIntersection = false );
    void updateFaceInfo(const GEOFaceType& face, int innerLevel, int innerTwist);

    //- constructors and destructors
    //! Construct a connector from a face and the twist seen from the inner
    //! element
    //! \note: The user is responsible for the consistency of the input data
    //! as well as for choosing the appropriate (i.e. most refined) face
    //! Copy constructor
    ALU3dGridFaceInfo(const GEOFaceType& face, int innerTwist);
    ALU3dGridFaceInfo(const ALU3dGridFaceInfo &orig);
    //! Destructor
    ~ALU3dGridFaceInfo();

  protected:
    //! returns true if outerEntity casts into a helement
    bool isElementLike() const;

  public:
    //- queries
    //! returns true if inside is a ghost entity
    bool innerBoundary() const;

    //! returns true if the face lies on the domain boundary
    //! and is not a periodic boundary
    bool outerBoundary() const;

    //! returns true if the face lies on the domain boundary
    bool boundary() const;

    //! returns true if outside is something meaningfull
    bool neighbor() const ;

    //! is the neighbour element a ghost element or a ghost face
    //! in case of face true is returned
    bool ghostBoundary () const;

    //! Returns the ALU3dGrid face
    const GEOFaceType& face() const;
    //! Returns the inner element at that face
    const GEOElementType& innerEntity() const;
    //! Returns the outer element at that face
    //! \note This function is only meaningful in the interior
    const GEOElementType& outerEntity() const;
    //! Returns the inner element at that face
    //! \note This function is only meaningful at a boundary
    const BNDFaceType& innerFace() const;
    //! Returns the boundary (outer) element at that face
    //! \note This function is only meaningful at a boundary
    const BNDFaceType& boundaryFace() const;

    //! Twist of the face seen from the inner element
    int innerTwist() const;
    //! Twist of the face seen from the outer element
    int outerTwist() const;

    //! Twist of the face seen from the inner element
    int duneTwist(const int faceIdx, const int aluTwist) const;

    //! Local number of the face in inner element (ALU3dGrid reference element)
    int innerALUFaceIndex() const;
    //! Local number of the face in outer element (ALU3dGrid reference element)
    int outerALUFaceIndex() const;

    int outsideLevel() const;

    //! return boundary segment id if intersection is with domain boundary
    int segmentId() const;

    //! return boundary id if intersection is with domain boundary
    int boundaryId() const;

    //! Description of conformance on the face
    ConformanceState conformanceState() const;

    //! return whether we are in a parallel environment or not
    bool parallel() const { return !std::is_same< Comm, ALUGridNoComm >::value; }

    //! return true if conforming refinement is enabled
    bool conformingRefinement () const { return conformingRefinement_; }

    //! return true if ghost cells are enabled
    bool ghostCellsEnabled () const { return ghostCellsEnabled_; }

    //! reset flags
    void setFlags( const bool conformingRefinement, const bool ghostCellsEnabled );

  private:
    //! Description of conformance on the face
    ConformanceState getConformanceState(const int innerLevel) const;

    //- forbidden methods
    const ALU3dGridFaceInfo &
    operator=(const ALU3dGridFaceInfo &orig);

  private:

    //- member data
    const GEOFaceType* face_;
    const HasFaceType* innerElement_;
    const HasFaceType* outerElement_;

    int  innerFaceNumber_;
    int  outerFaceNumber_;

    int  innerTwist_;
    int  outerTwist_;

    mutable int segmentId_;
    int bndId_;

    enum boundary_t { noBoundary          = 0, // no boundary, outside is normal element
                      periodicBoundary    = 1, // periodic boundary
                      innerGhostBoundary  = 2, // process boundary, inside is ghost, outside is normal element
                      domainBoundary      = 3, // boundary with domain, no outside
                      outerGhostBoundary  = 4};// process boundary, outside might be ghost

    boundary_t bndType_;

    ConformanceState conformanceState_;
    bool conformingRefinement_ ;       // true if conforming refinement is enabled
    bool ghostCellsEnabled_ ;          // true if ghost cells are present
    const bool levelIntersection_ ;    // true if called from a levelintersection iterator
  };


  // ALU3dGridSurfaceMappingFactory
  // ------------------------------

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  struct ALU3dGridSurfaceMappingFactory;

  template< int dim, int dimworld, class Comm >
  struct ALU3dGridSurfaceMappingFactory< dim, dimworld, tetra, Comm >
  {
    // this is the original ALUGrid LinearSurfaceMapping,
    // see mapp_tetra_3d.* in ALUGrid code
    typedef ALU3DSPACE LinearSurfaceMapping SurfaceMappingType;
    typedef typename ALU3dGridFaceInfo< dim, dimworld, tetra, Comm >::GEOFaceType GEOFaceType;

    static const int numVerticesPerFace = EntityCount< tetra >::numVerticesPerFace;

    typedef FieldMatrix< alu3d_ctype, numVerticesPerFace, 3 > CoordinateType;

    // old method, copies values for tetra twice
    SurfaceMappingType *buildSurfaceMapping ( const CoordinateType &coords ) const;
    // get face but doesn't copy values twice
    SurfaceMappingType *buildSurfaceMapping ( const GEOFaceType &face ) const;
  };

  template< int dim, int dimworld, class Comm >
  struct ALU3dGridSurfaceMappingFactory< dim, dimworld, hexa, Comm >
  {
    typedef BilinearSurfaceMapping SurfaceMappingType;
    typedef typename ALU3dGridFaceInfo< dim, dimworld, hexa, Comm >::GEOFaceType GEOFaceType;

    static const int numVerticesPerFace = EntityCount< hexa >::numVerticesPerFace;

    typedef FieldMatrix< alu3d_ctype, numVerticesPerFace, 3 > CoordinateType;

    // old method, copies values for tetra twice
    SurfaceMappingType *buildSurfaceMapping ( const CoordinateType &coords ) const;
    // get face but doesn't copy values twice
    SurfaceMappingType *buildSurfaceMapping ( const GEOFaceType &face ) const;
  };



  // ALU3dGridGeometricFaceInfoBase
  // ------------------------------

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  class ALU3dGridGeometricFaceInfoBase
  : public ALU3dGridSurfaceMappingFactory< dim, dimworld, type, Comm >
  {
    typedef ALU3dGridSurfaceMappingFactory< dim, dimworld, type, Comm > Base;

  public:
    typedef ElementTopologyMapping<type> ElementTopo;
    typedef FaceTopologyMapping<type> FaceTopo;
    typedef NonConformingFaceMapping< dim, dimworld, type, Comm > NonConformingMappingType;

    // type of container for reference elements
    typedef ReferenceElements< alu3d_ctype, 3 > ReferenceElementContainerType;
    // type of container for reference faces
    typedef ReferenceElements< alu3d_ctype, 2 > ReferenceFaceContainerType;

    // type of reference element
    typedef std::decay_t< decltype( ReferenceElementContainerType::general( std::declval< const Dune::GeometryType & >() ) ) > ReferenceElementType;
    // type of reference face
    typedef std::decay_t< decltype( ReferenceFaceContainerType::general( std::declval< const Dune::GeometryType & >() ) ) > ReferenceFaceType;

    enum SideIdentifier { INNER, OUTER };
    enum { numVerticesPerFace =
           EntityCount<type>::numVerticesPerFace };

    //- public typedefs
    typedef FieldVector<alu3d_ctype, 3> NormalType;
    typedef FieldMatrix<alu3d_ctype,
                        numVerticesPerFace,
                        dimworld> CoordinateType;

    typedef typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::GEOFaceType GEOFaceType;

  public:
    typedef ALU3dGridFaceInfo< dim, dimworld, type, Comm > ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoBase(const ConnectorType &);
    ALU3dGridGeometricFaceInfoBase(const ALU3dGridGeometricFaceInfoBase &) = default;

    //! reset status of faceGeomInfo
    void resetFaceGeom();

    //- functions
    const CoordinateType& intersectionSelfLocal() const;
    const CoordinateType& intersectionNeighborLocal() const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoBase &operator=(const ALU3dGridGeometricFaceInfoBase &);

  private:
    //- private methods
    void generateLocalGeometries() const;

    int globalVertexIndex(const int duneFaceIndex,
                          const int faceTwist,
                          const int duneFaceVertexIndex) const;

    void referenceElementCoordinatesRefined(SideIdentifier side,
                                            CoordinateType& result) const;
    void referenceElementCoordinatesUnrefined(SideIdentifier side,
                                              CoordinateType& result) const;

  protected:
    //- private data
    const ConnectorType& connector_;

    std::array< FieldVector< alu3d_ctype, 2 >, type == tetra ? 3 : 4 > childLocal_;

    mutable CoordinateType coordsSelfLocal_;
    mutable CoordinateType coordsNeighborLocal_;

    mutable bool generatedGlobal_;
    mutable bool generatedLocal_;

    inline static const ReferenceElementType& getReferenceElement()
    {

      return (type == tetra) ?
        ReferenceElementContainerType :: simplex() :
        ReferenceElementContainerType :: cube();
    }
  };

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  template< int dim, int dimworld, class Comm >
  class ALU3dGridGeometricFaceInfoTetra
  : public  ALU3dGridGeometricFaceInfoBase< dim, dimworld, tetra, Comm >
  {
    typedef ALU3dGridGeometricFaceInfoBase< dim, dimworld, tetra, Comm > Base;

  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, 3> NormalType;
    typedef typename Base::FaceTopo FaceTopo;
    typedef typename ALU3dGridFaceInfo< dim, dimworld, tetra, Comm >::GEOFaceType GEOFaceType;

    typedef ALU3dGridFaceInfo< dim, dimworld, tetra, Comm > ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoTetra(const ConnectorType& ctor);
    ALU3dGridGeometricFaceInfoTetra(const ALU3dGridGeometricFaceInfoTetra & orig);

    NormalType & outerNormal(const FieldVector<alu3d_ctype, 2>& local) const;

    //! reset status of faceGeomInfo
    void resetFaceGeom();

    //! update global geometry
    template <class GeometryImp>
    void buildGlobalGeom(GeometryImp& geo) const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoTetra & operator=(const ALU3dGridGeometricFaceInfoTetra &);

  protected:
    using Base::connector_;

  private:
    //- private data
    mutable NormalType outerNormal_;

    // false if surface mapping needs a update
    mutable bool normalUp2Date_;
  };

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  template< int dim, int dimworld, class Comm >
  class ALU3dGridGeometricFaceInfoHexa
  : public  ALU3dGridGeometricFaceInfoBase< dim, dimworld, hexa, Comm >
  {
    typedef ALU3dGridGeometricFaceInfoBase< dim, dimworld, hexa, Comm > Base;

  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, 3> NormalType;
    typedef typename Base::FaceTopo FaceTopo;
    typedef typename ALU3dGridFaceInfo< dim, dimworld, hexa, Comm >::GEOFaceType GEOFaceType;
    typedef SurfaceNormalCalculator SurfaceMappingType;

    typedef ALU3dGridFaceInfo< dim, dimworld, hexa, Comm > ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoHexa(const ConnectorType &);
    ALU3dGridGeometricFaceInfoHexa(const ALU3dGridGeometricFaceInfoHexa &);

    NormalType & outerNormal(const FieldVector<alu3d_ctype, 2>& local) const;

    //! reset status of faceGeomInfo
    void resetFaceGeom();

    //! update global geometry
    template <class GeometryImp>
    void buildGlobalGeom(GeometryImp& geo) const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoHexa & operator=(const ALU3dGridGeometricFaceInfoHexa &);

  protected:
    using Base::connector_;

  private:
    //- private data
    mutable NormalType outerNormal_;

    // surface mapping for calculating the outer normal
    mutable SurfaceMappingType mappingGlobal_;

    // false if surface mapping needs a update
    mutable bool mappingGlobalUp2Date_;
  };

  // ALU3dGridGeometricFaceInfoBase
  // ------------------------------

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  template< int dimworld, ALU3dGridElementType type, class Comm >
  class ALU3dGridGeometricFaceInfoBase<2, dimworld, type, Comm>
  {

  public:
    typedef ElementTopologyMapping<type> ElementTopo;
    typedef FaceTopologyMapping<type> FaceTopo;

    // type of container for reference elements
    typedef ReferenceElements< alu3d_ctype, 2 > ReferenceElementContainerType;
    // type of container for reference faces
    typedef ReferenceElements< alu3d_ctype, 1 > ReferenceFaceContainerType;

    // type of reference element
    typedef std::decay_t< decltype( ReferenceElementContainerType::general( std::declval< const Dune::GeometryType & >() ) ) > ReferenceElementType;
    // type of reference face
    typedef std::decay_t< decltype( ReferenceFaceContainerType::general( std::declval< const Dune::GeometryType & >() ) ) > ReferenceFaceType;

    enum SideIdentifier { INNER, OUTER };
    enum { numVerticesPerFace = 2 }; // A face in 2d is an edge

    //- public typedefs
    typedef FieldVector<alu3d_ctype, dimworld> NormalType;
    typedef FieldMatrix<alu3d_ctype,
                        numVerticesPerFace,
                        dimworld> CoordinateType;
     typedef FieldMatrix<alu3d_ctype,
                        numVerticesPerFace,
                        2> LocalCoordinateType;

    typedef typename ALU3dGridFaceInfo< 2, dimworld, type, Comm >::GEOFaceType GEOFaceType;

  public:
    typedef ALU3dGridFaceInfo< 2, dimworld, type, Comm > ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoBase(const ConnectorType &);
    ALU3dGridGeometricFaceInfoBase(const ALU3dGridGeometricFaceInfoBase &);

    //! reset status of faceGeomInfo
    void resetFaceGeom();

    //- functions
    const LocalCoordinateType& intersectionSelfLocal() const;
    const LocalCoordinateType& intersectionNeighborLocal() const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoBase &operator=(const ALU3dGridGeometricFaceInfoBase &);

  protected:
    //- protected methods
    void generateLocalGeometries() const;

  private:
    //- private methods

    int globalVertexIndex(const int duneFaceIndex,
                          const int faceTwist,
                          const int duneFaceVertexIndex) const;

    void referenceElementCoordinatesRefined(SideIdentifier side,
                                            LocalCoordinateType& result) const;
    void referenceElementCoordinatesUnrefined(SideIdentifier side,
                                              LocalCoordinateType& result) const;

  protected:
    //- private data
    const ConnectorType& connector_;

    mutable LocalCoordinateType coordsSelfLocal_;
    mutable LocalCoordinateType coordsNeighborLocal_;

    mutable bool generatedGlobal_;
    mutable bool generatedLocal_;

    inline static const ReferenceElementType& getReferenceElement()
    {
      return (type == tetra) ?
        ReferenceElementContainerType :: simplex() :
        ReferenceElementContainerType :: cube();
    }

  };

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  template< int dimworld, class Comm >
  class ALU3dGridGeometricFaceInfoTetra<2, dimworld, Comm>
  : public  ALU3dGridGeometricFaceInfoBase< 2, dimworld, tetra, Comm >
  {
    typedef ALU3dGridGeometricFaceInfoBase< 2, dimworld, tetra, Comm > Base;

  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, dimworld> NormalType;
    typedef typename Base::FaceTopo FaceTopo;
    typedef typename ALU3dGridFaceInfo< 2, dimworld, tetra, Comm >::GEOFaceType GEOFaceType;
    typedef typename ALU3dGridFaceInfo< 2, dimworld, tetra, Comm >::GEOElementType GEOElementType;
    typedef ALU3dGridFaceInfo< 2, dimworld, tetra, Comm > ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoTetra(const ConnectorType& ctor);
    ALU3dGridGeometricFaceInfoTetra(const ALU3dGridGeometricFaceInfoTetra & orig);

    NormalType & outerNormal(const FieldVector<alu3d_ctype, 1>& local) const;

    //! reset status of faceGeomInfo
    void resetFaceGeom();

    //! update global geometry
    template <class GeometryImp>
    void buildGlobalGeom(GeometryImp& geo) const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoTetra & operator=(const ALU3dGridGeometricFaceInfoTetra &);

  protected:
    using Base::connector_;

  private:
    //- private data
    mutable NormalType outerNormal_;

    // false if surface mapping needs a update
    mutable bool normalUp2Date_;
  };

  //! Helper class which provides geometric face information for the
  //! ALU3dGridIntersectionIterator
  template<  int dimworld, class Comm >
  class ALU3dGridGeometricFaceInfoHexa<2, dimworld, Comm>
  : public  ALU3dGridGeometricFaceInfoBase< 2, dimworld, hexa, Comm >
  {
    typedef ALU3dGridGeometricFaceInfoBase< 2, dimworld, hexa, Comm > Base;

  public:
    //- public typedefs
    typedef FieldVector<alu3d_ctype, dimworld> NormalType;
    typedef typename Base::FaceTopo FaceTopo;
    typedef typename ALU3dGridFaceInfo< 2, dimworld, hexa, Comm >::GEOFaceType GEOFaceType;
    typedef typename ALU3dGridFaceInfo< 2, dimworld, hexa, Comm >::GEOElementType GEOElementType;

    typedef ALU3dGridFaceInfo< 2, dimworld, hexa, Comm > ConnectorType;

    //- constructors and destructors
    ALU3dGridGeometricFaceInfoHexa(const ConnectorType &);
    ALU3dGridGeometricFaceInfoHexa(const ALU3dGridGeometricFaceInfoHexa &);

    NormalType & outerNormal(const FieldVector<alu3d_ctype, 1>& local) const;

    //! reset status of faceGeomInfo
    void resetFaceGeom();

    //! update global geometry
    template <class GeometryImp>
    void buildGlobalGeom(GeometryImp& geo) const;

  private:
    //- forbidden methods
    const ALU3dGridGeometricFaceInfoHexa & operator=(const ALU3dGridGeometricFaceInfoHexa &);

  protected:
    using Base::connector_;

  private:
    //- private data
    mutable NormalType outerNormal_;

    // false if surface mapping needs a update
    mutable bool normalUp2Date_;
  };


} // end namespace Dune

#include "faceutility_imp.cc"

#endif
