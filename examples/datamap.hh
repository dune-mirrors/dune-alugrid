#ifndef DATAMAP_HH
#define DATAMAP_HH

#include <dune/grid/common/gridenums.hh>
#include <dune/grid/common/datahandleif.hh>

#include "lbdatahandle.hh"

// DataMap::LoadBalanceHandle
// --------------------------
 /** \brief the communication data handle for load balancing
 */
template< class Container >
class LoadBalanceHandle
: public Dune::CommDataHandleIF< LoadBalanceHandle<Container>, Container >
{
  typedef LoadBalanceHandle This;
  typedef Dune::CommDataHandleIF< This, Container > Base;


public:
  // type of data transported in the stream
  typedef typename Base::DataType DataType;
  typedef typename Container::GridType::template Codim< 0 >::Entity Entity;

private:
  // data map 
  Container &data_;

public:
  //! create DiscreteOperator with a LocalOperator 
  LoadBalanceHandle ( Container &data )
  : data_( data )
  {}

  //! see documentation in Dune::CommDataHandleIF 
  bool contains ( int dim, int codim ) const
  {
    return (codim == 0);
  }

  //! see documentation in Dune::CommDataHandleIF 
  bool fixedsize ( int dim, int codim ) const
  {
    return (codim > 0);
  }

  //! see documentation in Dune::CommDataHandleIF (note that isLeaf available only on codimension 0 entity
  size_t size ( const Entity &entity ) const
  {
    return entity.isLeaf() ? 1 : 0 ;
  }

  //! see documentation in Dune::CommDataHandleIF (method for codim 0 entities)
  template< class Buffer >
  void gather ( Buffer &buffer, const Entity &entity ) const
  {
    // we only have data on the leaf level
    if( entity.isLeaf() )
    {
      // write data to stream  
      buffer.write( data_[ entity ] );
    }
  }

  //! see documentation in Dune::CommDataHandleIF (method for codim 0 entities)
  template< class Buffer >
  void scatter ( Buffer &buffer, const Entity &entity, size_t n )
  {
    assert( n == size( entity ) );

    data_.update();
    // we only have data on the leaf level
    if( entity.isLeaf() )
      buffer.read( data_[ entity ] );
  }

  //! see documentation in Dune::CommDataHandleIF (method for general entities)
  template <class E>
  size_t size ( const E &entity ) const
  {
    return 0;
  }

  //! see documentation in Dune::CommDataHandleIF (method for general entities)
  template< class Buffer, class E >
  void gather ( Buffer &buffer, const E &entity ) const
  {}

  //! see documentation in Dune::CommDataHandleIF (method for general entities)
  template< class Buffer, class E >
  void scatter ( Buffer &buffer, E &entity, size_t n )
  {
    assert( n == size( entity ) );
  }

  // return true if user defined partitioning methods should be used 
  bool userDefinedPartitioning () const 
  { 
    return true ; 
  }
  // return true if user defined load balancing weights are provided
  bool userDefinedLoadWeights () const 
  { 
    return false ; 
  }
  // returns true if user defined partitioning needs to be readjusted 
  bool repartition () const 
  { 
    angle_ += 2.*M_PI/100.;
    return true; 
  }
  // return load weight of given element 
  int loadWeight( const Entity &element ) const 
  { 
    return 1;
  }
  // return destination (i.e. rank) where the given element should be moved to 
  // this needs the methods userDefinedPartitioning to return true
  int destination( const Entity &element ) const 
  { 
    typename Entity::Geometry::GlobalCoordinate w = element.geometry().center();
    double phi=arg(std::complex<double>(w[0],w[1]));
    if (w[1]<0) phi+=2.*M_PI;
    phi += angle_;
    int p = int(phi) % 6;
    // int p = int(w[0]*2.);
    return p;
  }
  void compress () {}
private:
  static double angle_;
};

template< class Container >
double LoadBalanceHandle<Container>::angle_ = 0;

#endif // #ifndef DATAMAP_HH
