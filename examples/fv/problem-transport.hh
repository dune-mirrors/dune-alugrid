#ifndef PROBLEM_TRANSPORT_HH
#define PROBLEM_TRANSPORT_HH

#include <iostream>
#include <sstream>

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

  std::string gridFile ( const std::string &path ) const
  { 
    std::ostringstream dgfFileName;
    dgfFileName << path << "/unitcube" << dimDomain << ".dgf";
    return dgfFileName.str();
  }

  RangeType boundaryValue ( const DomainType &x, double time ) const                       
  {                                                                                        
    return initial( x );                                                               
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

  //! \copydoc ProblemData::initial
  RangeType initial ( const DomainType &x ) const
  {
    DomainType r(0);
    return ((x-r).two_norm() < 0.5 ? RangeType( 1 ) : RangeType( 0 ) );
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 0.8;
  }

  std::string gridFile ( const std::string &path ) const
  { 
    std::ostringstream dgfFileName;
    dgfFileName << path << "/unitcube" << dimDomain << ".dgf";
    return dgfFileName.str();
  }

  RangeType boundaryValue ( const DomainType &x, double time ) const                       
  {                                                                                        
    return initial( x );                                                               
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

  static const int dimDomain = Problem::dimDomain;
  static const int dimRange = Problem::dimRange;

  typedef typename Problem::DomainType DomainType;
  typedef typename Problem::RangeType RangeType;

  /** \brief constructor 
   *  \param problem switch between different data settings 
   */
  TransportModel ( int problem )
  {
    switch( problem )
    {
    case 1:
      problem_ = new TransportProblemData1< dimDomain >();
      break;
    case 2:
      problem_ = new TransportProblemData2< dimDomain >();
      break;
    default:
      std::cerr << "Problem " << problem << " does not exists." << std::endl;
      exit( 1 );
    }

    // set transport velocity
    velocity_ = DomainType( 1.25 );
  }

  /** \brief destructor
   */
  ~TransportModel ()
  {
    delete problem_;
  }

  /** \brief obtain problem */
  const Problem &problem () const
  {
    return *problem_;
  }

  /** \brief obtain the (constant) velocity for the transport problem */
  const DomainType &velocity () const
  {
    return velocity_;
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
    const double upwind = normal * velocity();
    flux = upwind * (upwind > 0 ? uLeft : uRight);
    return std::abs( upwind );
  }

  /** \brief evaluate the numerical flux on a boundary
   *
   *  \param[in]   bndId    boundary id
   *  \param[in]   normal   scaled normal of the boundary
   *  \param[in]   time     current time
   *  \param[in]   xGlobal  evaluation point in global coordinates
   *  \param[in]   uLeft    value of the solution in the inside entity
   *  \param[out]  flux     numercial flux
   *
   *  \returns the maximum wave speed
   */
  double boundaryFlux ( const int bndId, 
                        const DomainType &normal, 
                        const double time,
                        const DomainType &xGlobal,
                        const RangeType& uLeft,
                        RangeType &flux ) const
  {
    // exact solution is u0(x-ta)
    DomainType x0( xGlobal );                                                                    
    x0.axpy( -time, velocity_ );                                                           
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
    return problem().adaptationIndicator( uLeft, uRight );
  }

  /** \brief compute adaptation indicator at boundary
   *
   *  \param[in]   bndId    boundary id
   *  \param[in]   normal   scaled normal of the intersection
   *  \param[in]   time     current time
   *  \param[in]   xGlobal  evaluation point in global coordinates
   *  \param[in]   uLeft    value of the solution in the inside entity
   *
   *  \return value of indicator
   */
  double boundaryIndicator ( const int bndId, 
                             const DomainType &normal, 
                             const double time,
                             const DomainType &xGlobal,
                             const RangeType& uLeft) const
  {
    return indicator( normal,time,xGlobal, uLeft, problem().boundaryValue(xGlobal,time) );
  }

private:
  Problem *problem_;
  DomainType velocity_;
}; // end class TransportModel

#endif // PROBLEM_TRANSPORT_HH
