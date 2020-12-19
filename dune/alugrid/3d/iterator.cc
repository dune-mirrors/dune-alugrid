#ifndef DUNE_ALUGRID_ITERATOR_CC
#define DUNE_ALUGRID_ITERATOR_CC

// config.h is included via cmd line argument

#include "aluinline.hh"
#include "alu3dinclude.hh"

#include "geometry.hh"
#include "entity.hh"
#include "grid.hh"
#include "faceutility.hh"
#include "iterator.hh"

#include "alu3diterators_imp.cc"
#include "iterator_imp.cc"

namespace Dune {

/*************************************************************************
 #       ######  #    #  ######  #          #     #####  ######  #####
 #       #       #    #  #       #          #       #    #       #    #
 #       #####   #    #  #####   #          #       #    #####   #    #
 #       #       #    #  #       #          #       #    #       #####
 #       #        #  #   #       #          #       #    #       #   #
 ######  ######    ##    ######  ######     #       #    ######  #    #
*************************************************************************/
//--LevelIterator
// Constructor for begin iterator
template<int codim, PartitionIteratorType pitype, class GridImp >
alu_inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const GridImp& grd, int level, bool )
  : ALU3dGridEntityPointer<codim,GridImp> ()
  , grid_( &grd )
  , iter_()
  , level_(level)
{
  // the wrapper for the original iterator of the ALU3dGrid
  typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper< (GridImp::dimension == 2 && codim == 2) ? 3 : codim, pitype, Comm > IteratorImplType;
  iter_.reset(new IteratorImplType( grid(), level_, grid().nlinks() ));
  alugrid_assert( iter_ );
  this->firstItem( grid(), *this, level_);
}

// Constructor for end iterator
template<int codim, PartitionIteratorType pitype, class GridImp >
alu_inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const GridImp& grid, int level)
  : ALU3dGridEntityPointer<codim,GridImp> ()
  , grid_( &grid )
  , iter_()
  , level_(level)
{
  this->done();
}

template<int codim, PartitionIteratorType pitype, class GridImp >
alu_inline ALU3dGridLevelIterator<codim,pitype,GridImp> ::
  ALU3dGridLevelIterator(const ThisType & org )
  : ALU3dGridEntityPointer<codim,GridImp> ( org )
  , grid_( org.grid_ )
  , iter_()
  , level_( org.level_ )
{
  assign(org);
}

template<int codim, PartitionIteratorType pitype, class GridImp>
alu_inline ALU3dGridLevelIterator<codim, pitype, GridImp> ::
~ALU3dGridLevelIterator ()
{
  removeIter();
}

template<int codim, PartitionIteratorType pitype, class GridImp>
alu_inline void ALU3dGridLevelIterator<codim, pitype, GridImp> ::
removeIter ()
{
  this->done();
  iter_.reset();
}

template<int codim, PartitionIteratorType pitype, class GridImp>
alu_inline void ALU3dGridLevelIterator<codim, pitype, GridImp> ::
assign(const ThisType & org)
{
  alugrid_assert ( !iter_ );
  grid_ = org.grid_;
  ALU3dGridEntityPointer <codim,GridImp> :: clone (org);
  level_ = org.level_;
  if( org.iter_ )
  {
    iter_.reset( org.iter_->clone() );
    alugrid_assert ( iter_ );
    if(!(iter_->done()))
    {
      this->setItem( grid(), *this, *iter_, level_ );
      alugrid_assert ( this->equals(org) );
    }
  }
  else
  {
    this->done();
  }
}
template<int codim, PartitionIteratorType pitype, class GridImp>
alu_inline ALU3dGridLevelIterator<codim, pitype, GridImp> &
ALU3dGridLevelIterator<codim, pitype, GridImp> ::
operator = (const ThisType & org)
{
  removeIter();
  assign(org);
  return *this;
}

template<int codim, PartitionIteratorType pitype, class GridImp >
alu_inline void ALU3dGridLevelIterator<codim,pitype,GridImp> :: increment ()
{
  this->incrementIterator( grid(), *this, level_ );
}


//*******************************************************************
//
//  LEAFITERATOR
//
//--LeafIterator
//*******************************************************************
// constructor for end iterators
template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
ALU3dGridLeafIterator( const GridImp& grid, int level )
  : ALU3dGridEntityPointer <cdim,GridImp> ()
  , grid_( &grid )
  , iter_()
{
  this->done();
}

template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
ALU3dGridLeafIterator(const GridImp& grd, int level ,
                      bool isBegin)
  : ALU3dGridEntityPointer <cdim,GridImp> ()
  , grid_( &grd )
  , iter_()
{
  typedef typename ALU3DSPACE ALU3dGridLeafIteratorWrapper< (GridImp::dimension == 2 && cdim == 2) ? 3 : cdim, pitype, Comm > IteratorImplType ;
  // create interior iterator
  iter_.reset( new IteratorImplType ( grid(), level , grid().nlinks() ) );
  alugrid_assert ( iter_ );
  // -1 to identify as leaf iterator
  this->firstItem( grid(), *this, -1 );
}

template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
ALU3dGridLeafIterator(const ThisType & org)
 : ALU3dGridEntityPointer <cdim,GridImp> ()
 , grid_( org.grid_ )
 , iter_()
{
  // assign iterator without cloning entity pointer again
  assign(org);
}

template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
~ALU3dGridLeafIterator()
{
  removeIter();
}

template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline void ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
removeIter ()
{
  this->done();
  iter_.reset();
}

template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline ALU3dGridLeafIterator<cdim, pitype, GridImp> &
ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
operator = (const ThisType & org)
{
  removeIter();
  assign(org);
  return *this;
}

template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline void ALU3dGridLeafIterator<cdim, pitype, GridImp> ::
assign (const ThisType & org)
{
  alugrid_assert( !iter_ );
  grid_ = org.grid_;
  ALU3dGridEntityPointer <cdim,GridImp> :: clone (org);

  if( org.iter_ )
  {
    alugrid_assert ( !org.iter_->done() );
    iter_.reset( org.iter_->clone() );
    alugrid_assert ( iter_ );

    if( !(iter_->done() ))
    {
      alugrid_assert ( !iter_->done());
      alugrid_assert ( !org.iter_->done() );
      // -1 to identify leaf iterator
      this->setItem( grid(), *this, *iter_, -1 );
      alugrid_assert ( this->equals(org) );
    }
  }
  else
  {
    this->done();
  }
}

template<int cdim, PartitionIteratorType pitype, class GridImp>
alu_inline void ALU3dGridLeafIterator<cdim, pitype, GridImp> :: increment ()
{
  // -1 to identify leaf iterator
  this->incrementIterator( grid(), *this,-1 );
}


/************************************************************************************
#     #
#     #     #    ######  #####      #     #####  ######  #####
#     #     #    #       #    #     #       #    #       #    #
#######     #    #####   #    #     #       #    #####   #    #
#     #     #    #       #####      #       #    #       #####
#     #     #    #       #   #      #       #    #       #   #
#     #     #    ######  #    #     #       #    ######  #    #
************************************************************************************/
// --HierarchicIterator
template <class GridImp>
alu_inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator()
  : ALU3dGridEntityPointer<0,GridImp> ()
  , elem_( nullptr )
  , ghostElem_( )
  , maxlevel_( -1 )
{
}

// --HierarchicIterator
template <class GridImp>
alu_inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const HElementType & elem, int maxlevel ,bool end)
  : ALU3dGridEntityPointer<0,GridImp> ()
  , elem_(&elem)
  , ghostElem_( )
  , maxlevel_(maxlevel)
{
  if (!end)
  {
    HElementType * item = const_cast<HElementType *> (elem.down());
    if(item && item->level() <= maxlevel_)
    {
      // we have children and they lie in the disired level range
      this->updateEntityPointer( item );
      return ;
    }
  }

  // otherwise
  this->done();
}

template <class GridImp>
alu_inline ALU3dGridHierarchicIterator<GridImp> ::
  ALU3dGridHierarchicIterator(const HBndSegType& ghost,
                              int maxlevel,
                              bool end)
  : ALU3dGridEntityPointer<0,GridImp> ()
  , elem_( 0 )
  , ghostElem_( ghost )
  , maxlevel_(maxlevel)
{
  if( ! end )
  {
    ghostElem_ = const_cast<HBndSegType *> (ghost.down());

    // we have children and they lie in the disired level range
    if( ghostElem_ != 0 && ghostElem_->ghostLevel() <= maxlevel_)
    {
      this->updateGhostPointer( *ghostElem_ );
      return ;
    }
    else
      ghostElem_ = 0;
  }

  // otherwise do nothing
  this->done();
}

template <class GridImp>
alu_inline ALU3dGridHierarchicIterator<GridImp> ::
ALU3dGridHierarchicIterator(const ThisType& org)
  : ALU3dGridEntityPointer<0,GridImp> ( org )
{
  assign( org );
}

template <class GridImp>
alu_inline ALU3dGridHierarchicIterator<GridImp> &
ALU3dGridHierarchicIterator<GridImp> ::
operator = (const ThisType& org)
{
  assign( org );
  return *this;
}

template <class GridImp>
alu_inline void
ALU3dGridHierarchicIterator<GridImp> ::
assign(const ThisType& org)
{
  // copy my data
  elem_      = org.elem_;
  ghostElem_ = org.ghostElem_;
  maxlevel_  = org.maxlevel_;

  // copy entity pointer
  // this method will probably free entity
  ALU3dGridEntityPointer<0,GridImp> :: clone(org);
}

template <class GridImp>
alu_inline int
ALU3dGridHierarchicIterator<GridImp>::
getLevel(const HBndSegType* face) const
{
  // return ghost level
  alugrid_assert ( face );
  return face->ghostLevel();
}

template <class GridImp>
alu_inline int
ALU3dGridHierarchicIterator<GridImp>::
getLevel(const HElementType * item) const
{
  // return normal level
  alugrid_assert ( item );
  return item->level();
}
template <class GridImp>
template <class HItemType>
alu_inline HItemType*
ALU3dGridHierarchicIterator<GridImp>::
goNextElement(const HItemType* startElem, HItemType * oldelem )
{
  // strategy is:
  // - go down as far as possible and then over all children
  // - then go to father and next and down again

  HItemType * nextelem = oldelem->down();
  if(nextelem)
  {
    // use getLevel method
    if( getLevel(nextelem) <= maxlevel_)
      return nextelem;
  }

  nextelem = oldelem->next();
  if(nextelem)
  {
    // use getLevel method
    if( getLevel(nextelem) <= maxlevel_)
      return nextelem;
  }

  nextelem = oldelem->up();
  if(nextelem == startElem) return 0;

  while( !nextelem->next() )
  {
    nextelem = nextelem->up();
    if(nextelem == startElem) return 0;
  }

  if(nextelem) nextelem = nextelem->next();

  return nextelem;
}

template <class GridImp>
alu_inline void ALU3dGridHierarchicIterator<GridImp> :: increment ()
{
  alugrid_assert (this->seed_.item() != 0);

  if( ghostElem_.valid() )
  {
    ghostElem_ = goNextElement( ghostElem_.ghost(), ghostElem_.nextGhost() );
    if( ! ghostElem_ )
    {
      this->done();
      return ;
    }

    this->updateGhostPointer( *ghostElem_ );
  }
  else
  {
    HElementType * nextItem = goNextElement( elem_, this->seed_.item() );
    if( ! nextItem)
    {
      this->done();
      return ;
    }

    this->updateEntityPointer(nextItem);
  }
  return ;
}

#if ! COMPILE_ALUGRID_INLINE
  // Instantiation 3-3

  // Instantiation without MPI
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  // Instantiation

  // Instantiation with MPI
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  // Instantiation 2-3

  // Instantiation without MPI
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;


  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  // Instantiation

  // Instantiation with MPI
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;


  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;


  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  // Instantiation 2-2

  // Instantiation without MPI
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;


  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  // Instantiation

  // Instantiation with MPI
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

#endif // ! COMPILE_ALUGRID_INLINE

} // end namespace Dune
#endif // DUNE_ALUGRID_ITERATOR_IMP_CC
