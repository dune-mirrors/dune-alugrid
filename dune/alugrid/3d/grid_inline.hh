// Dune includes
#include <dune/common/stdstreams.hh>

// Local includes
#include "alu3dinclude.hh"
#include "entity.hh"
#include "iterator.hh"
#include "datahandle.hh"
#include "grid.hh"

namespace Dune
{

  // Implementation of ALU3dGrid
  // ---------------------------

  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline ALU3dGrid< actualDim, actualDimw, elType, Comm >
    ::ALU3dGrid ( const std::string &macroTriangFilename,
                  const MPICommunicatorType mpiComm,
                  const DuneBoundaryProjectionType *bndPrj,
                  const DuneBoundaryProjectionVector *bndVec,
                  const ALUGridRefinementType refinementType ) 
    : mygrid_( 0 )
    , maxlevel_( 0 )
    , coarsenMarked_( 0 )
    , refineMarked_( 0 )
    , geomTypes_() //dim+1, std::vector<GeometryType>(1) )
    , hIndexSet_ (*this)
    , globalIdSet_( 0 )
    , localIdSet_( *this )
    , levelIndexVec_(MAXL,0) , leafIndexSet_(0)
    , referenceElement_( elType == tetra
        ? ReferenceElements< alu3d_ctype, dimension > :: simplex()
        : ReferenceElements< alu3d_ctype, dimension > :: cube() )
    , sizeCache_ ( 0 )
    , factory_( *this )
    , lockPostAdapt_( false )
    , bndPrj_ ( bndPrj )
    , bndVec_ ( (bndVec) ? (new DuneBoundaryProjectionVector( *bndVec )) : 0 )
    , vertexProjection_( (bndPrj || bndVec) ? new ALUGridBoundaryProjectionType( *this ) : 0 )
    , communications_( new Communications( mpiComm ) )
    , refinementType_( refinementType )
    //, nonConformingGeoInFatherStorage_( makeGeometries() )
  {
    // check macro grid file for keyword 
    checkMacroGridFile( macroTriangFilename );
   
    mygrid_ = createALUGrid( macroTriangFilename );
    alugrid_assert ( mygrid_ );

    dverb << "************************************************" << std::endl;
    dverb << "Created grid on p=" << comm().rank() << std::endl;
    dverb << "************************************************" << std::endl;
    checkMacroGrid ();
  
    clearIsNewMarkers();
    calcExtras();
  } // end constructor


  /*
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  const typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::GeometryInFatherStorage& 
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::makeGeometries()
  {
    alugrid_assert ( elType == tetra || elType == hexa );

    geomTypes_.clear();
    geomTypes_.resize( dimension+1 );
    GeometryType tmpType;
    for( int codim = 0; codim <= dimension; ++codim ) 
    {
      if (elType == tetra)
        tmpType.makeSimplex( dimension - codim );
      else
        tmpType.makeCube( dimension - codim );
      
      geomTypes_[ codim ].push_back( tmpType );
    }

    // initialize static storage variables 
    ALU3dGridGeometry< 0, 3, const ThisType> :: geoProvider();
    ALU3dGridGeometry< 1, 3, const ThisType> :: geoProvider();
    ALU3dGridGeometry< 2, 3, const ThisType> :: geoProvider();
    ALU3dGridGeometry< 3, 3, const ThisType> :: geoProvider();

    // return non-conforming geometryInFather storage
    // true == non-conforming 
    return GeometryInFatherStorage :: storage( geomTypes_[ 0 ][ 0 ], true );
  }
  */


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< actualDim, actualDimw, elType, Comm >::global_size ( int codim ) const 
  {
    // return actual size of hierarchical index set 
    // this is always up to date 
    // maxIndex is the largest index used + 1 
    return myGrid().indexManager(codim).getMaxIndex();
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< actualDim, actualDimw, elType, Comm >::hierSetSize ( int codim ) const
  {
    // return actual size of hierarchical index set 
    return myGrid().indexManager(codim).getMaxIndex();
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< actualDim, actualDimw, elType, Comm >::maxLevel () const 
  { 
    return maxlevel_;
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::GitterImplType &
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::myGrid () const
  {
    alugrid_assert ( mygrid_ );
    return *mygrid_;
  }


  // lbegin methods 
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LevelIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::lbegin ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    // if we dont have this level return empty iterator 
    if( level > maxlevel_ )
      return this->template lend<cd,pitype> (level);

    return ALU3dGridLevelIterator< cd, pitype, const ThisType >( factory(), level, true );
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LevelIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::lend ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    return ALU3dGridLevelIterator< cd, pitype, const ThisType >( factory(), level );
  }


  // lbegin methods 
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< All_Partition >::LevelIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::lbegin ( int level ) const
  {
    return this->template lbegin<cd,All_Partition>( level );
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< All_Partition >::LevelIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::lend ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    return this->template lend<cd,All_Partition>( level );
  }


  //***********************************************************
  //
  // leaf methods , first all begin methods 
  //
  //***********************************************************
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::createLeafIteratorBegin ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    return ALU3dGridLeafIterator< cd, pitype, const ThisType >( factory(), level, true );
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafbegin ( int level ) const
  {
    return createLeafIteratorBegin<cd, pitype> (level) ;
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafbegin ( int level ) const
  {
    return createLeafIteratorBegin<cd, All_Partition> (level) ;
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafbegin () const
  {
    return createLeafIteratorBegin< cd, pitype > (maxlevel_) ;
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafbegin () const
  {
    return createLeafIteratorBegin< cd, All_Partition> (maxlevel_) ;
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::LeafIteratorType
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafbegin ( int level ) const
  {
    return createLeafIteratorBegin<0, All_Partition> (level) ;
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::LeafIteratorType
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafbegin () const
  {
    return createLeafIteratorBegin<0, All_Partition> (maxlevel_) ;
  }


  //****************************************************************
  //
  // all leaf end methods 
  //
  //****************************************************************
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::createLeafIteratorEnd ( int level ) const
  {
    alugrid_assert ( level >= 0 );
    return ALU3dGridLeafIterator<cd, pitype, const MyType> ( factory() , level);
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafend ( int level ) const
  {
    return createLeafIteratorEnd < cd, pitype> (level);
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafend ( int level ) const
  {
    return createLeafIteratorEnd < cd, All_Partition> (level);
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd, PartitionIteratorType pitype >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::template Partition< pitype >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafend () const
  {
    return createLeafIteratorEnd < cd, pitype> (maxlevel_);
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< int cd >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::Traits::template Codim< cd >::LeafIterator
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafend () const
  {
    return createLeafIteratorEnd < cd, All_Partition> (maxlevel_);
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::LeafIteratorType
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafend ( int level ) const
  {
    return createLeafIteratorEnd <0, All_Partition> (level);
  }


  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline typename ALU3dGrid< actualDim, actualDimw, elType, Comm >::LeafIteratorType
  ALU3dGrid< actualDim, actualDimw, elType, Comm >::leafend () const
  {
    return createLeafIteratorEnd <0,All_Partition> (maxlevel_);
  }


  //*****************************************************************

  // mark given entity  
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline bool ALU3dGrid< actualDim, actualDimw, elType, Comm >
    ::mark ( int ref, const typename Traits::template Codim< 0 >::Entity &entity )
  {
    bool marked = (this->getRealImplementation( entity )).mark(ref);
    if(marked) 
      {
        if(ref > 0) ++refineMarked_;
        if(ref < 0) ++coarsenMarked_;
      }
    return marked;
  }


  // get Mark of given entity  
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline int ALU3dGrid< actualDim, actualDimw, elType, Comm >
    ::getMark ( const typename Traits::template Codim< 0 >::Entity &entity ) const
  {
    return this->getRealImplementation( entity ).getMark();
  }


  // global refine 
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< class GridImp, class DataHandle >
  inline 
  void ALU3dGrid< actualDim, actualDimw, elType, Comm >
    ::globalRefine ( int refCount, AdaptDataHandleInterface< GridImp, DataHandle > &handle )
  {
    alugrid_assert ( (refCount + maxLevel()) < MAXL ); 
    
    for( int count = refCount; count > 0; --count )
    {
      const LeafIteratorType end = leafend();
      for( LeafIteratorType it = leafbegin(); it != end; ++it )
        mark( 1 , *it );
      adapt( handle );
    }
  }


  // adapt grid  
  // --adapt
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  template< class GridImp, class DataHandle >
  inline  
  bool ALU3dGrid< actualDim, actualDimw, elType, Comm >
    ::adapt ( AdaptDataHandleInterface< GridImp, DataHandle > &handle )
  {
    typedef AdaptDataHandleInterface< GridImp, DataHandle > AdaptDataHandle;

    typedef typename EntityObject::ImplementationType EntityImp;
    EntityObject father( EntityImp( factory(), this->maxLevel() ) );
    EntityObject son   ( EntityImp( factory(), this->maxLevel() ) );

    // true if at least one element was marked for coarsening
    bool mightCoarse = preAdapt();

    bool refined = false ; 
    if(globalIdSet_)
    {
      // if global id set exists then include into 
      // prolongation process 
      ALU3DSPACE AdaptRestrictProlongGlSet< MyType, AdaptDataHandle, GlobalIdSetImp >
      rp(*this,
         father,this->getRealImplementation(father), 
         son,   this->getRealImplementation(son),
         handle,
         *globalIdSet_);

      refined = myGrid().duneAdapt(rp); // adapt grid 
    }
    else 
    {
       ALU3DSPACE AdaptRestrictProlongImpl< MyType, AdaptDataHandle >
       rp(*this,
          father,this->getRealImplementation(father), 
          son,   this->getRealImplementation(son),
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


  
  // load balance grid ( lbData might be a pointer to NULL ) 
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline bool ALU3dGrid< actualDim, actualDimw, elType, Comm >::loadBalance( GatherScatterType* lbData )
  {
    if( comm().size() <= 1 )
        return false;

    // call load Balance 
    const bool changed = myGrid().loadBalance( lbData );

    if( changed )
    {
      // reset size and things  
      // maxLevel does not need to be recalculated 
      calcExtras();

      // build new Id Set. Only do that after calcExtras, because here
      // the item lists are needed 
      if( globalIdSet_ )
        globalIdSet_->updateIdSet();
      
      // compress data if lbData is valid and has user data
      if( lbData && lbData->hasUserData() ) 
        lbData->compress() ;
      else // this only needs to be done if no user is present
        clearIsNewMarkers(); 
    }
    return changed;
  }

  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline void ALU3dGrid< actualDim, actualDimw, elType, Comm >::finalizeGridCreation()
  {
    // distribute the grid 
    loadBalance();

    // free memory by reinitializing the grid 
    mygrid_ = GitterImplType :: compress( mygrid_ );

    // update all internal structures 
    updateStatus();

    // call post adapt
    clearIsNewMarkers();
  }


  // return Grid name 
  template< int actualDim, int actualDimw, ALU3dGridElementType elType, class Comm >
  inline std::string ALU3dGrid< actualDim, actualDimw, elType, Comm >::name ()
  {
    if( elType == hexa )
      return "ALUCubeGrid";
    else
      return "ALUSimplexGrid";
  }

} // end namespace Dune
