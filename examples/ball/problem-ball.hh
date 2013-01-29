#ifndef Problem_EULER_HH
#define Problem_EULER_HH

#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>

#include <dune/common/fvector.hh>

#include "problem.hh"

template< int dimD >
struct BallData
: public ProblemData< dimD >
{
  typedef ProblemData< dimD > Base;

  typedef typename Base::DomainType DomainType;

  const static int dimDomain = DomainType::dimension;

  BallData ()
  {}

  //! \copydoc ProblemData::gridFile
  std::string gridFile ( const std::string &path ) const
  { 
    std::ostringstream dgfFileName;
    dgfFileName << path << "/unitcube" << dimDomain << "d.dgf";
    return dgfFileName.str();
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 1.0;
  }

  //! \copydoc ProblemData::adaptationIndicator
  double adaptationIndicator ( double time, const DomainType &x ) const
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
struct BallModel
{
  typedef ProblemData< dimD > Problem;

  typedef typename Problem::DomainType DomainType;

  const static int dimDomain = DomainType::dimension;

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

  /** \copydoc TransportProblem::indicator */
  double indicator ( const double time,
                     const DomainType &xGlobal ) const
  {
    return problem().adaptationIndicator( time, xGlobal );
  }

  Problem *problem_;
};

#endif // #ifndef EULERFLUXES_HH
