#ifndef CALLBACKADAPT_HH
#define CALLBACKADAPT_HH

#include <dune/grid/common/adaptcallback.hh>

template< class Grid, class Vector, class Container >
class AdaptDataHandle : 
public Dune::AdaptDataHandle< Grid, AdaptDataHandle<Grid,Vector,Container> >
{
  typedef AdaptDataHandle< Grid, Vector, Container > This;
  typedef Dune::AdaptDataHandle< Grid, This > Base;
public:
  typedef typename Grid::template Codim< 0 >::Entity Entity;
  typedef typename Container::Value Value;
private:
  AdaptDataHandle () {}
  AdaptDataHandle ( const This & );
  This &operator= ( const This & );
public:
// constructor
  AdaptDataHandle( Container &container) 
  : container_(container) 
  {}
// interface methods
  void preAdapt ( const unsigned int estimateAdditionalElements )
  {}
  void postAdapt ()
  {}
  void preCoarsening ( const Entity &father ) const
  {
    Vector::restrictLocal( father, container_ );
  }
  void postRefinement ( const Entity &father ) const
  { 
    container_.resize();
    Vector::prolongLocal( father, container_ );
  }
  void restrictLocal( const Entity &father, const Entity& son, bool initialize ) const 
  {}
  void prolongLocal( const Entity &father, const Entity& son, bool initialize ) const 
  {}
  private:
  mutable Container container_;
};

#endif
