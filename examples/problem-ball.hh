#ifndef Problem_BALL_HH
#define Problem_BALL_HH

#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>

#include <dune/common/fvector.hh>

#include "problem.hh"

template< int dimD >
struct BallData
: public ProblemData< dimD,1 >
{
  typedef ProblemData< dimD,1 > Base;

  typedef typename Base::DomainType DomainType;
  typedef typename Base::RangeType RangeType;

  const static int dimDomain = DomainType::dimension;

  BallData ()
  {}

  //! \copydoc ProblemData::gridFile
  std::string gridFile ( const std::string &path ) const
  { 
    std::ostringstream dgfFileName;
    dgfFileName << path << "/cube_hc_512.hexa";
   // unitcube" << dimDomain << "d.dgf";
    return dgfFileName.str();
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 1.0;
  }

  int bndType( const DomainType &normal, const DomainType &x, const double time) const 
  {
    return 0;
  }

  //! \copydoc ProblemData::adaptationIndicator
  double adaptationIndicator ( const DomainType x, double time,
                               const RangeType &uLeft, const RangeType &uRight ) const 
  { 
    DomainType xx(x);
    xx -= DomainType(time);
    double r = xx.two_norm();
    return ( (r>0.45 && r<0.55)? 1 : 0 );
  } 

  //! \copydoc ProblemData::refineTol
  double refineTol () const
  {
    return 0.1;
  }

  //! \copydoc ProblemData::saveInterval
  double saveInterval() const 
  {
    return 0.1;
  }
};

// BallModel
// ------------

/** \brief Problem describing the Euler equation of gas dynamics 
 */
template <int dimD>
struct BallModel : public TransportModel<dimD>
{
  typedef ProblemData< dimD,1 > Problem;

  typedef typename Problem::DomainType DomainType;
  typedef typename Problem::RangeType RangeType;

  static const int dimDomain = DomainType::dimension;
  static const int dimRange = RangeType::dimension;
  static const bool hasFlux = false;

  /** \brief constructor 
   *  \param problem switch between different data settings 
   */
  BallModel( unsigned int problem )
  : problem_( 0 )
  {
    switch( problem )
    {
    case 1:
      problem_ = new BallData< dimDomain >();
      break;

    default:
      std::cerr << "ProblemData not defined - using problem 1!" << std::endl;
      problem_ = new BallData< dimDomain >();
    }
  }

  /** \brief destructor */
  ~BallModel() 
  {
    delete problem_;
  }

  /** \copydoc TransportProblem::data */
  const Problem &problem () const
  {
    return *problem_;
  }

  double fixedDt () const
  {
    return 0.01;
  }

  /** \copydoc TransportProblem::indicator */
  double indicator ( const DomainType &normal,
                     const double time,
                     const DomainType &xGlobal,
                     const RangeType &uLeft, const RangeType &uRight) const 
  {
    return problem().adaptationIndicator( xGlobal, time, uLeft, uRight );
  }

  /** \copydoc TransportProblem::boundaryIndicator */
  double boundaryIndicator ( const int bndId, 
                             const DomainType &normal, 
                             const double time,
                             const DomainType &xGlobal,
                             const RangeType& uLeft) const
  {
    return indicator( normal,time,xGlobal, uLeft, problem().boundaryValue(xGlobal,time) );
  }

  Problem *problem_;
};

#endif 
