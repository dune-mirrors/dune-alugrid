#ifndef DUNE_ALU3DGRIDINDEXSETS_CC
#define DUNE_ALU3DGRIDINDEXSETS_CC

// config.h is included via cmd line argument

#include "aluinline.hh"
#include "indexsets.hh"
#include "alu3diterators_imp.cc"

#include <dune/alugrid/3d/grid.hh>


namespace Dune
{

// ALU3dGlobalIdSet
// ----------------

template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
ALU3dGridGlobalIdSet(const GridType & grid)
  : grid_(grid), hset_(grid.hierarchicIndexSet())
{
  if(elType == hexa)
  {
    // see dune/alugrid/impl/serial/gitter_mgb.cc
    // InsertUniqueHexa
    const int vxKey[4] = {0,1,3,4};
    for(int i=0; i<4; i++) vertexKey_[i] = vxKey[i];
  }
  else
  {
    alugrid_assert ( elType == tetra );
    // see dune/alugrid/impl/serial/gitter_mgb.cc
    // InsertUniqueTetra
    const int vxKey[4] = {0,1,2,3};
    for(int i=0; i<4; i++) vertexKey_[i] = vxKey[i];
  }

  // setup the id set
  buildIdSet();
}

// update id set after adaptation
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
updateIdSet()
{
  // to be revised
  buildIdSet();
}

// print all ids
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
print () const
{
  for(int i=0 ;i<numCodim; ++i)
  {
    std::cout << "*****************************************************\n";
    std::cout << "Ids for codim " << i << "\n";
    std::cout << "*****************************************************\n";
    for(unsigned int k=0; k<ids_[i].size(); ++k)
    {
      std::cout << "Item[" << i << "," << k <<"] has id " << ids_[i][k] << "\n";
    }
    std::cout << "\n\n\n";
  }
}

// check id set for uniqueness
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
uniquenessCheck() const
{
  for(int i=0 ;i<numCodim; i++)
  {
    typedef typename std::map<int,IdType>::iterator IteratorType;
    IteratorType end = ids_[i].end();
    for(IteratorType it = ids_[i].begin(); it != end; ++it)
    {
      const IdType & id = (*it).second;
      if( id.isValid() )
        checkId(id,it);
    }
  }
}

// creates the id set
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildIdSet ()
{
  for(int i=0; i<numCodim; ++i)
  {
    ids_[i].clear();
  }

  GitterImplType &gitter = grid_.myGrid();

  // all interior and border vertices
  {
    typename ALU3DSPACE AccessIterator< VertexType >::Handle fw( gitter.container() );
    for( fw.first (); !fw.done(); fw.next() )
    {
      int idx = fw.item().getIndex();
      ids_[3][idx] = buildMacroVertexId( fw.item() );
    }
  }

  // all ghost vertices
  {
    typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper< 3, Ghost_Partition, Comm > IteratorType;
    IteratorType fw (grid_ , 0 , grid_.nlinks() );
    typedef typename IteratorType :: val_t val_t;
    for (fw.first () ; ! fw.done () ; fw.next ())
    {
      val_t & item = fw.item();
      alugrid_assert ( item.first );
      VertexType & vx = * (item.first);
      int idx = vx.getIndex();
      ids_[3][idx] = buildMacroVertexId( vx );
    }
  }

  {
    // create ids for all macro edges
    {
      typename ALU3DSPACE AccessIterator< HEdgeType >::Handle w( gitter.container() );
      for (w.first(); !w.done(); w.next())
      {
        int idx = w.item().getIndex();
        ids_[2][idx] = buildMacroEdgeId( w.item() );
        buildEdgeIds( w.item() , ids_[2][idx] , startOffSet_ );
      }
    }

    // all ghost edges
    {
      typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper< 2, Ghost_Partition, Comm > IteratorType;
      IteratorType fw( grid_, 0, grid_.nlinks() );
      typedef typename IteratorType :: val_t val_t;
      for (fw.first () ; ! fw.done () ; fw.next ())
      {
        val_t & item = fw.item();
        alugrid_assert ( item.first );
        HEdgeType & edge = * (item.first);
        int idx = edge.getIndex();

        ids_[2][idx] = buildMacroEdgeId( edge );
        buildEdgeIds( edge , ids_[2][idx] , startOffSet_ );
      }
    }
  }


  // for all macro faces and all children
  {
    typename ALU3DSPACE AccessIterator< HFaceType >::Handle w( gitter.container() );
    for (w.first () ; ! w.done () ; w.next ())
    {
      int idx = w.item().getIndex();
      ids_[1][idx] = buildMacroFaceId( w.item() );
      buildFaceIds( w.item() , ids_[1][idx] , startOffSet_ );
    }
  }

  // all ghost faces
  if( grid_.comm().size() > 1 )
  {
    typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper< 1, Ghost_Partition, Comm > IteratorType;
    IteratorType fw (grid_ , 0 , grid_.nlinks() );
    typedef typename IteratorType :: val_t val_t;
    for (fw.first () ; ! fw.done () ; fw.next ())
    {
      val_t & item = fw.item();
      alugrid_assert ( item.first );
      HFaceType & face = * (item.first);
      int idx = face.getIndex();
      ids_[1][idx] = buildMacroFaceId( face );
      buildFaceIds( face , ids_[1][idx] , startOffSet_ );
    }
  }

  // for all macro elements and all internal entities
  {
    typename ALU3DSPACE AccessIterator< HElementType >::Handle w( gitter.container() );
    for (w.first () ; ! w.done () ; w.next ())
    {
      int idx = w.item().getIndex();
      ids_[0][idx] = buildMacroElementId( w.item() );
      buildElementIds( w.item() , ids_[0][idx] , startOffSet_ );
    }
  }

  // all ghost elements
  if( grid_.comm().size() > 1 )
  {
    typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper< 0, Ghost_Partition, Comm > IteratorType;
    IteratorType fw (grid_ , 0 , grid_.nlinks() );
    typedef typename IteratorType :: val_t val_t;
    for (fw.first () ; ! fw.done () ; fw.next ())
    {
      val_t & item = fw.item();
      alugrid_assert ( item.second );
      HElementType & elem = * ( item.second->getGhost().first );
      int idx = elem.getIndex();
      ids_[0][idx] = buildMacroElementId( elem );
      buildElementIds( elem , ids_[0][idx] , startOffSet_ );
    }
  }

  // check uniqueness of id only in serial, because
  // in parallel some faces and edges of ghost exists more than once
  // but have the same id, but not the same index, there for the check
  // will fail for ghost elements
  // be carefull with this check, it's complexity is O(N^2)
#ifdef ALUGRID_CHECK_GLOBALIDSET_UNIQUENESS
#warning "GlobalIdSet uniqueness check enabled!"
  uniquenessCheck();
#endif
}

template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline
typename ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > :: IdType
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildMacroVertexId(const VertexType & item )
{
  int vx[4] = { item.ident(), -1, -1, -1};
  enum { codim = 3 };
  MacroKeyType key(vx[0],vx[1],vx[2],vx[3]);
  return MacroIdType(key, 0, codim, startOffSet_ );
}

template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline
typename ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > :: IdType
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildMacroEdgeId(const HEdgeType & item )
{
  const GEOEdgeType & edge = static_cast<const GEOEdgeType &> (item);
  int vx[4] = {-1,-1,-1,-1};
  for(int i=0; i<2; ++i)
  {
    vx[i] = edge.myvertex(i)->ident();
  }

  enum { codim = 2 };
  MacroKeyType key(vx[0],vx[1],vx[2],vx[3]);
  return MacroIdType(key, 0, codim, startOffSet_ );
}

template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline
typename ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > :: IdType
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildMacroFaceId(const HFaceType & item )
{
  const GEOFaceType & face = static_cast<const GEOFaceType &> (item);
  int vx[4] = {-1,-1,-1,-1};
  for(int i=0; i<3; ++i)
  {
    vx[i] = face.myvertex(i)->ident();
  }

  enum { codim = 1 };
  MacroKeyType key(vx[0],vx[1],vx[2],vx[3]);
  return MacroIdType(key,0, codim, startOffSet_ );
}

template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline
typename ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > :: IdType
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildMacroElementId(const HElementType & item )
{
  const GEOElementType & elem = static_cast<const GEOElementType &> (item);
  int vx[4] = {-1,-1,-1,-1};
  for(int i=0; i<4; ++i)
  {
    vx[i] = elem.myvertex(vertexKey_[i])->ident();
  }
  enum { codim = 0 };
  MacroKeyType key(vx[0],vx[1],vx[2],vx[3]);
  return MacroIdType(key,0, codim, startOffSet_ );
}

// build ids for all children of this element
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildElementIds(const HElementType & item , const IdType & macroId , int nChild)
{
  enum { codim = 0 };
  ids_[codim][item.getIndex()] = createId<codim>(item,macroId,nChild);

  const IdType & itemId = ids_[codim][item.getIndex()];

  buildInteriorElementIds(item,itemId);
}

// build ids for all children of this element
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildInteriorElementIds(const HElementType & item , const IdType & fatherId)
{
  alugrid_assert ( fatherId.isValid() );

  // build id for inner vertex
  {
    const VertexType * v = item.innerVertex() ;
    // for tetras there is no inner vertex, therefore check
    if(v) buildVertexIds(*v,fatherId );
  }

  // build edge ids for all inner edges
  {
    int inneredge = startOffSet_;
    for(const HEdgeType * e = item.innerHedge () ; e ; e = e->next ())
    {
      buildEdgeIds(*e,fatherId,inneredge);
      ++inneredge;
    }
  }

  // build face ids for all inner faces
  {
    int innerface = startOffSet_;
    for(const HFaceType * f = item.innerHface () ; f ; f = f->next ())
    {
      buildFaceIds(*f,fatherId,innerface);
      ++innerface;
    }
  }

  // build ids of all children
  {
    int numChild = startOffSet_;
    for(const HElementType * child = item.down(); child; child =child->next() )
    {
      alugrid_assert ( numChild == child->nChild() );
      buildElementIds(*child, fatherId, numChild);
      ++numChild;
    }
  }
}

// build ids for all children of this face
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildFaceIds(const HFaceType & face, const IdType & fatherId , int innerFace )
{
  enum { codim = 1 };
  ids_[codim][face.getIndex()] = createId<codim>(face,fatherId,innerFace);
  const IdType & faceId = ids_[codim][face.getIndex()];

  buildInteriorFaceIds(face,faceId);
}

// build ids for all children of this face
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildInteriorFaceIds(const HFaceType & face, const IdType & faceId)
{
  alugrid_assert ( faceId.isValid () );

  // build id for inner vertex
  {
    const VertexType * v = face.innerVertex() ;
    if(v) buildVertexIds(*v,faceId );
  }

  // build ids for all inner edges
  {
    int inneredge = startOffSet_;
    for (const HEdgeType * e = face.innerHedge () ; e ; e = e->next ())
    {
      buildEdgeIds(*e,faceId ,inneredge );
      ++inneredge;
    }
  }

  // build ids for all child faces
  {
    int child = startOffSet_;
    for(const HFaceType * f = face.down () ; f ; f = f->next ())
    {
      alugrid_assert ( child == f->nChild()+startOffSet_);
      buildFaceIds(*f,faceId,child);
      ++child;
    }
  }
}

// build ids for all children of this edge
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildEdgeIds(const HEdgeType & edge, const IdType & fatherId , int inneredge)
{
  enum { codim = 2 };
  ids_[codim][edge.getIndex()] = createId<codim>(edge,fatherId,inneredge);
  const IdType & edgeId = ids_[codim][edge.getIndex()];
  buildInteriorEdgeIds(edge,edgeId);
}

template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildInteriorEdgeIds(const HEdgeType & edge, const IdType & edgeId)
{
  alugrid_assert ( edgeId.isValid() );

  // build id for inner vertex
  {
    const VertexType * v = edge.innerVertex() ;
    if(v) buildVertexIds(*v,edgeId );
  }

  // build ids for all inner edges
  {
    int child = startOffSet_;
    for (const HEdgeType * e = edge.down () ; e ; e = e->next ())
    {
      alugrid_assert ( child == e->nChild()+ startOffSet_ );
      buildEdgeIds(*e,edgeId , child );
      ++child;
    }
  }
}

// build id for this vertex
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline void
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
buildVertexIds(const VertexType & vertex, const IdType & fatherId )
{
  enum { codim = 3 };
  // inner vertex number is 0
  ids_[codim][vertex.getIndex()] = createId<codim>(vertex,fatherId,0);
  alugrid_assert ( ids_[codim][vertex.getIndex()].isValid() );
}

// create ids for refined elements
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline int
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
postRefinement( HElementType & item )
{
  {
    enum { elCodim = 0 };
    const IdType & fatherId = ids_[elCodim][item.getIndex()];
    alugrid_assert ( fatherId.isValid() );
    buildInteriorElementIds(item, fatherId );
  }

  const IMPLElementType & elem = static_cast<const IMPLElementType &> (item);
  for(int i=0; i<EntityCountType::numFaces; ++i)
  {
    enum { faceCodim = 1 };
    const HFaceType & face  = *( elem.myhface( i ) );
    const IdType & id = ids_[faceCodim][face.getIndex()];
    alugrid_assert ( id.isValid() );
    buildInteriorFaceIds( face, id);
  }

  {
    for(int i=0; i<EntityCountType::numEdges; ++i)
    {
      enum { edgeCodim = 2 };
      const HEdgeType & edge  = *( elem.myhedge(i));
      const IdType & id = ids_[edgeCodim][edge.getIndex()];
      alugrid_assert ( id.isValid() );
      buildInteriorEdgeIds(edge,id);
    }
  }
  return 0;
}

// dummy functions
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline int
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
preCoarsening( HElementType & elem )
{
  /*
  const IdType & fatherId = ids_[0][item.getIndex()];

  removeElementIds(item,fatherId,item.nChild());

  for(int i=0; i<EntityCountType::numFaces; ++i)
    BuildIds<dim,elType>::buildFace(*this,item,i,ids_[1]);

  for(int i=0; i<EntityCountType::numEdges; ++i)
  {
    const IMPLElementType & elem = static_cast<const IMPLElementType &> (item);
    const HEdgeType & edge  = *( elem.myhedge(i));
    const HEdgeType * child = edge.down();
    alugrid_assert ( child );
    if( ids_[2][child->getIndex() ] > zero_ ) continue;
    buildEdgeIds(edge,ids_[2][edge.getIndex()],0);
  }
#ifdef ALUGRIDDEBUG
  //uniquenessCheck();
#endif
  */
  return 0;
}

// dummy functions
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline int
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
preCoarsening ( HBndSegType & el ) { return 0; }

//! prolong data, elem is the father
template<int dim, int dimworld, ALU3dGridElementType elType, class Comm >
alu_inline int
ALU3dGridGlobalIdSet< dim, dimworld, elType, Comm > ::
postRefinement ( HBndSegType & el ) { return 0; }

#if ! COMPILE_ALUGRID_INLINE
// Instantiation 2-2
template class ALU3dGridGlobalIdSet< 2, 2, hexa,  ALUGridNoComm >;
template class ALU3dGridGlobalIdSet< 2, 2, tetra, ALUGridNoComm >;

template class ALU3dGridGlobalIdSet< 2, 2, hexa,  ALUGridMPIComm >;
template class ALU3dGridGlobalIdSet< 2, 2, tetra, ALUGridMPIComm >;

// Instantiation 3-3
template class ALU3dGridGlobalIdSet< 2, 3, hexa,  ALUGridNoComm >;
template class ALU3dGridGlobalIdSet< 2, 3, tetra, ALUGridNoComm >;

template class ALU3dGridGlobalIdSet< 2, 3, hexa,  ALUGridMPIComm >;
template class ALU3dGridGlobalIdSet< 2, 3, tetra, ALUGridMPIComm >;

// Instantiation 3-3
template class ALU3dGridGlobalIdSet< 3, 3, hexa,  ALUGridNoComm >;
template class ALU3dGridGlobalIdSet< 3, 3, tetra, ALUGridNoComm >;

template class ALU3dGridGlobalIdSet< 3, 3, hexa,  ALUGridMPIComm >;
template class ALU3dGridGlobalIdSet< 3, 3, tetra, ALUGridMPIComm >;
#endif // ! COMPILE_ALUGRID_INLINE

} // end namespace Dune

#endif // #ifndef DUNE_ALU3DGRIDINDEXSETS_CC
