#ifndef DUNE_ALUGRID_ITERATOR_IMP_CC
#define DUNE_ALUGRID_ITERATOR_IMP_CC

#include "alu3dinclude.hh"

#include "geometry.hh"
#include "entity.hh"
#include "grid.hh"
#include "faceutility.hh"

namespace Dune {

/************************************************************************************
  ###
   #     #    #   #####  ######  #####    ####   ######   ####      #     #####
   #     ##   #     #    #       #    #  #       #       #    #     #       #
   #     # #  #     #    #####   #    #   ####   #####   #          #       #
   #     #  # #     #    #       #####        #  #       #          #       #
   #     #   ##     #    #       #   #   #    #  #       #    #     #       #
  ###    #    #     #    ######  #    #   ####   ######   ####      #       #
************************************************************************************/

// --IntersectionIterator
template<class GridImp>
alu_inline ALU3dGridIntersectionIterator<GridImp> ::
ALU3dGridIntersectionIterator(const bool levelIntersectionIterator) :
  connector_( levelIntersectionIterator ),
  geoProvider_(connector_),
  item_( nullptr ),
  ghost_( nullptr ),
  grid_( nullptr ),
  index_(-1)
{
}

template<class GridImp>
alu_inline void
ALU3dGridIntersectionIterator<GridImp> :: done ()
{
  item_  = nullptr;
  ghost_ = nullptr;
  grid_  = nullptr;
  // index < 0 indicates end iterator
  index_ = -1;
}

template<class GridImp>
alu_inline void ALU3dGridIntersectionIterator<GridImp> ::
setFirstItem (const HElementType & elem, int wLevel)
{
  ghost_      = nullptr;
  item_       = static_cast<const IMPLElementType *> (&elem);

  // Get first face
  const GEOFaceType* firstFace = getFace(*item_, index_);

  const GEOFaceType* childFace = firstFace->down();
  if( childFace ) firstFace = childFace;

  // Store the face in the connector
  setNewFace(*firstFace);
}

template<class GridImp>
alu_inline void ALU3dGridIntersectionIterator<GridImp> ::
setInteriorItem (const HElementType & elem, const BNDFaceType& ghost, int wLevel)
{
  // get correct face number
  index_ = ElementTopo::alu2duneFace( ghost.getGhost().second );

  // store ghost for method inside
  ghost_   = &ghost;

  // Get first face
  const GEOFaceType* firstFace = getFace( ghost, index_ );
  item_   = static_cast<const IMPLElementType *> (&elem);

  const GEOFaceType* childFace = firstFace->down();
  if( childFace ) firstFace = childFace;

  // Store the face in the connector
  setGhostFace(*firstFace);
}

template<class GridImp>
alu_inline void ALU3dGridIntersectionIterator<GridImp> ::
first (const EntityImp& en, int wLevel, const GridImp& grid )
{
  if( ! en.isLeaf() && en.level()>0)
  {
    done();
    return ;
  }

  // store grid point for boundarySegmentIndex
  grid_ = &grid;

  // adjust connector flags
  connector_.setFlags( grid.conformingRefinement(), grid.ghostCellsEnabled() );

  innerLevel_ = en.level();
  index_  = 0;

  if( en.isGhost() )
  {
    setInteriorItem(en.getItem(), en.getGhost(), wLevel);
  }
  else
  {
    // for the 2d version numFaces is smaller then the actual
    // stored nFaces of the element
    alugrid_assert ( dim == 3 ?
                    (numFaces == en.getItem().nFaces()) :
                    (numFaces  < en.getItem().nFaces())  );
    setFirstItem(en.getItem(), wLevel);
  }
}

// copy constructor
template<class GridImp>
alu_inline ALU3dGridIntersectionIterator<GridImp> ::
ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org) :
  connector_(org.connector_),
  geoProvider_(connector_),
  item_(org.item_),
  ghost_(org.ghost_),
  grid_( org.grid_ )
{
  if(org.item_)
  { // else it's a end iterator
    item_        = org.item_;
    innerLevel_  = org.innerLevel_;
    index_       = org.index_;
  }
  else
  {
    done();
  }
}

// copy constructor
template<class GridImp>
alu_inline void
ALU3dGridIntersectionIterator<GridImp> ::
assign(const ALU3dGridIntersectionIterator<GridImp> & org)
{
  if(org.item_)
  {
    // adjust connector flags
    connector_.setFlags( org.connector_.conformingRefinement(), org.connector_.ghostCellsEnabled() );

    // else it's an end iterator
    item_       = org.item_;
    ghost_      = org.ghost_;
    grid_       = org.grid_;
    innerLevel_ = org.innerLevel_;
    index_      = org.index_;
    connector_.updateFaceInfo(org.connector_.face(),innerLevel_,
                              item_->twist(ElementTopo::dune2aluFace(index_)));
    geoProvider_.resetFaceGeom();
  }
  else {
    done();
  }
  alugrid_assert ( equals(org) );
}

// check whether entities are the same or whether iterator is done
template<class GridImp>
alu_inline bool ALU3dGridIntersectionIterator<GridImp> ::
equals (const ALU3dGridIntersectionIterator<GridImp> & i ) const
{
  // this method is only to check equality of real iterators and end
  // iterators
  return ((item_  == i.item_) &&
          (index_ == i.index_ )
         );
}

template<class GridImp>
alu_inline void ALU3dGridIntersectionIterator<GridImp> :: increment ()
{
  // leaf increment
  alugrid_assert (item_);

  const GEOFaceType * nextFace = 0;

  // When neighbour element is refined, try to get the next child on the face
  if (connector_.conformanceState() == FaceInfoType::REFINED_OUTER)
  {
    nextFace = connector_.face().next();

    // There was a next child face...
    if (nextFace)
    {
      if( ImplTraits :: isGhost( ghost_ ) )
      {
        setGhostFace( *nextFace );
      }
      else
      {
        setNewFace(*nextFace);
      }
      return; // we found what we were looking for...
    }
  } // end if

  // Next face number of starting element
  ++index_;

  // When the face number is larger than the number of faces an element
  // can have, we've reached the end...
  // for ghost elements here is finito
  if (index_ >= numFaces || ghost_ )
  {
    done();
    return;
  }

  // ... else we can take the next face
  nextFace = getFace(connector_.innerEntity(), index_);
  alugrid_assert (nextFace);

  // Check whether we need to go down first
  //if (nextFace has children which need to be visited)
  const GEOFaceType * childFace = nextFace->down();
  if( childFace ) nextFace = childFace;

  alugrid_assert (nextFace);
  setNewFace(*nextFace);
  return;
}


template<class GridImp>
alu_inline typename ALU3dGridIntersectionIterator<GridImp>::EntityImp
ALU3dGridIntersectionIterator<GridImp>::outside () const
{
  alugrid_assert ( neighbor() );
  // make sure that outside is not called for an end iterator

  if( connector_.ghostBoundary() )
  {
    // create entity pointer with ghost boundary face
    return EntityImp( connector_.boundaryFace() );
  }

  alugrid_assert ( &connector_.outerEntity() );
  return EntityImp( connector_.outerEntity() );
}

template<class GridImp>
alu_inline typename ALU3dGridIntersectionIterator<GridImp>::EntityImp
ALU3dGridIntersectionIterator<GridImp>::inside () const
{
  if( ImplTraits :: isGhost( ghost_ ) )
  {
    return EntityImp( *ghost_ );
  }
  else
  {
    // make sure that inside is not called for an end iterator
    return EntityImp( connector_.innerEntity() );
  }
}

template<class GridImp>
alu_inline bool ALU3dGridIntersectionIterator<GridImp> :: boundary () const
{
  return connector_.boundary();
}

template<class GridImp>
alu_inline bool ALU3dGridIntersectionIterator<GridImp> :: neighbor () const
{
  return connector_.neighbor();
}

template<class GridImp>
alu_inline int
ALU3dGridIntersectionIterator< GridImp >::indexInInside () const
{
  alugrid_assert (ElementTopo::dune2aluFace(index_) == connector_.innerALUFaceIndex());
  return index_;
}

template< class GridImp >
alu_inline typename ALU3dGridIntersectionIterator< GridImp >::LocalGeometry
ALU3dGridIntersectionIterator< GridImp >::geometryInInside () const
{
  buildLocalGeometries();
  return LocalGeometry( intersectionSelfLocal_ );
}


template< class GridImp >
alu_inline int
ALU3dGridIntersectionIterator< GridImp >::indexInOutside () const
{
  return ElementTopo::alu2duneFace( connector_.outerALUFaceIndex() );
}

template< class GridImp >
alu_inline typename ALU3dGridIntersectionIterator< GridImp >::Twist
ALU3dGridIntersectionIterator< GridImp >::twistInInside () const
{
  return Twist( connector_.duneTwist( indexInInside(), connector_.innerTwist() ) );
}

template< class GridImp >
alu_inline typename ALU3dGridIntersectionIterator< GridImp >::Twist
ALU3dGridIntersectionIterator< GridImp >::twistInOutside () const
{
  const int indexOutside = indexInOutside();
  auto twist = Twist( connector_.duneTwist( indexOutside, connector_.outerTwist() ) );

  if constexpr ( GridImp :: dimension == 2 )
  {
    // fix for periodic boundaries (in 2d)
    if( neighbor() && boundary() )
    {
      const int indexInside = indexInInside();
      // max face number in 2d, 3 for quads and 2 for triangles
      constexpr int maxFaceNum  = (GridImp::elementType == hexa) ? 3 : 2;
      if( indexInside == indexOutside ||
          std::abs(indexInside - indexOutside ) == maxFaceNum )
      {
        // flip twist in these cases
        twist = Twist( 1 - int(twist) );
      }
    }
  }
  return twist;
}

template< class GridImp >
alu_inline typename ALU3dGridIntersectionIterator< GridImp >::LocalGeometry
ALU3dGridIntersectionIterator< GridImp >::geometryInOutside () const
{
  alugrid_assert (neighbor());
  buildLocalGeometries();
  return LocalGeometry( intersectionNeighborLocal_ );
}

template<class GridImp>
alu_inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
ALU3dGridIntersectionIterator<GridImp>::
integrationOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
{
  return this->outerNormal(local);
}

template<class GridImp>
alu_inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
ALU3dGridIntersectionIterator<GridImp>::
outerNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
{
  alugrid_assert (item_ != 0);

  if(GridImp::dimension == 2 && GridImp::dimensionworld == 3)
  {
    typedef typename LocalGeometry::GlobalCoordinate Coordinate;
    typedef typename GridImp::template Codim<0>::Geometry ElementGeometry;

    NormalType outerNormal;

    const auto& refElement = GridImp::referenceElement();

    Coordinate xInside = geometryInInside().global( local );
    Coordinate refNormal = refElement.integrationOuterNormal( indexInInside() );

    const ElementGeometry insideGeom = inside().geometry();
    insideGeom.jacobianInverseTransposed( xInside ).mv( refNormal, outerNormal );
    outerNormal *= insideGeom.integrationElement( xInside );
    if(connector_.conformanceState() == FaceInfoType::REFINED_OUTER) outerNormal *=0.5;
    return outerNormal;
  }

  return geoProvider_.outerNormal(local);
}

template<class GridImp>
alu_inline typename ALU3dGridIntersectionIterator<GridImp>::NormalType
ALU3dGridIntersectionIterator<GridImp>::
unitOuterNormal(const FieldVector<alu3d_ctype, dim-1>& local) const
{
  unitOuterNormal_ = this->outerNormal(local);
  unitOuterNormal_ *= (1.0/unitOuterNormal_.two_norm());
  return unitOuterNormal_;
}

template< class GridImp >
alu_inline typename ALU3dGridIntersectionIterator< GridImp >::Geometry
ALU3dGridIntersectionIterator< GridImp >::geometry () const
{
  geoProvider_.buildGlobalGeom( intersectionGlobal_ );
  return Geometry( intersectionGlobal_ );
}

template<class GridImp>
alu_inline GeometryType
ALU3dGridIntersectionIterator<GridImp>::
type () const
{
  return GridImp::elementType == tetra ?
    GeometryTypes::simplex(dim-1) : GeometryTypes::cube(dim-1);
}

template<class GridImp>
alu_inline int
ALU3dGridIntersectionIterator<GridImp>::boundaryId () const
{
  alugrid_assert ( item_ );
  return ( boundary() ) ? connector_.boundaryId() : 0;
}

template<class GridImp>
alu_inline size_t
ALU3dGridIntersectionIterator<GridImp>::boundarySegmentIndex() const
{
  alugrid_assert ( item_ );
  alugrid_assert ( boundary() );
  alugrid_assert ( grid_ );
  return grid_->macroBoundarySegmentIndexSet().index( segmentId() );
}

template<class GridImp>
alu_inline int
ALU3dGridIntersectionIterator<GridImp>::segmentId() const
{
  alugrid_assert ( item_ );
  alugrid_assert ( boundary() );
  return connector_.segmentId();
}

template< class GridImp >
alu_inline void ALU3dGridIntersectionIterator< GridImp >::buildLocalGeometries() const
{
  intersectionSelfLocal_.buildGeom( geoProvider_.intersectionSelfLocal() );
  if ( !connector_.outerBoundary() )
    intersectionNeighborLocal_.buildGeom( geoProvider_.intersectionNeighborLocal() );
}

template <class GridImp>
alu_inline const typename ALU3dImplTraits< tetra, typename GridImp::MPICommunicatorType >::GEOFaceType *
ALU3dGridIntersectionIterator<GridImp>::
getFace(const GEOTriangleBndType& bnd, int index) const
{
  return bnd.myhface3(0);
}

template <class GridImp>
alu_inline const typename ALU3dImplTraits< hexa, typename GridImp::MPICommunicatorType >::GEOFaceType *
ALU3dGridIntersectionIterator<GridImp>::
getFace(const GEOQuadBndType& bnd, int index) const
{
  return bnd.myhface4(0);
}

template <class GridImp>
alu_inline const typename ALU3dImplTraits< tetra, typename GridImp::MPICommunicatorType >::GEOFaceType *
ALU3dGridIntersectionIterator<GridImp>::
getFace(const GEOTetraElementType& elem, int index) const
{
  alugrid_assert (index >= 0 && index < numFaces);
  return elem.myhface(ElementTopo::dune2aluFace(index));
}

template <class GridImp>
alu_inline const typename ALU3dImplTraits< hexa, typename GridImp::MPICommunicatorType >::GEOFaceType *
ALU3dGridIntersectionIterator<GridImp>::
getFace(const GEOHexaElementType& elem, int index) const
{
  alugrid_assert (index >= 0 && index < numFaces);
  return elem.myhface(ElementTopo::dune2aluFace(index));
}

template <class GridImp>
alu_inline void ALU3dGridIntersectionIterator<GridImp>::
setNewFace(const GEOFaceType& newFace)
{
  alugrid_assert ( ! ghost_ );
  alugrid_assert ( innerLevel_ == item_->level() );
  connector_.updateFaceInfo(newFace,innerLevel_,
              item_->twist(ElementTopo::dune2aluFace(index_)) );
  geoProvider_.resetFaceGeom();
}

template <class GridImp>
alu_inline void ALU3dGridIntersectionIterator<GridImp>::
setGhostFace(const GEOFaceType& newFace)
{
  alugrid_assert ( ghost_ );
  alugrid_assert ( innerLevel_ == ghost_->level() );
  connector_.updateFaceInfo(newFace,innerLevel_, ghost_->twist(0) );
  geoProvider_.resetFaceGeom();
}

template <class GridImp>
alu_inline int
ALU3dGridIntersectionIterator<GridImp>::
level() const {
  alugrid_assert ( item_ && (innerLevel_ == item_->level()) );
  return innerLevel_;
}

/************************************************************************************
  ###
   #     #    #   #####  ######  #####    ####   ######   ####      #     #####
   #     ##   #     #    #       #    #  #       #       #    #     #       #
   #     # #  #     #    #####   #    #   ####   #####   #          #       #
   #     #  # #     #    #       #####        #  #       #          #       #
   #     #   ##     #    #       #   #   #    #  #       #    #     #       #
  ###    #    #     #    ######  #    #   ####   ######   ####      #       #
************************************************************************************/

// --IntersectionIterator
template<class GridImp>
alu_inline ALU3dGridLevelIntersectionIterator<GridImp> ::
ALU3dGridLevelIntersectionIterator()
  : ALU3dGridIntersectionIterator<GridImp>( true )
  , levelNeighbor_(false)
  , isLeafItem_(false)
{
}

template<class GridImp>
alu_inline void ALU3dGridLevelIntersectionIterator<GridImp> ::
first (const EntityImp& en, int wLevel, const GridImp& grid )
{
  // store grid point for boundarySegmentIndex
  grid_ = &grid;

  // adjust connector flags
  connector_.setFlags( grid.conformingRefinement(), grid.ghostCellsEnabled() );

  // if given Entity is not leaf, we create an end iterator
  index_  = 0;
  isLeafItem_   = en.isLeaf();

  if( en.isGhost() )
  {
    setInteriorItem(en.getItem(), en.getGhost(), wLevel);
  }
  else
  {
    // for the 2d version numFaces is smaller then the actual
    // stored nFaces of the element
    alugrid_assert ( dim == 3 ?
                    (numFaces == en.getItem().nFaces()) :
                    (numFaces  < en.getItem().nFaces())  );
    setFirstItem(en.getItem(), wLevel);
  }
}

template<class GridImp>
alu_inline void ALU3dGridLevelIntersectionIterator<GridImp> ::
setFirstItem (const HElementType & elem, int wLevel)
{
  ghost_       = 0;
  item_        = static_cast<const IMPLElementType *> (&elem);
  this->innerLevel_  = wLevel;
  // Get first face
  const GEOFaceType* firstFace = getFace(*item_, index_);
  // Store the face in the connector
  setNewFace(*firstFace);
}

template<class GridImp>
alu_inline void ALU3dGridLevelIntersectionIterator<GridImp> ::
setInteriorItem (const HElementType & elem, const BNDFaceType& ghost, int wLevel)
{
  // store ghost for method inside
  ghost_   = &ghost;
  item_   = static_cast<const IMPLElementType *> (&elem);
  // get correct face number
  index_ = ElementTopo::alu2duneFace( ghost.getGhost().second );

  innerLevel_  = wLevel;

  // Get first face
  const GEOFaceType* firstFace = getFace( ghost, index_ );

  // Store the face in the connector
  setNewFace(*firstFace);
}

// copy constructor
template<class GridImp>
alu_inline ALU3dGridLevelIntersectionIterator<GridImp> ::
ALU3dGridLevelIntersectionIterator(const ThisType & org)
  : ALU3dGridIntersectionIterator<GridImp>(org)
  , levelNeighbor_(org.levelNeighbor_)
  , isLeafItem_(org.isLeafItem_)
{
}

// copy constructor
template<class GridImp>
alu_inline void
ALU3dGridLevelIntersectionIterator<GridImp> ::
assign(const ALU3dGridLevelIntersectionIterator<GridImp> & org)
{
  ALU3dGridIntersectionIterator<GridImp>::assign(org);
  levelNeighbor_ = org.levelNeighbor_;
  isLeafItem_    = org.isLeafItem_;
}

template<class GridImp>
alu_inline void ALU3dGridLevelIntersectionIterator<GridImp> :: increment ()
{
  // level increment
  alugrid_assert ( item_ );

  // Next face number of starting element
  ++index_;

  // When the face number is larger than the number of faces an element
  // can have, we've reached the end...
  if ( index_ >= numFaces || ImplTraits::isGhost( ghost_ ) )
  {
    done();
    return;
  }

  // ... else we can take the next face
  const GEOFaceType * nextFace = getFace(connector_.innerEntity(), index_);
  alugrid_assert (nextFace);

  setNewFace(*nextFace);
  return;
}

template<class GridImp>
alu_inline bool ALU3dGridLevelIntersectionIterator<GridImp>::neighbor () const
{
  return levelNeighbor_ && (BaseType :: neighbor());
}

template <class GridImp>
alu_inline void ALU3dGridLevelIntersectionIterator<GridImp>::
setNewFace(const GEOFaceType& newFace)
{
  alugrid_assert ( item_->level() == innerLevel_ );
  levelNeighbor_ = (newFace.level() == innerLevel_);
  connector_.updateFaceInfo(newFace, innerLevel_,
              ( ImplTraits::isGhost( ghost_ ) ) ?
                 ghost_->twist(0) :
                 item_->twist(ElementTopo::dune2aluFace( index_ ))
              );
  geoProvider_.resetFaceGeom();

  // check again level neighbor because outer element might be coarser then
  // this element
  if( isLeafItem_ )
  {
    if( connector_.ghostBoundary() )
    {
      const BNDFaceType & ghost = connector_.boundaryFace();
      // if nonconformity occurs then no level neighbor
      levelNeighbor_ = (innerLevel_ == ghost.ghostLevel() );
    }
    else if ( ! connector_.outerBoundary() )
    {
      levelNeighbor_ = (connector_.outerEntity().level() == innerLevel_);
    }
  }
}

} // end namespace Dune

#endif // DUNE_ALUGRID_ITERATOR_IMP_CC
