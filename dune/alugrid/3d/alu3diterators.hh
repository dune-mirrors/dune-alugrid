#ifndef DUNE_ALU3DITERATORS_HH
#define DUNE_ALU3DITERATORS_HH

// Dune includes
#include <dune/grid/common/gridenums.hh>

// Local includes
#include "alu3dinclude.hh"

namespace ALUGrid
{

  //*************************************************************
  //  definition of original LeafIterators of ALUGrid
  //
  // default is element (codim = 0)
  template< int codim, class Comm >
  struct BSMacroIterator
  {
    typedef typename Dune::ALU3dBasicImplTraits< Comm >::HElementType HElementType;
    typedef typename AccessIterator< HElementType >::Handle IteratorType;
  };

  //******************************************************************
  //  LevelIterators
  //******************************************************************
  template< int codim, class Comm >
  struct ALUHElementType;

  template< class Comm >
  struct ALUHElementType< 0, Comm >
  {
    typedef typename Dune::ALU3dBasicImplTraits< Comm >::HElementType ElementType;
  };

  template< class Comm >
  struct ALUHElementType< 1, Comm >
  {
    typedef typename Dune::ALU3dBasicImplTraits< Comm >::HFaceType ElementType;
  };

  template< class Comm >
  struct ALUHElementType< 2, Comm >
  {
    typedef typename Dune::ALU3dBasicImplTraits< Comm >::HEdgeType ElementType;
  };

  template< class Comm >
  struct ALUHElementType< 3, Comm >
  {
    typedef typename Dune::ALU3dBasicImplTraits< Comm >::VertexType ElementType;
  };


  //*********************************************************
  //  LevelIterator Wrapper
  //*********************************************************
  template< class val_t >
  class IteratorWrapperInterface
  : public IteratorSTI< val_t >
  {
  public:
    virtual ~IteratorWrapperInterface () {}

    virtual int size  () = 0;
    virtual void next () = 0;
    virtual void first() = 0;
    virtual int done  () const = 0;
    virtual val_t & item () const = 0;
    virtual IteratorSTI< val_t > * clone () const { alugrid_assert (false); abort(); return 0; }
  };

  typedef Dune::PartitionIteratorType PartitionIteratorType;

  // defines the pair of element and boundary
  template< int codim, class Comm >
  struct IteratorElType
  {
    typedef typename ALUHElementType< codim, Comm >::ElementType ElType;
    typedef typename Dune::ALU3dBasicImplTraits< Comm >::HBndSegType HBndSegType;
    typedef std::pair< ElType *, HBndSegType * > val_t;
  };

} // end namespace ALUGrid

//#include "alu3diterators_imp.cc"
#endif // #ifndef DUNE_ALU3DITERATORS_HH
