#define DISABLE_DEPRECATED_METHOD_CHECK 1
//#define ALUGRID_CHECK_GLOBALIDSET_UNIQUENESS

#include <config.h>


#ifndef NDEBUG
#ifndef DUNE_DEVEL_MODE
#define DUNE_DEVEL_MODE
#endif
#define DUNE_INTERFACECHECK
#endif

// #define NO_2D
// #define NO_3D

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/version.hh>
#include <dune/common/tupleutility.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/geometry/referenceelements.hh>

#include <dune/grid/io/file/dgfparser/dgfwriter.hh>
#include <dune/alugrid/dgf.hh>

#include <dune/grid/test/gridcheck.hh>
#include <dune/grid/test/checkgeometryinfather.hh>
#include <dune/grid/test/checkiterators.hh>
#include <dune/grid/test/checkcommunicate.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

// use overloaded test because intersection test fails for manifold
// intersections, there is some discussion needed.
#include <dune/alugrid/test/checkintersectionit.hh>

#include <dune/grid/test/checkgridfactory.hh>

#if HAVE_DUNE_GRID_TESTGRIDS
#include <doc/grids/gridfactory/testgrids.hh>
#endif

#if ALU3DGRID_PARALLEL && HAVE_MPI
#define USE_PARALLEL_TEST 1
#endif

template< int dim, int dimworld >
struct EnableLevelIntersectionIteratorCheck< Dune::ALUGrid< dim, dimworld, Dune::simplex, Dune::conforming > >
{
  static const bool v = false;
};

template <class Grid>
void checkCapabilities(const Grid& grid)
{
   static_assert ( Dune::Capabilities::hasSingleGeometryType< Grid > :: v == true,
                  "hasSingleGeometryType is not set correctly");
   static_assert ( Dune::Capabilities::isLevelwiseConforming< Grid > :: v == false,
                  "isLevelwiseConforming is not set correctly");
   static_assert ( Dune::Capabilities::isLeafwiseConforming< Grid > :: v == (Grid::refinementType == Dune::conforming),
                  "isLevelwiseConforming is not set correctly");
   static const bool hasEntity = Dune::Capabilities::hasEntity<Grid, 1> :: v == true;
   static_assert ( hasEntity, "hasEntity is not set correctly");
   static_assert ( Dune::Capabilities::hasBackupRestoreFacilities< Grid > :: v == true,
                   "hasBackupRestoreFacilities is not set correctly");

   static const bool reallyCanCommunicate =
#if ALU3DGRID_PARALLEL
    true ;
#else
    false ;
#endif
   static const bool canCommunicate = Dune::Capabilities::canCommunicate< Grid, 1 > :: v
     == reallyCanCommunicate;
   static_assert ( canCommunicate, "canCommunicate is not set correctly");

   // only print for main rank
   if( grid.comm().rank() == 0 )
   {
     std::cout << "Sizes of interface implementation classes: " << std::endl;
     std::cout << "  Entity< " << 0 << " > = " << sizeof(typename Grid::template Codim<0>::Entity) << std::endl;
     std::cout << "  Entity< " << 1 << " > = " << sizeof(typename Grid::template Codim<1>::Entity) << std::endl;
     std::cout << "  Entity< " << 2 << " > = " << sizeof(typename Grid::template Codim<2>::Entity) << std::endl;
     std::cout << "  Entity< " << Grid::dimension << " > = " << sizeof(typename Grid::template Codim<Grid::dimension>::Entity) << std::endl;
     std::cout << "  LeafIntersection   = " << sizeof(typename Grid::Traits::LeafIntersection) << std::endl;
     std::cout << "  LevelIntersection  = " << sizeof(typename Grid::Traits::LevelIntersection) << std::endl;
     std::cout << "  GlobalId = " << sizeof(typename Grid::GlobalIdSet::IdType) << std::endl;
     std::cout << "  LocalId  = " << sizeof(typename Grid::LocalIdSet::IdType) << std::endl;
     std::cout << std::endl;
   }
}

template< class Grid >
struct SimplePartition
{
  typedef typename Grid :: Traits :: template Codim<0> :: Entity Element;
  typedef typename Grid :: Traits :: HierarchicIterator HierarchicIterator;
  typedef typename Grid::MacroGridView MacroView;

  SimplePartition( const Grid &grid )
    : grid_( grid ), macroView_( grid.macroView() )
  {}

  /** this method is called before invoking the re-partition
      method on the grid, to check if the user defined
      partitioning needs to be readjusted */
  bool repartition () { return true; }

  /** This method is called for each macro element to determine the new process number */
  int operator()( const Element &element ) const
  {
    const int id = macroView_.macroId( element );
    // return rank destination number
    return 0;///id % grid_.comm().size();
  }

  // recompute imported ranks
  bool importRanks( std::set<int> &ranks ) const { return false; }

protected:
  const Grid& grid_;
  const MacroView macroView_;
};


template <class GridType>
void makeNonConfGrid(GridType &grid,int level,int adapt)
{
  int myrank = grid.comm().rank();

  // test user specified load balance
  /*
  {
    typedef SimplePartition<GridType> Partitioner;
    Partitioner ldb(grid);
    grid.repartition( ldb );
  }
  */

  // wait for lb to finish
  grid.comm().barrier();

  // switch back to default
  grid.loadBalance();
  grid.globalRefine(level);
  grid.loadBalance();

  for (int i=0;i<adapt;i++)
  {
    if (myrank==0)
    {
      typedef typename GridType::template Codim< 0 >::template Partition< Dune::Interior_Partition >::LeafIterator LeafIterator;

      LeafIterator endit = grid.template leafend< 0, Dune::Interior_Partition >();
      int nr = 0;
      int size = grid.size(0);
      for(LeafIterator it    = grid.template leafbegin< 0, Dune::Interior_Partition >();
          it != endit ; ++it,nr++ )
      {
        grid.mark(1, *it );
        if (nr>size*0.8) break;
      }
    }
    grid.adapt();
    grid.postAdapt();
    grid.loadBalance();
  }
}

template <class GridType>
void checkIteratorAssignment(GridType & grid)
{
  // check Iterator assignment
  {
    enum { dim = GridType :: dimension };
    typedef typename GridType :: template Codim<dim> :: LevelIterator
      IteratorType;

    IteratorType it = grid.template lbegin<dim>(0);
    if( grid.maxLevel() > 0 ) it = grid.template lbegin<dim>(1);
  }
}


template <class EntityType, class LocalGeometryType>
int aluTwistCheck(const EntityType& en, const LocalGeometryType& localGeom,
                  const int face, const bool neighbor, const bool output )
{
  enum { dim = EntityType :: dimension };
  typedef typename EntityType :: Geometry :: ctype ctype;

  typedef Dune::FaceTopologyMapping< Dune::tetra > SimplexFaceMapping;
  typedef Dune::FaceTopologyMapping< Dune::hexa > CubeFaceMapping;

  // get reference element
  const auto& refElem = Dune::ReferenceElements< ctype, dim >::general( en.type() );

  const int vxSize = refElem.size( face, 1, dim );
  typedef Dune::FieldVector< ctype, dim > CoordinateVectorType;

  // now calculate twist by trial and error for all possible twists
  // the calculated twist is with respect to the ALUGrid
  // reference face, see twistprovider.cc
  int twistFound = -66;
  for(int twist = -vxSize; twist<vxSize; ++twist)
  {
    bool twistOk = true;
    // now check mapping with twist
    for(int i=0; i<vxSize; ++i)
    {
      int twistedDuneIndex = -1;
      if( localGeom.type().isCube() )
      {
        twistedDuneIndex = CubeFaceMapping::twistedDuneIndex( i, twist );
      }
      else
      {
        twistedDuneIndex = SimplexFaceMapping::twistedDuneIndex( i, twist );
      }

      // get face vertices of number in self face
      int vxIdx = refElem.subEntity( face, 1 , twistedDuneIndex , dim);

      // get position in reference element of vertex i
      CoordinateVectorType refPos = refElem.position( vxIdx, dim );

      // check coordinates again
      CoordinateVectorType localPos = localGeom.corner( i );
      if( (refPos - localPos).infinity_norm() > 1e-8 )
      {
        twistOk = false;
        break;
      }
    }

    if( twistOk )
    {
      twistFound = twist;
      break ;
    }
  }

  // if no twist found, then something is wrong
  if( twistFound == -66 )
  {
    assert (false);
    DUNE_THROW( Dune::GridError, "Not matching twist found" );
  }

  if( output )
  {
    std::string twistIn( (neighbor) ? "twistInOutside()" : "twistInInside()" );
    std::string numberIn( (neighbor) ? "indexInOutside()" : "indexInInside()" );
    std::cout << "ERROR: Face "<< face << " : twist = "<< twistFound << std::endl;
    std::cout << "\nPut twist = "<< twistFound << " In TwistUtility::"<< twistIn << " for " << numberIn << " = " << face << " ! \n";
    std::cout << "******************************************\n";
  }

  return twistFound;
}

template <class GridView>
void checkALUTwists( const GridView& gridView, const bool verbose = false )
{

  typedef typename GridView :: template Codim< 0 > :: Iterator Iterator ;
  typedef typename Iterator :: Entity Entity ;
  typedef typename GridView :: IntersectionIterator IntersectionIterator ;

  const Iterator endit = gridView.template end< 0 >();
  for( Iterator it = gridView.template begin< 0 >(); it != endit ; ++it )
  {
    const Entity& entity = *it ;
    const IntersectionIterator endnit = gridView.iend( entity );
    for( IntersectionIterator nit = gridView.ibegin( entity ); nit != endnit; ++nit )
    {
      typedef typename IntersectionIterator :: Intersection  Intersection;
      const Intersection& intersection = * nit ;

      // check twist of inside geometry
      const int twistInside = aluTwistCheck( entity, intersection.geometryInInside(),
                                             intersection.indexInInside(), false, verbose );
      const int twistIn = intersection.impl().twistInInside();

      if( twistInside != twistIn )
        std::cerr << "Error: inside twists " << twistInside << " (found)  and  " << twistIn << " (given) differ" << std::endl;

      if( intersection.neighbor() )
      {
        // check twist of inside geometry
        const int twistOutside = aluTwistCheck( intersection.outside(), intersection.geometryInOutside(),
                                                intersection.indexInOutside(), true, verbose );
        const int twistOut = intersection.impl().twistInOutside();
        if( twistOutside != twistOut )
          std::cerr << "Error: outside twists " << twistOutside << " (found)  and  " << twistOut << " (given) differ" << std::endl;
      }
    }
  }
}

template <int codim, class GridView>
void checkIteratorCodim(const GridView & gridView)
{
  typedef typename GridView::template Codim<codim>::
     template Partition<Dune::InteriorBorder_Partition>::Iterator
        IteratorInteriorBorder;

  typedef typename GridView::Grid::template Codim<codim>:: Geometry Geometry ;
  typedef typename GridView::Grid::template Codim<codim>:: Entity   Entity;
  typedef typename GridView::Grid:: ctype ctype;

  /** Loop only over the interior elements, not over ghost elements. */
  const IteratorInteriorBorder endIterator = gridView.template end< codim, Dune::InteriorBorder_Partition >();
  for( IteratorInteriorBorder iter = gridView.template begin< codim, Dune::InteriorBorder_Partition >(); iter != endIterator; ++iter )
  {
    const Entity& entity = *iter ;
    /** Provide geometry type of element. */
    const Geometry& geo = entity.geometry();
    if( geo.corners() > 1 )
    {
      Dune::FieldVector<ctype, GridView::Grid::dimensionworld>
        diff( geo.corner(0) - geo.corner(1) );
      if( diff.two_norm() < 1e-8 )
      {
        std::cout << diff << " twonorm = " << diff.two_norm() << " point 0 and 1 do not differ! " << std::endl;
        assert ( diff.two_norm() > 1e-8 );
      }
    }
  }
  // check intersection iterator
  if( codim == 0 )
    checkViewIntersectionIterator( gridView );
}

template <class GridType>
void checkALUIterators( GridType& grid )
{
  checkIteratorCodim< 0 > ( grid.leafGridView() );
  checkIteratorCodim< 1 > ( grid.leafGridView() );
  checkIteratorCodim< 2 > ( grid.leafGridView() );
  if(  GridType :: dimension > 2 )
    checkIteratorCodim< GridType :: dimension > ( grid.leafGridView() );

  checkIteratorCodim< 0 > ( grid.macroGridView() );
  checkIteratorCodim< 1 > ( grid.macroGridView() );
  checkIteratorCodim< 2 > ( grid.macroGridView() );
  if(  GridType :: dimension > 2 )
    checkIteratorCodim< GridType :: dimension > ( grid.macroGridView() );
}

template <int codim, class GridType>
void checkPersistentContainerCodim(GridType & grid)
{
  typedef Dune::PersistentContainer< GridType, int > ContainerType;

  ContainerType persistentContainer( grid, codim );
  typedef typename ContainerType::Iterator iterator;

  // clear container
  const iterator end = persistentContainer.end();
  for( iterator it = persistentContainer.begin(); it != end; ++ it )
    *it = 0;

  typedef typename GridType::template Codim<codim>::LeafIterator Iterator;
  typedef typename GridType::template Codim<codim>:: Entity   Entity;

  /** Loop only over the interior elements, not over ghost elements. */
  const Iterator endIterator = grid.template leafend<codim>();
  for (Iterator iter =
       grid.template leafbegin<codim>();
       iter!=endIterator; ++iter)
  {
    const Entity& entity = *iter ;
    persistentContainer[ entity ] = 1 ;
  }

  int sum = 0;
  for( iterator it = persistentContainer.begin(); it != end; ++ it )
    sum += *it;

  // the number of leaf entities should equal to what we just stored.
  if( grid.size( codim ) != sum )
    DUNE_THROW( Dune::InvalidStateException, "PersistentContainer for codim " << codim<< " gives wrong results." );
}

template <class GridType>
void checkPersistentContainer( GridType& grid )
{
  checkPersistentContainerCodim< 0 > ( grid );
  checkPersistentContainerCodim< 1 > ( grid );
  checkPersistentContainerCodim< 2 > ( grid );
  checkPersistentContainerCodim< GridType :: dimension > ( grid );
}

template <class GridType>
void checkLevelIndexNonConform(GridType & grid)
{
  typedef typename GridType :: template Codim<0> :: LeafIterator
          IteratorType;
  {
    IteratorType end = grid.template leafend<0>();
    for(IteratorType it = grid.template leafbegin<0>(); it!=end; ++it)
    {
      // call index of level index set
      grid.levelIndexSet(it->level()).index(*it);
    }
  }

  {
    IteratorType it = grid.template leafbegin<0>();
    if( it != grid.template leafend<0>() )
    {
      // mark first entity
      grid.mark(1, *it);
    }
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();

  {
    IteratorType end = grid.template leafend<0>();
    for(IteratorType it = grid.template leafbegin<0>(); it!=end; ++it)
    {
      // call index of level index set
      grid.levelIndexSet(it->level()).index(*it);
    }
  }
}

template <class GridView>
void writeFile( const GridView& gridView , int sequence = 0 )
{
  Dune::DGFWriter< GridView > writer( gridView );
  writer.write( "dump.dgf" );

  Dune::VTKWriter< GridView > vtk( gridView );
  vtk.write( "dump-" + std::to_string( sequence ) );
}

template <class GridType>
void checkGrid( GridType& grid )
{
  try {
    gridcheck(grid);
    checkIterators( grid.leafGridView() );
    checkIterators( grid.macroGridView() );
    checkIterators( grid.levelGridView(0) );
    if( ! grid.conformingRefinement() )
    {
      for( int level = 1; level <= grid.maxLevel(); ++level )
        checkIterators( grid.levelGridView( level ) );
    }
  }
  catch (const Dune::Exception& e )
  {
    std::cout << "Caught " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Caught unknown exception!" << std::endl;
    assert( false );
    std::abort();
  }
}

template <class GridType>
void checkForPeriodicBoundaries( GridType& grid )
{
  bool foundPeriodicBnd = false ;
  for (const auto& element : elements(grid.leafGridView()))
  {
    for (const auto& intersection : intersections(grid.leafGridView(), element))
    {
      if (intersection.neighbor() && intersection.boundary())
      {
        foundPeriodicBnd = true ;
      }
    }
  }

  // check if some process has found periodic bnds
  // due to load balancing issues periodic bnds
  // might only end up on one core
  foundPeriodicBnd = grid.comm().max( foundPeriodicBnd );
  if( ! foundPeriodicBnd )
  {
    DUNE_THROW( Dune::InvalidStateException, "No periodic boundaries found!" );
  }
}

template <class GridType>
void checkALUSerial(GridType & grid, int mxl = 2)
{
  const bool skipLevelIntersections = grid.conformingRefinement();//! EnableLevelIntersectionIteratorCheck< GridType > :: v ;
  {
    GridType* gr = new GridType();
    assert ( gr );
    delete gr;
  }

  writeFile( grid.leafGridView() );

  std::stringstream null;
  std::ostream& out = (grid.comm().rank() == 0) ? std::cout : null;

  out << "  CHECKING: grid size = " << grid.size( 0 ) << std::endl;

  // be careful, each global refine create 8 x maxlevel elements
  out << "  CHECKING: Macro" << std::endl;
  checkGrid(grid);

  out << "  CHECKING: Macro-intersections" << std::endl;
  checkIntersectionIterator(grid, skipLevelIntersections);

  if( GridType :: dimension == 3 )
  {
    // this only works for ALUGrid 3d
    out << "  CHECKING: 3d Twists " << std::endl;
    checkALUTwists( grid.leafGridView() );
  }

  // only check twists for simplex grids
  // const bool checkTwist = grid.geomTypes(0)[0].isSimplex();

  //if( checkTwist )
  //  checkTwists( grid.leafGridView(), NoMapTwist() );

  for(int i=0; i<mxl; i++)
  {
    grid.globalRefine( Dune::DGFGridInfo< GridType >::refineStepsForHalf() );
    out << "  CHECKING: Refined" << std::endl;
    checkGrid(grid);
    out << "  CHECKING: intersections" << std::endl;
    checkIntersectionIterator(grid, skipLevelIntersections);
    // if( checkTwist )
    //  checkTwists( grid.leafGridView(), NoMapTwist() );
  }

  writeFile( grid.leafGridView(), 1 );

  // check also non-conform grids
  makeNonConfGrid(grid,0,1);

  writeFile( grid.leafGridView(), 2 );

  // check iterators
  checkALUIterators( grid );

  out << "  CHECKING: non-conform" << std::endl;
  checkGrid(grid);
  out << "  CHECKING: twists " << std::endl;
  // if( checkTwist )
  //  checkTwists( grid.leafGridView(), NoMapTwist() );

  // check the method geometryInFather()
  if( GridType::dimension == GridType::dimensionworld )
  {
    out << "  CHECKING: geometry in father" << std::endl;
    checkGeometryInFather(grid);
  }
  // check the intersection iterator and the geometries it returns
  out << "  CHECKING: intersections" << std::endl;
  checkIntersectionIterator(grid, skipLevelIntersections);

  out << "  CHECKING: Iterator Assignment" << std::endl;
  // some checks for assignment of iterators
  checkIteratorAssignment(grid);

  out << "  CHECKING: Nonconforming Index Sets" << std::endl;
  // check level index sets on nonconforming grids
  checkLevelIndexNonConform(grid);

  // check life time of geometry implementation
  out << "  CHECKING: geometry lifetime" << std::endl;
  checkGeometryLifetime( grid.leafGridView() );

  // check persistent container
  out << "  CHECKING: persistent container" << std::endl;
  checkPersistentContainer( grid );

  out << std::endl << std::endl;
}

template <class GridType>
void checkALUParallel(GridType & grid, int gref, int mxl = 3)
{
#if USE_PARALLEL_TEST
  makeNonConfGrid(grid,gref,mxl);

  // check iterators
  checkALUIterators( grid );

  // -1 stands for leaf check
  checkCommunication(grid, -1, std::cout);

  if( ! grid.conformingRefinement() )
  {
    for(int l=0; l<= mxl; ++l)
      checkCommunication(grid, l , Dune::dvverb);
  }
#endif
}

struct EnableConformingRefinement
{
  EnableConformingRefinement() { setenv("ALUGRID_CONFORMING_REFINEMENT", "1", 1 ); }
  ~EnableConformingRefinement() { unsetenv("ALUGRID_CONFORMING_REFINEMENT"); }
  void operator = (int) {}
};


int main (int argc , char **argv) {

  // this method calls MPI_Init, if MPI is enabled
  Dune::MPIHelper &mpihelper = Dune::MPIHelper::instance( argc, argv );
  int myrank = mpihelper.rank();
  int mysize = mpihelper.size();

  try {
    /* use grid-file appropriate for dimensions */

    std::string key;
    bool initialize = true ;
    if( argc >= 2 )
    {
      key = argv[1];
      initialize = false;
    }
    else
    {
      std::cout << "usage:" << argv[0] << " <2d|2dsimp|2dcube|2dconf|3d|3dsimp|3dconf|3dcube|3dperiodic>" << std::endl;
    }

    const char *newfilename = 0;
    if( argc > 2 )
      newfilename = argv[ 2 ];

#ifndef NO_2D
    bool testALU2dSimplex = initialize ;
    bool testALU2dConform = initialize ;
    bool testALU2dCube    = initialize ;
    if( key == "2d" )
    {
      testALU2dSimplex = true ;
      testALU2dConform = true ;
      testALU2dCube   = true ;
    }
    if( key == "2dsimp" ) testALU2dSimplex = true ;
    if( key == "2dconf" ) testALU2dConform = true ;
    if( key == "2dcube" ) testALU2dCube    = true ;
#endif // #ifndef NO_2D

#ifndef NO_3D
    bool testALU3dSimplex = initialize ;
    bool testALU3dConform = initialize ;
    bool testALU3dCube    = initialize ;
    bool testALU3dPeriodic    = initialize ;
    if( key == "3d" )
    {
      testALU3dSimplex = true ;
      testALU3dConform = true ;
      testALU3dCube    = true ;
      testALU3dPeriodic = true ;
    }
    if( key == "3dnonc" )
    {
      testALU3dSimplex = true ;
      testALU3dCube    = true ;
    }
    if( key == "3dsimp" ) testALU3dSimplex = true ;
    if( key == "3dconf" ) testALU3dConform = true ;
    if( key == "3dcube" ) testALU3dCube    = true ;
    if( key == "3dperiodic" ) testALU3dPeriodic    = true ;
#endif // #ifndef NO_3D

    // extra-environment to check destruction
    {
      factorEpsilon = 5.e+5;
      // check empty grid


      // check empty grids

#ifndef NO_3D
      if (myrank == 0 && (testALU3dCube || testALU3dSimplex) )
        std::cout << "Check empty grids" << std::endl;

      if( testALU3dCube )
      {
        Dune::ALUGrid< 3, 3, Dune::cube > grid;
        checkALUSerial( grid );
      }

      if( testALU3dSimplex )
      {
        Dune::ALUGrid< 3, 3, Dune::simplex > grid;
        checkALUSerial( grid );
      }

      if( testALU3dConform )
      {
        EnableConformingRefinement conforming;
        Dune::ALUGrid< 3, 3, Dune::simplex > grid;
        checkALUSerial( grid );
        conforming = 0;
      }
#endif // #ifndef NO_3D


      // check grid factory (test only available for dune-grid 3.0 or later)

      if( myrank == 0 )
        std::cout << "Checking grid factory..." << std::endl;

#if HAVE_DUNE_GRID_TESTGRIDS
#ifndef NO_2D
      if( testALU2dCube )
        Dune::checkGridFactory< Dune::ALUGrid< 2, 2, Dune::cube > >( Dune::TestGrids::unitSquare );

      if( testALU2dSimplex )
        Dune::checkGridFactory< Dune::ALUGrid< 2, 2, Dune::simplex > >( Dune::TestGrids::kuhn2d );

      if( testALU2dConform )
      {
        EnableConformingRefinement conforming;
        Dune::checkGridFactory< Dune::ALUGrid< 2, 2, Dune::simplex > >( Dune::TestGrids::kuhn2d );
        conforming = 0;
      }
#endif // #ifndef NO_2D

#ifndef NO_3D
      if( testALU3dCube )
        Dune::checkGridFactory< Dune::ALUGrid< 3, 3, Dune::cube > >( Dune::TestGrids::unitCube );

      if( testALU3dSimplex )
        Dune::checkGridFactory< Dune::ALUGrid< 3, 3, Dune::simplex > >( Dune::TestGrids::kuhn3d );

      if( testALU3dConform )
      {
        EnableConformingRefinement conforming;
        Dune::checkGridFactory< Dune::ALUGrid< 3, 3, Dune::simplex > >( Dune::TestGrids::kuhn3d );
        conforming = 0;
      }
#endif // #ifndef NO_3D
#endif // HAVE_DUNE_GRID_TESTGRIDS

#ifndef NO_2D
      // check non-conform ALUGrid for 2d
      if( testALU2dCube )
      {
        typedef Dune::ALUGrid< 2, 2, Dune::cube > GridType;
        std::string filename;
        if( newfilename )
          filename = newfilename;
        else
          filename = "./dgf/cube-testgrid-2-2.dgf";
        std::cout << "READING from " << filename << std::endl;
        Dune::GridPtr< GridType > gridPtr( filename );
        gridPtr.loadBalance();

        GridType & grid = *gridPtr;

        checkCapabilities( grid );

        {
          std::cout << "Check serial grid" << std::endl;
          checkALUSerial(grid,
                         (mysize == 1) ? 1 : 0 );
        }

        // perform parallel check only when more then one proc
        if(mysize > 1)
        {
          if (myrank == 0) std::cout << "Check conform grid" << std::endl;
          checkALUParallel(grid,1,0);
          if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
          checkALUParallel(grid,0,2);
        }

        //CircleBoundaryProjection<2> bndPrj;
        //GridType grid("alu2d.triangle", &bndPrj );
        //checkALUSerial(grid,2);

        typedef Dune::ALUGrid< 2, 3, Dune::cube > SurfaceGridType;
        std::string surfaceFilename( "./dgf/cube-testgrid-2-3.dgf" );
        std::cout << "READING from '" << surfaceFilename << "'..." << std::endl;
        Dune::GridPtr< SurfaceGridType > surfaceGridPtr( surfaceFilename );
        SurfaceGridType & surfaceGrid = *surfaceGridPtr ;
        surfaceGrid.loadBalance();
        checkCapabilities( surfaceGrid );
        checkALUSerial( surfaceGrid, 1 );
      }

      // check non-conform ALUGrid for 2d
      if( testALU2dSimplex )
      {
        typedef Dune::ALUGrid< 2, 2, Dune::simplex > GridType;
        std::string filename;
        if( newfilename )
          filename = newfilename;
        else
          filename = "./dgf/simplex-testgrid-2-2.dgf";
        std::cout << "READING from " << filename << std::endl;
        Dune::GridPtr< GridType > gridPtr( filename );
        gridPtr.loadBalance();
        GridType & grid = *gridPtr;

        checkCapabilities( grid );

        {
          std::cout << "Check serial grid" << std::endl;
          checkALUSerial(grid,
                         (mysize == 1) ? 1 : 0 );
        }

        // perform parallel check only when more then one proc
        if(mysize > 1)
        {
          if (myrank == 0) std::cout << "Check conform grid" << std::endl;
          checkALUParallel(grid,1,0);
          if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
          checkALUParallel(grid,0,2);
        }

        //CircleBoundaryProjection<2> bndPrj;
        //GridType grid("alu2d.triangle", &bndPrj );
        //checkALUSerial(grid,2);

        typedef Dune::ALUGrid< 2, 3, Dune::simplex > SurfaceGridType;
        std::string surfaceFilename( "./dgf/simplex-testgrid-2-3-noproj.dgf" );
        std::cout << "READING from '" << surfaceFilename << "'..." << std::endl;

        std::cerr << "WARNING: surface projection disabled for ALUGrid< 2, 3, Dune::simplex >" << std::endl;
        Dune::GridPtr< SurfaceGridType > surfaceGridPtr( surfaceFilename );
        SurfaceGridType & surfaceGrid = *surfaceGridPtr ;
        surfaceGrid.loadBalance();
        checkCapabilities( surfaceGrid );
        checkALUSerial( surfaceGrid, 1 );
      }

      // check conform ALUGrid for 2d
      if( testALU2dConform )
      {
        EnableConformingRefinement conforming;

        //typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > GridType;
        typedef Dune::ALUGrid< 2, 2, Dune::simplex > GridType;
        std::string filename;
        if( newfilename )
          filename = newfilename;
        else
          filename = "./dgf/simplex-testgrid-2-2.dgf";
        Dune::GridPtr<GridType> gridPtr( filename );
        gridPtr.loadBalance();
        GridType & grid = *gridPtr;

        checkCapabilities( grid );

        {
          std::cout << "Check serial grid" << std::endl;
          checkALUSerial(grid,
                         (mysize == 1) ? 1 : 0 );
        }

        // perform parallel check only when more then one proc
        if(mysize > 1)
        {
          if (myrank == 0) std::cout << "Check conform grid" << std::endl;
          checkALUParallel(grid,1,0);
          if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
          checkALUParallel(grid,0,2);
        }

        //CircleBoundaryProjection<2> bndPrj;
        //GridType grid("alu2d.triangle", &bndPrj );
        //checkALUSerial(grid,2);

        typedef Dune::ALUGrid< 2, 3, Dune::simplex > SurfaceGridType;
        //typedef ALUConformGrid< 2, 3 > SurfaceGridType;
        std::string surfaceFilename( "./dgf/simplex-testgrid-2-3.dgf" );
        std::cout << "READING from '" << surfaceFilename << "'..." << std::endl;
        Dune::GridPtr< SurfaceGridType > surfaceGridPtr( surfaceFilename );
        surfaceGridPtr.loadBalance();
        SurfaceGridType & surfaceGrid = *surfaceGridPtr ;
        checkCapabilities( surfaceGrid );
        checkALUSerial( surfaceGrid, 1 );

        conforming = 0;
      }
#endif // #ifndef NO_2D

#ifndef NO_3D
      if( testALU3dCube )
      {
        std::string filename;
        if( newfilename )
          filename = newfilename;
        else
          filename = "./dgf/simplex-testgrid-3-3.dgf";

        typedef Dune::ALUGrid< 3, 3, Dune::cube > GridType;
        {
          Dune::GridPtr< GridType > gridPtr( filename );
          gridPtr.loadBalance();
          GridType & grid = *gridPtr;

          checkCapabilities( grid );

          {
            std::cout << "Check serial grid" << std::endl;
            checkALUSerial(grid,
                           (mysize == 1) ? 1 : 0 );
          }

          // perform parallel check only when more then one proc
          if(mysize > 1)
          {
            if (myrank == 0) std::cout << "Check conform grid" << std::endl;
            checkALUParallel(grid,1,0);
            if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
            checkALUParallel(grid,0,2);
          }
        }
      }

      if( testALU3dPeriodic )
      {
        // check periodic capabilities
        std::string filename;
        if( newfilename )
          filename = newfilename;
        else
          filename = "./dgf/periodic3.dgf";
        typedef Dune::ALUGrid< 3, 3, Dune::cube > GridType;
        // periodic boundaries require certain load balancing methods
        GridType::setLoadBalanceMethod( 10 );
        Dune::GridPtr< GridType > gridPtr( filename );
        gridPtr.loadBalance();
        GridType & grid = *gridPtr;

        {
          std::cout << "Check periodic grid" << std::endl;
          checkALUSerial(grid,
                         (mysize == 1) ? 1 : 0 );
          checkForPeriodicBoundaries( grid );
        }
      }

      if( testALU3dSimplex )
      {
        std::string filename;
        if( newfilename )
          filename = newfilename;
        else
          filename = "./dgf/simplex-testgrid-3-3.dgf";

        typedef Dune::ALUGrid< 3, 3, Dune::simplex > GridType;
        Dune::GridPtr< GridType > gridPtr( filename );
        gridPtr.loadBalance();
        GridType & grid = *gridPtr;
        checkCapabilities( grid );

        {
          std::cout << "Check serial grid" << std::endl;
          checkALUSerial(grid,
                         (mysize == 1) ? 1 : 0);
        }

        // perform parallel check only when more then one proc
        if(mysize > 1)
        {
          if (myrank == 0) std::cout << "Check conform grid" << std::endl;
          checkALUParallel(grid,0,0);  //1,3
          if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
          checkALUParallel(grid,0,2);  //1,3
        }
      }

      if( testALU3dConform )
      {
        EnableConformingRefinement conforming;

        std::string filename;
        if( newfilename )
          filename = newfilename;
        else
          filename = "./dgf/simplex-testgrid-3-3.dgf";

        typedef Dune::ALUGrid< 3, 3, Dune::simplex > GridType;
        Dune::GridPtr< GridType > gridPtr( filename );
        gridPtr.loadBalance();
        GridType & grid = *gridPtr;
        checkCapabilities( grid );

        {
          std::cout << "Check serial grid" << std::endl;
          checkALUSerial(grid,
                         (mysize == 1) ? 1 : 0);
        }

        // perform parallel check only when more then one proc
        if(mysize > 1)
        {
          if (myrank == 0) std::cout << "Check conform grid" << std::endl;
          checkALUParallel(grid,0,0);  //1,3
          if (myrank == 0) std::cout << "Check non-conform grid" << std::endl;
          checkALUParallel(grid,0,4);  //1,3
        }

        conforming = 0;
      }
#endif
    };

  }
  catch( Dune::Exception &e )
  {
    std::cerr << e << std::endl;
    return 1;
  }
  catch( ... )
  {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
}
