#ifndef FVSCHEME_HH
#define FVSCHEME_HH

#include <limits>

#include <dune/common/version.hh>
#include <dune/common/fvector.hh>

#include <dune/grid/common/gridenums.hh>

#include "adaptation.hh"

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

  void reset() { wasMarked_ = 0 ; }

private:
  Grid &grid_;
  const int minLevel_;
  const int maxLevel_;
  int wasMarked_;
  const bool adaptive_ ;
};

// FiniteVolumeScheme
// ------------------
/** \class FiniteVolumeScheme
 *  \brief the implementation of the finite volume scheme
 *
 *  \tparam  V    type of vector modelling a piecewise constant function
 *  \tparam  Model  discretization of the Model.
 *                  This template class must provide
 *                  the following types and methods:
 *  \code
      typedef ... RangeType;
      const ProblemData &problem () const;
      double numericalFlux ( const DomainType &normal,
                             const double time,
                             const DomainType &xGlobal,
                             const RangeType &uLeft,
                             const RangeType &uRight,
                             RangeType &flux ) const;
      double boundaryFlux ( const int bndId,
                            const DomainType &normal,
                            const double time,
                            const DomainType &xGlobal,
                            const RangeType& uLeft,
                            RangeType &flux ) const;
  * \endcode
  */
/** \class FiniteVolumeScheme
 *  Additional methods on the model
 *  class required for adaptation:
 *  \code
      double indicator ( const DomainType &normal,
                         const double time,
                         const DomainType &xGlobal,
                         const RangeType &uLeft, const RangeType &uRight) const
      double boundaryIndicator ( const int bndId,
                                 const DomainType &normal,
                                 const double time,
                                 const DomainType &xGlobal,
                                 const RangeType& uLeft) const
 *  \endcode
 */
template< class V, class Model >
struct FiniteVolumeScheme
{
  // first we extract some types
  typedef V Vector;
  typedef typename Vector::GridView GridView;
  typedef typename GridView::Grid Grid;
  static const int dim = GridView::dimension;
  static const int dimworld = GridView::dimensionworld;
  static const int dimRange = Model::dimRange;
  typedef typename Grid::ctype ctype;

  // only apply the scheme to interior elements
  static const Dune :: PartitionIteratorType ptype = Dune :: InteriorBorder_Partition ;

  // types of codim zero entity iterator and geometry
  typedef typename GridView::template Codim< 0 >:: template Partition< ptype > :: Iterator  Iterator;
  typedef typename Iterator::Entity                         Entity;
  typedef typename Entity::Geometry                         Geometry;

  // type of intersections and corresponding geometries
  typedef typename GridView::IntersectionIterator       IntersectionIterator;
  typedef typename IntersectionIterator::Intersection   Intersection;
  typedef typename Intersection::Geometry               IntersectionGeometry;

  // types of vectors
  typedef Dune::FieldVector< ctype, dim-1 >      FaceDomainType;
  typedef Dune::FieldVector< ctype, dim >        DomainType;
  typedef Dune::FieldVector< ctype, dimworld >   GlobalType;
  typedef typename Model::RangeType              RangeType;

public:
  /** \brief constructor
   *
   *  \param[in]  gridView  gridView to operate on
   *  \param[in]  model       discretization of the Model
   */
  FiniteVolumeScheme ( const GridView &gridView, const Model &model )
  : gridView_( gridView )
    , model_( model )
  {}

  /** \brief compute the update vector for one time step
   *
   *  \param[in]   time      current time
   *  \param[in]   solution  solution at time <tt>time</tt>
   *                         (arbitrary type with operator[](const Entity&) operator)
   *  \param[out]  update    result of the flux computation
   *
   *  \returns maximal time step
   */
  template <class Arg>
  double
  operator() ( const double time, const Arg &solution, Vector &update ) const;
  template <class Arg>
  double
  border ( const double time, const Arg &solution, Vector &update ) const;

  /** \brief set grid marker for refinement / coarsening
   *
   *  \param[in]  time      current time
   *  \param[in]  solution  solution at time <tt>time</tt>
   *  \param      marker    grid marker
   *
   *  \note The marker is responsible for limiting the grid depth.
   *
   *  \return number of interior elements
   */
  size_t
  mark ( const double time, const Vector &solution, GridMarker< Grid > &marker ) const;

  /** \brief obtain the grid view for this scheme
   *
   *  \returns the grid view
   */
  const GridView &gridView () const
  {
    return gridView_;
  }

private:
  template <class Arg>
  void apply (const Entity &entiy, const double time, const Arg &solution, Vector &update, double &dt ) const;

  const GridView gridView_;
  const Model &model_;
}; // end FiniteVolumeScheme

template< class V, class Model >
template< class Arg >
inline double FiniteVolumeScheme< V, Model >
  ::border ( const double time, const Arg &solution, Vector &update ) const
{
  // if model does not have numerical flux we have nothing to do here
  if( ! Model::hasFlux ) return model_.fixedDt();

  // time step size (using std:min(.,dt) so set to maximum)
  double dt = std::numeric_limits<double>::infinity();

  static const Dune :: PartitionIteratorType pghosttype = Dune :: Ghost_Partition ;
  typedef typename GridView::template Codim< 0 >:: template Partition< pghosttype > :: Iterator  Iterator;
  // compute update vector and optimum dt in one grid traversal
  const Iterator endit = gridView().template end< 0, pghosttype >();
  for( Iterator it = gridView().template begin< 0, pghosttype >(); it != endit; ++it )
  {
    const Entity &entity = *it;
    const IntersectionIterator iitend = gridView().iend( entity );
    for( IntersectionIterator iit = gridView().ibegin( entity ); iit != iitend; ++iit )
    {
      apply( iit->outside(), time, solution, update, dt );
    }
  } // end grid traversal

  // return time step
  return  dt;
}

template< class V, class Model >
template< class Arg >
inline double FiniteVolumeScheme< V, Model >
  ::operator() ( const double time, const Arg &solution, Vector &update ) const
{
  // if model does not have numerical flux we have nothing to do here
  if( ! Model::hasFlux ) return model_.fixedDt();

  // time step size (using std:min(.,dt) so set to maximum)
  double dt = std::numeric_limits<double>::infinity();

  // compute update vector and optimum dt in one grid traversal
  const Iterator endit = gridView().template end< 0, ptype >();
  for( Iterator it = gridView().template begin< 0, ptype >(); it != endit; ++it )
    apply( *it, time, solution, update, dt );

  // return time step
  return  dt;
}

template< class V, class Model >
template< class Arg >
inline void FiniteVolumeScheme< V, Model >
  ::apply ( const Entity &entity,
              const double time, const Arg &solution, Vector &update, double &dt ) const
{
  if ( ! update.visitElement( entity ) )
    return;

  const Geometry &geo = entity.geometry();

  // estimate for wave speed
  double waveSpeed = 0.0;

  // cell volume
  const double enVolume = geo.volume();

  // 1 over cell volume
  const double enVolume_1 = 1.0/enVolume;

  // run through all intersections with neighbors and boundary
  const IntersectionIterator iitend = gridView().iend( entity );
  for( IntersectionIterator iit = gridView().ibegin( entity ); iit != iitend; ++iit )
  {
    const Intersection &intersection = *iit;
    /* Fetch the intersection's geometry and reference element */
    const IntersectionGeometry &intersectionGeometry = intersection.geometry();

    /* Get some geometrical information about the intersection */
    const GlobalType point = intersectionGeometry.center();
    const GlobalType normal = intersection.centerUnitOuterNormal();
    const double faceVolume = intersection.geometry().volume();

    // handle interior face
    if( intersection.neighbor() )
    {
      // access neighbor
      const Entity &neighbor = intersection.outside();

      // compute flux from one side only
      if( update.visitElement( neighbor ) )
      {
        // calculate (1 / neighbor volume)
        const double nbVolume = neighbor.geometry().volume();
        const double nbVolume_1 = 1.0 / nbVolume;

        // evaluate data
        const RangeType uLeft  = solution.evaluate( entity, point );
        const RangeType uRight = solution.evaluate( neighbor, point );
        // apply numerical flux
        RangeType flux;
        double ws = model_.numericalFlux( normal, time, point, uLeft, uRight, flux );
        waveSpeed = ws * faceVolume;

        // calc update of entity
        update[ entity ].axpy( -enVolume_1 * faceVolume, flux );
        // calc update of neighbor
        update[ neighbor ].axpy( nbVolume_1 * faceVolume, flux );

        // compute dt restriction
        dt = std::min( dt, std::min( enVolume, nbVolume ) / waveSpeed );
      }
    }
    // handle boundary face
    else
    {
      // evaluate data
      const RangeType uLeft = solution.evaluate( entity, point );
      // apply boundary flux
      RangeType flux;
      double ws = model_.boundaryFlux( normal, time, point, uLeft, flux );
      waveSpeed = ws * faceVolume;

      // calc update on entity
      update[ entity ].axpy( -enVolume_1 * faceVolume, flux );

      // compute dt restriction
      dt = std::min( dt, enVolume / waveSpeed );
    }
  } // end all intersections

  // mark entity as done
  update.visited( entity );
}

template< class V, class Model >
inline size_t FiniteVolumeScheme< V, Model >
  ::mark ( const double time, const Vector &solution, GridMarker<Grid> &marker ) const
{
  size_t elements = 0;

  // clear grid markers internal flags
  marker.reset();

  // grid traversal
  const Iterator endit = gridView().template end< 0, ptype >();
  for( Iterator it = gridView().template begin< 0, ptype >(); it != endit; ++it, ++elements )
  {
    const Entity &entity = *it;

    // if marked for refinement nothing has to be done for this element
    if( marker.get( entity ) > 0 )
      continue;

    // maximum value of the indicator over all intersections
    double entityIndicator = 0.0;

    // need the value on the entity
    const RangeType &uLeft = solution[ entity ];

    // run through all intersections with neighbors and boundary
    const IntersectionIterator iiterend = gridView().iend( entity );
    for( IntersectionIterator iiter = gridView().ibegin( entity ); iiter != iiterend; ++iiter )
    {
      const Intersection &intersection = *iiter;

      // indicator for this intersection
      double localIndicator = 0.0;

      // geometry for this intersection
      const IntersectionGeometry &intersectionGeometry = intersection.geometry();
      // no neighbor?
      if( !intersection.neighbor() )
      {
        const GlobalType point = intersectionGeometry.center();
        GlobalType normal = intersection.centerUnitOuterNormal();
        // compute indicator for intersection
        localIndicator = model_.boundaryIndicator( normal, time, point, uLeft );
      }
      else
      {
        // access neighbor
        const Entity &neighbor = intersection.outside();

        const RangeType &uRight = solution[ neighbor ];

        const GlobalType point = intersectionGeometry.center();
        GlobalType normal = intersection.centerUnitOuterNormal();
        // compute indicator for this intersection
        localIndicator  = model_.indicator( normal, time, point, uLeft, uRight );
      }

      // for coarsening we need maximum indicator over all intersections
      entityIndicator = std::max( entityIndicator, localIndicator );

      // test if we can mark for refinement and quit this entity
      if( localIndicator > model_.problem().refineTol() )
      {
        marker.refine( entity );
        // we can now continue with next entity
        break;
      }
    } // end of loop over intersections

    // now see if this entity can be removed
    if( entityIndicator < model_.problem().coarsenTol() )
    {
      marker.coarsen( entity );
    }
  } // end of loop over entities

  // return number of elements
  return elements;
}

#endif // #ifndef FVSCHEME_HH
