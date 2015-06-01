#ifndef DUNE_ALUGRID_GRID_IMP_CC
#define DUNE_ALUGRID_GRID_IMP_CC

#if COMPILE_ALUGRID_INLINE == 0
#include <config.h>
#endif

// Dune includes
#include <dune/common/stdstreams.hh>

// Local includes
#include "entity.hh"
#include "iterator.hh"
#include "datahandle.hh"

#include "grid.hh"

#if COMPILE_ALUGRID_INLINE
#define alu_inline inline
#else
#define alu_inline
#endif

namespace Dune
{

  template< class Comm >
  template< class GridType >
  alu_inline
  void ALU3dGridVertexList< Comm >::
  setupVxList(const GridType & grid, int level)
  {
    // iterates over grid elements of given level and adds all vertices to
    // given list

    enum { codim = 3 };

    VertexListType & vxList = vertexList_;

    //we need Codim 3 instead of Codim dim because the ALUGrid IndexManager is called
    unsigned int vxsize = grid.hierarchicIndexSet().size(codim);
    if( vxList.size() < vxsize ) vxList.reserve(vxsize);
    std::vector<int> visited_(vxsize);

    for(unsigned int i=0; i<vxsize; i++)
    {
      visited_[i] = 0;
    }

    vxList.resize(0);

    const ALU3dGridElementType elType = GridType:: elementType;

    typedef ALU3DSPACE ALU3dGridLevelIteratorWrapper< 0, Dune::All_Partition, Comm > ElementLevelIteratorType;
    typedef typename ElementLevelIteratorType :: val_t val_t;

    typedef ALU3dImplTraits< elType, Comm > ImplTraits;
    typedef typename ImplTraits::IMPLElementType IMPLElementType;
    typedef typename ImplTraits::VertexType VertexType;

    enum { nVx = ElementTopologyMapping < elType > :: numVertices };

    ElementLevelIteratorType it ( grid, level, grid.nlinks() );

    int count = 0;
    for( it.first(); !it.done() ; it.next())
    {
      val_t & item = it.item();

      IMPLElementType * elem = 0;
      if( item.first )
        elem = static_cast<IMPLElementType *> (item.first);
      else if( item.second )
        elem = static_cast<IMPLElementType *> (item.second->getGhost().first);

      alugrid_assert ( elem );

      for(int i=0; i<nVx; ++i)
      {
        VertexType * vx = elem->myvertex(i);
        alugrid_assert ( vx );

        // insert only interior and border vertices
        if( vx->isGhost() ) continue;

        const int idx = vx->getIndex();
        if(visited_[idx] == 0)
        {
          vxList.push_back(vx);
          ++count;
        }
        visited_[idx] = 1;
      }
    }
    alugrid_assert ( count == (int) vxList.size());;
    up2Date_ = true;
  }


  template< class Comm >
  template< class GridType >
  alu_inline
  void ALU3dGridLeafVertexList< Comm >::
  setupVxList(const GridType & grid)
  {
    // iterates over grid elements of given level and adds all vertices to
    // given list
    enum { codim = 3 };

    VertexListType & vxList = vertexList_;

    //we need Codim 3 instead of Codim dim because the ALUGrid IndexManager is called
    size_t vxsize = grid.hierarchicIndexSet().size(codim);
    if( vxList.capacity() < vxsize) vxList.reserve(vxsize);
    vxList.resize(vxsize);

    for(size_t i=0; i<vxsize; ++i)
    {
      ItemType & vx = vxList[i];
      vx.first  = 0;
      vx.second = -1;
    }

    const ALU3dGridElementType elType = GridType:: elementType;

    typedef ALU3DSPACE ALU3dGridLeafIteratorWrapper< 0, Dune::All_Partition, Comm > ElementIteratorType;
    typedef typename ElementIteratorType :: val_t val_t;

    typedef ALU3dImplTraits< elType, Comm > ImplTraits;
    typedef typename ImplTraits::IMPLElementType IMPLElementType;
    typedef typename ImplTraits::VertexType VertexType;

    enum { nVx = ElementTopologyMapping < elType > :: numVertices };

    ElementIteratorType it ( grid, grid.maxLevel() , grid.nlinks() );

#ifdef ALUGRIDDEBUG
    int count = 0;
#endif
    for( it.first(); !it.done() ; it.next())
    {
      val_t & item = it.item();

      IMPLElementType * elem = 0;
      if( item.first )
        elem = static_cast<IMPLElementType *> (item.first);
      else if( item.second )
        elem = static_cast<IMPLElementType *> (item.second->getGhost().first);

      alugrid_assert ( elem );
      int level = elem->level();

      for(int i=0; i<nVx; ++i)
      {
        VertexType * vx = elem->myvertex(i);
        alugrid_assert ( vx );

        // insert only interior and border vertices
        if( vx->isGhost() ) continue;

        const int idx = vx->getIndex();
        ItemType & vxpair = vxList[idx];
        if( vxpair.first == 0 )
        {
          vxpair.first  = vx;
          vxpair.second = level;
#ifdef ALUGRIDDEBUG
          ++ count;
#endif
        }
        // always store max level of vertex as grdi definition says
        else
        {
          // set the max level for each vertex, see Grid definition
          if (vxpair.second < level) vxpair.second = level;
        }
      }
    }

    //std::cout << count << "c | s " << vxList.size() << "\n";
    // make sure that the found number of vertices equals to stored ones
    //alugrid_assert ( count == (int)vxList.size() );
    up2Date_ = true;
  }



  // ALU3dGrid
  // ---------

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  const ALU3dGrid< dim, dimworld, elType, Comm > &
  ALU3dGrid< dim, dimworld, elType, Comm >::operator= ( const ALU3dGrid< dim, dimworld, elType, Comm > &other )
  {
    DUNE_THROW(GridError,"Do not use assignment operator of ALU3dGrid! \n");
    return (*this);
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  ALU3dGrid< dim, dimworld, elType, Comm >::~ALU3dGrid ()
  {
    delete communications_;
    delete vertexProjection_;
    //delete bndPrj_;
    if( bndVec_ )
    {
      const size_t bndSize = bndVec_->size();
      for(size_t i=0; i<bndSize; ++i)
      {
        delete (*bndVec_)[i];
      }
      delete bndVec_; bndVec_ = 0;
    }

    for(unsigned int i=0; i<levelIndexVec_.size(); i++) delete levelIndexVec_[i];
    delete globalIdSet_; globalIdSet_ = 0;
    delete leafIndexSet_; leafIndexSet_ = 0;
    delete sizeCache_; sizeCache_ = 0;

    /*
    if(myGrid().container().iterators_attached())
    {
      dwarn << "WRANING: There still exists instances of iterators giving access to this grid which is about to be removed! in: " << __FILE__ << " line: " << __LINE__ << std::endl;
    }
    */
    delete mygrid_; mygrid_ = 0;
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
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
    if (dim == 3)
      return myGrid().numMacroBndSegments();
    else if(elType == tetra)
      return myGrid().numMacroBndSegments() - size(0);
    else if (elType == hexa)
      return myGrid().numMacroBndSegments() - 2*size(0);
  }


  // --size
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  int ALU3dGrid< dim, dimworld, elType, Comm >::size ( int level, GeometryType type ) const
  {
    if(elType == tetra && !type.isSimplex()) return 0;
    if(elType == hexa  && !type.isCube   ()) return 0;
    return size( level, dimension - type.dim() );
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  int ALU3dGrid< dim, dimworld, elType, Comm >::size ( int codim ) const
  {
    alugrid_assert ( codim >= 0 );
    alugrid_assert ( codim <= dimension );

    alugrid_assert ( sizeCache_ );
    return sizeCache_->size(codim);
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  int ALU3dGrid< dim, dimworld, elType, Comm >::size ( GeometryType type ) const
  {
    if(elType == tetra && !type.isSimplex()) return 0;
    if(elType == hexa  && !type.isCube   ()) return 0;
    return size( dimension - type.dim() );
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  int ALU3dGrid< dim, dimworld, elType, Comm >::ghostSize ( int codim ) const
  {
    return ( ghostCellsEnabled() && codim == 0 ) ? 1 : 0 ;
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  int ALU3dGrid< dim, dimworld, elType, Comm >::ghostSize ( int level, int codim ) const
  {
    return ghostSize( codim );
  }

  // calc all necessary things that might have changed
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::updateStatus()
  {
    calcMaxLevel();
    calcExtras();
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::calcMaxLevel ()
  {
    // old fashioned way
    int testMaxLevel = 0;
    typedef ALU3DSPACE ALU3dGridLeafIteratorWrapper< 0, All_Partition, Comm > IteratorType;
    IteratorType w (*this, maxLevel(), nlinks() );

    typedef typename IteratorType :: val_t val_t ;
    typedef typename ALU3dImplTraits< elType, Comm >::IMPLElementType IMPLElementType;

    for (w.first () ; ! w.done () ; w.next ())
    {
      val_t & item = w.item();

      IMPLElementType * elem = 0;
      if( item.first )
        elem = static_cast<IMPLElementType *> (item.first);
      else if( item.second )
        elem = static_cast<IMPLElementType *> (item.second->getGhost().first);

      alugrid_assert ( elem );

      int level = elem->level();
      if(level > testMaxLevel) testMaxLevel = level;
    }
    maxlevel_ = comm().max( testMaxLevel );
    alugrid_assert ( maxlevel_ == comm().max( maxlevel_ ));
  }


  // --calcExtras
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::calcExtras ( const bool recomputeMacroIndexSet )
  {
    // make sure maxLevel is the same on all processes ????
    //alugrid_assert ( maxlevel_ == comm().max( maxlevel_ ));

    if(sizeCache_) delete sizeCache_;
    sizeCache_ = new SizeCacheType (*this);

    // unset up2date before recalculating the index sets,
    // becasue they will use this feature
    leafVertexList_.unsetUp2Date();
    for(size_t i=0; i<MAXL; ++i)
    {
      vertexList_[i].unsetUp2Date();
      levelEdgeList_[i].unsetUp2Date();
    }

    if( comm().size() > 1 )
    {
      //here dimension has to be 3, as this is used ALU internally
      //  was    for( int i = 0; i < dimension; ++i )
      for( int i = 0; i < 3; ++i )
      {
        ghostLeafList_[i].unsetUp2Date();
        for(size_t l=0; l<MAXL; ++l) ghostLevelList_[i][l].unsetUp2Date();
      }
    }

    if( recomputeMacroIndexSet && levelIndexVec_[0])
    {
      (*(levelIndexVec_[0])).calcNewIndex( this->template lbegin<0>( 0 ),
                                           this->template lend<0>( 0 ) );
    }

    // update all index set that are already in use
    for(size_t i=1; i<levelIndexVec_.size(); ++i)
    {
      if(levelIndexVec_[i])
      {
        (*(levelIndexVec_[i])).calcNewIndex( this->template lbegin<0>( i ),
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
  alu_inline
  const typename ALU3dGrid< dim, dimworld, elType, Comm >::Traits::LeafIndexSet &
  ALU3dGrid< dim, dimworld, elType, Comm >::leafIndexSet () const
  {
    if(!leafIndexSet_) leafIndexSet_ = new LeafIndexSetImp ( *this,
                                                             this->template leafbegin<0>(),
                                                             this->template leafend<0>() );
    return *leafIndexSet_;
  }


  // global refine
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::globalRefine ( int refCount )
  {
    alugrid_assert ( (refCount + maxLevel()) < MAXL );

    for( int count = refCount; count > 0; --count )
    {
      const LeafIteratorType end = leafend();
      for( LeafIteratorType it = leafbegin(); it != end; ++it )
        mark( 1, *it );
      const bool refined = adapt();
      if( refined )
        postAdapt();
    }
  }

  // preprocess grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  bool ALU3dGrid< dim, dimworld, elType, Comm >::preAdapt()
  {
    return (coarsenMarked_ > 0);
  }


  // adapt grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
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

  // post process grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::clearIsNewMarkers ()
  {
    // old fashioned way
    typedef ALU3DSPACE ALU3dGridLeafIteratorWrapper< 0, All_Partition, Comm > IteratorType;
    IteratorType w (*this, maxLevel(), nlinks() );

    typedef typename IteratorType::val_t val_t;
    typedef typename ALU3dImplTraits< elType, Comm >::IMPLElementType IMPLElementType;

    for (w.first () ; ! w.done () ; w.next ())
    {
      val_t & item = w.item();

      alugrid_assert ( item.first || item.second );
      IMPLElementType * elem = 0;
      if( item.first )
        elem = static_cast<IMPLElementType *> (item.first);
      else if( item.second )
      {
        elem = static_cast<IMPLElementType *>( item.second->getGhost().first );
        alugrid_assert ( elem );
      }
      if (elem->hasBeenRefined())
      {
        elem->resetRefinedTag();
        // on bisected grids its possible that not only leaf elements where added so
        // we have to move up the hierarchy to make sure that the refined tag on parents are also removed
        while (elem->up())
        {
          elem = static_cast<IMPLElementType *>(elem->up());
          elem->resetRefinedTag();
        }
      }
    }
  }


  // post process grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::postAdapt ()
  {
    if( lockPostAdapt_ )
    {
      // clear all isNew markers on entities
      clearIsNewMarkers();

      // make that postAdapt has been called
      lockPostAdapt_ = false;
    }
  }

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  inline bool ALU3dGrid< dim, dimworld, elType, Comm >
    ::writeMacroGrid ( const std::string path, const std::string name,
                       const ALU3DSPACE MacroFileHeader::Format format ) const
  {
    std::stringstream filename;
    filename << path << "/" << name << "." << comm().rank();

    std::ofstream macro( filename.str().c_str() );

    if( macro )
    {
      // dump distributed macro grid as ascii files
      myGrid().container().dumpMacroGrid( macro, format );
    }
    else
      std::cerr << "WARNING: couldn't open file `" <<  filename.str() << "' for writing!" << std::endl;

    return true;
  }

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::
  backup( std::ostream& stream, const ALU3DSPACE MacroFileHeader::Format format  ) const
  {
    // backup grid to given stream
    myGrid().backup( stream, format );
  }

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::restore( std::istream& stream )
  {
    // if grid exists delete first
    if( mygrid_ ) delete mygrid_;
    // create new grid from stream
    mygrid_ = createALUGrid( stream );

    // check for grid
    if( ! mygrid_ )
    {
      DUNE_THROW(InvalidStateException,"ALUGrid::restore failed");
    }

    // check for element type
    this->checkMacroGrid ();

    // restore hierarchy from given stream
    myGrid().restore( stream );

    // calculate new maxlevel
    // calculate indices
    updateStatus();

    // reset refinement markers
    clearIsNewMarkers();
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::checkMacroGridFile ( const std::string filename )
  {
    if(filename == "") return;

    std::ifstream file(filename.c_str());
    if(!file)
    {
      std::cerr << "Couldn't open file '" << filename <<"' !" << std::endl;
      DUNE_THROW(IOError,"Couldn't open file '" << filename <<"' !");
    }

    const std::string aluid((elType == tetra) ? "!Tetrahedra" : "!Hexahedra");
    const std::string oldAluId((elType == tetra) ? "!Tetraeder" : "!Hexaeder");
    std::string idline;
    std::getline(file,idline);
    std::stringstream idstream(idline);
    std::string id;
    idstream >> id;

    if(id == aluid )
    {
      return;
    }
    else if ( id == oldAluId )
    {
      derr << "\nKeyword '" << oldAluId << "' is deprecated! Change it to '" << aluid << "' in file '" << filename<<"'! \n";
      return ;
    }
    else
    {
      std::cerr << "Delivered file '"<<filename<<"' does not contain keyword '"
        << aluid << "'. Found id '" <<id<< "'. Check the macro grid file! Bye." << std::endl;
      DUNE_THROW(IOError,"Wrong file format! ");
    }
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::checkMacroGrid ()
  {
    typedef typename ALU3dImplTraits< elType, Comm >::HElementType HElementType;
    typedef ALU3DSPACE PureElementLeafIterator< HElementType > IteratorType;
    IteratorType w( this->myGrid()  );
    for (w->first () ; ! w->done () ; w->next ())
    {
      ALU3dGridElementType type = (ALU3dGridElementType) w->item().type();
      if( type != elType )
      {
        derr << "\nERROR: " << elType2Name(elType) << " Grid tries to read a ";
        derr << elType2Name(type) << " macro grid file! \n\n";
        alugrid_assert (type == elType);
        DUNE_THROW(GridError,"\nERROR: " << elType2Name(elType) << " Grid tries to read a " << elType2Name(type) << " macro grid file! ");
      }
    }
  }


  alu_inline
  const char * elType2Name( ALU3dGridElementType elType )
  {
    switch( elType )
    {
      case tetra  : return "Tetrahedra";
      case hexa   : return "Hexahedra";
      case mixed  : return "Mixed";
      default     : return "Error";
    }
  }


#if COMPILE_ALUGRID_LIB
  // Instantiation
  template class ALU3dGrid< 2, 2, hexa, ALUGridNoComm >;
  template class ALU3dGrid< 2, 2, tetra, ALUGridNoComm >;

  template class ALU3dGrid< 2, 2, hexa, ALUGridMPIComm >;
  template class ALU3dGrid< 2, 2, tetra, ALUGridMPIComm >;

  //2-3
  template class ALU3dGrid< 2, 3, hexa, ALUGridNoComm >;
  template class ALU3dGrid< 2, 3, tetra, ALUGridNoComm >;

  template class ALU3dGrid< 2, 3, hexa, ALUGridMPIComm >;
  template class ALU3dGrid< 2, 3, tetra, ALUGridMPIComm >;

  //3-3
  template class ALU3dGrid< 3, 3, hexa, ALUGridNoComm >;
  template class ALU3dGrid< 3, 3, tetra, ALUGridNoComm >;

  template class ALU3dGrid< 3, 3, hexa, ALUGridMPIComm >;
  template class ALU3dGrid< 3, 3, tetra, ALUGridMPIComm >;

#endif // #if COMPILE_ALUGRID_LIB

} // end namespace Dune

#endif // end DUNE_ALUGRID_GRID_IMP_CC
