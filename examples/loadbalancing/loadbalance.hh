#ifndef LOADBALANCE_HH
#define LOADBALANCE_HH

#include <dune/grid/common/gridenums.hh>
#include <dune/grid/common/datahandleif.hh>

#include "zoltaninterface.hh"

template< class Grid >
class SimpleLoadBalanceHandle
: public Dune::LoadBalanceHandleIF< SimpleLoadBalanceHandle<Grid> >
{
  typedef SimpleLoadBalanceHandle This;
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
  SimpleLoadBalanceHandle ( const Grid &grid )
  : grid_( grid ),
    angle_( 0 )
  {}

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
  mutable double angle_;
};

template< class Grid >
class ZoltanLoadBalanceHandle
: public Dune::LoadBalanceHandleIF< ZoltanLoadBalanceHandle<Grid> >
{
  typedef ZoltanLoadBalanceHandle This;
  typedef Dune::LoadBalanceHandleIF< This > Base;

  typedef typename Grid::GlobalIdSet GlobalIdSet;
  typedef typename GlobalIdSet::IdType gIdType;

public:
  typedef typename Grid :: ObjectStreamType ObjectStream;

  static const int dimension = Grid :: dimension;

  template< int codim >
  struct Codim
  {
    typedef typename Grid :: Traits :: template Codim< codim > :: Entity Entity;
    typedef typename Grid :: Traits :: template Codim< codim > :: EntityPointer EntityPointer;
  };
  typedef typename Codim< 0 > :: Entity Element;

private:
  const Grid &grid_;

public:
  ZoltanLoadBalanceHandle ( const Grid &grid, ZOLTAN_PARTITIONING new_partitioning)
  : grid_( grid )
	, globalIdSet_( grid.globalIdSet() )
	, new_partitioning_( new_partitioning )
  {}

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
    gIdType bla = globalIdSet_.id(element);
    std::vector<int> elementGID(4); // because we have 4 vertices
    bla.getKey().extractKey(elementGID);

    // add one to the GIDs, so that they match the ones from Zoltan
    transform(elementGID.begin(), elementGID.end(), elementGID.begin(), bind2nd(std::plus<int>(), 1));

    int p = int(this->grid_.comm().rank());

    for (int i = 0; i<new_partitioning_.numExport; ++i)
    {
      if (std::equal(elementGID.begin(),elementGID.end(), &new_partitioning_.exportGlobalGids[i*new_partitioning_.numGidEntries]) )
      p = new_partitioning_.exportProcs[i];
    }
    return p;
  }
private:
  const GlobalIdSet &globalIdSet_;
  const ZOLTAN_PARTITIONING new_partitioning_;
};

#endif // #ifndef LOADBALNCE_HH
