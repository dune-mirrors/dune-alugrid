#ifndef DUNE_INTERSECTIONITERATORWRAPPER_HH
#define DUNE_INTERSECTIONITERATORWRAPPER_HH

#include <dune/common/nullptr.hh>

#include <dune/grid/common/intersectioniterator.hh>
#include <dune/alugrid/common/macrogridview.hh>

/** @file
 @author Robert Kloefkorn
 @brief Provides proxy classes for IntersectionsIterators
*/

namespace Dune {

//! \brief Class that wraps IntersectionIteratorImp of a grid and gets it's
//! internal object from a object stack hold by the grid
template <class GridImp, class IntersectionIteratorImpl>
class IntersectionIteratorWrapper
{
  enum { dim = GridImp :: dimension };
  enum { dimworld = GridImp :: dimensionworld };

  typedef IntersectionIteratorWrapper<GridImp,IntersectionIteratorImpl> ThisType;

  typedef IntersectionIteratorImpl IntersectionIteratorImp;

  typedef typename IntersectionIteratorImp :: StorageType IntersectionIteratorProviderType;

public:
  typedef typename GridImp :: GridObjectFactoryType   FactoryType;

  //! dimension
  enum { dimension      = dim };
  //! dimensionworld
  enum { dimensionworld = dimworld };

  //! define type used for coordinates in grid module
  typedef typename GridImp :: ctype ctype;

  //! Entity type
  typedef typename GridImp::template Codim<0>::Entity Entity;
  //! type of EntityPointer
  typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

  //! type of intersectionGlobal
  typedef typename GridImp::template Codim<1>::Geometry Geometry;
  //! type of intersection*Local
  typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;

  //! type of normal vector
  typedef FieldVector<ctype , dimworld> NormalType;

  typedef typename IntersectionIteratorImpl::Twists Twists;
  typedef typename Twists::Twist Twist;

  IntersectionIteratorWrapper () : factory_( nullptr ), it_( nullptr ) {}

  //! constructor called from the ibegin and iend method
  template <class EntityImp>
  IntersectionIteratorWrapper(const FactoryType& factory, const EntityImp & en, int wLevel , bool end)
    : factory_( &factory )
    , it_( &factory.getIntersection(wLevel, (IntersectionIteratorImpl *) 0) )
  {
    if(end)
      it().done( en );
    else
      it().first(en,wLevel);
  }

  //! The copy constructor
  IntersectionIteratorWrapper(const ThisType & org)
    : factory_( org.factory_ )
  {
    if( factory_ )
    {
      it_ = &factory().getIntersection( -1, static_cast< IntersectionIteratorImpl * >( nullptr ) );
      it_->assign( org.it() );
    }
  }

  //! the assignment operator
  ThisType & operator = (const ThisType & org)
  {
    if( factory_ )
    {
      if( !org.factory_ )
      {
        factory_->freeIntersection( *it_ );
        factory_ = nullptr;
      }
      else
        it_->assign( *org.it_ );
    }
    else if( org.factory_ )
    {
      factory_ = org.factory_;
      it_ = &factory().getIntersection( -1, static_cast< IntersectionIteratorImpl * >( nullptr ) );
      it_->assign( *org.it_ );
    }
    return *this;
  }

  //! The Destructor puts internal object back to stack
  ~IntersectionIteratorWrapper()
  {
    if( factory_ )
      factory_->freeIntersection( *it_ );
  }

  operator bool () const { return bool( factory_ ); }

  //! the equality method
  bool equals ( const ThisType &other ) const
  {
    return (factory_ == other.factory_) && (!factory_ || it().equals( other.it() ));
  }

  //! increment iterator
  void increment () { it().increment(); }

  //! access neighbor
  EntityPointer outside() const { return it().outside(); }

  //! access entity where iteration started
  EntityPointer inside() const { return it().inside(); }

  //! return true if intersection is with boundary. \todo connection with
  //! boundary information, processor/outer boundary
  bool boundary () const { return it().boundary(); }

  //! return true if across the intersection a neighbor on this level exists
  bool neighbor () const { return it().neighbor(); }

  //! return information about the Boundary
  int boundaryId () const { return it().boundaryId(); }

  //! return the boundary segment index
  size_t boundarySegmentIndex() const { return it().boundarySegmentIndex(); }

  //! intersection of codimension 1 of this neighbor with element where
  //! iteration started.
  //! Here returned element is in LOCAL coordinates of the element
  //! where iteration started.
  LocalGeometry geometryInInside () const
  {
    return it().geometryInInside();
  }

  //! intersection of codimension 1 of this neighbor with element where
  //!  iteration started.
  //! Here returned element is in GLOBAL coordinates of the element where
  //! iteration started.
  Geometry geometry () const
  {
    return it().geometry();
  }

  /** \brief obtain the type of reference element for this intersection */
  GeometryType type () const
  {
    return it().type();
  }

  //! local index of codim 1 entity in self where intersection is contained
  //!  in
  int indexInInside () const
  {
    return it().indexInInside();
  }

  //! intersection of codimension 1 of this neighbor with element where
  //! iteration started.
  //! Here returned element is in LOCAL coordinates of neighbor
  LocalGeometry geometryInOutside () const
  {
    return it().geometryInOutside();
  }

  //! local index of codim 1 entity in neighbor where intersection is
  //! contained
  int indexInOutside () const
  {
    return it().indexInOutside();
  }

  //! twist of the face seen from the inner element
  Twist twistInInside() const { return it().twistInInside(); }

  //! twist of the face seen from the outer element
  Twist twistInOutside() const { return it().twistInOutside(); }

  //! return unit outer normal, this should be dependent on local
  //! coordinates for higher order boundary
  const NormalType unitOuterNormal ( const FieldVector< ctype, dim-1 > &local ) const
  {
    return it().unitOuterNormal( local );
  }

  //! return unit outer normal, this should be dependent on local
  //! coordinates for higher order boundary
  const NormalType centerUnitOuterNormal ( ) const
  {
    GeometryType type = geometry().type();
    const ReferenceElement<ctype, dim-1> & refElement =
         ReferenceElements<ctype, dim-1>::general(type);
    return unitOuterNormal(refElement.position(0,0));
  }

  //! return outer normal, this should be dependent on local
  //! coordinates for higher order boundary
  const NormalType outerNormal ( const FieldVector< ctype, dim-1 > &local ) const
  {
    return it().outerNormal( local );
  }

  //! return outer normal, this should be dependent on local
  //! coordinates for higher order boundary
  const NormalType integrationOuterNormal ( const FieldVector< ctype, dim-1 > &local ) const
  {
    return it().integrationOuterNormal( local );
  }

  //! return level of iterator
  int level () const { return it().level(); }

  //! return true if intersection is conform (i.e. only one neighbor)
  bool conforming () const { return it().conforming(); }

  //! returns reference to underlying intersection iterator implementation
  IntersectionIteratorImp & it() { assert( *this ); return *it_; }
  const IntersectionIteratorImp & it() const { assert( *this ); return *it_; }

  //! return weight associated with graph edge between the neighboring elements
  int weight() const
  {
    return it().weight();
  }

private:
  const FactoryType &factory () const { assert( factory_ ); return *factory_; }

  const FactoryType *factory_ ;
  IntersectionIteratorImp *it_;
}; // end class IntersectionIteratorWrapper

template <class GridImp>
class LeafIntersectionWrapper
: public IntersectionIteratorWrapper<GridImp,typename GridImp::LeafIntersectionIteratorImp>
{
  typedef LeafIntersectionWrapper<GridImp> ThisType;
  typedef IntersectionIteratorWrapper<GridImp,typename GridImp::LeafIntersectionIteratorImp> BaseType;
public:
  typedef typename BaseType::FactoryType FactoryType;
  LeafIntersectionWrapper () {}

  //! constructor called from the ibegin and iend method
  template <class EntityImp>
  LeafIntersectionWrapper(const FactoryType& factory, const EntityImp & en, int wLevel , bool end )
    : BaseType(factory,en,wLevel,end)
  {
  }

  //! The copy constructor
  LeafIntersectionWrapper(const ThisType & org)
    : BaseType(org)
  {
  }

};

//! \brief Class that wraps IntersectionIteratorImp of a grid and gets it's
//! internal object from a object stack hold by the grid
template <class GridImp>
class LeafIntersectionIteratorWrapper
{
  typedef LeafIntersectionIteratorWrapper<GridImp> ThisType;
  typedef LeafIntersectionWrapper<GridImp> IntersectionImp;

public:
  typedef typename IntersectionImp::FactoryType FactoryType;
  typedef Dune::Intersection< GridImp, IntersectionImp > Intersection;

  //! dimension
  enum { dimension      = GridImp :: dimension  };
  //! dimensionworld
  enum { dimensionworld = GridImp :: dimensionworld };

  //! define type used for coordinates in grid module
  typedef typename GridImp :: ctype ctype;

  //! Entity type
  typedef typename GridImp::template Codim<0>::Entity Entity;
  //! type of EntityPointer
  typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

  //! type of intersectionGlobal
  typedef typename GridImp::template Codim<1>::Geometry Geometry;
  //! type of intersection*Local
  typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;

  //! type of normal vector
  typedef FieldVector<ctype , dimensionworld> NormalType;

  LeafIntersectionIteratorWrapper () {}

  //! constructor called from the ibegin and iend method
  template <class EntityImp>
  LeafIntersectionIteratorWrapper(const FactoryType& factory, const EntityImp & en, int wLevel , bool end )
  : intersection_( IntersectionImp(factory,en,wLevel,end) )
  {}

  //! The copy constructor
  LeafIntersectionIteratorWrapper(const ThisType & org)
  : intersection_( IntersectionImp( org.impl() ) )
  {}

  //! the f*cking assignment operator
  ThisType & operator = (const ThisType & org)
  {
    impl() = org.impl();
    return *this;
  }

  //! return reference to intersection
  const Intersection &dereference () const
  {
    return intersection_;
  }

  //! the equality method
  bool equals (const ThisType & i) const { return impl().equals( i.impl() ); }

  //! increment iterator
  void increment()
  {
    impl().increment();
  }
protected:
  // intersection object
  Intersection intersection_;

  // return reference to real implementation
  IntersectionImp& impl() { return GridImp :: getRealImplementation( intersection_ ); }
  // return reference to real implementation
  const IntersectionImp& impl() const { return GridImp :: getRealImplementation( intersection_ ); }
}; // end class IntersectionIteratorWrapper

//! \brief Class that wraps IntersectionIteratorImp of a grid and gets it's
//! internal object from a object stack hold by the grid
template <class GridImp>
class LevelIntersectionWrapper
: public IntersectionIteratorWrapper<GridImp,typename GridImp::LevelIntersectionIteratorImp>
{
  typedef LevelIntersectionWrapper<GridImp> ThisType;
  typedef IntersectionIteratorWrapper<GridImp,typename GridImp::LevelIntersectionIteratorImp> BaseType;
public:
  typedef typename BaseType::FactoryType FactoryType;
  LevelIntersectionWrapper () {}

  //! constructor called from the ibegin and iend method
  template <class EntityImp>
  LevelIntersectionWrapper(const FactoryType& factory, const EntityImp & en, int wLevel , bool end )
    : BaseType(factory,en,wLevel,end)
  {
  }

  //! The copy constructor
  LevelIntersectionWrapper(const ThisType & org)
    : BaseType(org)
  {
  }
};

//! \brief Class that wraps IntersectionIteratorImp of a grid and gets it's
//! internal object from a object stack hold by the grid
template <class GridImp>
class LevelIntersectionIteratorWrapper
{
  typedef LevelIntersectionIteratorWrapper<GridImp> ThisType;
  typedef LevelIntersectionWrapper<GridImp> IntersectionImp;
  typedef typename IntersectionImp::FactoryType FactoryType;
public:
  typedef Dune::Intersection< GridImp, IntersectionImp > Intersection;

  //! dimension
  enum { dimension      = GridImp :: dimension  };
  //! dimensionworld
  enum { dimensionworld = GridImp :: dimensionworld };

  //! define type used for coordinates in grid module
  typedef typename GridImp :: ctype ctype;

  //! Entity type
  typedef typename GridImp::template Codim<0>::Entity Entity;
  //! type of EntityPointer
  typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

  //! type of intersectionGlobal
  typedef typename GridImp::template Codim<1>::Geometry Geometry;
  //! type of intersection*Local
  typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;

  //! type of normal vector
  typedef FieldVector<ctype , dimensionworld> NormalType;

  LevelIntersectionIteratorWrapper () {}

  //! constructor called from the ibegin and iend method
  template <class EntityImp>
  LevelIntersectionIteratorWrapper(const FactoryType& factory, const EntityImp & en, int wLevel , bool end )
  : intersection_( IntersectionImp(factory,en,wLevel,end) )
  {}

  //! The copy constructor
  LevelIntersectionIteratorWrapper(const ThisType & org)
  : intersection_( IntersectionImp( org.impl() ) )
  {}

  //! the f*cking assignment operator
  ThisType & operator = (const ThisType & org)
  {
    impl() = org.impl();
    return *this;
  }

  //! return reference to intersection
  const Intersection &dereference () const
  {
    return intersection_;
  }

  //! the equality method
  bool equals (const ThisType & i) const { return impl().equals( i.impl() ); }

  //! increment iterator
  void increment()
  {
    impl().increment();
  }

  // template <class,PartitionIteratorType> friend class MacroGridView; // specialize
protected:
  // intersection object
  Intersection intersection_;

  // return reference to real implementation
  IntersectionImp& impl() { return GridImp :: getRealImplementation( intersection_ ); }
  // return reference to real implementation
  const IntersectionImp& impl() const { return GridImp :: getRealImplementation( intersection_ ); }
}; // end class IntersectionIteratorWrapper

} // end namespace Dune
#endif
