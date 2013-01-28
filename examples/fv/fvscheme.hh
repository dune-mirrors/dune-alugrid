#ifndef FVSCHEME_HH
#define FVSCHEME_HH

#include <limits>
#include <dune/common/fvector.hh>

#include <dune/grid/common/gridenums.hh>

#include "adaptation.hh"
#include "p1function.hh"

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
    // , grad_( gridView )
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

#if 0
  /** \brief compute the update vector for one time step for reconstructed
   *         finite-volume scheme
   *
   *  This is a specialized version of the previous operator taking
   *  where the argument type (\c solution) is the same as for the
   *  result type (\c update). First a reconstruction is computed
   *  and then the general operator() is called.
   *
   *  \param[in]   time      current time
   *  \param[in]   solution  solution at time <tt>time</tt> 
   *  \param[out]  update    result of the flux computation
   *
   *  \returns maximal time step
   */
  double
  operator() ( const double time, const Vector &solution, Vector &update ) ;
#endif

  /** \brief set grid marker for refinement / coarsening 
   *
   *  \param[in]  time      current time
   *  \param[in]  solution  solution at time <tt>time</tt>
   *  \param      marker    grid marker
   *
   *  \note The marker is responsible for limiting the grid depth.
   */
  void
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
  const GridView gridView_;
  const Model &model_;
  // void reconstruct(double time, const Vector &solution) ;
  // DomainType baryCenter(const Entity &entity) {return // entity.geometry().center();}
  // mutable PiecewiseFunction<GridView,Dune::FieldMatrix<double,RangeType::dimension,dim> > grad_;
  // std::vector<DomainType> C_; // constraint matrix
  // Dune::FieldVector<std::vector<double>,dimRange> b_; // constraint RHS
}; // end FiniteVolumeScheme

#if 0
// Implementation of FiniteVolumeScheme
// ------------------------------------
template< class V, class Model > 
inline void FiniteVolumeScheme< V, Model >
  ::reconstruct ( double time, const Vector &solution) 
{
  grad_.resize();
  grad_.clear();
  /****************************************
   * Assume first dimRange=1
   * Let wi=omega_i-omega_0
   *     ui=ubari-ubar0
   * Then H=ATA and g=ATU mit 
   * with A=(w1,...,wN)^T and U=(u1,...,uN)^T.
   * The constraints are
   * C=(-w1,w1,-w2,w2,...,-wN,wN)^T and
   * b=(b10,b11,b20,b21,...,bN0,bN1)^T satisfying
   *     bi0 = 0,  bi1 = ui with ui>0
   *     bi0 = ui, bi1 = 0  otherwise
   **********************************************
   * H,C are independent of dimRange
   * 1) we store w1,..,wN in C
   * 2) compute H and the full matrix C
   ********************************
   * Storage for dimRange>1:
   * for each r=0..dimRange-1
   * 1) store ui[r] in b[r][i]
   * 2) compute g[r]=ATb[r] and full b[r]
   * 3) compute D[r] = CLS(H^{-1},g[r],C,b[r])
   ************************************************/
  Dune::FieldMatrix<double,dim,dim> H;
  Dune::FieldVector<DomainType,dimRange> g,D;
  const Iterator endit = gridView().template end< 0 >();     
  for( Iterator it = gridView().template begin< 0 >(); it != endit; ++it ) {
    const Entity &entity = *it;
    C_.resize(0);
    for (int r=0;r<dimRange;++r)
      b_[r].resize(0);
    // Step 1
    DomainType omega0 = baryCenter(entity);
    RangeType ubar0   = solution[entity];    
    const IntersectionIterator iitend = gridView().iend( entity ); 
    for( IntersectionIterator iit = gridView().ibegin( entity ); iit != iitend; ++iit )
    {
      const Intersection &intersection = *iit;
      if( intersection.neighbor() )
      {
        // access neighbor
        const EntityPointer outside = intersection.outside();
        const Entity &neighbor = *outside;
        DomainType omegai = baryCenter(neighbor);
        RangeType ubari   = solution[neighbor];    
        C_.push_back(omegai-omega0);
        for (int r=0;r<dimRange;++r)
          b_[r].push_back(ubari[r]-ubar0[r]);
      }
      else
      { // TODO
      }
    }
    // Step 2 
    size_t numberOfNb = C_.size();
    H = 0;
    for (int r=0;r<dimRange;++r)
      g[r] = 0;
    for (size_t i=0;i<numberOfNb;++i) {
      // compute components of quadratic form
      for (int q=0;q<dim;++q)
        for (int p=0;p<dim;++p)
          H[q][p] += C_[i][q]*C_[i][p];
      for (int r=0;r<dimRange;++r) 
        for (int q=0;q<dim;++q)
          g[r][q] += C_[i][q]*b_[r][i];
      // set up constraints
      C_.push_back(C_[i]);
      C_[i] *= -1.;
      for (int r=0;r<dimRange;++r) {
        if (b_[r][i]>0) {
          b_[r].push_back(b_[r][i]);
          b_[r][i]=0.;
        } else {
          b_[r].push_back(0);
          b_[r][i] *= -1.;
        }
      }
    }
    H.invert();
    for (int r=0;r<dimRange;++r)
      D[r] = 0 ; // calc(H,g[r],C,b[r]);
  }
}


template< class V, class Model > 
inline double FiniteVolumeScheme< V, Model >
  ::operator() ( const double time, const Vector &solution, 
                 Vector &update ) 
{
  reconstruct(time,solution);
  P1Function<GridView,typename Vector::RangeType> recon(solution,grad_);
  return (*this)(time,recon,update);
}
#endif

template< class V, class Model > 
template< class Arg >
inline double FiniteVolumeScheme< V, Model >
  ::operator() ( const double time, const Arg &solution, Vector &update ) const
{
  // set update to zero 
  update.clear();

  // time step size (using std:min(.,dt) so set to maximum) 
  double dt = std::numeric_limits<double>::infinity(); 
  
  // compute update vector and optimum dt in one grid traversal
  const Iterator endit = gridView().template end< 0 >();     
  for( Iterator it = gridView().template begin< 0 >(); it != endit; ++it )
  {
    // get entity and geometry
    const Entity &entity = *it;
    const Geometry &geo = entity.geometry();

    // estimate for wave speed
    double waveSpeed = 0.0;

    // cell volume
    const double enVolume = geo.volume(); 
    
    // 1 over cell volume
    const double enVolume_1 = 1.0/enVolume; 

    // index of entity
    unsigned int enIdx = gridView().indexSet().index(entity);

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
        const EntityPointer outside = intersection.outside();
        const Entity &neighbor = *outside;
        unsigned int nbIdx = gridView().indexSet().index(neighbor);

        // compute flux from one side only
        // this should become easier with the new IntersectionIterator functionality!
        if( (entity.level() > neighbor.level())
            || ((entity.level() == neighbor.level()) && (enIdx < nbIdx))
            || (neighbor.partitionType() != Dune::InteriorEntity) )
        {
          // calculate (1 / neighbor volume)
          const double nbVolume = neighbor.geometry().volume();
          const double nbVolume_1 = 1.0 / nbVolume;

          // evaluate data
          const RangeType uLeft = solution.evaluate( entity, point );
          const RangeType uRight = solution.evaluate( neighbor, point );
          // apply numerical flux
          RangeType flux; 
          double ws = model_.numericalFlux( normal, time, point, uLeft, uRight, flux );
          waveSpeed += ws * faceVolume;

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
        double ws = model_.boundaryFlux( intersection.boundaryId(), normal, time, point, uLeft, flux );
        waveSpeed += ws * faceVolume;

        // calc update on entity
        update[ entity ].axpy( -enVolume_1 * faceVolume, flux );

        // compute dt restriction
        dt = std::min( dt, enVolume / waveSpeed );
      }
    } // end all intersections            
  } // end grid traversal                     

  // return time step
  return  dt;
}

template< class V, class Model > 
inline void FiniteVolumeScheme< V, Model >
  ::mark ( const double time, const Vector &solution, GridMarker<Grid> &marker ) const
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
        const int bndId = intersection.boundaryId();

        const GlobalType point = intersectionGeometry.center();
        GlobalType normal = intersection.centerUnitOuterNormal();
        // compute indicator for intersection
        localIndicator = model_.boundaryIndicator( bndId, normal, time, point, uLeft );
      }
      else
      {
        // access neighbor
        const EntityPointer outside = intersection.outside();
        const Entity &neighbor = *outside;
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
        // might be a good idea to refine a slightly larger region
        marker.refineNeighbors( gridView(), entity );
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
}

#endif // #ifndef FVSCHEME_HH
