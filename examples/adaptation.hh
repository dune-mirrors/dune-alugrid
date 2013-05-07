#ifndef ADAPTATION_HH 
#define ADAPTATION_HH 

/** include the grid capabilities 
 ** to distiguish grids without local adaptation **/
#include <dune/common/timer.hh>

#include <dune/grid/common/capabilities.hh>
#include <dune/grid/utility/persistentcontainer.hh>

// global counter of adaptation cyclces 
static int adaptationSequenceNumber = 0; 

#include "datamap.hh"

#define CALLBACK

#ifdef CALLBACK
#include "callbackadaptation.hh"
#endif 

// GridMarker
// ----------

/** \class GridMarker
 *  \brief class for marking entities for adaptation.
 *
 *  This class provides some additional strategies for marking elements
 *  which are not so much based on an indicator but more on mere
 *  geometrical and run time considerations. If based on some indicator
 *  an entity is to be marked, this class additionally tests for example
 *  that a maximal or minimal level will not be exceeded. 
 */
template< class Grid >
struct GridMarker 
{
  typedef typename Grid::template Codim< 0 >::Entity        Entity;
  typedef typename Grid::template Codim< 0 >::EntityPointer EntityPointer;

  /** \brief constructor
   *  \param grid     the grid. Here we can not use a grid view since they only
   *                  a constant reference to the grid and the
   *                  mark method can not be called.
   *  \param minLevel the minimum refinement level
   *  \param maxLevel the maximum refinement level
   */
  GridMarker( Grid &grid, int minLevel, int maxLevel )
  : grid_(grid),
    minLevel_( minLevel ),
    maxLevel_( maxLevel ),
    wasMarked_( 0 ),
    adaptive_( maxLevel_ > minLevel_ ) 
  {}

  /** \brief mark an element for refinement 
   *  \param entity  the entity to mark; it will only be marked if its level is below maxLevel.
   */
  void refine ( const Entity &entity )
  {
    if( entity.level() < maxLevel_ )
    {
      grid_.mark( 1, entity );
      wasMarked_ = 1;
    }
  }

  /** \brief mark all neighbors of a given entity for refinement 
   *  \param gridView the grid view from which to take the intersection iterator 
   *  \param entity the corresponding entity
   */
  template< class GridView >
  void refineNeighbors ( const GridView &gridView, const Entity &entity )
  {
    typedef typename GridView::IntersectionIterator IntersectionIterator;
    typedef typename IntersectionIterator::Intersection Intersection;

    const IntersectionIterator end = gridView.iend( entity );
    for( IntersectionIterator it = gridView.ibegin( entity ); it != end; ++it )
    {
      const Intersection &intersection = *it;
      if( intersection.neighbor() )
      {
        EntityPointer ep = intersection.outside() ;
        const Entity& outside = *ep;
        refine( outside );
      }
    }
  }

  /** \brief mark an element for coarsening
   *  \param entity  the entity to mark; it will only be marked if its level is above minLevel.
   */
  void coarsen ( const Entity &entity )
  {
    if( (get( entity ) <= 0) && (entity.level() > minLevel_) )
    {
      grid_.mark( -1, entity );
      wasMarked_ = 1;
    }
  }

  /** \brief get the refinement marker 
   *  \param entity entity for which the marker is required
   *  \return value of the marker
   */
  int get ( const Entity &entity ) const
  {
    if( adaptive_ ) 
      return grid_.getMark( entity );
    else
    {
      // return so that in scheme.mark we only count the elements
      return 1;
    }
  }

  /** \brief returns true if any entity was marked for refinement 
   */
  bool marked() 
  {
    if( adaptive_ ) 
    {
      wasMarked_ = grid_.comm().max (wasMarked_);
      return (wasMarked_ != 0);
    }
    return false ;
  }

private:
  Grid &grid_;
  const int minLevel_;
  const int maxLevel_;
  int wasMarked_;
  const bool adaptive_ ;
};

// LeafAdaptation
// --------------

/** \class LeafAdaptation
 *  \brief class used the adaptation procedure.
 *
 *  \tparam Grid     is the type of the underlying grid
 */
template< class Grid>
struct LeafAdaptation
{
  // dimensions of grid and world
  static const int dimGrid = Grid::dimension;
  static const int dimWorld = Grid::dimensionworld;

  // type used for coordinates in the grid
  typedef typename Grid::ctype ctype;

  // type of id set
  typedef typename Grid::Traits::LocalIdSet IdSet;

  // types of grid's level and hierarchic iterator
  static const Dune::PartitionIteratorType partition = Dune::Interior_Partition;
  typedef typename Grid::template Codim< 0 >::template Partition< partition >::LevelIterator LevelIterator;
  typedef typename Grid::Traits::HierarchicIterator HierarchicIterator;

  // types of entity, entity pointer and geometry
  typedef typename Grid::template Codim< 0 >::Entity Entity;

public:
  /** \brief constructor
   *  \param grid   the grid to be adapted
   */
  LeafAdaptation ( Grid &grid, const int balanceStep = 1 )
  : grid_( grid ),
    balanceStep_( balanceStep ),
    balanceCounter_( 0 ),
    adaptTime_( 0.0 ),
    lbTime_( 0.0 ),
    commTime_( 0.0 )
  {}

  /** \brief main method performing the adaptation and
             perserving the data.
      \param solution the data vector to perserve during 
             adaptation. This class must conform with the
             parameter class V in the DataMap class and additional
             provide a resize and communicate method.
  **/
  template< class Vector >
  void operator() ( Vector &solution );

  //! return time spent for the last adapation in sec 
  double adaptationTime() const { return adaptTime_; }
  //! return time spent for the last load balancing in sec
  double loadBalanceTime() const { return lbTime_; }
  //! return time spent for the last communication in sec
  double communicationTime() const { return commTime_; }

private:
  /** \brief do restriction of data on leafs which might vanish
   *         in the grid hierarchy below a given entity
   *  \param entity   the entity from where to start the restriction process
   *  \param dataMap  map containing the leaf data to be used to store the
   *                  restricted data.
   **/
  template< class Vector, class DataMap >
  void hierarchicRestrict ( const Entity &entity, DataMap &dataMap ) const;

  /** \brief do prolongation of data to new elements below the given entity
   *  \param entity  the entity from where to start the prolongation
   *  \param dataMap the map containing the data and used to store the
   *                 data prolongt to the new elements
   **/
  template< class Vector, class DataMap >
  void hierarchicProlong ( const Entity &entity, DataMap &dataMap ) const;

  Grid &grid_;

  // call loadBalance ervery balanceStep_ step
  const int balanceStep_ ;
  // count actual balance call
  int balanceCounter_;

  double adaptTime_;
  double lbTime_;
  double commTime_;
};

template< class Grid >
template< class Vector >
inline void LeafAdaptation< Grid >::operator() ( Vector &solution )
{
  if (Dune :: Capabilities :: isCartesian<Grid> :: v)
    return;

  adaptTime_ = 0.0;
  lbTime_    = 0.0;
  commTime_  = 0.0;
  Dune :: Timer adaptTimer ; 

  // copy complete solution vector to map
  typedef typename Vector::GridView GridView;
  typedef typename GridView
    ::template Codim< 0 >::template Partition< partition >::Iterator 
    Iterator;
  const GridView &gridView = solution.gridView();

  // container to keep data save during adaptation and load balancing
  typedef Dune::PersistentContainer<Grid,typename Vector::LocalDofVector> Container;
  // create persistent container for codimension 0
  Container container( grid_, 0 );

  // first store all leave data in container
  {
    const Iterator end = gridView.template end  < 0, partition >();
    for(  Iterator it  = gridView.template begin< 0, partition >(); it != end; ++it )
    {
      const Entity &entity = *it;
      solution.getLocalDofVector( entity, container[ entity ] );
    }
  }

#ifndef CALLBACK
  // check if elements might be removed in next adaptation cycle 
  const bool mightCoarsen = grid_.preAdapt();

  // if elements might be removed
  if( mightCoarsen )
  {
    // restrict data and save leaf level 
    const LevelIterator end = grid_.template lend< 0, partition >( 0 );
    for( LevelIterator it = grid_.template lbegin< 0, partition >( 0 ); it != end; ++it )
      hierarchicRestrict<Vector>( *it, container );
  }

  // adapt grid, returns true if new elements were created 
  const bool refined = grid_.adapt();

  // interpolate all new cells to leaf level 
  if( refined )
  {
    container.resize();
    const LevelIterator end = grid_.template lend< 0, partition >( 0 );
    for( LevelIterator it = grid_.template lbegin< 0, partition >( 0 ); it != end; ++it )
      hierarchicProlong<Vector>( *it, container );
  }
#else
  AdaptDataHandle<Grid,Vector,Container> adaptHandle( container );
  grid_.adapt( adaptHandle );
  // reduce container to size that is needed 
  container.shrinkToFit();
#endif

  bool callBalance = ( (balanceCounter_ >= balanceStep_) && (balanceStep_ > 0) );
  // make sure everybody is on the same track 
  assert( callBalance == grid_.comm().max( callBalance) );
  // increase balanceCounter if balancing is enabled 
  if( balanceStep_ > 0 ) ++balanceCounter_;

  adaptTime_ = adaptTimer.elapsed();

  if( callBalance ) 
  {
    Dune :: Timer lbTimer ;
    // re-balance grid 
    typedef DataHandle<Grid,Container> DH;
    DH dataHandle( grid_, container ) ;
    typedef Dune::CommDataHandleIF< DH, Container > DataHandleInterface;
    grid_.loadBalance( (DataHandleInterface&)(dataHandle) );
    lbTime_ = lbTimer.elapsed();
  }

  // reset timer to count again 
  adaptTimer.reset();

#ifndef CALLBACK
  // cleanup adaptation markers 
  grid_.postAdapt();

  // resize solution vector if elements might have been removed 
  // or were created 
  if( refined || mightCoarsen ) 
#endif // for CALLBACK resize anyway 
    solution.resize();

  // retrieve data from container and store on new leaf grid
  {
    const Iterator end = gridView.template end  < 0, partition >();
    for(  Iterator it  = gridView.template begin< 0, partition >(); it != end; ++it )
    {
      const Entity &entity = *it;
      solution.setLocalDofVector( entity, container[ entity ] );
    }
  }

  // store adaptation time 
  adaptTime_ += adaptTimer.elapsed();

  Dune::Timer commTimer ;
  // copy data to ghost entities
  solution.communicate();
  commTime_ = commTimer.elapsed();

  // increase adaptation secuence number 
  ++adaptationSequenceNumber;
}

template< class Grid >
template< class Vector, class DataMap >
inline void LeafAdaptation< Grid >
  ::hierarchicRestrict ( const Entity &entity, DataMap &dataMap ) const
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
      hierarchicRestrict<Vector>( child, dataMap );
      doRestrict &= child.mightVanish();
    }

    // if there is a child that does not vanish, this entity may not vanish
    assert( doRestrict || !entity.mightVanish() );

    if( doRestrict )
      Vector::restrictLocal( entity, dataMap );
  }
}

template< class Grid >
template< class Vector, class DataMap >
inline void LeafAdaptation< Grid >
  ::hierarchicProlong ( const Entity &entity, DataMap &dataMap ) const
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
      hierarchicProlong<Vector>( *hit, dataMap );
    }
  }
}

#endif 
