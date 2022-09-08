#ifndef DUNE_ALUGRID_GRID_IMP_CC
#define DUNE_ALUGRID_GRID_IMP_CC

// config.h is included via cmd line argument

#include <dune/common/stdstreams.hh>

#include "grid.hh"

#include "alu3diterators_imp.cc"
#include "aluinline.hh"

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

    for(unsigned int i=0; i<vxsize; ++i)
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
    [[maybe_unused]] int count = 0;
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

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void
  ALU3dGrid< dim, dimworld, elType, Comm >::makeGeometries()
  {
    // instantiate the static memory pool by creating an object
    ALU3dGridGeometry< 0,   dimworld, const ThisType >();
    ALU3dGridGeometry< 1,   dimworld, const ThisType >();
    ALU3dGridGeometry< 2,   dimworld, const ThisType >();
    ALU3dGridGeometry< dim, dimworld, const ThisType >();

    alugrid_assert ( elType == tetra || elType == hexa );

    geomTypes_.clear();
    geomTypes_.resize( dimension+1 );

    geomTypes_[ 0 ].push_back( ALU3dGridGeometry< dim,   dimworld, const ThisType >().type() );
    geomTypes_[ 1 ].push_back( ALU3dGridGeometry< dim-1, dimworld, const ThisType >().type() );
    geomTypes_[ 2 ].push_back( ALU3dGridGeometry< dim-2, dimworld, const ThisType >().type() );

    if( dimension == 3 )
    {
      geomTypes_[ 3 ].push_back( ALU3dGridGeometry< 0, dimworld, const ThisType >().type() );
    }
  }


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
  void ALU3dGrid< dim, dimworld, elType, Comm >::calcMaxLevel ()
  {
    // old fashioned way
    int testMaxLevel = 0;
    typedef ALU3DSPACE ALU3dGridLeafIteratorWrapper< 0, All_Partition, Comm > IteratorType;
    IteratorType w (*this, maxLevel(), nlinks() );

    typedef typename IteratorType :: val_t val_t ;
    typedef typename ALU3dImplTraits< elType, Comm >::HElementType HElementType;

    for (w.first () ; ! w.done () ; w.next ())
    {
      val_t & item = w.item();

      HElementType * elem = 0;
      if( item.first )
        elem = item.first;
      else if( item.second )
        elem = item.second->getGhost().first;

      alugrid_assert ( elem );

      // get maximum of local element levels
      testMaxLevel = std::max( testMaxLevel, int(elem->level()) );
    }
    maxlevel_ = comm().max( testMaxLevel );
    alugrid_assert ( maxlevel_ == comm().max( maxlevel_ ));
  }


  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline bool ALU3dGrid< dim, dimworld, elType, Comm >
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
    // create new grid from stream
    mygrid_.reset( createALUGrid( stream ) );

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
      std::cerr << "\nKeyword '" << oldAluId << "' is deprecated! Change it to '" << aluid << "' in file '" << filename<<"'! \n";
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
        std::cerr << "\nERROR: " << elType2Name(elType) << " Grid tries to read a ";
        std::cerr << elType2Name(type) << " macro grid file! \n\n";
        alugrid_assert (type == elType);
        DUNE_THROW(GridError,"\nERROR: " << elType2Name(elType) << " Grid tries to read a " << elType2Name(type) << " macro grid file! ");
      }
    }
  }


  alu_inline
  const char* elType2Name( ALU3dGridElementType elType )
  {
    switch( elType )
    {
      case tetra  : return "Tetrahedra";
      case hexa   : return "Hexahedra";
      case mixed  : return "Mixed";
      default     : return "Error";
    }
  }

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::finalizeGridCreation()
  {
    // distribute the grid
    loadBalance();

    // free memory by reinitializing the grid
    mygrid_.reset( GitterImplType :: compress( mygrid_.release() ) );

    // update all internal structures
    updateStatus();

    // call post adapt
    clearIsNewMarkers();
  }

  // load balance grid ( lbData might be a pointer to NULL )
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline bool ALU3dGrid< dim, dimworld, elType, Comm >::loadBalance( GatherScatterType* lbData )
  {
    if( comm().size() <= 1 )
        return false;

    // call load Balance
    const bool changed = myGrid().loadBalance( lbData );

    if( changed )
    {
      // reset boundary segment index
      macroBoundarySegmentIndexSet_.invalidate();

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

  // post process grid
  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  alu_inline
  void ALU3dGrid< dim, dimworld, elType, Comm >::clearIsNewMarkers ()
  {
    // old fashioned way
    typedef ALU3DSPACE ALU3dGridLeafIteratorWrapper< 0, All_Partition, Comm > IteratorType;
    IteratorType w (*this, maxLevel(), nlinks() );

    typedef typename IteratorType::val_t val_t;
    typedef typename ALU3dImplTraits< elType, Comm >::HElementType HElementType;

    for (w.first () ; ! w.done () ; w.next ())
    {
      val_t & item = w.item();

      alugrid_assert ( item.first || item.second );
      HElementType * elem = 0;
      if( item.first )
      {
        elem = item.first;
      }
      else if( item.second )
      {
        elem = item.second->getGhost().first;
      }
      alugrid_assert ( elem );

      if (elem->hasBeenRefined())
      {
        elem->resetRefinedTag();
        // on bisected grids its possible that not only leaf elements where added so
        // we have to move up the hierarchy to make sure that the refined tag on parents are also removed
        while ((elem = elem->up()))
        {
          elem->resetRefinedTag();
        }
      }
    }
  }


#if ! COMPILE_ALUGRID_INLINE
  // Instantiation
  template class ALU3dGrid< 2, 2, hexa, ALUGridNoComm >;
  template class ALU3dGrid< 2, 2, tetra, ALUGridNoComm >;

  template class ALU3dGrid< 2, 2, hexa, ALUGridMPIComm >;
  template class ALU3dGrid< 2, 2, tetra, ALUGridMPIComm >;

#if 0
  // codim 0
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 0 >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 0 > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 0 >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 0 > (int level) const;

  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 0 >::Partition< All_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 0, All_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 0 >::Partition< All_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 0, All_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< Interior_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 0, Interior_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< Interior_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 0, Interior_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< InteriorBorder_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 0, InteriorBorder_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< InteriorBorder_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 0, InteriorBorder_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< Overlap_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 0, Overlap_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< Overlap_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 0, Overlap_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< OverlapFront_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 0, OverlapFront_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits:: Codim< 0 >::template Partition< OverlapFront_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 0, OverlapFront_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits:: Codim< 0 >::template Partition< Ghost_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 0, Ghost_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 0 >::template Partition< Ghost_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 0, Ghost_Partition > (int level) const;

  // codim 1
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 1 >::Partition< All_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 1, All_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 1 >::Partition< All_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 1, All_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< Interior_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 1, Interior_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< Interior_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 1, Interior_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< InteriorBorder_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 1, InteriorBorder_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< InteriorBorder_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 1, InteriorBorder_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< Overlap_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 1, Overlap_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< Overlap_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 1, Overlap_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< OverlapFront_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 1, OverlapFront_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits:: Codim< 1 >::template Partition< OverlapFront_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 1, OverlapFront_Partition > (int level) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits:: Codim< 1 >::template Partition< Ghost_Partition >::LevelIterator
    ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: lbegin< 1, Ghost_Partition > (int level) const;
  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 1 >::template Partition< Ghost_Partition >::LevelIterator
    ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: lbegin< 1, Ghost_Partition > (int level) const;

  //template ALU3dGrid< 2, 2, hexa, ALUGridNoComm >::Traits::Codim< 0 >::LeafIterator
  //  ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: leafbegin< 0 > () const;
  //template ALU3dGrid< 2, 2, tetra, ALUGridNoComm >::Traits::Codim< 0 >::LeafIterator
  //  ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: leafbegin< 0 > () const;
#endif

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

  /*
  // Instantiation
  template class ALUGrid< 2, 2, simplex, conforming, ALUGridNoComm >;
  template class ALUGrid< 2, 2, simplex, nonconforming, ALUGridNoComm >;
  template class ALUGrid< 2, 3, simplex, conforming, ALUGridNoComm >;
  template class ALUGrid< 2, 3, simplex, nonconforming, ALUGridNoComm >;
  template class ALUGrid< 3, 3, simplex, conforming, ALUGridNoComm >;
  template class ALUGrid< 3, 3, simplex, nonconforming, ALUGridNoComm >;
  template class ALUGrid< 2, 2, cube, nonconforming, ALUGridNoComm >;
  template class ALUGrid< 2, 3, cube, nonconforming, ALUGridNoComm >;
  template class ALUGrid< 3, 3, cube, nonconforming, ALUGridNoComm >;

  template class ALUGrid< 2, 2, simplex, conforming, ALUGridMPIComm >;
  template class ALUGrid< 2, 2, simplex, nonconforming, ALUGridMPIComm >;
  template class ALUGrid< 2, 3, simplex, conforming, ALUGridMPIComm >;
  template class ALUGrid< 2, 3, simplex, nonconforming, ALUGridMPIComm >;
  template class ALUGrid< 3, 3, simplex, conforming, ALUGridMPIComm >;
  template class ALUGrid< 3, 3, simplex, nonconforming, ALUGridMPIComm >;
  template class ALUGrid< 2, 2, cube, nonconforming, ALUGridMPIComm >;
  template class ALUGrid< 2, 3, cube, nonconforming, ALUGridMPIComm >;
  template class ALUGrid< 3, 3, cube, nonconforming, ALUGridMPIComm >;
  */

#endif // #if COMPILE_ALUGRID_LIB

} // end namespace Dune

#endif // end DUNE_ALUGRID_GRID_IMP_CC
