#ifndef DUNE_ALU3DGRIDGRID_HH
#define DUNE_ALU3DGRIDGRID_HH

//- System includes
#include <vector>

//- Dune includes
#include <dune/grid/utility/grapedataioformattypes.hh>
#include <dune/grid/common/capabilities.hh>
#include <dune/alugrid/common/interfaces.hh>
#include <dune/common/bigunsignedint.hh>

#include <dune/geometry/referenceelements.hh>

#include <dune/grid/common/grid.hh>
#include <dune/alugrid/common/defaultindexsets.hh>
#include <dune/grid/common/sizecache.hh>
#include <dune/alugrid/common/intersectioniteratorwrapper.hh>
#include <dune/grid/common/datahandleif.hh>

// bnd projection stuff
#include <dune/grid/common/boundaryprojection.hh>
#include <dune/alugrid/common/bndprojection.hh>
#include <dune/alugrid/common/objectfactory.hh>
#include <dune/alugrid/common/backuprestore.hh>
#include <dune/alugrid/common/macrogridview.hh>
#include <dune/alugrid/common/twists.hh>

//- Local includes
#include "alu3dinclude.hh"
#include "topology.hh"
#include "indexsets.hh"
#include "datahandle.hh"

#include <dune/alugrid/3d/communication.hh>
#include <dune/alugrid/3d/gridview.hh>

#include <dune/common/parallel/mpihelper.hh>

#if ALU3DGRID_PARALLEL
#include <dune/common/parallel/mpicollectivecommunication.hh>
#else
#include <dune/common/parallel/collectivecommunication.hh>
#endif

namespace Dune
{
  // Forward declarations
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;
  template<int cd, PartitionIteratorType pitype, class GridImp >
  class ALU3dGridLevelIterator;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointerBase;
  template<int cd, class GridImp >
  class ALU3dGridEntitySeed;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointer;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;
  template<class GridImp>
  class ALU3dGridHierarchicIterator;
  template<class GridImp>
  class ALU3dGridIntersectionIterator;
  template<class GridImp>
  class ALU3dGridLevelIntersectionIterator;
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridLeafIterator;
  template <int mydim, int coorddim, class GridImp>
  class ALU3dGridMakeableEntity;
  template <class GridImp>
  class ALU3dGridFaceGeometryInfo;
  template< int, int, ALU3dGridElementType, class >
  class ALU3dGridGlobalIdSet;
  template< int, int, ALU3dGridElementType, class >
  class ALU3dGridLocalIdSet;
  template< int, int, ALU3dGridElementType, class >
  class ALU3dGridHierarchicIndexSet;
  template <class EntityImp>
  class ALUMemoryProvider;
  template< class >
  class ALU3dGridFactory;
  template <class GridImp, class GeometryImp, int nChild>
  class ALULocalGeometryStorage;



  // Internal Forward Declarations
  // -----------------------------

#if ALU3DGRID_PARALLEL
  template<int dim, int dimworld,  ALU3dGridElementType elType, class Comm = ALUGridMPIComm >
  class ALU3dGrid;
#else // #if ALU3DGRID_PARALLEL
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm = ALUGridNoComm >
  class ALU3dGrid;
#endif // #else // #if ALU3DGRID_PARALLEL


  // Internal Forward Declarations
  // -----------------------------

  template < int dim, int dimw, class Comm >
  struct ALUGridBaseGrid< dim, dimw, cube, Comm >
  {
    typedef ALU3dGrid< dim, dimw, hexa, Comm >  BaseGrid ;
  };

  template < int dim, int dimw, class Comm >
  struct ALUGridBaseGrid< dim, dimw, simplex, Comm >
  {
    typedef ALU3dGrid< dim, dimw, tetra, Comm >  BaseGrid ;
  };



  // ALU3dGridCommunications
  // -----------------------
  struct ALU3dGridCommunicationsBase
  {
    template < class GitterImpl >
    void checkForConformingRefinement( GitterImpl* grid,
                                       const bool conformingRefinement )
    {
      if( grid && conformingRefinement )
      {
        grid->enableConformingClosure();
        grid->disableGhostCells();
      }
    }
  };


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  struct ALU3dGridCommunications;

  template< int dim, int dimworld, ALU3dGridElementType elType >
  struct ALU3dGridCommunications< dim, dimworld, elType, ALUGridNoComm > : public ALU3dGridCommunicationsBase
  {
    using ALU3dGridCommunicationsBase :: checkForConformingRefinement ;

    typedef ALU3dGridLocalIdSet< dim, dimworld, elType, ALUGridNoComm > GlobalIdSet;
    typedef int GlobalId;

    typedef ALU3DSPACE GitterDuneImpl GitterImplType;

    typedef Dune::CollectiveCommunication< No_Comm > CollectiveCommunication;

    explicit ALU3dGridCommunications ( ALUGridNoComm comm ) {}

    int nlinks () const { return 0; }

    GitterImplType *createALUGrid ( const std::string &macroName, ALU3DSPACE ProjectVertex *projection,
                                    const bool conformingRefinement )
    {
      GitterImplType* grid = ( macroName.empty() ) ?
        new GitterImplType( dim ) : new GitterImplType ( dim, macroName.c_str(), projection );
      // check whether conforming refinement should be enabled
      checkForConformingRefinement( grid, conformingRefinement );
      return grid ;
    }

    GitterImplType *createALUGrid ( std::istream& stream, ALU3DSPACE ProjectVertex *projection,
                                    const bool conformingRefinement )
    {
      GitterImplType* grid = new GitterImplType ( dim, stream, projection );
      // check whether conforming refinement should be enabled
      checkForConformingRefinement( grid, conformingRefinement );
      return grid ;
    }

    static ALUGridNoComm defaultComm () { return ALUGridNoComm(); }

    static int getRank ( ALUGridNoComm comm ) { return 0; }

    static typename ALU3DSPACE Gitter::Geometric::BuilderIF &getBuilder ( GitterImplType &grid )
    {
      ALU3DSPACE Gitter::Geometric::BuilderIF* builder =
        dynamic_cast< ALU3DSPACE Gitter::Geometric::BuilderIF* >( &grid.container() );
      if( ! builder )
        DUNE_THROW(InvalidStateException,"dynamic_cast of ALUGrid builder failed");
      return *builder;
    }

    static void completeGrid ( GitterImplType &grid ) {}

    CollectiveCommunication ccobj_;
  };

#if ALU3DGRID_PARALLEL
  template< int dim, int dimworld, ALU3dGridElementType elType >
  struct ALU3dGridCommunications< dim, dimworld, elType, ALUGridMPIComm > : public ALU3dGridCommunicationsBase
  {
    using ALU3dGridCommunicationsBase :: checkForConformingRefinement ;

    typedef ALU3dGridGlobalIdSet< dim, dimworld, elType, ALUGridMPIComm > GlobalIdSet;
    typedef ALUGridId< ALUMacroKey > GlobalId;

    typedef ALU3DSPACE GitterDunePll GitterImplType;

    typedef Dune::CollectiveCommunication< MPI_Comm > CollectiveCommunication;

    explicit ALU3dGridCommunications ( MPI_Comm comm )
    : ccobj_( comm ), mpAccess_( comm )
    {}

    int nlinks () const { return mpAccess_.nlinks(); }

    GitterImplType *createALUGrid ( const std::string &macroName, ALU3DSPACE ProjectVertex *projection,
                                    const bool conformingRefinement )
    {
      GitterImplType* grid = new GitterImplType( dim, macroName.c_str(), mpAccess_, projection );
      // check whether conforming refinement should be enabled
      checkForConformingRefinement( grid, conformingRefinement );
      return grid;
    }

    GitterImplType *createALUGrid ( std::istream& stream, ALU3DSPACE ProjectVertex *projection,
                                    const bool conformingRefinement )
    {
      GitterImplType* grid = new GitterImplType ( dim, stream, mpAccess_, projection );
      // check whether conforming refinement should be enabled
      checkForConformingRefinement( grid, conformingRefinement );
      return grid ;
    }

    static MPI_Comm defaultComm () { return MPI_COMM_WORLD; }

    static int getRank ( MPI_Comm comm )
    {
      int rank = 0;
      MPI_Comm_rank( comm, &rank );
      return rank;
    }

    static typename ALU3DSPACE Gitter::Geometric::BuilderIF &getBuilder ( GitterImplType &grid )
    {
      ALU3DSPACE Gitter::Geometric::BuilderIF* builder =
        dynamic_cast< ALU3DSPACE Gitter::Geometric::BuilderIF* >( &grid.containerPll() );
      if( ! builder )
        DUNE_THROW(InvalidStateException,"dynamic_cast of ALUGrid builder failed");
      return *builder;
    }

    static void completeGrid ( GitterImplType &grid )
    {
      // setup communication patterns
      grid.notifyMacroGridChanges();
      // rebuild ghost cells
      grid.rebuildGhostCells();
    }

    CollectiveCommunication ccobj_;
    ALU3DSPACE MpAccessMPI mpAccess_;
  };
#endif // #if ALU3DGRID_PARALLEL



  // ALU3dGridTwist
  // --------------

  template< int dim, ALU3dGridElementType elType, int codim >
  struct ALU3dGridTwists;

  template<int dim>
  struct ALU3dGridTwists< dim, tetra, 0 >
  {
    typedef TrivialTwists< GenericGeometry::SimplexTopology< dim >::type::id, dim > Type;
  };

  template<int dim>
  struct ALU3dGridTwists< dim, hexa, 0 >
  {
    typedef TrivialTwists< GenericGeometry::CubeTopology< dim >::type::id, dim > Type;
  };

  template< int dim, ALU3dGridElementType elType >
  struct ALU3dGridTwists< dim, elType, 1 >
  {
    typedef ALUTwists< dim == 2 ? 2 : ElementTopologyMapping< elType >::numVerticesPerFace, dim-1 > Type;
  };

  template< ALU3dGridElementType elType >
  struct ALU3dGridTwists< 3, elType, 2 >
  {
    typedef ALUTwists< 2, 1 > Type;
  };

  template< ALU3dGridElementType elType >
  struct ALU3dGridTwists< 2, elType, 2 >
  {
    typedef TrivialTwists< 0u, 0 > Type;
  };

  template< int dim, ALU3dGridElementType elType >
  struct ALU3dGridTwists< dim, elType, 3 >
  {
    typedef TrivialTwists< 0u, 0 > Type;
  };



  // ALU3dGridFamily
  // ---------------

  template< int dimG, int dimW, ALU3dGridElementType elType, class Comm >
  struct ALU3dGridFamily
  {
    static const int dim = dimG;
    static const int dimworld = dimW;

    typedef ALU3dGrid< dim, dimworld, elType, Comm > GridImp;
    typedef ALU3dGridFamily< dim, dimworld, elType, Comm > GridFamily;

    //! Type of the local id set
    typedef ALU3dGridLocalIdSet< dim, dimworld, elType, Comm > LocalIdSetImp;

    //! Type of the global id set
    typedef typename ALU3dGridCommunications< dim, dimworld, elType, Comm >::GlobalIdSet GlobalIdSetImp;

    //! type of ALU3dGrids global id
    typedef typename ALU3dGridCommunications< dim, dimworld, elType, Comm >::GlobalId GlobalIdType;

    //! type of ALU3dGrids local id
    typedef int LocalIdType;

    struct Traits
    {
      //! type of ALU3dGrids local id
      typedef typename GridFamily::LocalIdType LocalIdType;

      //! type of ALU3dGrids global id
      typedef typename GridFamily::GlobalIdType GlobalIdType;

      typedef typename GridFamily::GridImp Grid;

      typedef Dune::Intersection< const Grid, LeafIntersectionWrapper< const Grid > > LeafIntersection;
      typedef Dune::Intersection< const Grid, LevelIntersectionWrapper< const Grid > > LevelIntersection;

      typedef Dune::IntersectionIterator< const Grid, LeafIntersectionIteratorWrapper< const Grid >, LeafIntersectionWrapper< const Grid > > IntersectionIterator;

      typedef Dune::IntersectionIterator< const Grid, LeafIntersectionIteratorWrapper< const Grid >, LeafIntersectionWrapper< const Grid > > LeafIntersectionIterator;
      typedef Dune::IntersectionIterator< const Grid, LevelIntersectionIteratorWrapper< const Grid >, LevelIntersectionWrapper< const Grid > > LevelIntersectionIterator;

      typedef Dune::EntityIterator< 0, const Grid, ALU3dGridHierarchicIterator< const Grid > > HierarchicIterator;

      typedef DuneBoundaryProjection< dimworld > DuneBoundaryProjectionType;
      typedef std::vector< const DuneBoundaryProjectionType * > DuneBoundaryProjectionVector;

      template< int cd >
      struct Codim
      {
        typedef typename ALU3dGridTwists< dim, elType, cd >::Type Twists;
        typedef typename Twists::Twist Twist;

        // IMPORTANT: Codim<codim>::Geometry == Geometry<dim-codim,dimw>
        typedef ALU3dGridGeometry< dim-cd, dimworld, const Grid > GeometryImpl;
        typedef ALU3dGridGeometry< dim-cd, dim, const Grid > LocalGeometryImpl;
        typedef Dune::Geometry< dim-cd, dimworld, const Grid, ALU3dGridGeometry > Geometry;
        typedef Dune::Geometry< dim-cd, dim, const Grid, ALU3dGridGeometry > LocalGeometry;

        typedef ALU3dGridEntity< cd, dim, const Grid > EntityImp;
        typedef Dune::Entity< cd, dim, const Grid, ALU3dGridEntity > Entity;

        // minimal information to generate entities
        typedef ALU3dGridEntitySeed< cd , const Grid> EntitySeed ;

#if DUNE_VERSION_NEWER(DUNE_GRID,2,5)
        typedef EntityImp EntityPointerImpl;
        typedef Entity    EntityPointer;
#else
        typedef ALU3dGridEntityPointer< cd, const Grid > EntityPointerImpl;
        typedef Dune::EntityPointer< const Grid, EntityPointerImpl > EntityPointer;
#endif

        template< PartitionIteratorType pitype >
        struct Partition
        {
          typedef Dune::EntityIterator< cd, const Grid, ALU3dGridLevelIterator< cd, pitype, const Grid > > LevelIterator;
          typedef Dune::EntityIterator< cd, const Grid, ALU3dGridLeafIterator< cd, pitype, const Grid > > LeafIterator;
        }; // struct Partition

        typedef typename Partition< All_Partition >::LevelIterator LevelIterator;
        typedef typename Partition< All_Partition >::LeafIterator LeafIterator;
      }; // struct Codim

      template< PartitionIteratorType pitype >
      struct Partition
      {
        typedef Dune::GridView< ALU3dLevelGridViewTraits< const Grid, pitype > > LevelGridView;
        typedef Dune::GridView< ALU3dLeafGridViewTraits< const Grid, pitype > > LeafGridView;
        typedef Dune::MacroGridView<const Grid, pitype> MacroGridView;
      }; // struct Partition
      typedef typename Partition< All_Partition > :: MacroGridView MacroGridView;

      //! Type of the level index set
      typedef DefaultIndexSet< GridImp, typename Codim< 0 > :: LevelIterator > LevelIndexSetImp;

      //! Type of the leaf index set
      typedef DefaultIndexSet< GridImp, typename Codim< 0 > :: LeafIterator > LeafIndexSetImp;

      typedef IndexSet< Grid, LevelIndexSetImp > LevelIndexSet;
      typedef IndexSet< Grid, LeafIndexSetImp > LeafIndexSet;
      typedef IdSet< Grid, LocalIdSetImp, LocalIdType > LocalIdSet;
      typedef IdSet< Grid, GlobalIdSetImp, GlobalIdType > GlobalIdSet;

      //! Type of the communication class
      typedef typename ALU3dGridCommunications< dim, dimworld, elType, Comm >::CollectiveCommunication CollectiveCommunication;
    }; // struct Traits

    //! Type of the level index set implementation
    typedef typename Traits :: LevelIndexSetImp  LevelIndexSetImp;

    //! Type of the leaf index set implementation
    typedef typename Traits :: LeafIndexSetImp   LeafIndexSetImp;

  }; // struct ALU3dGridFamily



  //**********************************************************************
  //
  // --ALU3dGrid
  // --Grid
  //
  //**********************************************************************

  /**
     \brief [<em> provides \ref Dune::Grid </em>]
     \brief 3D grid with support for hexahedrons and tetrahedrons.
     The ALU3dGrid implements the Dune GridInterface for 3d tetrahedral and
     hexahedral meshes. This grid can be locally adapted and used in parallel
     computations using dynamic load balancing.

     @note
     Adaptive parallel grid supporting dynamic load balancing, written
     mainly by Bernard Schupp. This grid supports hexahedrons and tetrahedrons.

     (see ALUGrid homepage: http://www.mathematik.uni-freiburg.de/IAM/Research/alugrid/)

     Two tools are available for partitioning :
     \li Metis ( version 4.0 and higher, see http://glaros.dtc.umn.edu/gkhome/views/metis/metis/ )
     \li ParMETIS ( http://glaros.dtc.umn.edu/gkhome/metis/parmetis/overview )

     For installation instructions see http://www.dune-project.org/external_libraries/install_alugrid.html .
     @author Robert Kloefkorn
  */
  template< int dim, int dimworld,  ALU3dGridElementType elType, class Comm >
  class ALU3dGrid
  : public GridDefaultImplementation< dim, dimworld, alu3d_ctype,
                                      ALU3dGridFamily< dim, dimworld, elType, Comm > >,
    public HasObjectStream,
    public HasHierarchicIndexSet
  {
    typedef ALU3dGrid< dim, dimworld, elType, Comm > ThisType;
    typedef GridDefaultImplementation< dim, dimworld, alu3d_ctype, ALU3dGridFamily< dim, dimworld, elType, Comm > > BaseType;

    // for compatibility: MyType := ThisType
    typedef ThisType MyType;

    // friend declarations
    friend class ALU3dGridEntity< 0, dim, const ThisType>;
    friend class ALU3dGridEntity< 1, dim, const ThisType>;
    friend class ALU3dGridEntity< 2, dim, const ThisType>;
    friend class ALU3dGridEntity< dim, dim, const ThisType>;

    friend class ALU3dGridIntersectionIterator< ThisType >;

    friend class ALU3dGridEntityPointerBase< 0, const ThisType >;
    friend class ALU3dGridEntityPointerBase< 1, const ThisType >;
    friend class ALU3dGridEntityPointerBase< 2, const ThisType >;
    friend class ALU3dGridEntityPointerBase< dim, const ThisType >;

    friend class ALU3dGridEntityPointer< 0, const ThisType >;
    friend class ALU3dGridEntityPointer< 1, const ThisType >;
    friend class ALU3dGridEntityPointer< 2, const ThisType >;
    friend class ALU3dGridEntityPointer< dim, const ThisType >;

    friend class ALU3dGridIntersectionIterator< const ThisType >;
    friend class ALU3dGridHierarchicIterator< const ThisType >;

    friend class ALU3dGridHierarchicIndexSet< dim, dimworld, elType, Comm >;
    friend class ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm >;
    friend class ALU3dGridLocalIdSet< dim, dimworld, elType, Comm >;

    friend class Conversion< ThisType, HasObjectStream >;
    friend class Conversion< const ThisType, HasObjectStream >;

    friend class Conversion< ThisType, HasHierarchicIndexSet >;
    friend class Conversion< const ThisType, HasHierarchicIndexSet >;

    // new intersection iterator is a wrapper which get itersectioniteratoimp as pointers
  public:
    typedef ALU3dGridIntersectionIterator<const ThisType>
      IntersectionIteratorImp;
    typedef ALU3dGridIntersectionIterator<const ThisType>
      LeafIntersectionIteratorImp;
    typedef ALU3dGridLevelIntersectionIterator<const ThisType>
      LevelIntersectionIteratorImp;

    friend class IntersectionIteratorWrapper < const ThisType, LeafIntersectionIteratorImp > ;
    friend class IntersectionIteratorWrapper < const ThisType, LevelIntersectionIteratorImp > ;
    friend class LeafIntersectionIteratorWrapper < const ThisType > ;
    friend class LevelIntersectionIteratorWrapper< const ThisType > ;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    enum { refineStepsForHalf = 1 };

    static const ALU3dGridElementType elementType = elType;

    typedef typename ALU3DSPACE GatherScatterType::ObjectStreamType ObjectStreamType;
    typedef ObjectStreamType  InStreamType ;
    typedef ObjectStreamType  OutStreamType ;

    typedef ALU3dGridFamily< dim, dimworld, elType, Comm > GridFamily;
    typedef typename GridFamily::Traits Traits;

    static const int dimension =      BaseType::dimension;
    static const int dimensionworld = BaseType::dimensionworld;

    template< int codim >
    struct Codim
      : public BaseType::template Codim< codim >
    {
      typedef typename Traits::template Codim< codim >::Twists Twists;
      typedef typename Twists::Twist Twist;
    };

  protected:
    typedef MakeableInterfaceObject< typename Traits::template Codim< 0 >::Geometry > GeometryObject;
    friend class ALULocalGeometryStorage< const ThisType, GeometryObject, 8 >;

  public:
    /** \brief Types for GridView */
    template <PartitionIteratorType pitype>
    struct Partition
    {
      typedef typename GridFamily::Traits::template Partition<pitype>::LevelGridView
         LevelGridView;
      typedef typename GridFamily::Traits::template Partition<pitype>::LeafGridView
         LeafGridView;
      typedef typename GridFamily::Traits::template Partition<pitype>::MacroGridView
         MacroGridView;
    };
    /** \brief View types for All_Partition */
    typedef typename Partition< All_Partition > :: LevelGridView LevelGridView;
    typedef typename Partition< All_Partition > :: LeafGridView LeafGridView;
    typedef typename Partition< All_Partition > :: MacroGridView MacroGridView;

    //! Type of the hierarchic index set
    typedef ALU3dGridHierarchicIndexSet< dim, dimworld, elType, Comm > HierarchicIndexSet;

    //! Type of the level index set, needed by data handle
    typedef typename GridFamily::LevelIndexSetImp LevelIndexSetImp;
    //! Type of the leaf index set, needed by data handle
    typedef typename GridFamily::LeafIndexSetImp LeafIndexSetImp;

    //! reference element type
    typedef ReferenceElement< alu3d_ctype, dimension > ReferenceElementType;

    //! \brief boundary projection type
    typedef typename Traits::DuneBoundaryProjectionType DuneBoundaryProjectionType;
    //! \brief boundary projection type
    typedef typename Traits::DuneBoundaryProjectionVector DuneBoundaryProjectionVector;

    //! type of ALUGrid Vertex Projection Interface
    typedef ALU3DSPACE ProjectVertex ALUGridVertexProjectionType;

    //! type of collective communication object
    typedef typename Traits::CollectiveCommunication CollectiveCommunication;

    typedef ALULeafCommunication< dim, dimworld, elType, Comm > LeafCommunication;
    typedef ALULevelCommunication< dim, dimworld, elType, Comm > LevelCommunication;

  public:
    typedef MakeableInterfaceObject<typename Traits::template Codim<0>::Entity> EntityObject;
    typedef MakeableInterfaceObject<typename Traits::template Codim<1>::Entity> FaceObject;
    typedef MakeableInterfaceObject<typename Traits::template Codim<2>::Entity> EdgeObject;
    typedef MakeableInterfaceObject<typename Traits::template Codim<3>::Entity> VertexObject;

    typedef ALUGridObjectFactory< ThisType >  GridObjectFactoryType;

  protected:
    friend class ALUGridBoundaryProjection< ThisType, alu3d_ctype >;
    // type of ALUGrid boundary projection wrapper
    typedef ALUGridBoundaryProjection< ThisType, alu3d_ctype > ALUGridBoundaryProjectionType;

    //! Type of the local id set
    typedef typename GridFamily::LocalIdSetImp LocalIdSetImp;

    typedef typename GridFamily::GlobalIdSetImp GlobalIdSetImp;

  public:
    //! Type of the global id set
    typedef typename Traits::GlobalIdSet GlobalIdSet;

    //! Type of the local id set
    typedef typename Traits::LocalIdSet LocalIdSet;

  protected:
    typedef ALU3dGridLeafIterator< 0, All_Partition, const ThisType > LeafIteratorImp;
    typedef typename Traits::template Codim< 0 >::LeafIterator LeafIteratorType;
    typedef typename Traits::template Codim< 0 >::LeafIterator LeafIterator;

    typedef ALU3dGridHierarchicIterator< const ThisType > HierarchicIteratorImp;

    typedef typename ALU3dImplTraits< elType, Comm >::GitterImplType GitterImplType;

    //! max number of levels
    enum {
      //! \brief maximal number of levels is 32
      MAXL = 32 };

    //! element chunk for refinement
    enum {
      //! \brief normal default number of new elements for new adapt method
      newElementsChunk_ = 128 };

    //! upper estimate on number of elements that could be created when a new element is created
    enum {
      /** \brief if one element is refined then it
          causes apporximately not more than
          this number of new elements  */
      refineEstimate_ = 8 };

  public:
    typedef Comm MPICommunicatorType;

    typedef ALU3dGridCommunications< dim, dimworld, elType, Comm > Communications;

  protected:
    typedef ALU3dGridVertexList< Comm > VertexListType;
    typedef ALU3dGridLeafVertexList< Comm > LeafVertexListType;

  public:
    //! Constructor which reads an ALU3dGrid Macro Triang file
    //! or given GridFile
    ALU3dGrid ( const std::string &macroTriangFilename,
                const MPICommunicatorType mpiComm,
                const DuneBoundaryProjectionType *bndPrj,
                const DuneBoundaryProjectionVector *bndVec,
                const ALUGridRefinementType refinementType );

    //! \brief Desctructor
    virtual ~ALU3dGrid();

    //! \brief for grid identification
    static inline std::string name ();

    /** \brief  Return maximum level defined in this grid. Levels are numbered
        maxLevel with 0 the coarsest level.
      */
    int maxLevel() const;

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
    lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int cd>
    typename Traits::template Codim<cd>::
    template Partition<All_Partition>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd>
    typename Traits::template Codim<cd>::
    template Partition<All_Partition>::LevelIterator
    lend (int level) const;

    typedef LeafIntersectionIteratorWrapper  < const ThisType > LefInterItWrapperType;
    typedef LevelIntersectionIteratorWrapper < const ThisType > LvlInterItWrapperType;

    typename Traits::LeafIntersectionIterator
    ileafbegin( const typename Traits::template Codim< 0 >::Entity& entity ) const
    {
      return LefInterItWrapperType( factory(),
                                    getRealImplementation( entity ),
                                    entity.level(), false );
    }

    typename Traits::LeafIntersectionIterator
    ileafend( const typename Traits::template Codim< 0 >::Entity& entity ) const
    {
      return LefInterItWrapperType( factory(),
                                    getRealImplementation( entity ),
                                    entity.level(), true );
    }

    typename Traits::LevelIntersectionIterator
    ilevelbegin( const typename Traits::template Codim< 0 >::Entity& entity ) const
    {
      return LvlInterItWrapperType( factory(),
                                    getRealImplementation( entity ),
                                    entity.level(), false );
    }

    typename Traits::LevelIntersectionIterator
    ilevelend( const typename Traits::template Codim< 0 >::Entity& entity ) const
    {
      return LvlInterItWrapperType( factory(),
                                    getRealImplementation( entity ),
                                    entity.level(), true );
    }

  private:
    //! General definiton for a leaf iterator
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafbegin(int level) const;

    //! General definition for an end iterator on leaf level
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafend(int level) const;

    //! General definiton for a leaf iterator
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafbegin(int level) const;

    //! General definition for an end iterator on leaf level
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafend(int level) const;

    //! Iterator to first entity of codim 0 on leaf level (All_Partition)
    LeafIteratorType leafbegin (int level) const;

    //! one past the end on this leaf level (codim 0 and All_Partition)
    LeafIteratorType leafend (int level) const;

    //! Iterator to first entity of codim 0 on leaf level (All_Partition)
    LeafIteratorType leafbegin () const;

    //! one past the end on this leaf level (codim 0 and All_Partition)
    LeafIteratorType leafend () const;

  public:
    //! General definiton for a leaf iterator
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafbegin() const;

    //! General definition for an end iterator on leaf level
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafend() const;

    //! General definiton for a leaf iterator
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafbegin() const;

    //! General definition for an end iterator on leaf level
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafend() const;

  private:
    //! General definiton for a leaf iterator
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    createLeafIteratorBegin (int level) const;

    //! General definition for an end iterator on leaf level
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    createLeafIteratorEnd(int level) const;

  public:
    //! number of grid entities per level and codim
    int size (int level, int cd) const;

    //! number of leaf entities per codim in this process
    int size (int codim) const;

    //! number of entities per level and geometry type in this process
    int size (int level, GeometryType type) const;

    //! number of boundary segments
    size_t numBoundarySegments() const;

    //! number of leaf entities per geometry type in this process
    int size (GeometryType type) const;

    //! number of grid entities on all levels for given codim
    int global_size (int cd) const ;

    // (no interface method) number of grid entities in the entire grid for given codim
    int hierSetSize (int cd) const;

    //! get global id set of grid
    const GlobalIdSet &globalIdSet () const
    {
      if( !globalIdSet_ )
        globalIdSet_ = new GlobalIdSetImp( *this );
      return *globalIdSet_;
    }

    //! View for te macro grid with some alu specific methods
    template<PartitionIteratorType pitype>
    typename Partition<pitype>::MacroGridView macroView() const
    {
      typedef typename Traits::template Partition<pitype>::MacroGridView View;
      return View(*this);
    }

    //! View for te macro grid with some alu specific methods (All_Partition)
    MacroGridView macroView() const
    {
      typedef MacroGridView View;
      return View(*this);
    }

    //! get global id set of grid
    const LocalIdSet & localIdSet () const { return localIdSet_; }

    //! get leaf index set of the grid
    const typename Traits :: LeafIndexSet & leafIndexSet () const;

    //! get level index set of the grid
    const typename Traits :: LevelIndexSet & levelIndexSet (int level) const
    {
      return *(levelIndexVec_[ level ] = getLevelIndexSet( level ).first);
    }

    template< int cd >
    typename Codim< cd >::Twists twists ( GeometryType type ) const
    {
      assert( type.dim() == dimension - cd );
      assert( elType == tetra ? type.isSimplex() : type.isCube() );
      return typename Traits::template Codim< cd >::Twists();
    }

  protected:
    typedef ALU3DSPACE GatherScatter GatherScatterType;

    /** \brief Calculates load of each process and repartition the grid if neccessary.
        For parameters of the load balancing process see the README file
        of the ALUGrid package.
       \param data the data handler class that must implement three methods:
          \code
          // calls data inline on macro element. From there the data of
          // all children can be written to the message buffer.
          // MessageBufferImp implements the MessageBufferIF interface.
          template<class MessageBufferImp>
          void inlineData ( MessageBufferImp& buff, Dune::Entity<0> & e);

          // calls data xtract on macro element. From there the data of
          // all children can be restored from the message buffer.
          // numChildren is the number of all children underneath the
          // macro element e.
          // MessageBufferImp implements the MessageBufferIF interface.
          template<class MessageBufferImp>
          void xtractData ( MessageBufferImp& buff, Dune::Entity<0> & e, size_t numChildren );

          // This method is called at the end of the load balancing process
          // before adaptation markers are removed. Here the user can apply
          // a data compression or other features. This method can be
          // empty if nothing should be done.
          void compress ();
          \endcode

         \return true if the grid has changed
    */
    bool loadBalance ( GatherScatterType* lbData );

  public:
    /** \brief Calculates load of each process and repartition by using ALUGrid's default partitioning method.
               The specific load balancing algorithm is selected from a file alugrid.cfg.
        \return true if grid has changed
    */
    bool loadBalance ()
    {
      return loadBalance( (GatherScatterType* ) 0 );
    }

    /** \brief Calculates load of each process and repartition by using ALUGrid's default partitioning method.
               The specific load balancing algorithm is selected from a file alugrid.cfg.
        \param  optional dataHandleIF data handle that implements the Dune::CommDataHandleIF interface to include
                user data during load balancing
        \return true if grid has changed
    */
    template< class DataHandleImpl, class Data >
    bool loadBalance ( CommDataHandleIF< DataHandleImpl, Data > &dataHandleIF )
    {
      typedef ALU3DSPACE GatherScatterLoadBalanceDataHandle
            < ThisType, GatherScatterType, DataHandleImpl, Data > DataHandleType;
      DataHandleType dataHandle( *this, dataHandleIF );

      // call the above loadBalance method with general GatherScatterType
      return loadBalance( &dataHandle );
    }

    /** \brief Calculates load of each process and repartition by using ALUGrid's default partitioning method,
               the partitioning can be optimized by providing weights for each element on the macro grid.
               The specific load balancing algorithm is selected from a file alugrid.cfg.
        \param  weights class with double operator()(const Entity<0>&) returning a weight for each element
                which the includes in its internal loadbalancing process - for ALUGrid these are all macro elements.
        \param  dataHandleIF data handle that implements the Dune::CommDataHandleIF interface to include
                user data during load balancing
        \return true if grid has changed
    */
    template< class LBWeights, class DataHandleImpl, class Data >
    bool loadBalance ( LBWeights &weights,
                       CommDataHandleIF< DataHandleImpl, Data > &dataHandleIF )
    {
      typedef ALU3DSPACE GatherScatterLoadBalanceDataHandle
            < ThisType, LBWeights, DataHandleImpl, Data > DataHandleType;
      DataHandleType dataHandle( *this, dataHandleIF, weights );

      // call the above loadBalance method with general GatherScatterType
      return loadBalance( &dataHandle );
    }

    /** \brief Distribute the grid based on a user defined partitioning.
        \param  destinations class with int operator()(const Entity<0>&) returning the new owner process
                of this element. A destination has to be provided for all elements in the grid hierarchy
                but depending on the grid implementation it is possibly called on a subset only.
                The elements for which the method is called will be moved to the new processor together
                with all children. ALUGrid requires destinations for all macro elements.
        \return true if grid has changed
    */
    template< class LBDestinations >
    bool repartition ( LBDestinations &destinations )
    {
      typedef ALU3DSPACE GatherScatterLoadBalance< ThisType, LBDestinations > LoadBalanceHandleType ;
      LoadBalanceHandleType loadBalanceHandle( *this, destinations, true );
      return loadBalance( &loadBalanceHandle );
    }

    /** \brief Distribute the grid based on a user defined partitioning.
        \param  destinations class with int operator()(const Entity<0>&) returning the new owner process
                of this element. A destination has to be provided for all elements in the grid hierarchy
                but depending on the grid implementation it is possibly called on a subset only.
                The elements for which the method is called will be moved to the new processor together
                with all children. ALUGrid requires destinations for all macro elements.
        \param  dataHandleIF data handle that implements the Dune::CommDataHandleIF interface to include
                user data during load balancing
        \return true if grid has changed
    */
    template< class LBDestinations, class DataHandleImpl, class Data >
    bool repartition ( LBDestinations &destinations,
                       CommDataHandleIF< DataHandleImpl, Data > &dataHandleIF )
    {
      typedef ALU3DSPACE GatherScatterLoadBalanceDataHandle< ThisType, LBDestinations, DataHandleImpl, Data > DataHandleType;
      DataHandleType dataHandle( *this, dataHandleIF, destinations, true );

      // call the above loadBalance method with general GatherScatterType
      return loadBalance( &dataHandle );
    }


    /** \brief ghostSize is one for codim 0 and zero otherwise for this grid  */
    int ghostSize (int level, int codim) const;

    /** \brief overlapSize is zero for this grid  */
    int overlapSize (int level, int codim) const { return 0; }

    /** \brief ghostSize is one for codim 0 and zero otherwise for this grid  */
    int ghostSize (int codim) const;

    /** \brief overlapSize is zero for this grid  */
    int overlapSize (int codim) const { return 0; }

    /** \brief @copydoc Dune::Grid::communicate */
    template< class DataHandle, class Data >
    LevelCommunication communicate ( CommDataHandleIF< DataHandle, Data > &data,
                                     InterfaceType iftype,
                                     CommunicationDirection dir,
                                     int level ) const
    {
      return LevelCommunication( *this, data, iftype, dir, level );
    }

    /** \brief Communicate information on distributed entities on the leaf grid.
       Template parameter is a model of Dune::CommDataHandleIF.
     */
    template< class DataHandle, class Data >
    LeafCommunication communicate ( CommDataHandleIF< DataHandle, Data > &data,
                                    InterfaceType iftype,
                                    CommunicationDirection dir ) const
    {
      return LeafCommunication( *this, data, iftype, dir );
    }

  protected:
    // load balance and compress memory if possible
    void finalizeGridCreation();

    //! clear all entity new markers
    void clearIsNewMarkers( );

  public:
    /** \brief @copydoc Dune::Grid::comm() */
    const CollectiveCommunication &comm () const { return communications().ccobj_; }

    //! returns if a least one entity was marked for coarsening
    bool preAdapt ( );

    //! clear all entity new markers if lockPostAdapt_ is set
    void postAdapt ( );

    /** \brief  @copydoc Dune::Grid::adapt() */
    bool adapt ();

    /** \brief  @copydoc Dune::Grid::adapt()
        \param handle handler for restriction and prolongation operations
        which is a Model of the AdaptDataHandleInterface class.
    */
    template< class GridImp, class DataHandle >
    bool adapt ( AdaptDataHandleInterface< GridImp, DataHandle > &handle );

    //! uses the interface, mark on entity and refineLocal
    void globalRefine ( int refCount );

    template< class GridImp, class DataHandle >
    void globalRefine ( int refCount, AdaptDataHandleInterface< GridImp, DataHandle > &handle );

    //**********************************************************
    // End of Interface Methods
    //**********************************************************

    /** \brief write macro grid in ALUGrid macro format to path/filename.rank */
    bool writeMacroGrid( const std::string path, const std::string filename,
                         const ALU3DSPACE MacroFileHeader::Format format = ALU3DSPACE MacroFileHeader::defaultFormat ) const ;

    /** \brief backup to ostream */
    void backup( std::ostream&, const ALU3DSPACE MacroFileHeader::Format format ) const ;

    /** \brief restore from istream */
    void restore( std::istream& ) ;

    // (no interface method) get hierarchic index set of the grid
    const HierarchicIndexSet & hierarchicIndexSet () const { return hIndexSet_; }

    // no interface method, but has to be public
    void updateStatus ();

    //! @copydoc Dune::Grid::mark
    bool mark( int refCount , const typename Traits::template Codim<0>::Entity & e);

    //! @copydoc Dune::Grid::getMark
    int getMark( const typename Traits::template Codim<0>::Entity & e) const;

  public:
    static MPICommunicatorType defaultCommunicator ()
    {
      return Communications::defaultComm();
    }

    using BaseType :: getRealImplementation ;

    template< class IntersectionType >
    static const typename BaseType
      :: template ReturnImplementationType< IntersectionType >
      :: ImplementationType &
    getRealIntersection ( const IntersectionType &intersection )
    {
      return getRealImplementation( intersection );
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes (int codim) const { return geomTypes_[codim]; }

    // return reference to org ALU3dGrid
    // private method, but otherwise we have to friend class all possible
    // types of LevelIterator ==> later
    GitterImplType &myGrid () const;

    virtual GitterImplType *createALUGrid ( const std::string &macroName )
    {
      alugrid_assert ( communications_ );
      return communications_->createALUGrid( macroName, vertexProjection(), conformingRefinement() );
    }

    virtual GitterImplType *createALUGrid ( std::istream& stream )
    {
      alugrid_assert ( communications_ );
      return communications_->createALUGrid( stream, vertexProjection(), conformingRefinement() );
    }

    ALUGridVertexProjectionType* vertexProjection() { return (ALUGridVertexProjectionType *) vertexProjection_; }

    // return appropriate ALUGrid builder
    virtual typename ALU3DSPACE Gitter::Geometric::BuilderIF &getBuilder () const
    {
      return Communications::getBuilder( myGrid() );
    }

    // helper function for factory
    virtual void completeGrid ()
    {
      Communications::completeGrid( myGrid() );
    }

    //! return reference to Dune reference element according to elType
    const ReferenceElementType & referenceElement() const { return referenceElement_; }

    template < class EntitySeed >
    typename Traits :: template Codim< EntitySeed :: codimension > :: EntityPointer
    entityPointer( const EntitySeed& seed ) const
    {
      enum { codim = EntitySeed :: codimension };
      typedef ALU3dGridEntityPointer < codim, const ThisType > ALUPointer ;
      return ALUPointer( factory(), seed ) ;
    }

    // number of links to other processors, for internal use only
    int nlinks () const { return communications().nlinks(); }

    LeafVertexListType & getLeafVertexList() const
    {
      if( !leafVertexList_.up2Date() ) leafVertexList_.setupVxList(*this);
      return leafVertexList_;
    }

    int getLevelOfLeafVertex ( const typename ALU3dImplTraits< elType, Comm >::VertexType &vertex ) const
    {
      alugrid_assert ( leafVertexList_.up2Date() );
      return leafVertexList_.getLevel(vertex);
    }

    VertexListType & getVertexList(int level) const
    {
      alugrid_assert ( level >= 0 );
      alugrid_assert ( level <= maxLevel() );
      VertexListType & vxList = vertexList_[level];
      if(!vxList.up2Date()) vxList.setupVxList(*this,level);
      return vxList;
    }

    ALU3dGridItemListType & getGhostLeafList(int codim) const
    {
      alugrid_assert ( codim >= 1 );
      alugrid_assert ( codim <= 3 );
      return ghostLeafList_[codim-1];
    }

    ALU3dGridItemListType & getGhostLevelList(int codim, int level) const
    {
      alugrid_assert ( codim >= 1 );
      alugrid_assert ( codim <= 3 );

      alugrid_assert ( level >= 0 );
      alugrid_assert ( level <= maxLevel() );
      return ghostLevelList_[codim-1][level];
    }

    ALU3dGridItemListType & getEdgeList(int level) const
    {
      alugrid_assert ( level >= 0 );
      alugrid_assert ( level <= maxLevel() );
      return levelEdgeList_[level];
    }

  protected:
    //! Copy constructor should not be used
    ALU3dGrid( const ThisType & );

    //! assignment operator should not be used
    const ThisType &operator= ( const ThisType & );

    //! reset size and global size, update Level- and LeafIndexSet, if they exist
    void calcExtras( const bool recomputeMacroIndexSet = false );

    //! calculate maxlevel
    void calcMaxLevel();

    //! make grid walkthrough and calc global size
    void recalcGlobalSize();

    //! check whether macro grid format is of our type
    void checkMacroGridFile (const std::string filename);

    //! check whether macro grid has the right element type
    void checkMacroGrid ();

    //! return boudanry projection for given segment Id
    const DuneBoundaryProjectionType* boundaryProjection(const int segmentIndex) const
    {
      if( bndPrj_ )
      {
        return bndPrj_;
      }
      else
      {
        // pointer can be zero (which is emulates the identity mapping then)
        alugrid_assert ( bndVec_ );
        alugrid_assert ( segmentIndex < (int) bndVec_->size() );
        return (*bndVec_)[ segmentIndex ];
      }
    }

    const Communications &communications () const
    {
      alugrid_assert ( communications_ );
      return *communications_;
    }

    // geometry in father storage
    typedef ALULocalGeometryStorage< const ThisType, typename Traits::template Codim< 0 >::LocalGeometryImpl, 8 > GeometryInFatherStorage ;
    // return geometryInFather for non-conforming grids
    const GeometryInFatherStorage& nonConformingGeometryInFatherStorage() const { return nonConformingGeoInFatherStorage_; }
    // initialize geometry types and return correct geometryInFather storage
    const GeometryInFatherStorage& makeGeometries();

  public:
    const GridObjectFactoryType &factory () const { return factory_; }

    std::pair< LevelIndexSetImp *, bool > getLevelIndexSet ( int level ) const
    {
      assert( (level >= 0) && (level < int( levelIndexVec_.size() )) );
      std::pair< LevelIndexSetImp *, bool > indexSet( levelIndexVec_[ level ], bool( levelIndexVec_[ level ] ) );
      if( !indexSet.second )
        indexSet.first = new LevelIndexSetImp( *this, lbegin< 0 >( level ), lend< 0 >( level ), level );
      return indexSet;
    }

    // return true if conforming refinement is enabled
    bool conformingRefinement() const
    {
      return (refinementType_ == conforming) ;
    }

    // return true if ghost cells are available
    bool ghostCellsEnabled () const
    {
      return comm().size() > 1 && myGrid().ghostCellsEnabled();
    }

    //! return current thread number
    static int thread () { return ALUMemoryProvider< int > :: thread(); }

    //! return max number of threads
    static int maxThreads () { return ALUMemoryProvider< int > :: maxThreads(); }
  protected:
    /////////////////////////////////////////////////////////////////
    //
    // Internal variables
    //
    /////////////////////////////////////////////////////////////////

    // the real ALU grid
    mutable GitterImplType *mygrid_;

    // max level of grid
    int maxlevel_;

    // count how much elements where marked
    mutable int coarsenMarked_;
    mutable int refineMarked_;

    // at the moment the number of different geom types is 1
    enum { numberOfGeomTypes = 1 };
    std::vector< std::vector<GeometryType> > geomTypes_;

    // our hierarchic index set
    HierarchicIndexSet hIndexSet_;

    // out global id set
    mutable GlobalIdSetImp *globalIdSet_;

    // out global id set
    LocalIdSetImp localIdSet_;

    // the level index set ( default type )
    mutable std::vector < LevelIndexSetImp * > levelIndexVec_;

    // the leaf index set
    mutable LeafIndexSetImp * leafIndexSet_;

    // the reference element
    const ReferenceElementType& referenceElement_;

    mutable VertexListType vertexList_[MAXL];

    //the ghostleaf list is used in alu3diterators, where we use the internal aluIterators
    // the vertex codim there is 3, so the list has to fulfill that
    mutable ALU3dGridItemListType ghostLeafList_[ 3 ];
    mutable ALU3dGridItemListType ghostLevelList_[ 3 ][MAXL];

    mutable ALU3dGridItemListType levelEdgeList_[MAXL];

    mutable LeafVertexListType leafVertexList_;

    // the type of our size cache
    typedef SizeCache<MyType> SizeCacheType;
    SizeCacheType * sizeCache_;

    GridObjectFactoryType factory_;

    // variable to ensure that postAdapt ist called after adapt
    bool lockPostAdapt_;

    // pointer to Dune boundary projection
    const DuneBoundaryProjectionType* bndPrj_;

    // pointer to Dune boundary projection
    const DuneBoundaryProjectionVector* bndVec_;

    // boundary projection for vertices
    ALUGridBoundaryProjectionType* vertexProjection_ ;

    // pointer to communications object
    Communications *communications_;

    // refinement type (nonconforming or conforming)
    const ALUGridRefinementType refinementType_ ;

    // local geometry storage for geometries in father
    const GeometryInFatherStorage& nonConformingGeoInFatherStorage_ ;
  }; // end class ALU3dGrid


    bool checkMacroGrid ( ALU3dGridElementType elType ,
                          const std::string filename );
    const char* elType2Name( ALU3dGridElementType elType );

  namespace Capabilities
  {

    template< int dim, int dimworld, ALU3dGridElementType elType, class Comm, int cdim >
    struct hasEntity< Dune::ALU3dGrid< dim, dimworld, elType, Comm >, cdim >
    {
      static const bool v = true;
    };

    template< int dim, int dimworld,  ALU3dGridElementType elType, class Comm >
    struct isParallel< ALU3dGrid< dim, dimworld, elType, Comm > >
    {
      static const bool v = true;
    };

    template< int dim, int dimworld,  ALU3dGridElementType elType, class Comm >
    struct isLevelwiseConforming< ALU3dGrid< dim, dimworld, elType, Comm > >
    {
      static const bool v = true;
    };

    template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
    struct hasBackupRestoreFacilities< ALU3dGrid< dim, dimworld, elType, Comm > >
    {
      static const bool v = true;
    };

  } // end namespace Capabilities

} // end namespace Dune

#include "grid_inline.hh"
#if COMPILE_ALUGRID_INLINE
  #include "grid_imp.cc"
#endif
#endif
