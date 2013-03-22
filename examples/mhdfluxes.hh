#ifndef DUNE_FEM_DG_MHDFLUXES_HH
#define DUNE_FEM_DG_MHDFLUXES_HH
#warning "Using Mhd NumFluxes"

#include <cmath>

#include <dune/fem-dg/operator/fluxes/mhd_eqns.hh>
#include <dune/fem-dg/operator/fluxes/rotator.hh>

// Dai-Woodward 
template < int dimDomain >
class DWNumFlux;

// HLLEM
template < int dimDomain >
class HLLEMNumFlux;

// ************************************************
template <int dimDomain>
class ConsVec : public Dune :: FieldVector< double, dimDomain+2> 
{
public:
  explicit ConsVec (const double& t) : Dune :: FieldVector<double,dimDomain+2>(t) {}
  ConsVec () : Dune :: FieldVector<double,dimDomain+2>(0) {}
};

namespace Mhd {
  typedef enum { DW, HLLEM } MhdFluxType;
}

// ***********************
template < int dimDomain, Mhd :: MhdFluxType fluxtype >
class MHDNumFluxBase
{
public:
  typedef Mhd::MhdSolver MhdSolverType;
  typedef double value_t[ 9 ];

  typedef Dune::FieldVector< double, dimDomain   > DomainType;
  typedef Dune::FieldVector< double, dimDomain+2 > RangeType;

protected:  
  MHDNumFluxBase(const double gamma ) 
   : eos( MhdSolverType::Eosmode::me_ideal ),
     numFlux_(eos, gamma, 1.0 ),
     rot_(1) 
  {
    if( fluxtype == Mhd :: HLLEM ) 
    {
      numFlux_.init(Mhd :: MhdSolver :: mf_rghllem ); 
    }
  }
  
public:
  // Return value: maximum wavespeed*length of integrationOuterNormal
  // gLeft,gRight are fluxed * length of integrationOuterNormal
  inline double
  numericalFlux( const RangeType& uLeft, const RangeType& uRight,
                 const DomainType &unitNormal, RangeType &flux ) const
  {
    RangeType ul(uLeft);
    RangeType ur(uRight);

    rot_.rotateForth(ul, unitNormal);
    rot_.rotateForth(ur, unitNormal);

    enum { e = DomainType :: dimension + 1 };

    value_t res;
    const double dummy[ 3 ] = { 0, 0, 0 };

    value_t entity = { 0,0,0,0,0,0,0,0 };
    value_t neigh  = { 0,0,0,0,0,0,0,0 };
    for(int i=0; i<e; ++i) 
    {
      entity[ i ] = ul[ i ];
      neigh [ i ] = ur[ i ];
    }

    entity[ 7 ] = ul[ e ];
    neigh [ 7 ] = ur[ e ];

    const double ldt = numFlux_(entity, neigh, dummy, res);

    // copy first components 
    for(int i=0; i<e; ++i) 
      flux[ i ] = res[ i ];

    // copy energy 
    flux[ e ] = res[ 7 ];

    // rotate flux 
    rot_.rotateBack( flux, unitNormal );

    // conservation
    return ldt;
  }
protected:
  const typename MhdSolverType::Eosmode::meos_t eos;
  mutable MhdSolverType numFlux_;
  Adi::FieldRotator<DomainType, RangeType> rot_;
};

//////////////////////////////////////////////////////////
//
//  Flux Implementations 
//
//////////////////////////////////////////////////////////

template <int dimDomain>
class DWNumFlux : public MHDNumFluxBase< dimDomain, Mhd::DW >
{
  typedef MHDNumFluxBase< dimDomain, Mhd::DW > BaseType ; 
public:  
  DWNumFlux( const double gamma ) 
    : BaseType( gamma ) 
  {}
  static std::string name () { return "DW (Mhd)"; }
};

template <int dimDomain>
class HLLEMNumFlux : public MHDNumFluxBase< dimDomain, Mhd::HLLEM >
{
  typedef MHDNumFluxBase< dimDomain, Mhd::HLLEM > BaseType ; 
public:  
  HLLEMNumFlux( const double gamma ) 
    : BaseType( gamma ) 
  {}
  static std::string name () { return "HLLEM (Mhd)"; }
};

#endif // DUNE_MHDFLUXES_HH
