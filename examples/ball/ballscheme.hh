#ifndef FVSCHEME_HH
#define FVSCHEME_HH

#include <limits>
#include <dune/common/fvector.hh>

#include <dune/grid/common/gridenums.hh>

#include "adaptation.hh"

template < class GridView, class Model >
struct BallScheme
{
  // first we extract some types
  typedef typename GridView::Grid Grid;
  static const int dim = GridView::dimension;
  static const int dimworld = GridView::dimensionworld;
  static const int dimRange = Model::dimRange;
  typedef typename Grid::ctype ctype;

  // types of codim zero entity iterator and geometry
  typedef typename GridView::template Codim< 0 >::Iterator  Iterator;
  typedef typename Iterator::Entity                         Entity;
  typedef typename Entity::EntityPointer                    EntityPointer;
  typedef typename Entity::Geometry                         Geometry;

  // type of intersections and corresponding geometries
  typedef typename GridView::IntersectionIterator       IntersectionIterator;
  typedef typename IntersectionIterator::Intersection   Intersection;
  typedef typename Intersection::Geometry               IntersectionGeometry;

  // types of vectors
  typedef Dune::FieldVector< ctype, dim-1 >      FaceDomainType;
  typedef Dune::FieldVector< ctype, dim >        DomainType;
  typedef Dune::FieldVector< ctype, dimworld >   GlobalType;

public:
  /** \brief constructor
   *
   *  \param[in]  gridView  gridView to operate on
   */
  BallScheme ( const GridView &gridView, const Model &model )
  : gridView_( gridView )
  , model_( model )
  {}

  double operator()(double time) const
  {
    return 0.01;
  }
  /** \brief set grid marker for refinement / coarsening 
   *
   *  \param[in]  time      current time
   *  \param      marker    grid marker
   *
   *  \note The marker is responsible for limiting the grid depth.
   */
  void
  mark ( const double time, GridMarker< Grid > &marker ) const;

  /** \brief obtain the grid view for this scheme
   *
   *  \returns the grid view
   */
  const GridView &gridView () const
  {
    return gridView_;
  }

private:
  const GridView gridView_;
  const Model &model_;
}; // end FiniteVolumeScheme

template< class GridView, class Model > 
inline void BallScheme< GridView, Model >
  ::mark ( const double time, GridMarker<Grid> &marker ) const
{
  // grid traversal
  const Iterator endit = gridView().template end< 0 >();     
  for( Iterator it = gridView().template begin< 0 >(); it != endit; ++it )
  {
    const Entity &entity = *it;

    // if marked for refinement nothing has to be done for this element
    if( marker.get( entity ) > 0 )
      continue;
    
    // maximum value of the indicator over all intersections
    double entityIndicator = model_.indicator( time, entity.geometry().center() );

    // test if we can mark for refinement and quit this entity
    if( entityIndicator > model_.problem().refineTol() )
    {
      marker.refine( entity );
      // might be a good idea to refine a slightly larger region
      // marker.refineNeighbors( gridView(), entity );
      // we can now continue with next entity
    }
    // now see if this entity can be removed
    if( entityIndicator < model_.problem().coarsenTol() )
    {
      marker.coarsen( entity );
    }
  } // end of loop over entities
}

#endif // #ifndef FVSCHEME_HH
