#ifndef DUNE_ALU3DGRID_FACTORY_HH
#define DUNE_ALU3DGRID_FACTORY_HH

#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <vector>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/geometry/referenceelements.hh>

#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/common/boundaryprojection.hh>

#include <dune/alugrid/common/transformation.hh>
#include <dune/alugrid/3d/alugrid.hh>

#include <dune/alugrid/common/hsfc.hh>

namespace Dune
{
  /** \brief Factory class for ALUGrids */
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

    template< int codim >
    struct Codim
    {
      typedef typename Grid::template Codim< codim >::Entity Entity;
    };

    typedef unsigned int VertexId;
    typedef unsigned int GlobalIdType;

    typedef ALUGridTransformation< ctype, dimensionworld > Transformation;

    //! type of vector for world coordinates
    typedef typename Transformation::WorldVector WorldVector;
    //! type of matrix from world coordinates to world coordinates
    typedef typename Transformation::WorldMatrix WorldMatrix;

    typedef typename Grid::Communication Communication;

    // typedef grid pointer type based on what the grid factory interface defines
    typedef decltype(std::declval< Dune::GridFactoryInterface< Grid >* >()->createGrid())  GridPtrType;

  private:
    static_assert ( (elementType == tetra || elementType == hexa),
                    "ALU3dGridFactory supports only grids containing "
                    "tetrahedrons or hexahedrons exclusively." );

    //! \brief type of boundary projection class
    typedef DuneBoundaryProjection< dimensionworld >  DuneBoundaryProjectionType;

    typedef Dune::BoundarySegmentWrapper< dimension, dimensionworld > BoundarySegmentWrapperType;
    typedef ALUGridBoundaryProjection< Grid > ALUProjectionType;

    static const unsigned int numCorners = EntityCount< elementType >::numVertices;
    static const unsigned int numFaces = EntityCount< elementType >::numFaces;
    static const unsigned int numFaceCorners = EntityCount< elementType >::numVerticesPerFace;

    typedef ElementTopologyMapping< elementType > ElementTopologyMappingType;
    typedef FaceTopologyMapping< elementType > FaceTopologyMappingType;

    // type of vertex coordinates put into the factory
    typedef FieldVector< ctype, dimensionworld > VertexInputType;
    typedef SpaceFillingCurveOrdering< VertexInputType > SpaceFillingCurveOrderingType;

    // type of vertex coordinates stored inside the factory
    typedef FieldVector< ctype, 3 > VertexType;

    typedef std::vector< unsigned int > ElementType;
    typedef std::array< unsigned int, numFaceCorners > FaceType;

    struct FaceLess;

    typedef std::vector< std::pair< VertexType, GlobalIdType > > VertexVector;
    typedef std::vector< ElementType > ElementVector;
    typedef std::pair< FaceType, int > BndPair ;
    typedef std::map < FaceType, int > BoundaryIdMap;
    typedef std::vector< std::pair< BndPair, BndPair > > PeriodicBoundaryVector;
    typedef std::pair< unsigned int, int > SubEntity;
    typedef std::map< FaceType, SubEntity, FaceLess > FaceMap;

    typedef std::map< FaceType, const DuneBoundaryProjectionType* > BoundaryProjectionMap;
    typedef std::vector< const DuneBoundaryProjectionType* > BoundaryProjectionVector;

    typedef std::vector< Transformation > FaceTransformationVector;

    static void copy ( const std::initializer_list< unsigned int > &vertices, FaceType &faceId )
    {
      std::copy_n( vertices.begin(), faceId.size(), faceId.begin() );
    }

    static FaceType makeFace ( const std::vector< unsigned int > &vertices )
    {
      if( vertices.size() != (dimension == 2 ? 2 : numFaceCorners) )
        DUNE_THROW( GridError, "Wrong number of face vertices passed: " << vertices.size() << "." );

      FaceType faceId;
      if( dimension == 2 )
      {
        if( elementType == tetra )
          copy( { 0, vertices[ 1 ]+1, vertices[ 0 ]+1 }, faceId );
        else if( elementType == hexa )
          copy( { 2*vertices[ 0 ], 2*vertices[ 1 ], 2*vertices[ 0 ]+1, 2*vertices[ 1 ]+1 }, faceId );
      }
      else if( dimension == 3 )
        std::copy_n( vertices.begin(), numFaceCorners, faceId.begin() );
      return faceId;
    }

    static BndPair makeBndPair ( const FaceType &face, const int id )
    {
      BndPair bndPair;
      for( unsigned int i = 0; i < numFaceCorners; ++i )
      {
        const unsigned int j = FaceTopologyMappingType::dune2aluVertex( i );
        bndPair.first[ j ] = face[ i ];
      }
      bndPair.second = id;
      return bndPair;
    }

    void markLongestEdge( std::vector< bool >& elementOrientation, const bool resortElements = true  ) ;
    void markLongestEdge();

  private:
    // return grid object
    virtual Grid* createGridObj( const std::string& name ) const
    {
      ALU3DSPACE ProjectVertexPtrPair pv = std::make_pair( globalProjection_, surfaceProjection_ );
      return new Grid( communicator_, pv, name, realGrid_ );
    }

  protected:
    /** \brief constructor taking verbose flag */
    explicit ALU3dGridFactory ( const bool verbose, const MPICommunicatorType &communicator );

  public:
    /** \brief default constructor */
    explicit ALU3dGridFactory ( const MPICommunicatorType &communicator = Grid::defaultCommunicator(),
                                bool removeGeneratedFile = true );

    /** \brief constructor taking filename for temporary outfile */
    explicit ALU3dGridFactory ( const std::string &filename,
                                const MPICommunicatorType &communicator = Grid::defaultCommunicator() );

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
                            const std::shared_ptr<BoundarySegment<dimension,dimensionworld> >& boundarySegment ) ;

    /** \brief insert a boundary projection object, (a copy is made)
     *
     *  \param[in]  bndProjection instance of an ALUGridBoundaryProjection projecting vertices to a curved
     */
    virtual void insertBoundaryProjection ( const DuneBoundaryProjectionType& bndProjection, const bool isSurfaceProjection = (dimension != dimensionworld) );

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
    GridPtrType createGrid ();

    GridPtrType createGrid ( const bool addMissingBoundaries, const std::string dgfName = "" );

    GridPtrType createGrid ( const bool addMissingBoundaries, bool temporary, const std::string dgfName = "" );

    virtual unsigned int
    insertionIndex ( const typename Codim< 0 >::Entity &entity ) const
    {
      alugrid_assert( entity.impl().getIndex() < int(ordering_.size()) );
      return ordering_[ entity.impl().getIndex() ];
    }

    virtual unsigned int
    insertionIndex ( const typename Codim< dimension >::Entity &entity ) const
    {
      if(dimension == 2 && elementType == hexa )
        // for quadrilaterals we simply half the number, see gridfactory.cc doInsertVertex
        return entity.impl().getIndex()/2;
      else if ( dimension == 2 && elementType == tetra )
        // for triangles we have to substract 1, see gridfactory.cc doInsertVertex
        return entity.impl().getIndex() - 1;
      else  // dimension 3
        return entity.impl().getIndex();
    }

    virtual unsigned int insertionIndex ( const typename Grid::LevelIntersection &intersection ) const
    {
      return boundaryInsertionIndex( intersection.inside(), intersection.indexInInside() );
    }

    virtual unsigned int insertionIndex ( const typename Grid::LeafIntersection &intersection ) const
    {
      return boundaryInsertionIndex( intersection.inside(), intersection.indexInInside() );
    }

    virtual bool wasInserted ( const typename Grid::LevelIntersection &intersection ) const
    {
      return intersection.boundary() && (insertionIndex(intersection) < std::numeric_limits<unsigned int>::max());
    }

    virtual bool wasInserted ( const typename Grid::LeafIntersection &intersection ) const
    {
      return intersection.boundary() && (insertionIndex(intersection) < std::numeric_limits<unsigned int>::max());
    }

    const std::vector<unsigned int>& ordering () const { return ordering_; }


    //! set longest edge marking for biscetion grids (default is off)
    void setLongestEdgeFlag (bool flag = true) { markLongestEdge_ = flag ; }

    /** \brief Return the Communication used by the grid factory
     *
     * Use the Communication available from the grid.
     */
    Communication comm() const
    {
      return Communication(communicator_);
    }

  private:
    unsigned int boundaryInsertionIndex ( const typename Codim< 0 >::Entity &entity, int face ) const
    {
      const auto& refElem = Dune::ReferenceElements< double, dimension >::general( entity.type() );
      const int vxSize = refElem.size( face, 1, dimension );
      std::vector< unsigned int > vertices( vxSize );
      for( int i = 0; i < vxSize; ++i )
        vertices[ i ] = insertionIndex( entity.template subEntity< dimension >( refElem.subEntity( face, 1, i, dimension ) ) );

      FaceType faceId = makeFace( vertices );
      std::sort( faceId.begin(), faceId.end() );

      const auto pos = insertionOrder_.find( faceId );
      return (pos != insertionOrder_.end() ? pos->second : std::numeric_limits< unsigned int >::max());
    }

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

    const VertexInputType inputPosition ( const VertexId &id ) const
    {
      alugrid_assert ( id < vertices_.size() );
      VertexType vertex = vertices_[ id ].first;
      VertexInputType iVtx(0.);
      for(unsigned i = 0 ; i < dimensionworld ; ++i)
        iVtx[i] = vertex[i];
      return iVtx;
    }

    void assertGeometryType( const GeometryType &geometry );
    static void generateFace ( const ElementType &element, const int f, FaceType &face );
    void generateFace ( const SubEntity &subEntity, FaceType &face ) const;
    void correctElementOrientation ();
    bool identifyFaces ( const Transformation &transformation, const FaceType &key1, const FaceType &key2, const int defaultId );
    void searchPeriodicNeighbor ( FaceMap &faceMap, typename FaceMap::iterator &pos, const int defaultId  );
    void reinsertBoundary ( const FaceMap &faceMap, const typename FaceMap::const_iterator &pos, const int id );
    void recreateBoundaryIds ( const int defaultId = 1 );

    // sort elements according to hilbert space filling curve (if Zoltan is available)
    void sortElements( const VertexVector& vertices, const ElementVector& elements, std::vector< unsigned int >& ordering );

    int rank_;

    VertexVector vertices_;
    ElementVector elements_;
    BoundaryIdMap boundaryIds_,insertionOrder_;
    PeriodicBoundaryVector periodicBoundaries_;
    ALU3DSPACE ProjectVertexPtr globalProjection_ ;
    ALU3DSPACE ProjectVertexPtr surfaceProjection_ ;
    BoundaryProjectionMap boundaryProjections_;
    FaceTransformationVector faceTransformations_;
    unsigned int numFacesInserted_;
    bool realGrid_;
    const bool allowGridGeneration_;
    bool foundGlobalIndex_ ;

    MPICommunicatorType communicator_;

    typename SpaceFillingCurveOrderingType :: CurveType curveType_;
    std::vector< unsigned int > ordering_;

    bool markLongestEdge_;
  };



  template< class ALUGrid >
  struct ALU3dGridFactory< ALUGrid >::FaceLess
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
                               "ALUGrid< 3, 3, simplex, refrule >." << geometry  );
    }
    else
    {
      if( !geometry.isCube() )
        DUNE_THROW( GridError, "Only cube geometries can be inserted into "
                               "ALUGrid< 3, 3, cube, refrule >." );
    }
  }

  /** \brief Specialization of the generic GridFactory for ALUGrid
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

    /** \brief Default constructor ignoring MPIComm */
    template <class MPIComm>
    explicit GridFactory ( const MPIComm & )
    : BaseType( Grid::defaultCommunicator() )
    {}

    /** \brief constructor taking filename */
    explicit GridFactory ( const std::string &filename,
                           const MPICommunicatorType &communicator = Grid::defaultCommunicator() )
    : BaseType( filename, communicator )
    {}

    /** \brief constructor taking filename and ignoring MPIComm */
    template <class MPIComm>
    explicit GridFactory ( const std::string &filename,
                           const MPIComm & )
    : BaseType( filename, Grid::defaultCommunicator() )
    {}
  };

  template< class Grid >
  class ReferenceGridFactory;

  // Specialization of the ReferenceGridFactory for ALUGrid
  template<int dim, int dimw, ALUGridElementType eltype, ALUGridRefinementType refinementtype , class Comm >
  class ReferenceGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > >
  : public ALU3dGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > >
  {
    typedef ReferenceGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > > ThisType;
    typedef ALU3dGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > > BaseType;

  public:
    typedef typename BaseType::Grid Grid;

    typedef typename BaseType::MPICommunicatorType MPICommunicatorType;

    /** \brief Default constructor */
    ReferenceGridFactory()
      : BaseType(false, Grid::defaultCommunicator() )
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
    surfaceProjection_ ( 0 ),
    numFacesInserted_ ( 0 ),
    realGrid_( true ),
    allowGridGeneration_( rank_ == 0 ),
    foundGlobalIndex_( false ),
    communicator_( communicator ),
    curveType_( SpaceFillingCurveOrderingType :: DefaultCurve ),
    markLongestEdge_( ALUGrid::dimension == 2 )
  {
    BoundarySegmentWrapperType::registerFactory();
    ALUProjectionType::registerFactory();
  }

  template< class ALUGrid >
  inline
  ALU3dGridFactory< ALUGrid >
    :: ALU3dGridFactory ( const std::string &filename,
                          const MPICommunicatorType &communicator )
  : rank_( ALU3dGridCommunications< ALUGrid::dimension, ALUGrid::dimensionworld, elementType, MPICommunicatorType >::getRank( communicator ) ),
    globalProjection_ ( 0 ),
    surfaceProjection_ ( 0 ),
    numFacesInserted_ ( 0 ),
    realGrid_( true ),
    allowGridGeneration_( rank_ == 0 ),
    foundGlobalIndex_( false ),
    communicator_( communicator ),
    curveType_( SpaceFillingCurveOrderingType :: DefaultCurve ),
    markLongestEdge_( ALUGrid::dimension == 2 )
  {
    BoundarySegmentWrapperType::registerFactory();
    ALUProjectionType::registerFactory();
  }

  template< class ALUGrid >
  inline
  ALU3dGridFactory< ALUGrid >
    :: ALU3dGridFactory ( const bool realGrid,
                          const MPICommunicatorType &communicator )
  : rank_( ALU3dGridCommunications< ALUGrid::dimension, ALUGrid::dimensionworld, elementType, MPICommunicatorType >::getRank( communicator ) ),
    globalProjection_ ( 0 ),
    surfaceProjection_ ( 0 ),
    numFacesInserted_ ( 0 ),
    realGrid_( realGrid ),
    allowGridGeneration_( true ),
    foundGlobalIndex_( false ),
    communicator_( communicator ),
    curveType_( SpaceFillingCurveOrderingType :: DefaultCurve ),
    markLongestEdge_( ALUGrid::dimension == 2 )
  {
    BoundarySegmentWrapperType::registerFactory();
    ALUProjectionType::registerFactory();
  }

  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid > ::
  insertBoundarySegment ( const std::vector< unsigned int >& vertices )
  {
    FaceType faceId = makeFace( vertices );

    boundaryIds_.insert( makeBndPair( faceId, 1 ) );

    std::sort( faceId.begin(), faceId.end() );
    if( boundaryProjections_.find( faceId ) != boundaryProjections_.end() )
      DUNE_THROW( GridError, "Only one boundary projection can be attached to a face." );

    boundaryProjections_[ faceId ] = nullptr;

    insertionOrder_.insert( std::make_pair( faceId, insertionOrder_.size() ) );
  }

  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid > ::
  insertProcessBorder ( const std::vector< unsigned int >& vertices )
  {
    FaceType faceId = makeFace( vertices );

    boundaryIds_.insert( makeBndPair( faceId, ALU3DSPACE ProcessorBoundary_t ) );

    std::sort( faceId.begin(), faceId.end() );
    boundaryProjections_[ faceId ] = nullptr;
  }

  template< class ALUGrid >
  inline void ALU3dGridFactory< ALUGrid > ::
  insertBoundarySegment ( const std::vector< unsigned int >& vertices,
                          const std::shared_ptr<BoundarySegment<dimension,dimensionworld> >& boundarySegment )
  {
    const std::size_t numVx = vertices.size();

    GeometryType type = (elementType == tetra) ?
        GeometryTypes::simplex(dimension-1) :
        GeometryTypes::cube(dimension-1);

    // we need double here because of the structure of BoundarySegment
    // and BoundarySegmentWrapper which have double as coordinate type
    typedef FieldVector< double, dimensionworld > CoordType;
    std::vector< CoordType > coords( numVx );
    for( std::size_t i = 0; i < numVx; ++i )
    {
      // adapt vertex index for 2d grids
      const std::size_t vtx = (dimension == 2 ? (elementType == tetra ? vertices[ i ] + 1 : 2 * vertices[ i ]) : vertices[ i ]);

      // if this assertions is thrown vertices were not inserted at first
      alugrid_assert ( vertices_.size() > vtx );

      // get global coordinate and copy it
      std::copy_n( position( vtx ).begin(), dimensionworld, coords[ i ].begin() );
    }

    std::unique_ptr< BoundarySegmentWrapperType > prj( new BoundarySegmentWrapperType( type, coords, boundarySegment ) );

    // consistency check
    for( std::size_t i = 0; i < numVx; ++i )
    {
      CoordType global = (*prj)( coords [ i ] );
      if( (global - coords[ i ]).two_norm() > 1e-6 )
        DUNE_THROW( GridError, "BoundarySegment does not map face vertices to face vertices." );
    }

    FaceType faceId = makeFace( vertices );

    boundaryIds_.insert( makeBndPair( faceId, 1 ) );

    std::sort( faceId.begin(), faceId.end() );
    if( boundaryProjections_.find( faceId ) != boundaryProjections_.end() )
      DUNE_THROW( GridError, "Only one boundary projection can be attached to a face." );

    boundaryProjections_[ faceId ] = prj.release();

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
