#ifndef ADAPTATION_HH 
#define ADAPTATION_HH 

/** include the grid capabilities 
 ** to distiguish grids without local adaptation **/
#include <dune/grid/common/capabilities.hh>

#include "datamap.hh"

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
  typedef typename Grid::template Codim< 0 >::Entity Entity;

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
    wasMarked_( 0 )
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
        refine( *intersection.outside() );
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
    return grid_.getMark( entity );
  }
  /** \brief returns true if any entity was marked for refinement 
   */
  bool marked() 
  {
    wasMarked_ = grid_.comm().max (wasMarked_);
    return (wasMarked_ != 0);
  }

private:
  Grid &grid_;
  int minLevel_;
  int maxLevel_;
  int wasMarked_;
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
  LeafAdaptation ( Grid &grid )
  : grid_( grid )
  {}

  /** \brief main method performing the adaptation and
             perserving the data.
      \param solution the data vector to perserve during 
             adaptation. This class must conform with the
             parameter class V in the DataMap class and additional
             provide a resize and communicate method.
  **/
  void operator() ( );

  Grid &grid_;
};

template< class Grid >
inline void LeafAdaptation< Grid >::operator( ) ( )
{
  if (Dune :: Capabilities :: isCartesian<Grid> :: v)
    return;
  // copy complete solution vector to map
  typedef typename Grid::template Partition< Dune::Interior_Partition >::LeafGridView GridView;
  typedef typename GridView
    ::template Codim< 0 >::template Partition< partition >::Iterator Iterator;
  const GridView gridView = grid_.template leafView< Dune::Interior_Partition >();

  // check if elements might be removed in next adaptation cycle 
  grid_.preAdapt();

  // adapt grid, returns true if new elements were created 
  grid_.adapt();

  // re-balance grid 
  LoadBalanceHandle loadBalanceHandle;
  typedef Dune::CommDataHandleIF< LoadBalanceHandle, void > DataHandleInterface;
  grid_.loadBalance( (DataHandleInterface&)(loadBalanceHandle) );

  // cleanup adaptation markers 
  grid_.postAdapt();
}
#endif 
