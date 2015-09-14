#ifndef DUNE_ALU3DGRID_FACTORY_HH
#define DUNE_ALU3DGRID_FACTORY_HH

#include <map>
#include <vector>

#include <dune/common/array.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/geometry/referenceelements.hh>

#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/common/boundaryprojection.hh>

#include <dune/alugrid/common/transformation.hh>
#include <dune/alugrid/3d/alugrid.hh>

namespace Dune
{
  /** \brief Factory class for 3d ALUGrids */
  template< class ALUGrid >
  class ALU3dGridFactory
  : public GridFactoryInterface< ALUGrid >
  {
    typedef ALU3dGridFactory< ALUGrid > ThisType;
    typedef GridFactoryInterface< ALUGrid > BaseType;

  public:
    typedef ALUGrid Grid;

    typedef typename Grid::ctype ctype;

    static const ALU3dGridElementType elementType = Grid::elementType;

    static const unsigned int dimension      = Grid::dimension;
    static const unsigned int dimensionworld = Grid::dimensionworld;

    typedef typename Grid::MPICommunicatorType MPICommunicatorType;

    //! \brief type of boundary projection class
    typedef DuneBoundaryProjection< dimensionworld >  DuneBoundaryProjectionType;

    template< int codim >
    struct Codim
    {
      typedef typename Grid::template Codim< codim >::Entity Entity;
      typedef typename Grid::template Codim< codim >::EntityPointer EntityPointer;
    };

    typedef unsigned int VertexId;
    typedef unsigned int GlobalIdType;

    typedef ALUGridTransformation< ctype, dimensionworld > Transformation;

    //! type of vector for world coordinates
    typedef typename Transformation::WorldVector WorldVector;
    //! type of matrix from world coordinates to world coordinates
    typedef typename Transformation::WorldMatrix WorldMatrix;

  private:
    static_assert ( (elementType == tetra || elementType == hexa),
                    "ALU3dGridFactory supports only grids containing "
                    "tetrahedrons or hexahedrons exclusively." );

    typedef Dune::BoundarySegmentWrapper< dimension, dimensionworld > BoundarySegmentWrapperType;

    static const unsigned int numCorners = EntityCount< elementType >::numVertices;
    static const unsigned int numFaces = EntityCount< elementType >::numFaces;
    static const unsigned int numFaceCorners = EntityCount< elementType >::numVerticesPerFace;

    static const unsigned int boundaryId2d = 87;

    typedef ElementTopologyMapping< elementType > ElementTopologyMappingType;
    typedef FaceTopologyMapping< elementType > FaceTopologyMappingType;

    // type of vertex coordinates put into the factory
    typedef FieldVector< ctype, dimensionworld > VertexInputType;

    // type of vertex coordinates stored inside the factory
    typedef FieldVector< ctype, 3 > VertexType;

    typedef std::vector< unsigned int > ElementType;
    typedef array< unsigned int, numFaceCorners > FaceType;

    struct FaceLess;

    typedef std::vector< std::pair< VertexType, GlobalIdType > > VertexVector;
    typedef std::vector< ElementType > ElementVector;
    typedef std::pair< FaceType, int > BndPair ;
    typedef std::map< FaceType,  int > BoundaryIdMap;
    typedef std::vector< std::pair< BndPair, BndPair > > PeriodicBoundaryVector;
    typedef std::pair< unsigned int, int > SubEntity;
    typedef std::map< FaceType, SubEntity, FaceLess > FaceMap;

    typedef std::map< FaceType, const DuneBoundaryProjectionType* > BoundaryProjectionMap;
    typedef std::vector< const DuneBoundaryProjectionType* > BoundaryProjectionVector;

    typedef std::vector< Transformation > FaceTransformationVector;

    // copy vertex numbers and store smalled #dimension ones
    void copyAndSort ( const std::vector< unsigned int > &vertices, FaceType &faceId ) const
    {
      std::vector<unsigned int> tmp( vertices );
      std::sort( tmp.begin(), tmp.end() );

      // copy only the first dimension vertices (enough for key)
      for( size_t i = 0; i < faceId.size(); ++i ) faceId[ i ] = tmp[ i ];
    }

  private:
    // return grid object
    virtual Grid* createGridObj( BoundaryProjectionVector* bndProjections, const std::string& name ) const
    {
      return new Grid( communicator_, globalProjection_, bndProjections , name, realGrid_ );
    }

  public:
    /** \brief default constructor */
    explicit ALU3dGridFactory ( const MPICommunicatorType &communicator = Grid::defaultCommunicator(),
                                bool removeGeneratedFile = true );

    /** \brief constructor taking filename for temporary outfile */
    explicit ALU3dGridFactory ( const std::string &filename,
                                const MPICommunicatorType &communicator = Grid::defaultCommunicator() );

    /** \brief constructor taking verbose flag */
    explicit ALU3dGridFactory ( const bool verbose, const MPICommunicatorType &communicator );

    /** \brief Destructor */
    virtual ~ALU3dGridFactory ();

    /** \brief insert a vertex into the coarse grid
     *
     *  \param[in]  pos  position of the vertex
     */
    virtual void insertVertex ( const VertexInputType &pos );

    /** \brief insert a vertex into the coarse grid including the vertex's globally unique id
     *
     *  \param[in]  pos       position of the vertex
     *  \param[in]  globalId  globally unique id for vertex
     */
    void insertVertex ( const VertexInputType &pos, const VertexId globalId );

    /** \brief insert an element into the coarse grid
     *
     *  \note The order of the vertices must coincide with the vertex order in
     *        the corresponding DUNE reference element.
     *
     *  \param[in]  geometry  GeometryType of the new element
     *  \param[in]  vertices  vertices of the new element
     */
    virtual void
    insertElement ( const GeometryType &geometry,
                    const std::vector< VertexId > &vertices );

    /** \brief insert a boundary element into the coarse grid
     *
     *  \note The order of the vertices must coincide with the vertex order in
     *        the corresponding DUNE reference element.
     *
     *  \param[in]  geometry      GeometryType of the boundary element
     *  \param[in]  faceVertices  vertices of the boundary element
     *  \param[in]  boundaryId    boundary identifier of the boundary element,
     *                            the default value is 1
     */

    virtual void
    insertBoundary ( const GeometryType &geometry, const std::vector< VertexId > &faceVertices, int boundaryId = 1 );


    /** \brief mark a face as boundary (and assign a boundary id)
     *
     *  \param[in]  element     index of the element, the face belongs to
     *  \param[in]  face        local number of the face within the element
     *  \param[in]  boundaryId  boundary id to assign to the face,
     *                          the default value is 1
     */
    virtual void insertBoundary ( int element, int face, int boundaryId = 1 );

    // for testing parallel GridFactory
    void insertProcessBorder ( int element, int face )
    {
      insertBoundary( element, face, ALU3DSPACE ProcessorBoundary_t );
    }

    /** \brief insert a boundary projection into the macro grid
     *
     *  \param[in]  type        geometry type of boundary face
     *  \param[in]  vertices    vertices of the boundary face
     *  \param[in]  projection  boundary projection
     *
     *  \note The grid takes control of the projection object.
     */
    virtual void
    insertBoundaryProjection ( const GeometryType &type,
                               const std::vector< VertexId > &vertices,
                               const DuneBoundaryProjectionType *projection );

    /** \brief insert a boundary segment into the macro grid
     *
     *  \param[in]  vertices         vertex indices of boundary face
     */
    virtual void
    insertBoundarySegment ( const std::vector< VertexId >& vertices ) ;

    virtual void
    insertProcessBorder ( const std::vector< VertexId >& vertices );

    /** \brief insert a shaped boundary segment into the macro grid
     *
     *  \param[in]  vertices         vertex indices of boundary face
     *  \param[in]  boundarySegment  geometric realization of shaped boundary
     */
    virtual void
    insertBoundarySegment ( const std::vector< VertexId >& vertices,
                            const shared_ptr<BoundarySegment<dimension,dimensionworld> >& boundarySegment ) ;

    /** \brief insert a boundary projection object, (a copy is made)
     *
     *  \param[in]  bndProjection instance of an ALUGridBoundaryProjection projecting vertices to a curved
     */
    virtual void insertBoundaryProjection ( const DuneBoundaryProjectionType& bndProjection );

    /** \brief add a face transformation (for periodic identification)
     *
     *  A face transformation is an affine mapping T from world coordinates
     *  to world coordinates. The grid factory then glues two faces f and g
     *  if T( f ) = g or T( g ) = f.
     *
     *  \param[in]  matrix  matrix describing the linear part of T
     *  \param[in]  shift   vector describing T( 0 )
     */
    void insertFaceTransformation ( const WorldMatrix &matrix, const WorldVector &shift );

    /** \brief finalize the grid creation and hand over the grid
     *
     *  The caller takes responsibility for deleing the grid.
     */
    Grid *createGrid ();

    Grid *createGrid ( const bool addMissingBoundaries, const std::string dgfName = "" );

    Grid *createGrid ( const bool addMissingBoundaries, bool temporary, const std::string dgfName = "" );

    virtual unsigned int
    insertionIndex ( const typename Codim< 0 >::Entity &entity ) const
    {
      return Grid::getRealImplementation( entity ).getIndex();
    }

    virtual unsigned int
    insertionIndex ( const typename Codim< dimension >::Entity &entity ) const
    {
      if(dimension == 2)
        return Grid::getRealImplementation( entity ).getIndex()/2;
      else  // dimension == 3
        return Grid::getRealImplementation( entity ).getIndex();
    }

    virtual unsigned int
    insertionIndex ( const typename Grid::LeafIntersection &intersection ) const
    {
      std::vector< unsigned int > vertices;
#if DUNE_VERSION_NEWER(DUNE_GRID,2,4)
      const typename Codim< 0 >::Entity& in = intersection.inside();
#else
      const typename Codim< 0 >::EntityPointer inPtr = intersection.inside();
      const typename Codim< 0 >::Entity &in = *inPtr;
#endif

      const Dune::ReferenceElement< double, dimension > &refElem =
          Dune::ReferenceElements< double, dimension >::general( in.type() );
      int faceNr = intersection.indexInInside();
      const int vxSize = refElem.size( faceNr, 1, dimension );
      for (int i=0;i<vxSize;++i)
      {
        int vxIdx = refElem.subEntity( faceNr, 1 , i , dimension);
#if DUNE_VERSION_NEWER(DUNE_GRID,2,4)
        vertices.push_back( insertionIndex( in.template subEntity<dimension>(vxIdx) ) );
#else
        vertices.push_back( insertionIndex( *(in.template subEntity<dimension>(vxIdx) ) ) );
#endif
      }
      FaceType faceId;
      copyAndSort( vertices, faceId );
      typename BoundaryIdMap::const_iterator pos = insertionOrder_.find( faceId );
      if( pos != insertionOrder_.end() )
        return pos->second;
      else
        return std::numeric_limits<unsigned int>::max();
    }

    virtual bool
    wasInserted ( const typename Grid::LeafIntersection &intersection ) const
    {
      return intersection.boundary() &&
             (insertionIndex(intersection) < std::numeric_limits<unsigned int>::max());
    }

    const std::vector<int>& ordering () const { return ordering_; }

  private:
    void doInsertVertex ( const VertexInputType &pos, const GlobalIdType globalId );
    void doInsertBoundary ( int element, int face, int boundaryId );

    GlobalIdType globalId ( const VertexId &id ) const
    {
      alugrid_assert ( id < vertices_.size() );
      return vertices_[ id ].second;
    }

    const VertexType &position ( const VertexId &id ) const
    {
      alugrid_assert ( id < vertices_.size() );
      return vertices_[ id ].first;
    }

    void assertGeometryType( const GeometryType &geometry );
    static void generateFace ( const ElementType &element, const int f, FaceType &face );
    void generateFace ( const SubEntity &subEntity, FaceType &face ) const;
    void correctElementOrientation ();
    bool identifyFaces ( const Transformation &transformation, const FaceType &key1, const FaceType &key2, const int defaultId );
    void searchPeriodicNeighbor ( FaceMap &faceMap, const typename FaceMap::iterator &pos, const int defaultId  );
    void reinsertBoundary ( const FaceMap &faceMap, const typename FaceMap::const_iterator &pos, const int id );
    void recreateBoundaryIds ( const int defaultId = 1 );

    // sort elements according to hilbert space filling curve (if Zoltan is available)
    void sortElements( const VertexVector& vertices, const ElementVector& elements, std::vector< int >& ordering );

    int rank_;

    VertexVector vertices_;
    ElementVector elements_;
    BoundaryIdMap boundaryIds_,insertionOrder_;
    PeriodicBoundaryVector periodicBoundaries_;
    const DuneBoundaryProjectionType* globalProjection_ ;
    BoundaryProjectionMap boundaryProjections_;
    FaceTransformationVector faceTransformations_;
    unsigned int numFacesInserted_;
    bool realGrid_;
    const bool allowGridGeneration_;
    bool foundGlobalIndex_ ;

    MPICommunicatorType communicator_;

    std::vector< int > ordering_;
  };



  template< class ALUGrid >
  struct ALU3dGridFactory< ALUGrid >::FaceLess
  : public std::binary_function< FaceType, FaceType, bool >
  {
    bool operator() ( const FaceType &a, const FaceType &b ) const
    {
      for( unsigned int i = 0; i < numFaceCorners; ++i )
      {
        if( a[ i ] != b[ i ] )
          return (a[ i ] < b[ i ]);
      }
      return false;
    }
  };


  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid >
    ::assertGeometryType( const GeometryType &geometry )
  {
    if( elementType == tetra )
    {
      if( !geometry.isSimplex() )
        DUNE_THROW( GridError, "Only simplex geometries can be inserted into "
                               "ALUGrid< 3, 3, simplex, refrule >." );
    }
    else
    {
      if( !geometry.isCube() )
        DUNE_THROW( GridError, "Only cube geometries can be inserted into "
                               "ALUGrid< 3, 3, cube, refrule >." );
    }
  }

  /** \brief Specialization of the generic GridFactory for ALUCubeGrid<3,3>
   *  \ingroup GridFactory
   */
  template<int dim, int dimw, ALUGridElementType eltype, ALUGridRefinementType refinementtype , class Comm >
  class GridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > >
  : public ALU3dGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > >
  {
    typedef GridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > > ThisType;
    typedef ALU3dGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > > BaseType;

  public:
    typedef typename BaseType::Grid Grid;

    typedef typename BaseType::MPICommunicatorType MPICommunicatorType;

    /** \brief Default constructor */
    explicit GridFactory ( const MPICommunicatorType &communicator = Grid::defaultCommunicator() )
    : BaseType( communicator )
    {}

    /** \brief constructor taking filename */
    GridFactory ( const std::string &filename,
                  const MPICommunicatorType &communicator = Grid::defaultCommunicator() )
    : BaseType( filename, communicator )
    {}

  protected:
    template< class, class, int > friend class ALULocalGeometryStorage;
    /** \brief constructor taking verbosity flag */
    GridFactory ( const bool realGrid,
                  const MPICommunicatorType &communicator) 
    : BaseType( realGrid, communicator )
    {}
  };



  // Implementation of ALU3dGridFactory
  // ----------------------------------

  template< class ALUGrid >
  inline
  ALU3dGridFactory< ALUGrid >
    :: ALU3dGridFactory ( const MPICommunicatorType &communicator,
                          bool removeGeneratedFile )
  : rank_( ALU3dGridCommunications< ALUGrid::dimension, ALUGrid::dimensionworld, elementType, MPICommunicatorType >::getRank( communicator ) ),
    globalProjection_ ( 0 ),
    numFacesInserted_ ( 0 ),
    realGrid_( true ),
    allowGridGeneration_( rank_ == 0 ),
    foundGlobalIndex_( false ),
    communicator_( communicator )
  {}

  template< class ALUGrid >
  inline
  ALU3dGridFactory< ALUGrid >
    :: ALU3dGridFactory ( const std::string &filename,
                          const MPICommunicatorType &communicator )
  : rank_( ALU3dGridCommunications< ALUGrid::dimension, ALUGrid::dimensionworld, elementType, MPICommunicatorType >::getRank( communicator ) ),
    globalProjection_ ( 0 ),
    numFacesInserted_ ( 0 ),
    realGrid_( true ),
    allowGridGeneration_( rank_ == 0 ),
    foundGlobalIndex_( false ),
    communicator_( communicator )
  {}

  template< class ALUGrid >
  inline
  ALU3dGridFactory< ALUGrid >
    :: ALU3dGridFactory ( const bool realGrid,
                          const MPICommunicatorType &communicator )
  : rank_( ALU3dGridCommunications< ALUGrid::dimension, ALUGrid::dimensionworld, elementType, MPICommunicatorType >::getRank( communicator ) ),
    globalProjection_ ( 0 ),
    numFacesInserted_ ( 0 ),
    realGrid_( realGrid ),
    allowGridGeneration_( true ),
    foundGlobalIndex_( false ),
    communicator_( communicator )
  {}

  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid > ::
  insertBoundarySegment ( const std::vector< unsigned int >& vertices )
  {
    if( vertices.size() != numFaceCorners )
      DUNE_THROW( GridError, "Wrong number of face vertices passed: " << vertices.size() << "." );

    FaceType faceId;
    copyAndSort( vertices, faceId );

    if( boundaryProjections_.find( faceId ) != boundaryProjections_.end() )
      DUNE_THROW( GridError, "Only one boundary projection can be attached to a face." );

    boundaryProjections_[ faceId ] = 0;

    BndPair boundaryId;
    for( unsigned int i = 0; i < numFaceCorners; ++i )
    {
      const unsigned int j = FaceTopologyMappingType::dune2aluVertex( i );
      boundaryId.first[ j ] = vertices[ i ];
    }
    boundaryId.second = 1;
    boundaryIds_.insert( boundaryId );

    insertionOrder_.insert( std::make_pair( faceId, insertionOrder_.size() ) );
  }

  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid > ::
  insertProcessBorder ( const std::vector< unsigned int >& vertices )
  {
    if( vertices.size() != numFaceCorners )
      DUNE_THROW( GridError, "Wrong number of face vertices passed: " << vertices.size() << "." );

    FaceType faceId;
    copyAndSort( vertices, faceId );

    boundaryProjections_[ faceId ] = 0;

    BndPair boundaryId;
    for( unsigned int i = 0; i < numFaceCorners; ++i )
    {
      const unsigned int j = FaceTopologyMappingType::dune2aluVertex( i );
      boundaryId.first[ j ] = vertices[ i ];
    }
    boundaryId.second = ALU3DSPACE ProcessorBoundary_t ;
    boundaryIds_.insert( boundaryId );
  }

  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid > ::
  insertBoundarySegment ( const std::vector< unsigned int >& vertices,
                          const shared_ptr<BoundarySegment<dimension,dimensionworld> >& boundarySegment )
  {
    if( vertices.size() != numFaceCorners )
      DUNE_THROW( GridError, "Wrong number of face vertices passed: " << vertices.size() << "." );

    FaceType faceId;
    copyAndSort( vertices, faceId );

    if( boundaryProjections_.find( faceId ) != boundaryProjections_.end() )
      DUNE_THROW( GridError, "Only one boundary projection can be attached to a face." );

    const size_t numVx = vertices.size();
    GeometryType type;
    if( numVx == 3 )
      type.makeSimplex( dimension-1 );
    else
      type.makeCube( dimension-1 );

    // we need double here because of the structure of BoundarySegment
    // and BoundarySegmentWrapper which have double as coordinate type
    typedef FieldVector< double, dimensionworld > CoordType;
    std::vector< CoordType > coords( numVx );
    for( size_t i = 0; i < numVx; ++i )
    {
      // if this assertions is thrown vertices were not inserted at first
      alugrid_assert ( vertices_.size() > vertices[ i ] );

      // get global coordinate and copy it
      const VertexType &x = position( vertices[ i ] );
      for( unsigned int j = 0; j < dimensionworld; ++j )
        coords[ i ][ j ] = x[ j ];
    }

    BoundarySegmentWrapperType* prj
      = new BoundarySegmentWrapperType( type, coords, boundarySegment );
    boundaryProjections_[ faceId ] = prj;
#ifdef ALUGRIDDEBUG
    // consistency check
    for( size_t i = 0; i < numVx; ++i )
    {
      CoordType global = (*prj)( coords [ i ] );
      if( (global - coords[ i ]).two_norm() > 1e-6 )
        DUNE_THROW(GridError,"BoundarySegment does not map face vertices to face vertices.");
    }
#endif

    BndPair boundaryId;
    for( unsigned int i = 0; i < numFaceCorners; ++i )
    {
      const unsigned int j = FaceTopologyMappingType::dune2aluVertex( i );
      boundaryId.first[ j ] = vertices[ i ];
    }
    boundaryId.second = 1;
    boundaryIds_.insert( boundaryId );

    insertionOrder_.insert( std::make_pair( faceId, insertionOrder_.size() ) );
  }


  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid >
    ::generateFace ( const SubEntity &subEntity, FaceType &face ) const
  {
    generateFace( elements_[ subEntity.first ], subEntity.second, face );
  }

} // end namespace Dune

#if COMPILE_ALUGRID_INLINE
  #include "gridfactory.cc"
#endif
#endif
