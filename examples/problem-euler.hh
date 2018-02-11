#ifndef Problem_EULER_HH
#define Problem_EULER_HH

#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>

#include <dune/common/fvector.hh>

#include "problem.hh"

/** \class EulerProblemData1
 *  \brief Mach 3 flow from left hitting a forward facing step
 */
template< int dimD >
struct EulerProblemFFS
: public ProblemData< dimD, dimD+2 >
{
  typedef ProblemData< dimD, dimD+2 > Base;

  typedef typename Base::DomainType DomainType;
  typedef typename Base::RangeType RangeType;

  static const int dimDomain = DomainType::dimension;
  static const int dimRange = RangeType::dimension;
protected:
  const int problem_;

public:
  EulerProblemFFS ( const int problem ) : problem_( problem )
  {}

  //! \copydoc ProblemData::gridFile
  std::string gridFile ( const std::string &path, const int mpiSize ) const
  {
    std::ostringstream dgfFileName;
    if( problem_ < 12 )
      dgfFileName << path << "/dgf/ffs" << dimDomain << "d.dgf";
    else
      dgfFileName << path << "/dgf/ffs" << dimDomain << "d_fine.dgf";
    return dgfFileName.str();
  }

  //! \copydoc ProblemData::initial
  RangeType initial ( const DomainType &x ) const
  {
    // set all values to zero
    RangeType val( 0 );

    val[ 0 ] = 1.4; // rho
    val[ 1 ] = 4.2; // m_1
    val[ dimDomain+1 ] = 8.8; // e

    return val;
  }

  //! \copydoc ProblemData::boundaryValue
  RangeType boundaryValue ( const DomainType &x, double time ) const
  {
    return initial( x );
  }

  int bndType( const DomainType &normal, const DomainType &x, const double time) const
  {
    if (normal[0]<-0.1 && x[0]<0.1)
      return 1; // inflow
    else if (normal[0]>0.1 && x[0]>1)
      return 2; // outflow
    else if (normal[0]>0.1 && x[0] >= 0.59 && x[0] <= 0.61 )
      return 4; // slip

    return 3; // reflection
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 4;
  }

  //! \copydoc ProblemData::adaptationIndicator
  double adaptationIndicator ( const DomainType& x, double time,
                               const RangeType &uLeft, const RangeType &uRight ) const
  {
    return std::abs( uLeft[ 0 ] - uRight[ 0 ] )/(0.5*(uLeft[0]+uRight[0]));
  }

  //! \copydoc ProblemData::refineTol
  double refineTol () const
  {
    return 0.1;
  }

  //! \copydoc ProblemData::saveInterval
  double saveInterval() const
  {
    return 0.025 * endTime();
  }
};

// ShockBubble
template< int dimD >
class EulerProblemShockBubble
: public ProblemData< dimD, dimD+2 >
{
public:
  typedef ProblemData< dimD, dimD+2 > Base;

  typedef typename Base::DomainType DomainType;
  typedef typename Base::RangeType RangeType;

  static const int dimDomain = DomainType::dimension;
  static const int dimRange = RangeType::dimension;

  EulerProblemShockBubble( const int problem )
   : gamma(1.4)
   , center_(0.5)
   , radius2_( 0.2 * 0.2 )
   , problem_( problem )
  {
    center_[dimDomain-1] = 0;
  }

  //! \copydoc ProblemData::gridFile
  std::string gridFile ( const std::string &path, const int mpiSize ) const
  {
    std::ostringstream dgfFileName;
    if( dimD == 3 )
    {
      if( problem_ == 21 )
        dgfFileName << path << "/dgf/cube_hc_512.dgf";
      else if( problem_ == 22 )
        dgfFileName << path << "/dgf/cube_hc_4096.dgf";
      else if( problem_ == 23 )
        dgfFileName << path << "/dgf/cube_hc_32768.dgf";
      else if( problem_ == 25 )
        dgfFileName << path << "/dgf/sb3d_" << mpiSize << ".dgf";
      else
        dgfFileName << path << "/dgf/sb" << dimDomain << "d.dgf";
    }
    else if( dimD == 2 )
    {
      if( problem_ == 22 )
        dgfFileName << path << "/dgf/sb" << dimDomain << "d_6400.dgf";
      else if( problem_ == 23 )
        dgfFileName << path << "/dgf/sb" << dimDomain << "d_65536.dgf";
      else
        dgfFileName << path << "/dgf/sb" << dimDomain << "d.dgf";
    }
    return dgfFileName.str();
  }

  //! \copydoc ProblemData::initial
  RangeType initial ( const DomainType &x ) const
  {
    RangeType val( 0 );

    enum { dimR = RangeType :: dimension };

    // behind shock
    if ( x[0] <= 0.2 )
    {
      const double gamma1 = gamma-1.;
      // pressure left of shock
      const double pinf = 5;
      const double rinf = ( gamma1 + (gamma+1)*pinf )/( (gamma+1) + gamma1*pinf );
      const double vinf = (1.0/std::sqrt(gamma)) * (pinf - 1.)/
              std::sqrt( 0.5*((gamma+1)/gamma) * pinf + 0.5*gamma1/gamma);

      val[0] = rinf;
      val[dimR-1] = 0.5*rinf*vinf*vinf + pinf/gamma1;
      val[1] = vinf * rinf;
    }
    else if( (x - center_).two_norm2() <= radius2_ )
    {
      val[0] = 0.1;
      // pressure in bubble
      val[dimR-1] = 2.5;
    }
    // elsewhere
    else
    {
      val[0] = 1;
      val[dimR-1] = 2.5;
    }
    return val;
  }

  //! \copydoc ProblemData::boundaryValue
  RangeType boundaryValue ( const DomainType &x, double time ) const
  {
    return initial( x );
  }

  int bndType( const DomainType &normal, const DomainType &x, const double time) const
  {
    if (normal[0]<-0.1 && x[0]<0.1)
      return 1;
    else if (normal[0]>0.1 && x[0]>1)
      return 2;
    return 3;
  }

  //! \copydoc ProblemData::endTime
  double endTime () const
  {
    return 0.25;
  }

  //! \copydoc ProblemData::adaptationIndicator
  double adaptationIndicator ( const DomainType& x, double time,
                               const RangeType &uLeft, const RangeType &uRight ) const
  {
    return std::abs( uLeft[ 0 ] - uRight[ 0 ] )/(0.5*(uLeft[0]+uRight[0]));
  }

  //! \copydoc ProblemData::refineTol
  double refineTol () const
  {
    return 0.1;
  }

  //! \copydoc ProblemData::saveInterval
  double saveInterval() const
  {
    return 0.04 * endTime();
  }

  //! only every 10th timestep we want load balancing
  int balanceStep() const { return 25; }

  unsigned int maxTimeSteps() const { return (problem_ == 25) ? 50 : Base::maxTimeSteps(); }

  private:
  const double gamma;
  DomainType center_;
  const double radius2_;
  const int problem_;
};


// Enumerations
// ------------

enum EulerFluxType { LLF, HLL, HLLC };

// EulerFlux
// ---------

/** \class EulerFlux
 *  \brief Compute numerical flux function for Euler equations.
 *
 *  This class wrapps a few C functions using that a RangeType vector \c U
 *  can be transformed to \c double* using &(U[0]) which holds for example
 *  for \c Dune::FieldVector
 *
 *  \tparam dim space dimension
 *  \tparam flux switch between local Lax-Friedrich (LLF), and
 *          Harten-Lax-vanLeer (HLL) flux, and
 *          Harten-Lax-vanLeer-Contact (HLLC) flux
 *
 */
template< int dim, EulerFluxType flux_type = HLLC >
struct EulerFlux
{
  /** \internal
   *  \brief constructor
   *
   *  \param[in]  gamma  adiabatic constant in the pressure equation
   */
  EulerFlux ( double gamma )
  : _gamma( gamma )
  {}

  /** \internal
   *  \brief evaluate the numerical flux
   *
   *  \param[in]   uLeft       left (inside) state
   *  \param[in]   uRight      right (outside) state
   *  \param[in]   unitNormal  unit outer normal
   *  \param[out]  flux        evaluated numerical flux
   */
  template< class RangeType, class DomainType >
  double numFlux ( const RangeType &uLeft, const RangeType &uRight,
                   const DomainType &unitNormal, RangeType &flux ) const
  {
    return num_flux( &(uLeft[ 0 ]), &(uRight[ 0 ]), &(unitNormal[ 0 ]), &(flux[ 0 ]) );
  }

  /** \internal
   *  \brief rotate the velocity field such that the normal would be \f$e_1\f$
   *
   *  \param[in]   normal  unit outer normal
   *  \param[in]   u       unrotated state
   *  \param[out]  uRot    rotated state
   */
  template< class RangeType, class DomainType >
  void
  rotate ( const DomainType &normal, const RangeType &u, RangeType &uRot ) const
  {
    rotate( &(normal[ 0 ]), &(u[ 1 ]), &(uRot[ 1 ]) );
  }

  /** \internal
   *  \brief rotate the velocity back
   *
   *  \param[in]   normal  unit outer normal
   *  \param[in]   uRot    rotated state
   *  \param[out]  u       unrotated state
   */
  template< class RangeType, class DomainType >
  void
  rotateInv ( const DomainType &normal, const RangeType &uRot, RangeType &u ) const
  {
    rotate_inv( &(normal[ 0 ]), &(uRot[ 1 ]), &(u[ 1 ]));
  }

private:
  void flux(const double U[dim+2], double *f[dim]) const;

  double num_flux(const double Uj[dim+2], const double Un[dim+2],
                  const double normal[dim], double gj[dim+2]) const;

  double num_flux_LLF(const double Uj[dim+2], const double Un[dim+2],
                      const double normal[dim], double gj[dim+2]) const;

  double num_flux_HLL(const double Uj[dim+2], const double Un[dim+2],
                      const double normal[dim], double gj[dim+2]) const;

  double num_flux_HLLC(const double Uj[dim+2], const double Un[dim+2],
                       const double normal[dim], double gj[dim+2]) const;

  static void rotate(const double normal[dim],
                     const double u[dim], double u_rot[dim]);

  static void rotate_inv(const double normal[dim],
                         const double u_rot[dim], double u[dim]);

  const double _gamma;
};

// EulerProblem
// ------------

/** \brief Problem describing the Euler equation of gas dynamics
 */
template< int dimD, EulerFluxType flux_type = HLLC >
struct EulerModel
{
  typedef ProblemData< dimD, dimD+2 > Problem;

  typedef typename Problem::DomainType DomainType;
  typedef typename Problem::RangeType RangeType;

  static const int dimDomain = DomainType::dimension;
  static const int dimRange = RangeType::dimension;
  static const bool hasFlux = true;

  /** \brief constructor
   *  \param problem switch between different data settings
   */
  EulerModel( unsigned int problem )
  : problem_( 0 ),
    numFlux_( 1.4 )
  {
    switch( problem )
    {
    case 1:
    case 11:
    case 12:
    case 13:
      problem_ = new EulerProblemFFS< dimDomain >( problem );
      break;
    case 2:
    case 21:
    case 22:
    case 23:
    case 25:
      problem_ = new EulerProblemShockBubble< dimDomain >( problem );
      break;

    default:
      std::cerr << "ProblemData not defined - using problem 1!" << std::endl;
      problem_ = new EulerProblemFFS< dimDomain >( problem );
    }
  }

  /** \brief destructor */
  ~EulerModel()
  {
    delete problem_;
  }

  double fixedDt () const
  {
    return -1;
  }

  /** \copydoc TransportProblem::data */
  const Problem &problem () const
  {
    return *problem_;
  }

  double pressure( const double gamma, const RangeType &u ) const
  {
    assert( u[0] >= 1e-10 );
    double v = u[1]*u[1] ;
    for( int i=2; i<dimDomain+1; ++i )
      v += u[i]*u[i];

    const double rhoe = u[dimDomain+1]-0.5*(v)/u[0];

    assert( rhoe>1e-10 );
    return (gamma-1.0)*rhoe;
  }

  /** \copydoc TransportProblem::numericalFlux */
  double numericalFlux ( const DomainType &normal,
                         const double time,
                         const DomainType &xGlobal,
                         const RangeType &uLeft, const RangeType &uRight,
                         RangeType &flux ) const
  {
    assert( std::abs( normal.two_norm() - 1.0 ) < 1e-12 );

    // apply numerical flux
    return numFlux_.numFlux( uLeft, uRight, normal, flux );
  }

  /** \brief boundary ids for different types of boundary typical for the
   *         Euler euqaitons
   */
  // enum { Inflow = 1 , Outflow = 2, Reflection = 3 };

  /** \copydoc TransportProblem::boundaryFlux */
  double boundaryFlux ( const DomainType &normal,
                        const double time,
                        const DomainType &xGlobal,
                        const RangeType& uLeft,
                        RangeType &flux ) const
  {
    int bndType = problem_->bndType(normal,xGlobal,time);
    if( bndType == 4 ) // slip
    {
      flux = 0;
      // slip boundary
      const double p = pressure(1.4, uLeft);
      for (int i=0;i<dimDomain; ++i)
      {
        flux[i+1] = normal[i] * p;
      }
      return 0.;
    }

    RangeType uRight;
    boundaryValue(bndType, normal,time,xGlobal,uLeft,uRight);
    return numericalFlux(normal,time,xGlobal,uLeft,uRight,flux);
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
    RangeType uRight;
    int bndType = problem_->bndType(normal,xGlobal,time);
    boundaryValue( bndType, normal,time,xGlobal,uLeft,uRight);
    return indicator( normal,time,xGlobal, uLeft, uRight );
  }

private:
  /** \brief the boundary flux inserts a ghost cell value into the
   *         numerical flux - this function computes these values for the
   *         different boundary types */
  void boundaryValue( const int bndType,
                      const DomainType &normal,
                      const double time,
                      const DomainType &xGlobal,
                      const RangeType& uLeft,
                      RangeType& uRight) const
  {
    if (bndType == 1) // Dirichlet
      uRight = problem().boundaryValue(xGlobal,time);
    else if (bndType == 2) // Neumann
      uRight = uLeft;
    else if (bndType == 3)
      {
        DomainType unitNormal( normal );
        const double faceVol = normal.two_norm();
        unitNormal *= 1.0 / faceVol;
        RangeType uBnd ( uLeft );
        RangeType uTmp ( uLeft );
        numFlux_.rotate( unitNormal, uLeft, uTmp );
        // Specific for euler: opposite sign for first component of momentum
        uTmp[1] = -uTmp[1];
        numFlux_.rotateInv( unitNormal, uTmp, uBnd );
        uRight = uBnd;
      }
  }

  /** \brief rotate the velocity field into the face frame of reference */
  void
  rotate( const DomainType &normal, const RangeType &u, RangeType &u_rot) const
  {
    numFlux_.rotate( normal, u, u_rot );
  }

  /** \brief rotate the velocity field from the face frame of reference */
  void
  rotateInv( const DomainType &normal, const RangeType &u_rot, RangeType &u) const
  {
    numFlux_.rotateInv( normal, u_rot, u );
  }

  Problem *problem_;
  EulerFlux< dimD, flux_type > numFlux_;
};

// ======= class Euler inline implementation =================
template< int dim, EulerFluxType flux_type >
inline void EulerFlux< dim, flux_type >
  ::rotate ( const double n[ dim ], const double u[ dim ], double u_rot[ dim ] )
{
  if (dim == 1){
    u_rot[0] = n[0] * u[0];
  }

  if (dim == 2){
    u_rot[0] = n[0]*u[0] + n[1]*u[1];
    u_rot[1] = -n[1]*u[0] + n[0]*u[1];
  }

  if (dim == 3){
    double d = std::sqrt(n[0]*n[0]+n[1]*n[1]);

    if (d > 1.0e-8) {
      double d_1 = 1.0/d;
      u_rot[0] = n[0]*u[0]           + n[1]*u[1]          + n[2]*u[2];
      u_rot[1] = -n[1]*d_1*u[0]      + n[0]*d_1* u[1];
      u_rot[2] = -n[0]*n[2]*d_1*u[0] - n[1]*n[2]*d_1*u[1] + d*u[2];
    }
    else {
      u_rot[0] = n[2]*u[2];
      u_rot[1] = u[1];
      u_rot[2] = -n[2]*u[0];
    }

    //assert(0); // test it, not tested up to now
  }

  if (dim > 3) assert(0);
}

template< int dim, EulerFluxType flux_type >
inline void EulerFlux< dim, flux_type >
  ::rotate_inv ( const double n[ dim ], const double u_rot[ dim ], double u[ dim ] )
{
  if (dim == 1){
    u[0] = n[0] * u_rot[0];
  }

  if (dim == 2){
    u[0] = n[0]*u_rot[0] - n[1]*u_rot[1];
    u[1] = n[1]*u_rot[0] + n[0]*u_rot[1];
  }

  if (dim == 3){
    double d = std::sqrt(n[0]*n[0]+n[1]*n[1]);

    if (d > 1.0e-8) {
      double d_1 = 1.0/d;
      u[0] = n[0]*u_rot[0] - n[1]*d_1*u_rot[1] - n[0]*n[2]*d_1*u_rot[2];
      u[1] = n[1]*u_rot[0] + n[0]*d_1*u_rot[1] - n[1]*n[2]*d_1*u_rot[2];
      u[2] = n[2]*u_rot[0]                     + d*u_rot[2];
    }
    else {
      u[0] = -n[2]*u_rot[2];
      u[1] = u_rot[1];
      u[2] = n[2]*u_rot[0];
    }

    //assert(0); // test it, not tested up to now
  }

  if (dim > 3) assert(0);
}

// U[0] = rho, (U[1],...,U[dim]) = rho_\vect u, U[dim+1] = E
template< int dim, EulerFluxType flux_type >
inline void EulerFlux< dim, flux_type >
  ::flux( const double U[ dim+2 ], double *f[ dim ] ) const
{
  const double rho = U[0];
  const double *rho_u = &U[1];
  const double E = U[dim+1];

  double u[dim], Ekin2 = 0.0;
  for(int i=0; i<dim; i++){
    u[i] = (1.0/rho) * rho_u[i];
    Ekin2 += rho_u[i] * u[i];
  }

  const double p = (_gamma-1.0)*(E - 0.5*Ekin2);

  for(int i=0; i<dim; i++){
    f[i][0] = rho_u[i];

    for(int j=0; j<dim; j++) f[i][1+j] = rho_u[i] * u[j];
    f[i][1+i] += p;

    f[i][dim+1] = (E+p) * u[i];
  }
}

// returns fastest wave speed
// U[0] = rho, (U[1],...,U[dim]) = rho_\vect u, U[dim+1] = E
template< int dim, EulerFluxType flux_type >
inline double EulerFlux< dim, flux_type >
  ::num_flux ( const double Uj[ dim+2 ], const double Un[ dim+2 ],
               const double normal[ dim ], double gj[ dim+2 ] ) const
{
  switch( flux_type )
  {
  case LLF:
    return num_flux_LLF( Uj, Un, normal, gj );

  case HLL:
    return num_flux_HLL( Uj, Un, normal, gj );

  case HLLC:
    return num_flux_HLLC( Uj, Un, normal, gj );

  default:
    std::cerr << "Invalid numerical flux selected." << std::endl;
    abort();
    return 0.0;
  }
}

// returns fastest wave speed
// U[0] = rho, (U[1],...,U[dim]) = rho_\vect u, U[dim+1] = E
template< int dim, EulerFluxType flux_type >
inline double EulerFlux< dim, flux_type >
  ::num_flux_LLF( const double Uj[ dim+2 ], const double Un[ dim+2 ],
                  const double normal[ dim ], double gj[ dim+2 ] ) const
{
  const double rhoj = Uj[0];
  const double *rho_uj = &Uj[1];
  const double Ej = Uj[dim+1];
  const double rhon = Un[0];
  const double *rho_un = &Un[1];
  const double En = Un[dim+1];

  double uj[dim], Ekin2j=0.0, un[dim], Ekin2n=0.0;
  double u_normal_j=0.0, u_normal_n=0.0;
  for(int i=0; i<dim; i++){
    uj[i] = (1.0/rhoj) * rho_uj[i];
    un[i] = (1.0/rhon) * rho_un[i];
    Ekin2j += rho_uj[i] * uj[i];
    Ekin2n += rho_un[i] * un[i];
    u_normal_j += uj[i] * normal[i];
    u_normal_n += un[i] * normal[i];
  }

  const double pj = (_gamma-1.0)*(Ej - 0.5*Ekin2j);
  const double cj = sqrt(_gamma*pj/rhoj);
  const double pn = (_gamma-1.0)*(En - 0.5*Ekin2n);
  const double cn = sqrt(_gamma*pn/rhon);

  assert(rhoj>0.0 && pj>0.0 && rhoj>0.0 && pj>0.0);

  const double alphaj = std::abs(u_normal_j) + cj;
  const double alphan = std::abs(u_normal_n) + cn;
  const double alpha = (alphaj > alphan)? alphaj : alphan;

  gj[0] = gj[dim+1] = 0.0;
  for(int i=0; i<dim; i++) gj[1 + i] = 0.0;

  for(int j=0; j<dim; j++){
    gj[0] += ( rho_uj[j] + rho_un[j] ) * normal[j];

    for(int i=0; i<dim; i++){
      gj[1 + i] += (rho_uj[i]*uj[j] + rho_un[i]*un[j]) * normal[j];
    }

    gj[dim+1] += ( (Ej+pj)*uj[j] + (En+pn)*un[j] ) * normal[j];
  }

  gj[0] = 0.5 * (gj[0] - alpha*(rhon - rhoj));
  for(int i=0; i<dim; i++){
    gj[1+i] = 0.5*(gj[1+i] + (pj+pn)*normal[i] - alpha*(rho_un[i]-rho_uj[i]));
  }
  gj[dim+1] = 0.5 * (gj[dim+1] - alpha*(En - Ej));

  return alpha;
}

// returns fastest wave speed
// U[0] = rho, (U[1],...,U[dim]) = rho_\vect u, U[dim+1] = E
template< int dim, EulerFluxType flux_type >
inline double EulerFlux< dim, flux_type >
  ::num_flux_HLL ( const double Uj[ dim+2 ], const double Un[ dim+2 ],
                   const double normal[ dim ], double gj[ dim+2 ] ) const

{
  const double rhoj = Uj[0];
  const double Ej = Uj[dim+1];
  const double rhon = Un[0];
  const double En = Un[dim+1];

  double rho_uj[dim], rho_un[dim], uj[dim], un[dim];
  double Ekin2j=0.0, Ekin2n=0.0;
  rotate(normal, Uj+1, rho_uj);
  rotate(normal, Un+1, rho_un);
  for(int i=0; i<dim; i++){
    uj[i] = (1.0/rhoj) * rho_uj[i];
    un[i] = (1.0/rhon) * rho_un[i];
    Ekin2j += rho_uj[i] * uj[i];
    Ekin2n += rho_un[i] * un[i];
  }

  const double pj = (_gamma-1.0)*(Ej - 0.5*Ekin2j);
  const double cj = sqrt(_gamma*pj/rhoj);
  const double pn = (_gamma-1.0)*(En - 0.5*Ekin2n);
  const double cn = sqrt(_gamma*pn/rhon);

  assert(rhoj>0.0 && pj>0.0 && rhoj>0.0 && pj>0.0);

  const double rho_bar = 0.5 * (rhoj + rhon);
  const double c_bar = 0.5 * (cj + cn);
  const double p_star = 0.5 * ( (pj+pn) - (un[0]-uj[0])*rho_bar*c_bar );
  const double u_star = 0.5 * ( (uj[0]+un[0]) - (pn-pj)/(rho_bar*c_bar) );
  const double tmp = 0.5*(_gamma+1.0)/_gamma;
  const double qj = (p_star > pj)? sqrt( 1.0 + tmp*(p_star/pj - 1.0) ): 1.0;
  const double qn = (p_star > pn)? sqrt( 1.0 + tmp*(p_star/pn - 1.0) ): 1.0;

  const double sj = uj[0] - cj*qj;
  const double sn = un[0] + cn*qn;

  double guj[dim];

  if (u_star > 0.0){
    if (sj >= 0.0){
      gj[0] = rho_uj[0];

      for(int i=0; i<dim; i++) guj[i] = rho_uj[i]*uj[0];
      guj[0] += pj;

      gj[dim+1] = (Ej+pj)*uj[0];
    }
    else{
      const double tmp1 = sj * sn;
      const double tmp2 = 1.0/(sn - sj);
      gj[0] = tmp2 * ( sn*rho_uj[0] - sj*rho_un[0] + tmp1*(rhon - rhoj) );

      for(int i=0; i<dim; i++){
        guj[i] = tmp2*((sn*uj[0]-tmp1)*rho_uj[i] - (sj*un[0]-tmp1)*rho_un[i]);
      }
      guj[0] += tmp2 * (sn*pj - sj*pn);

      gj[dim+1] = tmp2 * (sn*(Ej+pj)*uj[0]-sj*(En+pn)*un[0] + tmp1*(En - Ej));
    }
  }
  else{ // u_star <= 0
    if (sn <= 0.0){
      gj[0] = rho_un[0];

      for(int i=0; i<dim; i++) guj[i] = rho_un[i]*un[0];
      guj[0] += pn;

      gj[dim+1] = (En+pn)*un[0];
    }
    else{
      const double tmp1 = sj * sn;
      const double tmp2 = 1.0/(sn - sj);
      gj[0] = tmp2 * ( sn*rho_uj[0] - sj*rho_un[0] + tmp1*(rhon - rhoj) );

      for(int i=0; i<dim; i++){
        guj[i] = tmp2*((sn*uj[0]-tmp1)*rho_uj[i] - (sj*un[0]-tmp1)*rho_un[i]);
      }
      guj[0] += tmp2 * (sn*pj - sj*pn);

      gj[dim+1] = tmp2 * (sn*(Ej+pj)*uj[0]-sj*(En+pn)*un[0] + tmp1*(En - Ej));

    }
  }

  rotate_inv(normal, guj, gj+1);
  return std::max( std::abs(sj), std::abs(sn) );
}

template< int dim, EulerFluxType flux_type >
inline double EulerFlux< dim, flux_type >
::num_flux_HLLC ( const double Um[ dim+2 ], const double Up[ dim+2 ],
                  const double normal[ dim ], double g[ dim+2 ] ) const
{
  const double rhom = Um[0];
  const double rhop = Up[0];
  const double Em = Um[dim+1];
  const double Ep = Up[dim+1];

  double rho_um[dim], rho_up[dim];
  rotate( normal, Um+1, rho_um );
  rotate( normal, Up+1, rho_up );

  double Ekinm = 0.;
  double Ekinp = 0.;
  double um[dim], up[dim];
  for( int i=0; i<dim; ++i )
  {
    um[i] = rho_um[i] / rhom;
    up[i] = rho_up[i] / rhop;
    Ekinm += rho_um[i] * um[i];
    Ekinp += rho_up[i] * up[i];
  }

  const double pm = (_gamma-1.0)*(Em - 0.5*Ekinm);
  const double pp = (_gamma-1.0)*(Ep - 0.5*Ekinp);

  assert( rhom>0.0 && pm>0.0 && rhop>0.0 && pp>0.0 );

  const double cm = sqrt(_gamma*pm/rhom);
  const double cp = sqrt(_gamma*pp/rhop);

  const double rho_bar = 0.5 * (rhom + rhop);
  const double c_bar = 0.5 * (cm + cp);
  const double p_star = 0.5 * ( (pm+pp) - (up[0]-um[0])*rho_bar*c_bar );
  const double u_star = 0.5 * ( (um[0]+up[0]) - (pp-pm)/(rho_bar*c_bar) );
  const double tmp = 0.5*(_gamma+1.0)/_gamma;
  const double qm = (p_star > pm) ? sqrt( 1.0 + tmp*(p_star/pm - 1.0) ) : 1.0;
  const double qp = (p_star > pp) ? sqrt( 1.0 + tmp*(p_star/pp - 1.0) ) : 1.0;

  const double sm = um[0] - cm*qm;
  const double sp = up[0] + cp*qp;

  double guj[dim];

  if (sm >= 0.0)
  {
    g[0] = rho_um[0];

    for(int i=0; i<dim; i++)
      guj[i] = rho_um[i]*um[0];
    guj[0] += pm;

    g[dim+1] = (Em+pm)*um[0];
  }
  else if (sp <= 0.0)
  {
    g[0] = rho_up[0];

    for(int i=0; i<dim; i++)
      guj[i] = rho_up[i]*up[0];
    guj[0] += pp;

    g[dim+1] = (Ep+pp)*up[0];
  }
  else
  {
    const double tmpm = sm*(sm-um[0])/(sm-u_star);
    const double tmpp = sp*(sp-up[0])/(sp-u_star);

    if (u_star >= 0.0)
    {
      g[0] = rho_um[0] + rhom*(tmpm-sm);

      for(int i=0; i<dim; i++)
        guj[i] = rho_um[i]*um[0] + rhom*um[i]*tmpm - sm*rho_um[i];
      guj[0] += pm + rhom*(u_star-um[0])*tmpm;

      g[dim+1] = (Em+pm)*um[0] + Em*(tmpm-sm)
        + tmpm*(u_star-um[0])*( rhom*u_star + pm/(sm-um[0]) );
    }
    else
    {
      g[0] = rho_up[0] + rhop*(tmpp-sp);

      for(int i=0; i<dim; i++)
        guj[i] = rho_up[i]*up[0] + rhop*up[i]*tmpp - sp*rho_up[i];
      guj[0] += pp + rhop*(u_star-up[0])*tmpp;

      g[dim+1] = (Ep+pp)*up[0] + Ep*(tmpp-sp)
        + tmpp*(u_star-up[0])*( rhop*u_star + pp/(sp-up[0]) );
    }
  }

  rotate_inv( normal, guj, g+1 );
  return std::max( std::abs(sm), std::abs(sp) );
}

#endif // #ifndef EULERFLUXES_HH
