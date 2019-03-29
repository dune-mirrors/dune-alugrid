#ifndef ALUGRID_GRID_INLINE_HH
#define ALUGRID_GRID_INLINE_HH

// Dune includes
#include <dune/common/stdstreams.hh>

// Local includes
#include "alu3dinclude.hh"
#include "entity.hh"
#include "iterator.hh"
#include "datahandle.hh"
#include "grid.hh"

#define alu_inline_tmp inline

namespace Dune
{

  // Implementation of ALU3dGrid
  // ---------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline ALU3dGrid< dim, dimworld, elType, Comm >
    ::ALU3dGrid ( const std::string &macroTriangFilename,
                  const MPICommunicatorType mpiComm,
                  const ALUGridVertexProjectionPairType& bndPrj,
                  const ALUGridRefinementType refinementType )
    : mygrid_()
    , maxlevel_( 0 )
    , coarsenMarked_( 0 )
    , refineMarked_( 0 )
    , geomTypes_() //dim+1, std::vector<GeometryType>(1) )
    , hIndexSet_ (*this)
    , globalIdSet_()
    , localIdSet_( *this )
    , levelIndexVec_(1) , leafIndexSet_()
    , sizeCache_ ()
    , lockPostAdapt_( false )
    , vertexProjections_( bndPrj )
    , communications_( new Communications( mpiComm ) )
    , refinementType_( refinementType )
  {
    // generate geometry storage and geometry type vector
    makeGeometries();

    // check macro grid file for keyword
    checkMacroGridFile( macroTriangFilename );

    mygrid_.reset( createALUGrid( macroTriangFilename ) );
    alugrid_assert ( mygrid_ );

    dverb << "************************************************" << std::endl;
    dverb << "Created grid on p=" << comm().rank() << std::endl;
    dverb << "************************************************" << std::endl;
    checkMacroGrid ();

    clearIsNewMarkers();
    calcExtras();
  } // end constructor


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< dim, dimworld, elType, Comm >::global_size ( int codim ) const
  {
    // return actual size of hierarchical index set
    // this is always up to date
    // maxIndex is the largest index used + 1
    return myGrid().indexManager(codim).getMaxIndex();
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< dim, dimworld, elType, Comm >::hierSetSize ( int codim ) const
  {
    // return actual size of hierarchical index set
    return myGrid().indexManager(codim).getMaxIndex();
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< dim, dimworld, elType, Comm >::maxLevel () const
  {
    return maxlevel_;
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::GitterImplType &
  ALU3dGrid< dim, dimworld, elType, Comm >::myGrid () const
  {
    alugrid_assert ( mygrid_ );
    return *mygrid_;
  }

  // lbegin methods
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LevelIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::lbegin ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    // if we dont have this level return empty iterator
    if( level > maxlevel_ )
      return this->template lend<cd,pitype> (level);

    return ALU3dGridLevelIterator< cd, pitype, const ThisType >( *this, level, true );
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LevelIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::lend ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    return ALU3dGridLevelIterator< cd, pitype, const ThisType >( *this, level );
  }


  // lbegin methods
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::template Partition< All_Partition >::LevelIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::lbegin ( int level ) const
  {
    return this->template lbegin<cd,All_Partition>( level );
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::template Partition< All_Partition >::LevelIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::lend ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    return this->template lend<cd,All_Partition>( level );
  }


  //***********************************************************
  //
  // leaf methods , first all begin methods
  //
  //***********************************************************
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::leafbegin () const
  {
    return ALU3dGridLeafIterator< cd, pitype, const ThisType >( *this, maxlevel_, true );
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::LeafIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::leafbegin () const
  {
    return leafbegin< cd, All_Partition> () ;
  }


  //****************************************************************
  //
  // all leaf end methods
  //
  //****************************************************************
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::leafend () const
  {
    return ALU3dGridLeafIterator<cd, pitype, const MyType> ( *this, maxlevel_);
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::template Codim< cd >::LeafIterator
  ALU3dGrid< dim, dimworld, elType, Comm >::leafend () const
  {
    return leafend< cd, All_Partition> ();
  }

  //*****************************************************************

  // mark given entity
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline bool ALU3dGrid< dim, dimworld, elType, Comm >
    ::mark ( int ref, const typename Traits::template Codim< 0 >::Entity &entity )
  {
    bool marked = entity.impl().mark( ref, conformingRefinement() );
    if(marked)
      {
        if(ref > 0) ++refineMarked_;
        if(ref < 0) ++coarsenMarked_;
      }
    return marked;
  }


  // get Mark of given entity
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< dim, dimworld, elType, Comm >
    ::getMark ( const typename Traits::template Codim< 0 >::Entity &entity ) const
  {
    return entity.impl().getMark();
  }


  // global refine
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< class GridImp, class DataHandle >
  inline
  void ALU3dGrid< dim, dimworld, elType, Comm >
    ::globalRefine ( int refCount, AdaptDataHandleInterface< GridImp, DataHandle > &handle )
  {
    for( int count = std::abs(refCount); count > 0; --count )
    {
      const LeafIteratorType end = leafend();
      for( LeafIteratorType it = leafbegin(); it != end; ++it )
        mark( refCount>0?1:-1 , *it );
      adapt( handle );
    }
  }


  // adapt grid
  // --adapt
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  template< class GridImp, class DataHandle >
  inline
  bool ALU3dGrid< dim, dimworld, elType, Comm >
    ::adapt ( AdaptDataHandleInterface< GridImp, DataHandle > &handle )
  {
    typedef AdaptDataHandleInterface< GridImp, DataHandle > AdaptDataHandle;

    // true if at least one element was marked for coarsening
    bool mightCoarse = preAdapt();

    bool refined = false ;
    if(globalIdSet_)
    {
      // if global id set exists then include into
      // prolongation process
      ALU3DSPACE AdaptRestrictProlongGlSet< MyType, AdaptDataHandle, GlobalIdSetImp >
      rp(*this,
         handle,
         *globalIdSet_);

      refined = myGrid().duneAdapt(rp); // adapt grid
    }
    else
    {
       ALU3DSPACE AdaptRestrictProlongImpl< MyType, AdaptDataHandle >
       rp(*this,
          handle);

      refined = myGrid().duneAdapt(rp); // adapt grid
    }

    if(refined || mightCoarse)
    {
      // only calc extras and skip maxLevel calculation, because of
      // refinement maxLevel was calculated already
      updateStatus();

      // no need to call postAdapt here, because markers
      // are cleand during refinement callback
    }

    return refined;
  }


  // return Grid name
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline std::string ALU3dGrid< dim, dimworld, elType, Comm >::name ()
  {
    if( elType == hexa )
      return "ALUCubeGrid";
    else
      return "ALUSimplexGrid";
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  int ALU3dGrid< dim, dimworld, elType, Comm >::size ( int level, int codim ) const
  {
    // if we dont have this level return 0
    if( (level > maxlevel_) || (level < 0) ) return 0;

    alugrid_assert ( codim >= 0);
    alugrid_assert ( codim < dimension+1 );

    alugrid_assert ( sizeCache_ );
    return sizeCache_->size(level,codim);
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  size_t ALU3dGrid< dim, dimworld, elType, Comm >::numBoundarySegments () const
  {
    return macroBoundarySegmentIndexSet().size();
  }


  // --size
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  int ALU3dGrid< dim, dimworld, elType, Comm >::size ( int level, GeometryType type ) const
  {
    if(elType == tetra && !type.isSimplex()) return 0;
    if(elType == hexa  && !type.isCube   ()) return 0;
    return size( level, dimension - type.dim() );
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  int ALU3dGrid< dim, dimworld, elType, Comm >::size ( int codim ) const
  {
    alugrid_assert ( codim >= 0 );
    alugrid_assert ( codim <= dimension );

    alugrid_assert ( sizeCache_ );
    return sizeCache_->size(codim);
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  int ALU3dGrid< dim, dimworld, elType, Comm >::size ( GeometryType type ) const
  {
    if(elType == tetra && !type.isSimplex()) return 0;
    if(elType == hexa  && !type.isCube   ()) return 0;
    return size( dimension - type.dim() );
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  int ALU3dGrid< dim, dimworld, elType, Comm >::ghostSize ( int codim ) const
  {
    return ( ghostCellsEnabled() && codim == 0 ) ? 1 : 0 ;
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  int ALU3dGrid< dim, dimworld, elType, Comm >::ghostSize ( int level, int codim ) const
  {
    return ghostSize( codim );
  }


  // calc all necessary things that might have changed
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  void ALU3dGrid< dim, dimworld, elType, Comm >::updateStatus()
  {
    calcMaxLevel();
    calcExtras();
  }


  // --calcExtras
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  void ALU3dGrid< dim, dimworld, elType, Comm >::calcExtras ()
  {
    // make sure maxLevel is the same on all processes ????
    //alugrid_assert ( maxlevel_ == comm().max( maxlevel_ ));

    sizeCache_.reset( new SizeCacheType (*this) );

    // unset up2date before recalculating the index sets,
    // because they will use this feature
    leafVertexList_.unsetUp2Date();

    vertexList_.resize( maxlevel_+1 );
    levelEdgeList_.resize( maxlevel_+1 );

    for(int i=0; i<=maxlevel_; ++i)
    {
      vertexList_[i].unsetUp2Date();
      levelEdgeList_[i].unsetUp2Date();
    }

    {
      //here dimension has to be 3, as this is used ALU internally
      //  was for( int i = 0; i < dimension; ++i )
      for( int i = 0; i < 3; ++i )
      {
        ghostLeafList_[i].unsetUp2Date();
        ghostLevelList_[i].resize( maxlevel_+1 );
        for(int l=0; l<=maxlevel_; ++l)
          ghostLevelList_[i][l].unsetUp2Date();
      }
    }

    levelIndexVec_.resize( maxlevel_ + 1 );

    // update all index set that are already in use
    for(size_t i=0; i<levelIndexVec_.size(); ++i)
    {
      if(levelIndexVec_[i])
      {
        levelIndexVec_[i]->calcNewIndex( this->template lbegin<0>( i ),
                                         this->template lend<0>( i ) );
      }
    }

    if(leafIndexSet_)
    {
      leafIndexSet_->calcNewIndex( this->template leafbegin<0>(), this->template leafend<0>() );
    }

    // build global ID set new (to be revised)
    if( globalIdSet_ ) globalIdSet_->updateIdSet();

    coarsenMarked_ = 0;
    refineMarked_  = 0;
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  const typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::LeafIndexSet &
  ALU3dGrid< dim, dimworld, elType, Comm >::leafIndexSet () const
  {
    if(!leafIndexSet_)
    {
      leafIndexSet_.reset( new LeafIndexSetImp ( *this,
                                                 this->template leafbegin<0>(),
                                                 this->template leafend<0>() ) );
    }
    return *leafIndexSet_;
  }

  //! get level index set of the grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  const typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::LevelIndexSet &
  ALU3dGrid< dim, dimworld, elType, Comm >::levelIndexSet (int level) const
  {
    assert( (level >= 0) && (level < int( levelIndexVec_.size() )) );
    if( ! levelIndexVec_[ level ] )
    {
      levelIndexVec_[ level ] = createLevelIndexSet( level );
    }
    return (*levelIndexVec_[ level ]);
  }

  /** \brief return instance of level index set
      \note if index set for this level has not been created then this
      instance will be deleted once the shared_ptr goes out of scope.
  */
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  std::shared_ptr< typename ALU3dGrid< dim, dimworld, elType, Comm >::LevelIndexSetImp >
  ALU3dGrid< dim, dimworld, elType, Comm >::accessLevelIndexSet ( int level ) const
  {
    assert( (level >= 0) && (level < int( levelIndexVec_.size() )) );
    if( levelIndexVec_[ level ] )
    {
      return levelIndexVec_[ level ];
    }
    else
    {
      return createLevelIndexSet( level );
    }
  }

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  std::shared_ptr< typename ALU3dGrid< dim, dimworld, elType, Comm >::LevelIndexSetImp >
  ALU3dGrid< dim, dimworld, elType, Comm >::createLevelIndexSet ( int level ) const
  {
    return std::make_shared< LevelIndexSetImp > ( *this, lbegin< 0 >( level ), lend< 0 >( level ), level );
  }


  // global refine
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  void ALU3dGrid< dim, dimworld, elType, Comm >::globalRefine ( int refCount )
  {
    int marker = refCount > 0 ? 1: -1 ;
    for( int count = std::abs(refCount); count > 0; --count )
    {
      const auto  end = leafend< 0, Interior_Partition >();
      for( auto it = leafbegin< 0, Interior_Partition >(); it != end; ++it )
      {
        mark( marker, *it );
      }
      preAdapt();
      adapt();
      postAdapt();
    }
  }

  // preprocess grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  bool ALU3dGrid< dim, dimworld, elType, Comm >::preAdapt()
  {
    return (coarsenMarked_ > 0);
  }


  // adapt grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline_tmp
  bool ALU3dGrid< dim, dimworld, elType, Comm >::adapt ()
  {
    bool ref = false;

    if( lockPostAdapt_ == true )
    {
      DUNE_THROW(InvalidStateException,"Make sure that postAdapt is called after adapt was called and returned true!");
    }

    bool mightCoarse = preAdapt();
    // if prallel run, then adapt also global id set
    if(globalIdSet_)
    {
      //std::cout << "Start adapt with globalIdSet prolong \n";
      int defaultChunk = newElementsChunk_;
      int actChunk     = refineEstimate_ * refineMarked_;

      // guess how many new elements we get
      int newElements = std::max( actChunk , defaultChunk );

      globalIdSet_->setChunkSize( newElements );
      ref = myGrid().duneAdapt(*globalIdSet_); // adapt grid
    }
    else
    {
      ref = myGrid().adaptWithoutLoadBalancing();
    }

    // in parallel this is different
    if( this->comm().size() == 1 )
    {
      ref = ref && refineMarked_ > 0;
    }

    if(ref || mightCoarse)
    {
      // calcs maxlevel and other extras
      updateStatus();

      // notify that postAdapt must be called
      lockPostAdapt_ = true;
    }
    return ref;
  }

#undef alu_inline_tmp
} // end namespace Dune
#endif
