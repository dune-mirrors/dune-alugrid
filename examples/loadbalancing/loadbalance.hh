#ifndef LOADBALANCE_HH
#define LOADBALANCE_HH

#include <dune/grid/common/gridenums.hh>
#include <dune/grid/common/datahandleif.hh>

template< class Grid >
class LoadBalanceHandle
: public Dune::LoadBalanceHandleIF< LoadBalanceHandle<Grid> >
{
  typedef LoadBalanceHandle This;
  typedef Dune::LoadBalanceHandleIF< This > Base;

public:
  typedef typename Grid :: ObjectStreamType ObjectStream;

  static const int dimension = Grid :: dimension;

  template< int codim >
  struct Codim
  {
    typedef typename Grid :: Traits :: template Codim< codim > :: Entity Entity;
    typedef typename Grid :: Traits :: template Codim< codim > :: EntityPointer
      EntityPointer;
  };
  typedef typename Codim< 0 > :: Entity Element;

private:
  const Grid &grid_;

public:
  LoadBalanceHandle ( const Grid &grid )
  : grid_( grid )
  {}

  /*
  void inlineData ( ObjectStream &stream, const Element &element ) const
  {
    return;
  }
  void xtractData ( ObjectStream &stream, const Element &element, size_t newElements )
  {
    return;
  }
  void compress ()
  {}
  */

  // return true if user defined partitioning methods should be used
  bool userDefinedPartitioning () const
  {
    return true;
  }
  // return true if user defined load balancing weights are provided
  bool userDefinedLoadWeights () const
  {
    return false;
  }

  // returns true if user defined partitioning needs to be readjusted 
  bool repartition () const 
  { 
    angle_ += 2.*M_PI/50.;
    return true;
  }
  // return load weight of given element 
  int loadWeight( const Element &element ) const 
  { 
    return 1;
  }
  // return destination (i.e. rank) where the given element should be moved to 
  // this needs the methods userDefinedPartitioning to return true
  int destination( const Element &element ) const 
  { 
    typename Element::Geometry::GlobalCoordinate w = element.geometry().center();
    double phi=arg(std::complex<double>(w[0],w[1]));
    if (w[1]<0) phi+=2.*M_PI;
    phi += angle_;
    phi *= double(this->grid_.comm().size())/(2.*M_PI);
    int p = int(phi) % this->grid_.comm().size();
    return p;
  }
private:
  static double angle_;
};

template< class Grid >
double LoadBalanceHandle<Grid>::angle_ = 0;

#endif // #ifndef LOADBALNCE_HH
