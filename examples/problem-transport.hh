#ifndef PROBLEM_TRANSPORT_HH
#define PROBLEM_TRANSPORT_HH

#include <iostream>
#include <sstream>
#include <memory>

#include <dune/common/fvector.hh>

#include "problem.hh"

/**
 * \brief Smooth initial data problem:
    \f$ \sin( 2\pi x\cdot x ) \f$
 */
template< int dimD >
class TransportProblemData1
: public ProblemData< dimD, 1 >
{
  typedef ProblemData< dimD, 1 > Base;

public:
  const static int dimDomain = Base::dimDomain;
  const static int dimRange = Base::dimRange;

  typedef typename Base::DomainType DomainType;
  typedef typename Base::RangeType RangeType;

protected:
  std::string gridFile_;
  DomainType velocity_;
public:
  TransportProblemData1( const int problem )
  {
    if( problem == 3 )
    {
      gridFile_ = "/dgf/periodic" + std::to_string(dimDomain) + ".dgf";
    }
    else
    {
      gridFile_ = "/dgf/unitcube" + std::to_string(dimDomain) + "d.dgf";
    }

    if( problem < 3 )
    {
      // set transport velocity
      velocity_ = DomainType( 1.25 );
    }
    else
    {
      velocity_ = 0.0;
      velocity_[ 0 ] = 1.25;
    }
  }

  virtual void velocity ( const DomainType &x, double time, const RangeType& u, DomainType& v ) const
  {
    v = velocity_;
  }

  //! \copydoc ProblemData::initial
  RangeType initial ( const DomainType &x ) const
  {
    return sin( 2 * M_PI * (x*x) );
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 0.8;
  }

  std::string gridFile ( const std::string &path, const int mpiSize ) const
  {
    return path + gridFile_;
  }

  RangeType boundaryValue ( const DomainType &x, double time ) const
  {
    return initial( x );
  }

  int bndType( const DomainType &normal, const DomainType &x, const double time) const
  {
    return 1;
  }

  double saveInterval () const
  {
    return 0.05;
  }

  //! \copydoc ProblemData::refineTol
  double refineTol () const
  {
    return 0.1;
  }
  //! \copydoc ProblemData::adaptationIndicator
  double adaptationIndicator ( const DomainType& x, double time,
                               const RangeType &uLeft, const RangeType &uRight ) const
  {
    return std::abs( uLeft[ 0 ] - uRight[ 0 ] );
  }
};

/**
 * \brief Discontinuous initial data problem: characteristic function for
    \f$ \{ x\colon |x| < \frac{1}{2} \} \f$
 */
template< int dimD >
class TransportProblemData2
: public ProblemData< dimD, 1 >
{
  typedef ProblemData< dimD, 1  > Base;

public:
  const static int dimDomain = Base::dimDomain;
  const static int dimRange = Base::dimRange;

  typedef typename Base::DomainType DomainType;
  typedef typename Base::RangeType RangeType;

protected:
  std::string gridFile_;
  DomainType velocity_;
public:
  TransportProblemData2( const int problem )
  {
    if( problem == 4 )
    {
      gridFile_ = "/dgf/periodic" + std::to_string(dimDomain) + ".dgf";
    }
    else
    {
      gridFile_ = "/dgf/unitcube" + std::to_string(dimDomain) + "d.dgf";
    }

    if( problem < 3 )
    {
      // set transport velocity
      velocity_ = DomainType( 1.25 );
    }
    else
    {
      velocity_ = 0.0;
      velocity_[ 0 ] = 1.25;
    }
  }

  virtual void velocity ( const DomainType &x, double time, const RangeType& u, DomainType& v ) const
  {
    v = velocity_;
  }

  //! \copydoc ProblemData::initial
  RangeType initial ( const DomainType &x ) const
  {
    DomainType r(0.5);
    return ((x-r).two_norm() < 0.25 ? RangeType( 1 ) : RangeType( 0 ) );
    //return ((x-r).two_norm() < 0.5 ? RangeType( 1 ) : RangeType( 0 ) );
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 0.8;
  }

  //! \copydoc ProblemData::gridFile
  std::string gridFile ( const std::string &path, const int mpiSize ) const
  {
    return path + gridFile_;
  }

  RangeType boundaryValue ( const DomainType &x, double time ) const
  {
    return initial( x );
  }

  int bndType( const DomainType &normal, const DomainType &x, const double time) const
  {
    return 1;
  }

  double saveInterval () const
  {
    return 0.05;
  }
  //! \copydoc ProblemData::refineTol
  double refineTol () const
  {
    return 0.1;
  }
  double adaptationIndicator ( const DomainType& x, double time,
                               const RangeType &uLeft, const RangeType &uRight ) const
  {
    return std::abs( uLeft[ 0 ] - uRight[ 0 ] );
  }
};

/**
 * \brief Solid body rotation
 */
template< int dimD >
class SolidBodyRotation
: public ProblemData< dimD, 1 >
{
  typedef ProblemData< dimD, 1  > Base;

  std::string gridFile_;
public:
  SolidBodyRotation( const int problem )
  {
    gridFile_ = "/dgf/unitcube" + std::to_string(dimDomain) + "d.dgf";
  }

  const static int dimDomain = Base::dimDomain;
  const static int dimRange = Base::dimRange;

  typedef typename Base::DomainType DomainType;
  typedef typename Base::RangeType RangeType;

  virtual void velocity ( const DomainType &x, double time, const RangeType& u, DomainType& v ) const
  {
    //v = velocity_;
    double angular_velocity = 1.0;
    DomainType rotation_centre = DomainType(0.5);
    DomainType radius = x - rotation_centre;
    v[0] = -angular_velocity*radius[1];
    v[1] = angular_velocity*radius[0];
  }

  //! \copydoc ProblemData::initial
  RangeType initial ( const DomainType &x ) const
  {
    RangeType res(0) ;

    DomainType c1( 0.5 ), c2( 0.5 ), c3( 0.5 ), c4( 0.5 );
    c1[ 0 ] = 0.5;  c1[ 1 ] = 0.75;
    c2[ 0 ] = 0.5;  c2[ 1 ] = 0.25;
    c3[ 0 ] = 0.25; c3[ 1 ] = 0.5;
    c4[ 0 ] = 0.35; c4[ 1 ] = 0.65;
    const double r = 0.15;

    // slotted cylinder
    if( (x - c1).two_norm() < r )
    {
      if( (std::abs( x[ 0 ] - c1[ 0 ] ) >= 0.025) || (x[ 1 ] >= c1[ 1 ] + 0.1) )
        return RangeType( 1.0 );
      else
        return RangeType( 0.0 );
    }

    // cone
    if( (x - c2).two_norm() < r )
      return RangeType( (r - (x - c2).two_norm()) / r );

    // hump
    if( (x - c3).two_norm() < r )
      return RangeType((1.0 + std::cos( M_PI * (x - c3).two_norm() / r )) / 4.0 );

    // default
    return RangeType( 0.0 );
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 1.5708;
  }

  //! \copydoc ProblemData::gridFile
  std::string gridFile ( const std::string &path, const int mpiSize ) const
  {
    return path + gridFile_;
  }

  RangeType boundaryValue ( const DomainType &x, double time ) const
  {
    return initial( x );
  }

  int bndType( const DomainType &normal, const DomainType &x, const double time) const
  {
    return 1;
  }

  double saveInterval () const
  {
    return 0.05;
  }
  //! \copydoc ProblemData::refineTol
  double refineTol () const
  {
    return 0.1;
  }
  double adaptationIndicator ( const DomainType& x, double time,
                               const RangeType &uLeft, const RangeType &uRight ) const
  {
    return std::abs( uLeft[ 0 ] - uRight[ 0 ] );
  }
};

// TransportModel
// ----------------

/** \class TransportModel
 *  \brief description of a transport problem
 *
 *  This class describes the following transport problem:
 *  \f{eqnarray*}
 *  \partial_t c + \nabla \cdot (v c)
 *    &=& 0 \quad\mbox{in $\Omega \times ]0,T[$}\\
 *  c &=& g \quad\mbox{on $\Gamma_{\mathrm{in}}$}\\
 *  c &=& c_0 \quad\mbox{on $\Omega \times \lbrace 0 \rbrace$}
 *  \f}
 */
template< int dimD >
struct TransportModel
{
  typedef ProblemData< dimD,1 > Problem;

  typedef typename Problem::DomainType DomainType;
  typedef typename Problem::RangeType RangeType;

  static const int dimDomain = Problem::dimDomain;
  static const int dimRange = Problem::dimRange;
  static const bool hasFlux = true;

  /** \brief constructor
   *  \param problem switch between different data settings
   */
  TransportModel ( int problem )
  {
    switch( problem )
    {
    case 1:
    case 3:
      problem_.reset( new TransportProblemData1< dimDomain >( problem ) );
      break;
    case 2:
    case 4:
      problem_.reset( new TransportProblemData2< dimDomain >( problem ) );
      break;
    case 5:
      problem_.reset( new SolidBodyRotation< dimDomain >( problem ) );
      break;
    default:
      std::cerr << "Problem " << problem << " does not exists." << std::endl;
      std::abort();
    }
  }

  /** \brief obtain problem */
  const Problem &problem () const
  {
    return *problem_;
  }

  double fixedDt () const
  {
    return -1;
  }

  /** \brief evaluate the numerical flux on an intersection
   *
   *  \param[in]   normal   scaled normal of the intersection
   *  \param[in]   time     current time
   *  \param[in]   xGlobal  evaluation point in global coordinates
   *  \param[in]   uLeft    value of the solution in the inside entity
   *  \param[in]   uRight   value of the solution in the outside entity
   *  \param[out]  flux     numercial flux
   *
   *  \returns the maximum wave speed
   */
  double numericalFlux ( const DomainType &normal,
                         const double time,
                         const DomainType &xGlobal,
                         const RangeType &uLeft, const RangeType &uRight,
                         RangeType &flux ) const
  {
    DomainType velocity( 0 );
    problem().velocity( xGlobal, time, uLeft, velocity );
    const double upwind = normal * velocity;
    flux = upwind * (upwind > 0 ? uLeft : uRight);
    return std::abs( upwind );
  }

  /** \brief evaluate the numerical flux on a boundary
   *
   *  \param[in]   normal   scaled normal of the boundary
   *  \param[in]   time     current time
   *  \param[in]   xGlobal  evaluation point in global coordinates
   *  \param[in]   uLeft    value of the solution in the inside entity
   *  \param[out]  flux     numercial flux
   *
   *  \returns the maximum wave speed
   */
  double boundaryFlux ( const DomainType &normal,
                        const double time,
                        const DomainType &xGlobal,
                        const RangeType& uLeft,
                        RangeType &flux ) const
  {
    // exact solution is u0(x-ta)
    DomainType x0( xGlobal );
    DomainType velocity( 0 );
    problem().velocity( xGlobal, time, uLeft, velocity );
    x0.axpy( -time, velocity );
    RangeType uRight = problem().boundaryValue( x0, time );
    return numericalFlux( normal, time, xGlobal, uLeft, uRight, flux );
  }

  /** \brief compute adaptation indicator at intersection
   *
   *  \param[in]   normal   scaled normal of the intersection
   *  \param[in]   time     current time
   *  \param[in]   xGlobal  evaluation point in global coordinates
   *  \param[in]   uLeft    value of the solution in the inside entity
   *  \param[in]   uRight   value of the solution in the outside entity
   *
   *  \return value of indicator
   */
  double indicator ( const DomainType &normal,
                     const double time,
                     const DomainType &xGlobal,
                     const RangeType &uLeft, const RangeType &uRight) const
  {
    return problem().adaptationIndicator( xGlobal, time, uLeft, uRight );
  }

  /** \brief compute adaptation indicator at boundary
   *
   *  \param[in]   normal   scaled normal of the intersection
   *  \param[in]   time     current time
   *  \param[in]   xGlobal  evaluation point in global coordinates
   *  \param[in]   uLeft    value of the solution in the inside entity
   *
   *  \return value of indicator
   */
  double boundaryIndicator ( const DomainType &normal,
                             const double time,
                             const DomainType &xGlobal,
                             const RangeType& uLeft) const
  {
    DomainType x0( xGlobal );
    DomainType velocity( 0 );
    problem().velocity( xGlobal, time, uLeft, velocity );
    x0.axpy( -time, velocity );
    return indicator( normal,time,xGlobal, uLeft, problem().boundaryValue(x0,time) );
  }

protected:
  TransportModel ( ) : problem_() {}
private:
  std::unique_ptr< Problem > problem_;
}; // end class TransportModel

#endif // PROBLEM_TRANSPORT_HH
