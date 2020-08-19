#ifndef Problem_BALL_HH
#define Problem_BALL_HH

#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>
#include <memory>

#include <dune/common/fvector.hh>

#include "problem.hh"
#include "problem-transport.hh"

template< int dimD >
struct BallData
: public ProblemData< dimD,1 >
{
  typedef ProblemData< dimD,1 > Base;

  typedef typename Base::DomainType DomainType;
  typedef typename Base::RangeType RangeType;

  const static int dimDomain = DomainType::dimension;

  explicit BallData (const int problem)
    : c_(0.5), r0_(0.3), problem_( problem ) //dimDomain == 3 ? problem : 0)
  {}

  //! \copydoc ProblemData::gridFile
  std::string gridFile ( const std::string &path, const int mpiSize ) const
  {
    std::ostringstream dgfFileName;
    if( dimDomain == 3 && problem_ < 4 )
    {
      if( problem_ == 1 )
        dgfFileName << path << "/dgf/cube_hc_512.dgf";
      else if ( problem_ == 2 )
        dgfFileName << path << "/dgf/cube_hc_4096.dgf";
      else if ( problem_ == 3 )
        dgfFileName << path << "/dgf/cube_hc_32768.dgf";
    }
    else if ( problem_ == 4 )
      dgfFileName << path << "/dgf/input" << dimDomain << ".dgf";
    else if ( problem_ == 5 )
      dgfFileName << path << "/dgf/periodic" << dimDomain << ".dgf";
    else
      dgfFileName << path << "/dgf/unitcube" << dimDomain << "d.dgf";
    return dgfFileName.str();
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 1.5;
  }

  int bndType( const DomainType &normal, const DomainType &x, const double time) const
  {
    return 0;
  }

  //! \copydoc ProblemData::adaptationIndicator
  double adaptationIndicator ( const DomainType& x, double time,
                               const RangeType& uLeft, const RangeType &uRight ) const
  {
    DomainType xx(x);
    xx -= c_;
    DomainType y(0);

    if( problem_ < 5 )
    {
      y[0] = std::cos(time*2.*M_PI)*r0_;
      y[1] = std::sin(time*2.*M_PI)*r0_;
    }
    else
    {
      // lateral translation of marking zone
      int inter = int(time);
      y[0] = time - double(inter) -0.5;
    }

    xx -= y;
    double r = xx.two_norm();
    return ( (r>0.15 && r<0.25)? 1 : 0 );
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

private:
  DomainType c_;
  double r0_;
  const int problem_;
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
  : problem_( new BallData< dimDomain >( problem ) )
  {
  }

  /** \copydoc TransportProblem::data */
  const Problem &problem () const
  {
    return *problem_;
  }

  double fixedDt () const
  {
    return 0.01 / 0.15;
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
  double boundaryIndicator ( const DomainType &normal,
                             const double time,
                             const DomainType &xGlobal,
                             const RangeType& uLeft) const
  {
    return indicator( normal,time,xGlobal, uLeft, problem().boundaryValue(xGlobal,time) );
  }

  std::unique_ptr< Problem > problem_;
};

#endif
