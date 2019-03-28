#ifndef ALUGRID_ENTITY_INLINE_HH
#define ALUGRID_ENTITY_INLINE_HH
#include <dune/common/exceptions.hh>

#include "geometry.hh"
#include "grid.hh"


namespace Dune {

  /////////////////////////////////////////////////////////////////
  //
  //  --Entity0
  //  --Codim0Entity
  //
  ////////////////////////////////////////////////////////////////
  /*
  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  removeElement ()
  {
    item_  = 0;
    ghost_ = 0;
    geo_.invalidate();
  }

  template<int dim, class GridImp>
  inline void ALU3dGridEntity<0,dim,GridImp> ::
  reset (int walkLevel )
  {
    item_       = 0;
    ghost_      = 0;

    // reset geometry information
    geo_.invalidate();
  }

  // works like assignment
  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setEntity(const ALU3dGridEntity<0,dim,GridImp> & org)
  {
    item_          = org.item_;
    ghost_         = org.ghost_;

    // reset geometry information
    geo_.invalidate();
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp>::
  setElement(const EntitySeed& key )
  {
    if( ! key.isGhost() )
      setElement( *key.interior() );
    else
      setGhost( *key.ghost() );
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp>::
  setElement(HElementType & element)
  {
    item_ = static_cast<IMPLElementType *> (&element);
    alugrid_assert ( item_ );
    // make sure this method is not called for ghosts
    alugrid_assert ( ! item_->isGhost() );
    ghost_   = 0;

    // reset geometry information
    geo_.invalidate();
  }

  template<int dim, class GridImp>
  inline void
  ALU3dGridEntity<0,dim,GridImp> :: setGhost(HBndSegType & ghost)
  {
    // use element as ghost
    item_  = static_cast<IMPLElementType *> ( ghost.getGhost().first );

    // method getGhost can return 0, but then is something wrong
    alugrid_assert (item_);
    alugrid_assert (item_->isGhost());

    // remember pointer to ghost face
    ghost_ = static_cast<BNDFaceType *> (&ghost);
    alugrid_assert ( ghost_ );

    // check wether ghost is leaf or not, ghost leaf means
    // that this is the ghost that we want in the leaf iterator
    // not necessarily is real leaf element
    // see Intersection Iterator, same story

    // reset geometry information
    geo_.invalidate();
  }

  template<int dim, class GridImp>
  inline int
  ALU3dGridEntity<0,dim,GridImp> :: level() const
  {
    alugrid_assert( item_ );
    return item_->level();
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> ::
  equals (const ALU3dGridEntity<0,dim,GridImp> &org ) const
  {
    return (item_ == org.item_);
  }

  template<int dim, class GridImp>
  inline GeometryType
  ALU3dGridEntity<0,dim,GridImp> :: type () const
  {
    return geo_.type();
  }

  template<int dim, class GridImp>
  inline int ALU3dGridEntity<0,dim,GridImp> :: getIndex() const
  {
    alugrid_assert ( item_ );
    return (*item_).getIndex();
  }

  template<int dim, class GridImp>
  template<int cc>
  inline int ALU3dGridEntity<0,dim,GridImp> :: count () const
  {
    return subEntities( cc );
  }

  template<int dim, class GridImp>
  inline unsigned int ALU3dGridEntity<0,dim,GridImp> :: subEntities (unsigned int codim) const
  {
    return GridImp::referenceElement().size( codim );
  }

  template<int dim, class GridImp>
  inline PartitionType ALU3dGridEntity<0,dim,GridImp> ::
  partitionType () const
  {
    alugrid_assert ( item_ );
    // make sure we really got a ghost
    alugrid_assert ( (isGhost()) ? item_->isGhost() : true );
    return (isGhost() ?  GhostEntity : InteriorEntity);
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: isLeaf() const
  {
    alugrid_assert( item_ );
    if( isGhost() )
    {
      alugrid_assert( ghost_ );
      // for ghost elements the situation is more complicated
      // we have to compare the ghost level with our current level
      BNDFaceType * dwn = static_cast<BNDFaceType *> (ghost_->down());
      return ( dwn ) ? (dwn->ghostLevel() == level()) : true;
    }
    else
    {
      // no children means leaf entity
      return ! item_->down();
    }
  }

  template<int dim, class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp>
  ALU3dGridEntity<0,dim,GridImp> :: hbegin (int maxlevel) const
  {
    alugrid_assert (item_ != 0);
    // if isGhost is true the end iterator will be returned
    if( isGhost() )
    {
      return ALU3dGridHierarchicIterator<GridImp>( *ghost_, maxlevel, isLeaf() );
    }
    else
      return ALU3dGridHierarchicIterator<GridImp>( *item_,  maxlevel, isLeaf() );
  }

  template<int dim, class GridImp>
  inline ALU3dGridHierarchicIterator<GridImp> ALU3dGridEntity<0,dim,GridImp> :: hend (int maxlevel) const
  {
    alugrid_assert (item_ != 0);
    return ALU3dGridHierarchicIterator<GridImp> ( *item_, maxlevel, true);
  }

  // Adaptation methods
  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: isNew () const
  {
    alugrid_assert ( item_ );
    return item_->hasBeenRefined();
  }

  template<int dim, class GridImp>
  inline bool ALU3dGridEntity<0,dim,GridImp> :: mightVanish () const
  {
    alugrid_assert ( item_ );
    return ((*item_).requestrule() == coarse_element_t);
  }
  */

  //*******************************************************************
  //
  //  --EntityPointer
  //  --EnPointer
  //
  //*******************************************************************
  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase( const HElementType &item )
    : seed_( item )
    , entity_( EntityImp( item ) )
  {
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase( const HBndSegType & ghostFace )
    : seed_( ghostFace )
    , entity_ ( EntityImp( ghostFace ) )
  {
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase( const ALU3dGridEntitySeedType& key )
    : seed_( key )
    , entity_( EntityImp( seed_ ) )
  {
  }

  // constructor Level,Leaf and HierarchicIterator
  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase()
    : seed_()
    , entity_ ( EntityImp() )
  {
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> ::
  ALU3dGridEntityPointerBase(const ALU3dGridEntityPointerType & org)
    : seed_( org.seed_ )
    , entity_( org.entity_.impl() )
  {
    alugrid_assert( seed_   == org.seed_ );
    alugrid_assert( entity_ == org.entity_ );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityPointerBase<codim,GridImp> &
  ALU3dGridEntityPointerBase<codim,GridImp> ::
  operator = (const ALU3dGridEntityPointerType & org)
  {
    clone( org );
    return *this;
  }

  template<int codim, class GridImp >
  inline void
  ALU3dGridEntityPointerBase<codim,GridImp> ::
  clone (const ALU3dGridEntityPointerType & org)
  {
    // copy seed
    seed_ = org.seed_;

    if( seed_.isValid() )
    {
      // update entity if seed is valid
      entityImp().setEntity( org.entityImp() );
    }
    else // otherwise mark as finished (iterators)
    {
      this->done();
    }
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointerBase<codim,GridImp>::done ()
  {
    seed_.clear();
  }

  template<int codim, class GridImp >
  inline bool ALU3dGridEntityPointerBase<codim,GridImp>::
  equals (const ALU3dGridEntityPointerBase<codim,GridImp>& i) const
  {
    // check equality of underlying items
    return (seed_.equals( i.seed_ ));
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointerBase<codim,GridImp>::
  updateGhostPointer( HBndSegType & ghostFace )
  {
    seed_.set( ghostFace );
    if( seed_.isValid() )
    {
      entityImp().setGhost( ghostFace );
    }
  }

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointerBase<codim,GridImp>::
  updateEntityPointer( HElementType * item , int )
  {
    seed_.set( *item );
    if( seed_.isValid() )
    {
      entityImp().setElement( *item );
    }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  specialisation for higher codims
  //
  ///////////////////////////////////////////////////////////////////

  template<int codim, class GridImp >
  inline void ALU3dGridEntityPointer<codim,GridImp>::
  updateEntityPointer( HElementType * item, int level)
  {
    seed_.set( *item, level );
    if( seed_.isValid() )
    {
      entityImp().setElement( seed_ );
    }
  }

} // end namespace Dune
#endif
