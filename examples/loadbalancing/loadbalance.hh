#ifndef NEWDATAMAP_HH
#define NEWDATAMAP_HH

#include <dune/grid/common/gridenums.hh>
#include <dune/grid/common/datahandleif.hh>

template< class Grid >
class LoadBalanceHandle
: public Dune::LoadBalanceHandleIF< LoadBalanceHandle<Grid,Container>, Container >
{
  typedef LoadBalanceHandle This;
  typedef Dune::CommDataHandleIF< This, Container > Base;
{
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
  MyLoadBalanceHandle ( const Grid &grid )
  : grid_( grid )
  {}

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
    phi /= 2.*M_PI*this->grid_.comm().size();
    int p = int(phi) % this->grid_.comm().size();
    std::cout << "destination: " << p << std::endl;
    return p;
  }
};

// DataMap::LoadBalanceHandle
// --------------------------
 /** \brief the communication data handle for load balancing
 */
template< class Grid, class Container >
class LoadBalanceHandle
: public Dune::CommDataHandleIF< LoadBalanceHandle<Grid,Container>, Container >
:  public Dune::LoadBalanceDataHandleIF< NewLoadBalanceHandle< Grid, Container >, Container >
{
  typedef NewLoadBalanceHandle This;
  typedef Dune::LoadBalanceDataHandleIF< This, Container > Base;

protected:
  // data map 
  Container &data_;
  const Grid &grid_;

public:
  // type of data transported in the stream
  typedef typename Base::DataType DataType;
  typedef typename Grid::template Codim< 0 >::Entity Entity;

  //! create DiscreteOperator with a LocalOperator 
  NewLoadBalanceHandle ( const Grid &grid, Container &data )
  : data_( data )
  , grid_(grid)
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
    std::cout << "usedDefinedPartitioning" << std::endl;
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
    std::cout << "repartition" << std::endl;
    angle_ += 2.*M_PI/100.;
    return true; 
  }
  // return load weight of given element 
  int loadWeight( const Entity &element ) const 
  { 
    std::cout << "loadWeight" << std::endl;
    return -1;
  }
  // return destination (i.e. rank) where the given element should be moved to 
  // this needs the methods userDefinedPartitioning to return true
  int destination( const Entity &element ) const 
  { 
    typename Entity::Geometry::GlobalCoordinate w = element.geometry().center();
    double phi=arg(std::complex<double>(w[0],w[1]));
    if (w[1]<0) phi+=2.*M_PI;
    phi += angle_;
    int p = int(phi) % this->grid_.comm().size();
    std::cout << "destination: " << p << std::endl;
    return p;
  }
private:
  static double angle_;
};

template< class Grid, class Container >
double NewLoadBalanceHandle<Grid,Container>::angle_ = 0;

#endif // #ifndef NEWDATAMAP_HH
