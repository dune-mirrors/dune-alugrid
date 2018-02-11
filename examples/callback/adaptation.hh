#ifndef ADAPTATION_HH
#define ADAPTATION_HH

/** include the grid capabilities
 ** to distiguish grids without local adaptation **/
#include <dune/common/version.hh>
#include <dune/common/timer.hh>

#include <dune/grid/common/capabilities.hh>
#include <dune/grid/utility/persistentcontainer.hh>

// global counter of adaptation cyclces
static int adaptationSequenceNumber = 0;

#include "datamap.hh"

// interface class for callback adaptation
#include <dune/grid/common/adaptcallback.hh>

// LeafAdaptation
// --------------

/** \class LeafAdaptation
 *  \brief class used the adaptation procedure.
 *
 *  \tparam Grid     is the type of the underlying grid
 *  \tparam Vector   is the type of the solution vector
 */
template< class Grid, class Vector >
class LeafAdaptation : public Dune::AdaptDataHandle< Grid, LeafAdaptation< Grid, Vector > >
{
  typedef LeafAdaptation<Grid,Vector> ThisType;
public:
  // dimensions of grid and world
  static const int dimGrid = Grid::dimension;
  static const int dimWorld = Grid::dimensionworld;

  // type used for coordinates in the grid
  typedef typename Grid::ctype ctype;

  // types of grid's level and hierarchic iterator
  static const Dune::PartitionIteratorType partition = Dune::Interior_Partition;
  typedef typename Grid::template Codim< 0 >::template Partition< partition >::LevelIterator LevelIterator;
  typedef typename Grid::Traits::HierarchicIterator HierarchicIterator;

  // types of entity, entity pointer and geometry
  typedef typename Grid::template Codim< 0 >::Entity Entity;

#ifdef USE_VECTOR_FOR_PWF
  // container to keep data save during adaptation and load balancing
  typedef Dune::PersistentContainer<Grid,typename Vector::LocalDofVector> Container;
#else
  typedef typename Vector::VectorType Container;
#endif

  // type of grid view used
  typedef typename Vector :: GridView  GridView;

  typedef typename GridView
      ::template Codim< 0 >::template Partition< partition >::Iterator
      Iterator;
public:
  /** \brief constructor
   *  \param grid   the grid to be adapted
   */
  LeafAdaptation ( Grid &grid, const int balanceStep = 1, const int balanceCounter = -1 )
  : grid_( grid ),
#ifdef USE_VECTOR_FOR_PWF
    // create persistent container for codimension 0
    container_( grid_, 0 ),     // in this version we need to provide extra storage for the dofs
#endif
    solution_( 0 ),
    adaptTimer_(),
    balanceStep_( balanceStep ),
    balanceCounter_( balanceCounter < 0 ? balanceStep-1 : balanceCounter ),
    adaptTime_( 0.0 ),
    restProlTime_( 0.0 ),
    lbTime_( 0.0 ),
    commTime_( 0.0 )
  {}

  /** \brief main method performing the adaptation and
             perserving the data.
      \param solution  the data vector to perserve during
                       adaptation. This class must conform with the
                       parameter class V in the DataMap class and additional
                       provide a resize and communicate method.
  **/
  void operator() ( Vector &solution );

  //! return time spent for the last adapation in sec
  double adaptationTime() const { return adaptTime_; }
  //! return time spent for the last adapation in sec
  double restProlTime() const { return restProlTime_; }
  //! return time spent for the last load balancing in sec
  double loadBalanceTime() const { return lbTime_; }
  //! return time spent for the last communication in sec
  double communicationTime() const { return commTime_; }

  // this is called before the adaptation process starts
  void initialize ();

  // this is called before after the adaptation process is finished
  void finalize ();

  //--------------------------------------------------
  //  Interface methods for callback adaptation
  //--------------------------------------------------
  // called when children of father are going to vanish
  void preCoarsening ( const Entity &father )
  {
#ifndef USE_VECTOR_FOR_PWF
    Container &container_ = getSolution().container();
#endif
    Vector::restrictLocal( father, container_ );
  }

  // called when children of father where newly created
  void postRefinement ( const Entity &father )
  {
#ifndef USE_VECTOR_FOR_PWF
    Container &container_ = getSolution().container();
#endif
    container_.resize();
    Vector::prolongLocal( father, container_ );
  }

private:
  /** \brief do restriction of data on leafs which might vanish
   *         in the grid hierarchy below a given entity
   *  \param entity   the entity from where to start the restriction process
   *  \param dataMap  map containing the leaf data to be used to store the
   *                  restricted data.
   **/
  void hierarchicRestrict ( const Entity &entity, Container &dataMap ) const;

  /** \brief do prolongation of data to new elements below the given entity
   *  \param entity  the entity from where to start the prolongation
   *  \param dataMap the map containing the data and used to store the
   *                 data prolongt to the new elements
   **/
  void hierarchicProlong ( const Entity &entity, Container &dataMap ) const;

  Vector& getSolution()             { assert( solution_ ); return *solution_; }
  const Vector& getSolution() const { assert( solution_ ); return *solution_; }

  Grid&              grid_;
#ifdef USE_VECTOR_FOR_PWF
  mutable Container  container_;
#endif
  Vector*            solution_;

  Dune :: Timer      adaptTimer_ ;
  // call loadBalance ervery balanceStep_ step
  const int balanceStep_ ;
  // count actual balance call
  int balanceCounter_;

  double adaptTime_;
  double restProlTime_;
  double lbTime_;
  double commTime_;
};

template< class Grid, class Vector >
inline void LeafAdaptation< Grid, Vector >::operator() ( Vector &solution )
{
  if (Dune :: Capabilities :: isCartesian<Grid> :: v)
    return;

  // set pointer to solution
  solution_ = & solution ;

  adaptTime_ = 0.0;
  lbTime_    = 0.0;
  commTime_  = 0.0;
  restProlTime_ = 0.0;

  // reset timer
  adaptTimer_.reset() ;

  // copy data to container
  initialize();

#ifndef CALLBACK_ADAPTATION
#ifndef USE_VECTOR_FOR_PWF
  Container &container_ = solution.container();
#endif

  // check if elements might be removed in next adaptation cycle
  const bool mightCoarsen = grid_.preAdapt();

  // if elements might be removed
  if( mightCoarsen )
  {
    // restrict data and save leaf level
    const LevelIterator end = grid_.template lend< 0, partition >( 0 );
    for( LevelIterator it = grid_.template lbegin< 0, partition >( 0 ); it != end; ++it )
      hierarchicRestrict( *it, container_ );
  }

  restProlTime_ = adaptTimer_.elapsed();
  adaptTimer_.reset();

  // adapt grid, returns true if new elements were created
  const bool refined = grid_.adapt();

  adaptTime_ = adaptTimer_.elapsed();
  adaptTimer_.reset();

  // interpolate all new cells to leaf level
  if( refined )
  {
    container_.resize();
    const LevelIterator end = grid_.template lend< 0, partition >( 0 );
    for( LevelIterator it = grid_.template lbegin< 0, partition >( 0 ); it != end; ++it )
      hierarchicProlong( *it, container_ );
  }

  restProlTime_ += adaptTimer_.elapsed();

#else // CALLBACK_ADAPTATION
  // callback adaptation, see interface methods above
  grid_.adapt( *this );

  adaptTime_ = adaptTimer_.elapsed();

#endif // CALLBACK_ADAPTATION

  // reset adaptation information in grid
  grid_.postAdapt();

  // copy data back to solution, load balance, and communication
  finalize();

  // increase adaptation secuence number
  ++adaptationSequenceNumber;
}

template< class Grid, class Vector >
inline void LeafAdaptation< Grid, Vector >
  ::initialize()
{
#ifdef USE_VECTOR_FOR_PWF
  const Vector& solution = getSolution();
  const GridView &gridView = solution.gridView();

  // first store all leave data in container
  const Iterator end = gridView.template end  < 0, partition >();
  for(  Iterator it  = gridView.template begin< 0, partition >(); it != end; ++it )
  {
    const Entity &entity = *it;
    solution.getLocalDofVector( entity, container_[ entity ] );
  }
#endif
}

template< class Grid, class Vector >
inline void LeafAdaptation< Grid, Vector >::finalize()
{
  Vector& solution = getSolution();
#ifndef USE_VECTOR_FOR_PWF
  Container &container_ = solution.container();
#endif

  adaptTime_ = adaptTimer_.elapsed();

  bool callBalance = ((balanceStep_ > 0) && (++balanceCounter_ % balanceStep_ == 0));
  // make sure everybody is on the same track
  assert( callBalance == grid_.comm().max( callBalance) );

  if( callBalance )
  {
    Dune :: Timer lbTimer ;
#if BALL
    grid_.loadBalance() ;
#else
    // re-balance grid
    DataHandle<Grid,Container> dataHandle( grid_, container_ ) ;
    grid_.loadBalance( dataHandle );
#endif
    lbTime_ = lbTimer.elapsed();
  }


  // reduce size of container, if possible
  container_.resize();

#ifdef USE_VECTOR_FOR_PWF
  // reset timer to count again
  adaptTimer_.reset();

  // resize to current grid size
  solution.resize();

  // retrieve data from container and store on new leaf grid
  const GridView &gridView = solution.gridView();
  const Iterator end = gridView.template end  < 0, partition >();
  for(  Iterator it  = gridView.template begin< 0, partition >(); it != end; ++it )
  {
    const Entity &entity = *it;
    solution.setLocalDofVector( entity, container_[ entity ] );
  }

  // store adaptation time
  adaptTime_ += adaptTimer_.elapsed();
#endif

  Dune::Timer commTimer ;
  // copy data to ghost entities
  solution.communicate();
  commTime_ = commTimer.elapsed();

  // reset pointer
  solution_ = 0;
}

template< class Grid, class Vector >
inline void LeafAdaptation< Grid, Vector >
  ::hierarchicRestrict ( const Entity &entity, Container &dataMap ) const
{
  // for leaf entities just copy the data to the data map
  if( !entity.isLeaf() )
  {
    // check all children first
    bool doRestrict = true;
    const int childLevel = entity.level() + 1;
    const HierarchicIterator hend = entity.hend( childLevel );
    for( HierarchicIterator hit = entity.hbegin( childLevel ); hit != hend; ++hit )
    {
      const Entity &child = *hit;
      hierarchicRestrict( child, dataMap );
      doRestrict &= child.mightVanish();
    }

    // if there is a child that does not vanish, this entity may not vanish
    assert( doRestrict || !entity.mightVanish() );

    if( doRestrict )
      Vector::restrictLocal( entity, dataMap );
  }
}

template< class Grid, class Vector >
inline void LeafAdaptation< Grid, Vector >
  ::hierarchicProlong ( const Entity &entity, Container &dataMap ) const
{
  if ( !entity.isLeaf() )
  {
    const int childLevel = entity.level() + 1;
    const HierarchicIterator hend = entity.hend( childLevel );
    HierarchicIterator hit = entity.hbegin( childLevel );

    const bool doProlong = hit->isNew();
    if( doProlong )
      Vector::prolongLocal( entity, dataMap );

    // if the children have children then we have to go deeper
    for( ; hit != hend; ++hit )
    {
      assert(doProlong == hit->isNew());
      hierarchicProlong( *hit, dataMap );
    }
  }
}

#endif
