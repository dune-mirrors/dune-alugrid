#ifndef DUNE_ALU3DGRIDITERATOR_HH
#define DUNE_ALU3DGRIDITERATOR_HH

// System includes
#include <type_traits>

// Dune includes
#include <dune/grid/common/grid.hh>
#include <dune/alugrid/common/intersectioniteratorwrapper.hh>
#include <dune/alugrid/common/memory.hh>
#include <dune/alugrid/common/twists.hh>

// Local includes
#include "alu3dinclude.hh"
#include "topology.hh"
#include "faceutility.hh"
#include "alu3diterators.hh"

namespace Dune {
  // Forward declarations
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;
  template<int cd, PartitionIteratorType pitype, class GridImp >
  class ALU3dGridLevelIterator;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointer;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;
  template<class GridImp>
  class ALU3dGridHierarchicIterator;
  template<class GridImp>
  class ALU3dGridIntersectionIterator;
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridLeafIterator;
  template< int, int, ALU3dGridElementType, class >
  class ALU3dGrid;
  template< int, int, ALU3dGridElementType, class >
  class ALU3dGridFaceInfo;
  template< ALU3dGridElementType, class >
  class ALU3dGridGeometricFaceInfo;

//**********************************************************************
//
// --ALU3dGridIntersectionIterator
// --IntersectionIterator
/*!
  Mesh entities of codimension 0 ("elements") allow to visit all neighbors, wh
  a neighbor is an entity of codimension 0 which has a common entity of codimens
  These neighbors are accessed via a IntersectionIterator. This allows the implement
  non-matching meshes. The number of neigbors may be different from the number o
  of an element!
 */
template<class GridImp>
class ALU3dGridIntersectionIterator
//: public IntersectionIteratorDefaultImplementation <GridImp,ALU3dGridIntersectionIterator>
{
  enum { dim       = GridImp::dimension };
  enum { dimworld  = GridImp::dimensionworld };

  typedef typename GridImp::MPICommunicatorType Comm;

  typedef ALU3dImplTraits< GridImp::elementType, Comm > ImplTraits;

  typedef typename ImplTraits::HElementType  HElementType ;
  typedef typename ImplTraits::HBndSegType   HBndSegType;
  typedef typename ImplTraits::GEOElementType GEOElementType;
  typedef typename ImplTraits::IMPLElementType IMPLElementType;
  typedef typename ImplTraits::GEOFaceType GEOFaceType;
  typedef typename ImplTraits::NeighbourPairType NeighbourPairType;
  typedef typename ImplTraits::BNDFaceType BNDFaceType;

  typedef typename ALU3dImplTraits< tetra, Comm >::GEOElementType GEOTetraElementType;
  typedef typename ALU3dImplTraits< tetra, Comm >::BNDFaceType    GEOTriangleBndType;
  typedef typename ALU3dImplTraits< hexa,  Comm >::GEOElementType GEOHexaElementType;
  typedef typename ALU3dImplTraits< hexa,  Comm >::BNDFaceType    GEOQuadBndType;

  typedef ALU3dGridFaceInfo< dim, dimworld, GridImp::elementType, Comm > FaceInfoType;

  typedef typename std::conditional<
    tetra == GridImp::elementType,
    ALU3dGridGeometricFaceInfoTetra< dim, dimworld, Comm >,
    ALU3dGridGeometricFaceInfoHexa< dim, dimworld, Comm > >::type GeometryInfoType;

  typedef ElementTopologyMapping<GridImp::elementType> ElementTopo;
  typedef FaceTopologyMapping<GridImp::elementType> FaceTopo;

  enum { numFaces = (dim==3) ? EntityCount<GridImp::elementType>::numFaces : (GridImp::elementType==tetra ? 3 : 4) };
  enum { numVerticesPerFace =
         GeometryInfoType::numVerticesPerFace };
  enum { numVertices = (dim==3) ? EntityCount<GridImp::elementType>::numVertices  : (GridImp::elementType==tetra ? 3 : 4)};

  typedef ALU3dGridIntersectionIterator<GridImp> ThisType;

  friend class ALU3dGridEntity<0,dim,GridImp>;
  friend class IntersectionIteratorWrapper<GridImp,ThisType>;

protected:
  enum IntersectionIteratorType { IntersectionLeaf , IntersectionLevel, IntersectionBoth };

  typedef typename GridImp::Traits::template Codim< 1 >::GeometryImpl       GeometryImpl;
  typedef typename GridImp::Traits::template Codim< 1 >::LocalGeometryImpl  LocalGeometryImpl;

public:
  typedef ALUTwists< (dim == 3 ) ? GridImp::elementType == tetra ? 3 : 4 : 2, dim-1 > Twists;
  typedef typename Twists::Twist Twist;

  typedef typename GridImp::template Codim<0>::Entity         Entity;
  typedef typename GridImp::template Codim<0>::EntityImp      EntityImp;

  typedef typename GridImp::template Codim<1>::Geometry       Geometry;
  typedef typename GridImp::template Codim<1>::LocalGeometry  LocalGeometry;

  typedef ALU3dGridIntersectionIterator< GridImp > ImplementationType;
  //! type of the intersection
  typedef Dune::Intersection< GridImp, Dune::ALU3dGridIntersectionIterator< GridImp > > Intersection;

  typedef FieldVector<alu3d_ctype, dimworld> NormalType;

  //! The default Constructor
  explicit ALU3dGridIntersectionIterator( const bool levelIntersectionIterator = false );

  //! The copy constructor
  ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org);

  //! assignment of iterators
  void assign(const ALU3dGridIntersectionIterator<GridImp> & org);

  //! The copy constructor
  bool equals (const ALU3dGridIntersectionIterator<GridImp> & i) const;

  //! increment iterator
  void increment ();

  //! access neighbor
  EntityImp outside() const;

  //! access entity where iteration started
  EntityImp inside() const;

  //! return true if intersection is with boundary.
  bool boundary () const;

  //! return true if across the face an neighbor on leaf exists
  bool neighbor () const;

  //! return information about the Boundary
  int boundaryId () const;

  //! return the boundary segment index
  size_t boundarySegmentIndex() const;

  //! return the segment id (non-consecutive)
  int segmentId() const;

  //! intersection of codimension 1 of this neighbor with element where
  //! iteration started.
  //! Here returned element is in LOCAL coordinates of the element
  //! where iteration started.
  LocalGeometry geometryInInside () const;

  //! intersection of codimension 1 of this neighbor with element where
  //!  iteration started.
  //! Here returned element is in GLOBAL coordinates of the element where
  //! iteration started.
  Geometry geometry () const;

  /** \brief obtain the type of reference element for this intersection */
  GeometryType type () const;

  //! local index of codim 1 entity in self where intersection is contained
  //!  in
  int indexInInside () const;

  //! intersection of codimension 1 of this neighbor with element where
  //! iteration started.
  //! Here returned element is in LOCAL coordinates of neighbor
  LocalGeometry geometryInOutside () const;

  //! local index of codim 1 entity in neighbor where intersection is
  //! contained
  int indexInOutside () const;

  //! returns twist of face compared to inner element
  Twist twistInInside () const;

  //! returns twist of face compared to outer element
  Twist twistInOutside () const;

  //! return unit outer normal, this should be dependent on local
  //! coordinates for higher order boundary
  NormalType unitOuterNormal (const FieldVector<alu3d_ctype, dim-1>& local) const ;

  //! return outer normal, this should be dependent on local
  //! coordinates for higher order boundary
  NormalType outerNormal (const FieldVector<alu3d_ctype, dim-1>& local) const;

  //! return outer normal, this should be dependent on local
  //! coordinates for higher order boundary
  NormalType integrationOuterNormal (const FieldVector<alu3d_ctype, dim-1>& local) const;

  //! return level of iterator (level of item)
  int level () const;

  int outsideLevel () const { return connector_.outsideLevel(); }

  //! return true if intersection is conforming
  bool conforming () const
  {
    return (connector_.conformanceState() == FaceInfoType::CONFORMING);
  }

  //! return current face
  const GEOFaceType& getItem() const { return connector_.face(); }

  //! return communication weight
  int weight() const
  {
    return this->getItem().weight();
  }

protected:
  // set interator to end iterator
  void done () ;
  template< class EntityType > void done ( const EntityType &en ) { done(); }

  // reset IntersectionIterator to first neighbour
  void setFirstItem(const HElementType & elem, int wLevel);

  // reset IntersectionIterator to first neighbour
  void setInteriorItem(const HElementType  & elem,
                       const BNDFaceType& bnd, int wLevel);

  // reset IntersectionIterator to first neighbour
  void first(const EntityImp& en, int wLevel, const GridImp& grid );

  // set new face
  void setNewFace(const GEOFaceType& newFace);

private:
  // set new face (only LeafIntersectionIterator)
  void setGhostFace(const GEOFaceType& newFace);

protected:
  // generate local geometries
  void buildLocalGeometries() const;

  // get the face corresponding to the index
  const typename ALU3dImplTraits< tetra, Comm >::GEOFaceType *
  getFace ( const GEOTriangleBndType &bnd, int index ) const;

  // get the face corresponding to the index
  const typename ALU3dImplTraits< hexa, Comm >::GEOFaceType *
  getFace ( const GEOQuadBndType &bnd, int index ) const;

  // get the face corresponding to the index
  const typename ALU3dImplTraits< tetra, Comm >::GEOFaceType *
  getFace ( const GEOTetraElementType &elem, int index ) const;

  const typename ALU3dImplTraits< hexa, Comm >::GEOFaceType *
  getFace ( const GEOHexaElementType &elem, int index ) const;

  //! structure containing the topological and geometrical information about
  //! the face which the iterator points to
  mutable FaceInfoType      connector_;
  mutable GeometryInfoType  geoProvider_; // need to initialise

  //! current element from which we started the intersection iterator
  const IMPLElementType* item_;

  //! current pointer to ghost face if iterator was started from ghost element
  const BNDFaceType* ghost_;

  //! pointer to grid implementation
  const GridImp* grid_;

  mutable int innerLevel_;
  mutable int index_;

  mutable GeometryImpl      intersectionGlobal_;
  mutable LocalGeometryImpl intersectionSelfLocal_;
  mutable LocalGeometryImpl intersectionNeighborLocal_;

  // unit outer normal
  mutable NormalType unitOuterNormal_;

public:
  // used by SharedPointer
  void invalidate() { done(); }
  // refCount used by SharedPointer
  unsigned int refCount_;
};

template<class GridImp>
class ALU3dGridLevelIntersectionIterator :
public ALU3dGridIntersectionIterator<GridImp>
{
  enum { dim       = GridImp::dimension };
  enum { dimworld  = GridImp::dimensionworld };

  typedef typename GridImp::MPICommunicatorType Comm;

  typedef ALU3dImplTraits< GridImp::elementType, Comm > ImplTraits;

  typedef typename ImplTraits::HElementType  HElementType ;
  typedef typename ImplTraits::GEOElementType GEOElementType;
  typedef typename ImplTraits::IMPLElementType IMPLElementType;
  typedef typename ImplTraits::GEOFaceType GEOFaceType;
  typedef typename ImplTraits::NeighbourPairType NeighbourPairType;
  typedef typename ImplTraits::BNDFaceType BNDFaceType;

  typedef ALU3dGridFaceInfo< dim, dimworld,  GridImp::elementType, Comm > FaceInfoType;

  typedef typename std::conditional<
    tetra == GridImp::elementType,
    ALU3dGridGeometricFaceInfoTetra< dim, dimworld, Comm >,
    ALU3dGridGeometricFaceInfoHexa< dim, dimworld, Comm > >::type GeometryInfoType;

  typedef ElementTopologyMapping<GridImp::elementType> ElementTopo;
  typedef FaceTopologyMapping<GridImp::elementType> FaceTopo;

  enum { numFaces = (dim==3) ? EntityCount<GridImp::elementType>::numFaces : (GridImp::elementType==tetra ? 3 : 4) };
  enum { numVerticesPerFace =
         GeometryInfoType::numVerticesPerFace };
  enum { numVertices = (dim==3) ? EntityCount<GridImp::elementType>::numVertices  : (GridImp::elementType==tetra ? 3 : 4)};

  typedef ALU3dGridIntersectionIterator<GridImp>      BaseType;
  typedef ALU3dGridLevelIntersectionIterator<GridImp> ThisType;

  typedef typename BaseType :: EntityImp  EntityImp;

  friend class ALU3dGridEntity<0,dim,GridImp>;
  friend class IntersectionIteratorWrapper<GridImp,ThisType>;
protected:
  using BaseType :: item_;
  using BaseType :: ghost_;
  using BaseType :: grid_;
  using BaseType :: innerLevel_;
  using BaseType :: index_;
  using BaseType :: connector_;
  using BaseType :: geoProvider_;
  using BaseType :: boundary;
  using BaseType :: done ;
  using BaseType :: getFace;
  using BaseType :: neighbor ;

public:
  //! The default Constructor
  ALU3dGridLevelIntersectionIterator();

  //! The copy constructor
  ALU3dGridLevelIntersectionIterator(const ThisType & org);

  //! assignment of iterators
  void assign(const ThisType & org);

  //! increment iterator
  void increment ();

  // reset IntersectionIterator to first neighbour
  void first(const EntityImp& en, int wLevel, const GridImp& grid );

  //! return true if across the edge an neighbor on this level exists
  bool neighbor () const;

  //! return true if intersection is conforming
  bool conforming () const
  {
    alugrid_assert ( ( ! connector_.conformingRefinement() ) ?
      ( !neighbor() || this->connector_.conformanceState() == FaceInfoType::CONFORMING ) : true );
    // for conforming refinement use base implementation
    // otherwise its true
    return connector_.conformingRefinement() ?
      BaseType :: conforming() : true ;
  }

private:
  // set new face
  void setNewFace(const GEOFaceType& newFace);

  // reset IntersectionIterator to first neighbour
  void setFirstItem(const HElementType & elem, int wLevel);

  // reset IntersectionIterator to first neighbour
  void setInteriorItem(const HElementType  & elem,
                       const BNDFaceType& bnd, int wLevel);

  bool levelNeighbor_;
  bool isLeafItem_;
};

//////////////////////////////////////////////////////////////////////////////
//
//  --IterationImpl
//
//////////////////////////////////////////////////////////////////////////////
template <class InternalIteratorType >
class ALU3dGridTreeIterator
{
public:
  typedef typename InternalIteratorType :: val_t val_t;

  // here the items level will do
  template <class GridImp, int dim, int codim>
  class GetLevel
  {
  public:
    template <class ItemType>
    static int getLevel(const GridImp & grid, const ItemType & item, int level )
    {
      alugrid_assert ( & item );
      return (level < 0) ? item.level() : level;
    }
  };

  // level is not needed for codim = 0
  template <class GridImp, int dim>
  class GetLevel<GridImp,dim,0>
  {
  public:
    template <class ItemType>
    static int getLevel(const GridImp & grid, const ItemType & item, int level )
    {
      return level;
    }
  };

  template <class GridImp, int dim>
  class GetLevel<GridImp, dim, dim>
  {
  public:
    template <class ItemType>
    static int getLevel(const GridImp & grid, const ItemType & item, int level)
    {
      return (level < 0) ? grid.getLevelOfLeafVertex(item) : level;
    }
  };

protected:
  // set iterator to first item
  template <class GridImp, class IteratorImp>
  void firstItem(const GridImp & grid, IteratorImp & it, int level )
  {
    InternalIteratorType & iter = it.internalIterator();
    iter.first();
    ValidItem<IteratorImp::codimension, GridImp> validate;
    while(!validate(grid,iter))
    {
      iter.next();
      if(iter.done())
      {
        it.removeIter();
        return ;
      }
    }
    if( ! iter.done() )
    {
      alugrid_assert ( iter.size() > 0 );
      setItem(grid,it,iter,level);
    }
    else
    {
      it.removeIter();
    }
  }

  // set the iterators entity to actual item
  template <class GridImp, class IteratorImp>
  void setItem (const GridImp & grid, IteratorImp & it, InternalIteratorType & iter, int level)
  {
    enum { codim = IteratorImp :: codimension };
    val_t & item = iter.item();
    alugrid_assert ( item.first || item.second );
    if( item.first )
    {
      it.updateEntityPointer( item.first ,
          GetLevel<GridImp,GridImp::dimension,codim>::getLevel(grid, *(item.first) , level) );
    }
    else
      it.updateGhostPointer( *item.second );
  }

  // increment iterator
  template <class GridImp, class IteratorImp>
  void incrementIterator(const GridImp & grid, IteratorImp & it, int level)
  {
    // if iter_ is zero, then end iterator
    InternalIteratorType & iter = it.internalIterator();
    ValidItem<IteratorImp::codimension, GridImp> validate;
    do{
      iter.next();

      if(iter.done())
      {
        it.removeIter();
        return ;
      }

    }while(!(validate(grid,iter) ) );

    setItem(grid,it,iter,level);
    return ;
  }

private:
  // in 2d check if item is valid
  template <int codim, class GridImp>
  struct ValidItem
  {
    bool operator()(const GridImp & grid, InternalIteratorType & iter)
    {
      if(GridImp::dimension ==3 || iter.done())  return true;
      else if (GridImp::dimension == 2)
      {
        typedef typename ALU3dImplTraits<GridImp::elementType, typename GridImp::MPICommunicatorType>::template Codim<GridImp::dimension, codim>::ImplementationType GEOElementType;
        val_t & item = iter.item();
        alugrid_assert ( item.first || item.second );
        if( item.first )
        {
          GEOElementType* elem = static_cast<GEOElementType*> (item.first);
          //an element is valid if the 2d flag is set
          return elem->is2d();
        }
        //if we have a ghost entity, it is the right one, as we did not insert non-2d elements into the ghostlist
        // see alu3diterators.hh method updateGhostlist
        else if( item.second )
          return true;
      }
      return false;
    }
  };

  template <class GridImp>
  struct ValidItem<0, GridImp>
  {
    bool operator()(const GridImp & grid, InternalIteratorType & iter)
    {
      return true;
    }
  };

};

//**********************************************************************
//
// --ALU3dGridLevelIterator
// --LevelIterator
/*!
 Enables iteration over all entities of a given codimension and level of a grid.
 */
template<int cd, PartitionIteratorType pitype, class GridImp>
class ALU3dGridLevelIterator
: public ALU3dGridEntityPointer< cd, GridImp >,
  public ALU3dGridTreeIterator<typename ALU3DSPACE IteratorSTI<
        typename ALU3DSPACE IteratorElType< (GridImp::dimension == 2 && cd == 2) ? 3 : cd, typename GridImp::MPICommunicatorType >::val_t > >
  //ALU3DSPACE ALU3dGridLevelIteratorWrapper< (GridImp::dimension == 2 && cd == 2) ? 3 : cd, pitype, typename GridImp::MPICommunicatorType > >
{
  enum { dim       = GridImp::dimension };
  enum { dimworld  = GridImp::dimensionworld };

  typedef typename GridImp::MPICommunicatorType Comm;

  friend class ALU3dGridEntity<3,dim,GridImp>;
  friend class ALU3dGridEntity<2,dim,GridImp>;
  friend class ALU3dGridEntity<1,dim,GridImp>;
  friend class ALU3dGridEntity<0,dim,GridImp>;
  friend class ALU3dGrid< dim, dimworld, GridImp::elementType, Comm >;

  friend class ALU3dGridTreeIterator<
    typename ALU3DSPACE IteratorSTI<
        typename ALU3DSPACE IteratorElType< (GridImp::dimension == 2 && cd == 2) ? 3 : cd, typename GridImp::MPICommunicatorType >::val_t >
    //ALU3DSPACE ALU3dGridLevelIteratorWrapper< (GridImp::dimension == 2 && cd == 2) ? 3 : cd, pitype, Comm >
    >;
  typedef ALU3dGridEntityPointer< cd, GridImp > BaseType;

public:
  typedef typename GridImp::template Codim<cd>::Entity Entity;
  typedef ALU3dGridVertexList< Comm > VertexListType;

  //! typedef of my type
  typedef ALU3dGridLevelIterator<cd,pitype,GridImp> ThisType;
  // the wrapper for the original iterator of the ALU3dGrid
  //typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper< (GridImp::dimension == 2 && cd == 2) ? 3 : cd, pitype, Comm > IteratorType;

  typedef typename ALU3DSPACE IteratorElType< (GridImp::dimension == 2 && cd == 2) ? 3 : cd, Comm >::val_t val_t;
  typedef typename ALU3DSPACE IteratorSTI< val_t > IteratorType ;
  typedef IteratorType InternalIteratorType;

  /** \brief default constructor */
  ALU3dGridLevelIterator () : grid_( nullptr ), iter_(), level_( 0 ) {}

  //! Constructor for begin iterator
  ALU3dGridLevelIterator(const GridImp& grid, int level, bool);

  //! Constructor for end iterator
  ALU3dGridLevelIterator(const GridImp& grid, int level);

  //! Constructor
  ALU3dGridLevelIterator(const ThisType & org);

  // destructor
  ~ALU3dGridLevelIterator();

  //! prefix increment
  void increment ();

  //! release entity
  void releaseEntity () {}

  //! assignment of iterators
  ThisType & operator = (const ThisType & org);
private:
  //! do assignment
  void assign (const ThisType & org);

  const GridImp &grid () const { alugrid_assert( grid_ ); return *grid_; }

  // reference to factory class (ie grid)
  const GridImp *grid_;

  // the internal iterator
  std::unique_ptr< IteratorType > iter_ ;

  // actual level
  int level_;

  // deletes iter_
  void removeIter ();

  IteratorType & internalIterator ()
  {
    alugrid_assert ( iter_ );
    return *iter_;
  }
};

//********************************************************************
//
//  --ALU3dGridLeafIterator
//  --LeafIterator
//
//********************************************************************
//! Leaf iterator
template<int cdim, PartitionIteratorType pitype, class GridImp>
class ALU3dGridLeafIterator
: public ALU3dGridEntityPointer< cdim, GridImp >,
  public ALU3dGridTreeIterator<typename ALU3DSPACE IteratorSTI<
        typename ALU3DSPACE IteratorElType< (GridImp::dimension == 2 && cdim == 2) ? 3 : cdim, typename GridImp::MPICommunicatorType >::val_t > >
{
  enum { dim = GridImp :: dimension };

  friend class ALU3dGridEntity<cdim,dim,GridImp>;
  enum { codim = cdim };

  typedef typename GridImp::MPICommunicatorType Comm;

  typedef ALU3dGridEntityPointer< cdim, GridImp > BaseType;

public:
  typedef typename GridImp::template Codim<cdim>::Entity Entity;

  typedef typename ALU3DSPACE IteratorElType< (GridImp::dimension == 2 && cdim == 2) ? 3 : cdim, Comm >::val_t val_t;
  typedef typename ALU3DSPACE IteratorSTI< val_t > IteratorType ;
  friend class ALU3dGridTreeIterator< IteratorType > ;

  typedef IteratorType InternalIteratorType;

  typedef ALU3dGridLeafIterator<cdim, pitype, GridImp> ThisType;

  /** \brief default constructor */
  ALU3dGridLeafIterator () : grid_( nullptr ), iter_() {}

  //! Constructor for end iterators
  ALU3dGridLeafIterator(const GridImp& grid, int level);

  //! Constructor for begin Iterators
  ALU3dGridLeafIterator(const GridImp& grid, int level , bool isBegin);

  //! copy Constructor
  ALU3dGridLeafIterator(const ThisType & org);

  //! destructor deleting real iterator
  ~ALU3dGridLeafIterator();

  //! prefix increment
  void increment ();

  //! release entity
  void releaseEntity () {}

  //! assignment of iterators
  ThisType & operator = (const ThisType & org);

private:
  const GridImp &grid () const { alugrid_assert( grid_ ); return *grid_; }

  // reference to grid class (ie grid)
  const GridImp *grid_;

  // the internal iterator
  std::unique_ptr< IteratorType > iter_;

  //! do assignment
  void assign (const ThisType & org);

  // deletes iter_
  void removeIter () ;

  // return reference to iter_
  InternalIteratorType & internalIterator ()
  {
    alugrid_assert ( iter_ );
    return *iter_;
  }
};

// - HierarchicIteraor
// --HierarchicIterator
template<class GridImp>
class ALU3dGridHierarchicIterator
: public ALU3dGridEntityPointer<0,GridImp>
// public HierarchicIteratorDefaultImplementation <GridImp,ALU3dGridHierarchicIterator>
{
  typedef ALU3dGridHierarchicIterator<GridImp> ThisType;
  enum { dim = GridImp::dimension };

  typedef typename GridImp::MPICommunicatorType Comm;

  typedef ALU3dImplTraits< GridImp::elementType, Comm > ImplTraits;
  typedef typename ImplTraits::HElementType HElementType;
  typedef typename ImplTraits::HBndSegType HBndSegType;

  template < class PointerType, class CommT >
  class GhostElementStorage;

  //! empty implementation for
  template < class PointerType >
  class GhostElementStorage< PointerType, ALUGridNoComm >
  {
  public:
    GhostElementStorage() {}
    explicit GhostElementStorage( const PointerType& ) {}
    PointerType& operator * () {  PointerType* p = 0; alugrid_assert ( false ); abort(); return *p; }
    const PointerType* ghost () const { return 0; }
    PointerType* nextGhost () const { return 0; }
    PointerType* operator -> () const { return 0; }
    bool operator != (const PointerType* ) const { return false; }
    bool operator ! () const { return true ; }
    GhostElementStorage& operator= (const GhostElementStorage& ) { return *this; }
    GhostElementStorage& operator= (const PointerType* )  { return *this;  }
    bool valid () const { return false; }
  };

  //! implementation holding two ghost pointer
  template < class PointerType >
  class GhostElementStorage< PointerType, ALUGridMPIComm >
  {
  private:
    // pointers to ghost and current ghost
    const HBndSegType * ghost_;
    HBndSegType * nextGhost_;
  public:
    GhostElementStorage() : ghost_( 0 ), nextGhost_( 0 ) {}
    explicit GhostElementStorage( const PointerType& gh ) : ghost_( &gh ), nextGhost_( 0 ) {}
    GhostElementStorage( const GhostElementStorage& org )
      : ghost_( org.ghost_ ), nextGhost_( org.nextGhost_ ) {}

    PointerType& operator * () { alugrid_assert ( nextGhost_ ); return *nextGhost_; }
    const PointerType* ghost () const { return ghost_; }
    PointerType* nextGhost () const { return nextGhost_; }
    PointerType* operator -> () { return nextGhost_; }
    bool operator != (const PointerType* p ) const { return (nextGhost_ != p); }
    bool operator ! () const { return nextGhost_ == 0; }
    GhostElementStorage& operator= (const GhostElementStorage& org)
    {
      ghost_ = org.ghost_;
      nextGhost_ = org.nextGhost_;
      return *this;
    }
    GhostElementStorage& operator= (PointerType* p)
    {
      nextGhost_ = p;
      return *this;
    }
    bool valid () const { return (ghost_ != 0); }
  };

public:
  typedef typename GridImp::template Codim<0>::Entity Entity;
  typedef typename GridImp::ctype ctype;

  //! the normal Constructor
  ALU3dGridHierarchicIterator(const HElementType & elem,
                              int maxlevel, bool end );

  //! start constructor for ghosts
  ALU3dGridHierarchicIterator(const HBndSegType& ghost,
                              int maxlevel,
                              bool end);

  //! the normal Constructor
  ALU3dGridHierarchicIterator(const ThisType &org);

  //! the default Constructor
  ALU3dGridHierarchicIterator();

  //! increment
  void increment();

  //! release entity
  void releaseEntity () {}

  //! the assignment operator
  ThisType & operator = (const ThisType & org);

private:
  // assign iterator
  void assign(const ThisType & org);

  //! return level of item
  int getLevel(const HElementType* item) const;

  //! return correct level for ghosts
  int getLevel(const HBndSegType* face) const;

  // go to next valid element
  template <class HItemType>
  HItemType* goNextElement (const HItemType* startElem, HItemType * oldEl);

  //! element from where we started
  const HElementType * elem_;

  // pointers to ghost and current ghost
  GhostElementStorage< HBndSegType, Comm > ghostElem_;

  //! maximal level to go down
  int maxlevel_;
};


} // end namespace Dune

#if COMPILE_ALUGRID_INLINE
  #include "iterator.cc"
#endif
#endif // header guard
