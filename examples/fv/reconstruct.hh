#if 0
#ifndef RECONSTRUCT_HH
#define RECONSTRUCT_HH

// Reconstruction on an entity
// ---------------------------

/* \class ReconstructOperator
 * \brief Class which calculates the linear reconstruction
 *        in one entity, using minmod limiter
 *
 * This class provides a method to calculate the linear reconstruction 
 * in one entity. The reconstruction is limited by the minmod limiter.
 */

template < class GridView, class Problem >
class ReconstructOperator
{
  typedef GridView GridViewType;
  // some basic typdefs we need
  typedef typename GridView::Grid Grid;  
  static const int dim = GridViewType::dimension;
  static const int dimworld = GridViewType::dimensionworld;
  static const int dimRange = Problem::dimRange;  
  typedef typename GridViewType::Grid::ctype ctype;

  // types of Codim zero entity iterator and geometry
  typedef typename GridView::template Codim< 0 >::Iterator Iterator;
  typedef typename Iterator::Entity Entity;
  typedef typename Entity::EntityPointer EntityPointer;
  typedef typename Entity::Geometry Geometry;

  // type of intersections and corresponding geometries
  typedef typename GridView::IntersectionIterator IntersectionIterator;
  typedef typename IntersectionIterator::Intersection Intersection;
  typedef typename Intersection::Geometry IntersectionGeometry;

  // type of vectors and matrices
  typedef typename Problem::RangeType RangeType;
  typedef typename RangeType::field_type RangeFieldType;    
  typedef Dune::FieldVector< ctype, dim > DomainType;
  typedef Dune::FieldVector< ctype, dimworld > GlobalType;      
  typedef Dune::FieldMatrix< RangeFieldType, dim , dim > MatrixType;
  typedef Dune::FieldVector< DomainType , dimRange > GradientType;

  const GridViewType gridView_;
  const Problem &problem_;
public:  
  /** \brief constructor
   *  \param gridView the gridView, to know where the neighbors of
   *            an entity are
   *  \param problem to calculated proper boundary reconstructions
   */
  ReconstructOperator(const GridViewType &gridView, const Problem &problem): 
    gridView_(gridView), 
    problem_(problem)
  {}

  virtual ~ReconstructOperator(){}

  /* \brief returns the reconstruction
   * \param entity The entity on which the reconstruction is calculated
   * \param x The point in which the reconstruction is calculated
   * \param solution The solution to get the values one the neighbors
   * \param time The Time the reconstruction is calced
   */
  template <class VectorType>
  RangeType operator() (const Entity &entity, 
                     const GlobalType &x,
                     const VectorType &solution,
                     const double &time )
  {
    /* the value on the entity it self */
    RangeType enVal = solution[entity];
    /* Some Vectors to store the barycenters and the values of the
     * neighbors */
    std::vector< DomainType > barys ;
    std::vector< RangeType > nbVals ;
    barys.reserve( dim * dim );
    nbVals.reserve( dim * dim );
    // set to size zero since values are determined new
    barys.resize( 0 );
    nbVals.resize( 0 );

    // chache the geometry of the entity
    const Geometry& geo = entity.geometry();

    // number of neighbors for the calc of the gradient
    int nrOfNeighbors = 0;

    // the Barycenter of the entity
    const DomainType enBary = baryCenter(geo) ;

    // get geometry data of neighbors, if existend
    const IntersectionIterator endIt = gridView_.iend(entity);
    for(IntersectionIterator iIt = gridView_.ibegin(entity); iIt != endIt; 
        ++iIt)
    {
      // increase the number of neighbors
      ++nrOfNeighbors;

      // on interior
      if( iIt ->neighbor() )      
      { 
        // get the outside entity
        const EntityPointer outside = iIt->outside();
        const Entity& neighbor = *outside;
        // and its geometry
        const Geometry& geoOut = neighbor.geometry();

        // get vals of the entity outside
        RangeType nbVal = solution[neighbor];      
        nbVal -= enVal;
        // add diffrenc of vals
        nbVals.push_back(nbVal);

        // calc the barycenter outside
        DomainType nbBary = baryCenter(geoOut);
        nbBary -= enBary;
        // add barycenter
        barys.push_back( nbBary );                       
      }
    
      // on boundary
      if( iIt -> boundary())       
      {
        // fetch intersection and its geometry
        const Intersection& intersection = *iIt;
        const IntersectionGeometry &interGeo =
          intersection.intersectionGlobal(); //geometry();

        // the bary center of the face 
        DomainType lambda = baryCenter(interGeo);

        // getting boundary vals from problem
        RangeType nbVal = problem_.data().boundaryValue(x , time);
        nbVal -= enVal;
        nbVals.push_back( nbVal );

        // store the barycenter of the face
        lambda -= enBary;
        barys.push_back(lambda);
      }
    }

    //calculate the Gradients
    std::vector < GradientType > gradients;
    gradients.reserve( dim * dim );
    gradients.resize(0);

    for(int per = 0; per <nrOfNeighbors  ;++per)    
    {
      DomainType x(0);
      GradientType locGradient(x);
      for(int r=0;r<dimRange;++r)
      {
        MatrixType matrix;
        for(int i=0; i<dim; ++i)
        {
          matrix[i] = barys[ (i+per)%nrOfNeighbors ];
        }       
        MatrixType invers;

        RangeFieldType det = 1;
        det = Dune :: FMatrixHelp :: invertMatrix(matrix,invers);
                
        DomainType rhs(0);
        for(int i=0;i<dim;++i)
          rhs[i] = nbVals[ (per+i)%nrOfNeighbors ][r];

        DomainType helper(0);
        invers.umv(rhs, helper);
        locGradient[r] = helper;
      }
      gradients.push_back(locGradient);      
    }

    // get the minimal gradient and limit it
    const GradientType &b = minGrad(gradients);
    RangeFieldType limit = limiter(nbVals);

    RangeType ret (0);   
    for(int r = 0; r< dimRange; ++r)
    {
      ret[r] = b[r] * ( x - enBary );
      ret[r] *= limit;
    }

    ret += enVal;
    return ret;
  }
private:

  /* \brief Gives back the minimal Gradient
   * \param gradients Vector with all gradients inside
   */
  GradientType minGrad ( const std::vector<GradientType> &gradients) const
  {
    const DomainType x(0);
    GradientType ret(x);
    RangeType min(0);

    const int numGradients = gradients.size();
    const int startGradient = 0;
    std::vector< int >  index(dimRange, startGradient);
    
    for(int r=0; r<dimRange; ++r)
    {
      const RangeFieldType firstGradient = gradients[startGradient][r].two_norm();
      min[r] = ( firstGradient == firstGradient) ? firstGradient : std::numeric_limits<double>::infinity();
    }

    for(int i=1; i < numGradients ;++i)
    {
      for(int r=0; r<dimRange;++r)
      {
        const RangeFieldType D_abs = gradients[i][r].two_norm();
        if( min[r] > D_abs )
        { 
          index[r] = i;
          min[r] = D_abs;
        }
      }
    }
    
    for(int r=0; r<dimRange;++r)
    {
      ret[r] = gradients[index[r]][r];
    }
    return ret;
  }

  /* \brief limit value for a given entity, 0 if val is a local max or min
   * \param vals A vector of the diffrence of the values of neighbors to
   * the value on the entity itself
   *
   * The idea here is to limit if the minmal or maximal diffrence is 
   * greater or smaller than zero
   */
  RangeFieldType limiter(const std::vector<RangeType> &vals) const
  {
    RangeFieldType ret ( 1 );
    // var for the maximal and minimal values
    RangeType min ( std::numeric_limits<double>::infinity() );
    RangeType max ( 0 );
    const int numFunctions = vals.size();
    for(int i=0; i< numFunctions; ++i)
    {
      for(int r=0; r<dimRange;++r)
      {
        min[r] = std::min(vals[i][r],min[r]);
        max[r] = std::max(vals[i][r],max[r]);
      }
    }

    for(int r=0;r<dimRange;++r)
    {
      if( min[r] > 0 )
        ret = 0;
      else if( max[r] < 0 )
        ret = 0;
    }
    
    return ret;
  }

  /* \brief calculate the barycenter of an entity by the given geometry
   * \param geometry The geometry of the entity
   */
  template< class GeometryType >
  GlobalType baryCenter(const GeometryType& geometry) const
  {
    GlobalType ret(0);
    const int corners = geometry.corners();
    for (int i=0;i<corners;++i) 
    {
      ret += geometry.corner(i);
    }
    ret /= corners;
    return ret;
  }
};

#endif
#endif
