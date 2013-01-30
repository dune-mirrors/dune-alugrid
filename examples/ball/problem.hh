#ifndef PROBLEM_HH
#define PROBLEM_HH

#include <sstream>

#include <dune/common/fvector.hh>

/** \class ProblemData
 *  \brief virtual base class for our problems
 *
 *  \tparam  dimD  dimension of the domain
 */
template< int dimD >
struct ProblemData
{
  // dimension of domain and range
  static const int dimDomain = dimD;

  // type of domain and range vectors
  typedef double DomainFieldType;
  typedef double RangeFieldType;
  typedef Dune::FieldVector< DomainFieldType , dimDomain > DomainType;

  /** \brief virtual destructor */
  virtual ~ProblemData() {}

  /** \brief obtain the file name of the macro grid for this problem
   *
   *  \param[in]  path  path to the macro grids
   *
   *  \returns the file name of the macro grid
   */
  virtual std::string gridFile ( const std::string &path ) const = 0;

  /** \brief obtain the end time for the evolution problem */
  virtual double endTime () const = 0;

  /** \brief obtain the interval for writing data to disk */
  virtual double saveInterval () const = 0;

  /** \brief compute a jump type indicator 
   *
   *  \param uLeft left state
   *  \param uRight right state
   *  \return the indicator
   */
  virtual double adaptationIndicator ( double time, const DomainType &x ) const = 0;
  /** \brief refine tolerance to use */
  virtual double refineTol () const = 0;
  /** \brief coarsening tolerance (defaults to 1/3 of refinement tolerance 
   *         Default implementation is 1/3 of the refinement tolerance
   */
  virtual double coarsenTol () const 
  {
    return refineTol()/3.;
  }
}; // end class ProblemData
// Code moved to problem-transport.hh 

#endif // PROBLEM_HH
