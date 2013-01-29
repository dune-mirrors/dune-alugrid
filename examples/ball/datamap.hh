#ifndef DATAMAP_HH
#define DATAMAP_HH

#include <dune/grid/common/gridenums.hh>
#include <dune/grid/common/datahandleif.hh>

// DataMap::LoadBalanceHandle
// --------------------------
 /** \brief the communication data handle for load balancing
 */
class LoadBalanceHandle
: public Dune::CommDataHandleIF< LoadBalanceHandle, void >
{
  typedef LoadBalanceHandle This;
  typedef Dune::CommDataHandleIF< This, void > Base;

public:
  //! create DiscreteOperator with a LocalOperator 
  LoadBalanceHandle ( )
  {}

  //! see documentation in Dune::CommDataHandleIF 
  bool contains ( int dim, int codim ) const
  {
    return false;
  }

  //! see documentation in Dune::CommDataHandleIF 
  bool fixedsize ( int dim, int codim ) const
  {
    return true;
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
};
#endif // #ifndef DATAMAP_HH
