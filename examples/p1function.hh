#if 0
#ifndef P1FUNCTION_HH
#define P1FUNCTION_HH

#include <vector>
#include <fstream>

#include <dune/geometry/referenceelements.hh>
#include "piecewisefunction.hh"

template< class View, class Range >
struct P1Function;

template< class View, int dimR >
struct P1Function< View, Dune::FieldVector< double, dimR > >
{ 
  typedef P1Function< View, Dune::FieldVector< double, dimR > > This;
  
  typedef View GridView;
  static const int dimRange = dimR;

  static const unsigned int order = 1;

  /* first we extract the dimensions of the grid */
  static const int dim = GridView::dimension;
  static const int dimworld = GridView::dimensionworld;

  /* get type of iterator over leaf entities of codimension zero */
  typedef typename GridView::template Codim< 0 >::Iterator Iterator;

  /* types of entity and geometry */
  typedef typename GridView::template Codim< 0 >::Entity Entity;
  typedef typename GridView::template Codim< 0 >::Geometry Geometry;

  /* types for range vectors and gradients */
  typedef Dune::FieldVector< double, dim > DomainType;
  typedef Dune::FieldVector< double, dimworld > GlobalType;
  typedef Dune::FieldVector< double, dimRange > RangeType;
  typedef Dune::FieldMatrix< double, dimRange, dimworld > GradientType;
  typedef Dune::FieldMatrix< double, dimworld, dim > GeometryJacobianType;

  /* type for average values and gradient */
  typedef PiecewiseFunction< GridView, RangeType > P0Data;
  typedef PiecewiseFunction< GridView, GradientType > P0Gradient;

public:
  // constructor
  // how to reset average values without
  // i)  storing a complete new vector
  // ii) reserving new memory each time
  // -> stacking or store average as pointer
  P1Function ( const P0Data &average, const P0Gradient &gradient )
  : average_( average ),
    gradient_( gradient )
  {}

  // allow data access to average
  const RangeType &operator[] ( const Entity &entity ) const
  {
    return average_[ entity ];
  }

  // allow data access to gradient
  // on reference element
  const GradientType &gradient ( const Entity &entity ) const
  {
    return gradient_[ entity ];
  }

  RangeType evaluate ( const Entity &entity, const DomainType &x ) const
  {
    DomainType x0 = baryCenter( entity );
    x0 -= x;

    RangeType ret = average_[ entity ];
    gradient_[ entity ].umv( x0, ret );
    return ret;
  }

  // return the gradient in global coordinates
  GradientType evaluateGradient( const Entity &entity,
                                 const DomainType &x ) const 
  {
    const GeometryJacobianType jacobianInverseT
      =  entity.geometry().jacobianInverseTranspose( x );

    GradientType ret;
    Dune::FMatrixHelp::multMatrix( jacobianInverseT, gradient_[ entity ], ret );
    return ret;
  }

private:   
  // get barycenter on reference element
  DomainType baryCenter( const Entity &entity ) const
  {
    typedef Dune::ReferenceElement< double, dim > ReferenceElement;
    typedef Dune::ReferenceElements< double, dim > ReferenceElements;

    const ReferenceElement &refElement = ReferenceElements::general( entity.type() );
    return refElement.position( 0, 0 );
  }

  const P0Data average_;
  const P0Gradient gradient_;
};

#endif // #ifndef P1FUNCTION_HH
#endif
