#ifndef DUNE_ALUGRID_ALU3DINCLUDE_HH
#define DUNE_ALUGRID_ALU3DINCLUDE_HH

#include "aluinline.hh"

// all methods and classes of the ALUGrid are defined in the namespace
#define ALU3DSPACE ::ALUGrid::

#include <dune/alugrid/common/declaration.hh>

#include <dune/alugrid/impl/serial/gatherscatter.hh>
#include <dune/alugrid/impl/serial/key.h>
#include <dune/alugrid/impl/serial/myalloc.h>
#include <dune/alugrid/impl/serial/serialize.h>

#include <dune/alugrid/impl/parallel/mpAccess.h>
#include <dune/alugrid/impl/parallel/gitter_pll_ldb.h>

#include <dune/alugrid/impl/serial/gitter_sti.h>

#include <dune/alugrid/impl/serial/gitter_hexa_top.h>
#include <dune/alugrid/impl/serial/mapp_tetra_3d_ext.h>
#include <dune/alugrid/impl/serial/gitter_tetra_top.h>
#include <dune/alugrid/impl/serial/walk.h>
#include <dune/alugrid/impl/serial/gitter_impl.h>
#include <dune/alugrid/impl/serial/gitter_mgb.h>
#include <dune/alugrid/impl/serial/key.h>
#include <dune/alugrid/impl/serial/lock.h>

#include <dune/alugrid/impl/duneinterface/gitter_dune_impl.h>

namespace ALUGrid
{

  typedef Gitter::AdaptRestrictProlong AdaptRestrictProlongType;

  static const int ProcessorBoundary_t = Gitter::hbndseg_STI::closure;

  // general GatherScatter type
  typedef GatherScatter GatherScatterType;

} // namespace ALUGrid


// headers for parallel grid structures
#include <dune/alugrid/impl/parallel/gitter_pll_sti.h>
#include <dune/alugrid/impl/parallel/gitter_pll_impl.h>
#include <dune/alugrid/impl/parallel/gitter_pll_ldb.h>
#include <dune/alugrid/impl/parallel/gitter_tetra_top_pll.h>
#include <dune/alugrid/impl/parallel/gitter_hexa_top_pll.h>
#include <dune/alugrid/impl/parallel/gitter_pll_mgb.h>
#include <dune/alugrid/impl/duneinterface/gitter_dune_pll_impl.h>

#if ALU3DGRID_PARALLEL
// if MPI was found include MPI communications
#include <dune/alugrid/impl/parallel/mpAccess_MPI.h>
#endif // #if ALU3DGRID_PARALLEL

//- local includes
#include <dune/alugrid/3d/topology.hh>

namespace Dune
{

  // typedef of ALU3dGridElementType see topology.hh

  // i.e. double or float
  typedef double alu3d_ctype;


  // ALU3dBasicImplTraits
  // --------------------

  template< class Comm >
  struct ALU3dBasicImplTraits;

  template<>
  struct ALU3dBasicImplTraits< ALUGridNoComm >
  {
    typedef ALU3DSPACE Gitter GitterType;
    typedef ALU3DSPACE GitterDuneImpl GitterImplType;

    typedef GitterType::helement_STI  HElementType;   // Interface Element
    typedef GitterType::hface_STI     HFaceType;      // Interface Face
    typedef GitterType::hedge_STI     HEdgeType;      // Interface Edge
    typedef GitterType::vertex_STI    VertexType;     // Interface Vertex
    typedef GitterType::hbndseg_STI   HBndSegType;
    typedef GitterType::ghostpair_STI GhostPairType;

    typedef HElementType PllElementType;

    typedef GitterType::Geometric::hedge1_GEO GEOEdgeType;

    //! method for ghost check
    template <class BndFaceType>
    static bool isGhost( const BndFaceType* ghost )
    {
      return false ;
    }
  };

  template<>
  struct ALU3dBasicImplTraits< ALUGridMPIComm >
  {
    typedef ALU3DSPACE GitterDunePll GitterType;
    typedef ALU3DSPACE GitterDunePll GitterImplType;

    typedef GitterType::helement_STI  HElementType;   // Interface Element
    typedef GitterType::hface_STI     HFaceType;      // Interface Face
    typedef GitterType::hedge_STI     HEdgeType;      // Interface Edge
    typedef GitterType::vertex_STI    VertexType;     // Interface Vertex
    typedef GitterType::hbndseg_STI   HBndSegType;
    typedef GitterType::ghostpair_STI GhostPairType;

    typedef ALU3DSPACE ElementPllXIF_t PllElementType;

    typedef GitterType::Geometric::hedge1_GEO GEOEdgeType;

    // method for ghost check
    template <class BndFaceType>
    static bool isGhost( const BndFaceType* ghost )
    {
      return ( ghost != 0 );
    }
  };


  // ALU3dCodimImplTraits
  // --------------------

  template< ALU3dGridElementType elType, class Comm, int dim, int codim >
  struct ALU3dCodimImplTraits;

  template< class Comm, int dim>
  struct ALU3dCodimImplTraits< tetra, Comm, dim, 0 >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;
    typedef typename ALU3dBasicImplTraits< Comm >::GitterImplType GitterImplType;

    typedef typename GitterType::helement_STI                 InterfaceType;
    typedef typename GitterType::Geometric::hasFace3          EntitySeedType;
    typedef typename GitterImplType::Objects::tetra_IMPL      ImplementationType;
    typedef typename GitterType::hbndseg_STI                  GhostInterfaceType;
    typedef typename GitterImplType::Objects::Hbnd3Default    GhostImplementationType;
  };

  template< class Comm, int dim >
  struct ALU3dCodimImplTraits< hexa, Comm, dim, 0 >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType     GitterType;
    typedef typename ALU3dBasicImplTraits< Comm >::GitterImplType GitterImplType;

    typedef typename GitterType::helement_STI                 InterfaceType;
    typedef typename GitterType::Geometric::hasFace4          EntitySeedType;
    typedef typename GitterImplType::Objects::hexa_IMPL       ImplementationType;
    typedef typename GitterType::hbndseg_STI                  GhostInterfaceType;
    typedef typename GitterImplType::Objects::Hbnd4Default    GhostImplementationType;
  };

  template< class Comm, int dim >
  struct ALU3dCodimImplTraits< tetra, Comm, dim, 1 >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;

    typedef typename GitterType::hface_STI                    InterfaceType;
    typedef InterfaceType                                     EntitySeedType;
    typedef typename GitterType::Geometric::hface3_GEO        ImplementationType;
  };

  template< class Comm, int dim >
  struct ALU3dCodimImplTraits< hexa, Comm, dim, 1 >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;

    typedef typename GitterType::hface_STI                    InterfaceType;
    typedef InterfaceType                                     EntitySeedType;
    typedef typename GitterType::Geometric::hface4_GEO        ImplementationType;
  };

  template< ALU3dGridElementType elType, class Comm >
  struct ALU3dCodimImplTraits< elType, Comm, 3, 2 >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;

    typedef typename GitterType::hedge_STI                    InterfaceType;
    typedef InterfaceType                                     EntitySeedType;
    typedef typename GitterType::Geometric::hedge1_GEO        ImplementationType;
  };


  template< ALU3dGridElementType elType, class Comm >
  struct ALU3dCodimImplTraits< elType, Comm, 2, 2 >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;

    typedef typename GitterType::vertex_STI                   InterfaceType;
    typedef InterfaceType                                     EntitySeedType;
    typedef typename GitterType::Geometric::VertexGeo         ImplementationType;
  };

  template< ALU3dGridElementType elType, class Comm >
  struct ALU3dCodimImplTraits< elType, Comm, 3, 3 >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;

    typedef typename GitterType::vertex_STI                   InterfaceType;
    typedef InterfaceType                                     EntitySeedType;
    typedef typename GitterType::Geometric::VertexGeo         ImplementationType;
  };

  // Refinement rules in general
  template< class MarkRuleType, ALU3dGridElementType elType >
  struct ALU3dRefinementTraits {};

  // Refinement rules for simplices
  template< class MarkRuleType >
  struct ALU3dRefinementTraits < MarkRuleType, tetra >
  {
    // refinement and coarsening enum
    enum { bisect_element_t  = MarkRuleType::bisect  };
    enum { refine_element_t  = MarkRuleType::regular };
    enum { coarse_element_t  = MarkRuleType::crs     };
    enum { nosplit_element_t = MarkRuleType::nosplit };
  };

  // Refinement rules for cubes
  template< class MarkRuleType >
  struct ALU3dRefinementTraits < MarkRuleType, hexa >
  {
    // refinement and coarsening enum
    enum { bisect_element_t  = MarkRuleType::regular };
    enum { refine_element_t  = MarkRuleType::regular };
    enum { coarse_element_t  = MarkRuleType::crs     };
    enum { nosplit_element_t = MarkRuleType::nosplit };
  };


  // ALU3dImplTraits
  // ---------------

  template< ALU3dGridElementType elType, class Comm >
  struct ALU3dImplTraits;

  template< class Comm >
  struct ALU3dImplTraits< tetra, Comm >
  : public ALU3dBasicImplTraits< Comm >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;
    typedef typename ALU3dBasicImplTraits< Comm >::GitterImplType GitterImplType;

    typedef typename GitterType::Geometric::hface3_GEO GEOFaceType;
    typedef typename GitterType::Geometric::VertexGeo GEOVertexType;
    typedef typename GitterImplType::Objects::tetra_IMPL IMPLElementType;
    typedef typename GitterType::Geometric::tetra_GEO     GEOElementType;
    typedef typename GitterType::Geometric::periodic3_GEO GEOPeriodicType;
    typedef typename GitterType::Geometric::hasFace3 HasFaceType;
    typedef typename GitterType::Geometric::Hface3Rule HfaceRuleType;
    typedef typename GitterImplType::Objects::Hbnd3Default BNDFaceType;
    typedef typename GitterImplType::Objects::hbndseg3_IMPL ImplBndFaceType;

    typedef typename GitterType::Geometric::TetraRule MarkRuleType;

    struct RefinementRules
    : public ALU3dRefinementTraits<MarkRuleType, tetra>
    {};

    typedef std::pair< GEOFaceType *, int > NeighbourFaceType;
    typedef std::pair< HasFaceType *, int > NeighbourPairType;

    template< int dim, int codim >
    struct Codim
    : public ALU3dCodimImplTraits< tetra, Comm, dim, codim >
    {};

    // access of faces
    template <class Elem>
    static const GEOFaceType* getFace( const Elem& elem, const int aluFace )
    {
      return elem.myhface( aluFace );
    }
  };

  template< class Comm >
  struct ALU3dImplTraits< hexa, Comm >
  : public ALU3dBasicImplTraits< Comm >
  {
    typedef typename ALU3dBasicImplTraits< Comm >::GitterType GitterType;
    typedef typename ALU3dBasicImplTraits< Comm >::GitterImplType GitterImplType;

    typedef typename GitterType::Geometric::hface4_GEO GEOFaceType;
    typedef typename GitterType::Geometric::VertexGeo GEOVertexType;
    typedef typename GitterImplType::Objects::hexa_IMPL IMPLElementType;
    typedef typename GitterType::Geometric::hexa_GEO GEOElementType;
    typedef typename GitterType::Geometric::periodic4_GEO GEOPeriodicType;
    typedef typename GitterType::Geometric::hasFace4 HasFaceType;
    typedef typename GitterType::Geometric::Hface4Rule HfaceRuleType;
    typedef typename GitterImplType::Objects::Hbnd4Default BNDFaceType;
    typedef typename GitterImplType::Objects::hbndseg4_IMPL ImplBndFaceType;

    typedef typename GitterType::Geometric::HexaRule MarkRuleType;

    struct RefinementRules
    : public ALU3dRefinementTraits<MarkRuleType, hexa>
    {};

    typedef std::pair< GEOFaceType *, int > NeighbourFaceType;
    typedef std::pair< HasFaceType *, int > NeighbourPairType;

    template< int dim, int codim >
    struct Codim
    : public ALU3dCodimImplTraits< hexa, Comm, dim, codim >
    {};

    // access of faces
    template <class Elem>
    static const GEOFaceType* getFace( const Elem& elem, const int aluFace )
    {
      return elem.myhface( aluFace );
    }
  };



  //! contains list of vertices of one level
  //! needed for VertexLevelIterator
  template< class Comm >
  struct ALU3dGridVertexList
  {
    // level vertex iterator list
    typedef typename ALU3dBasicImplTraits< Comm >::VertexType VertexType;
    typedef std::vector< VertexType * > VertexListType;
    typedef typename VertexListType::iterator IteratorType;

    ALU3dGridVertexList ()
    : up2Date_( false )
    {}

    size_t size () const  { return vertexList_.size(); }

    bool up2Date () const { return up2Date_;  }
    void unsetUp2Date ()  { up2Date_ = false; }

    // make grid walkthrough and calc global size
    template <class GridType>
    void setupVxList (const GridType & grid, int level);

    IteratorType begin () { return vertexList_.begin(); }
    IteratorType end   () { return vertexList_.end(); }

    VertexListType & getItemList() { return vertexList_; }

  private:
    bool up2Date_;
     //careful: due to the setupVxList structure the ordering of vertexList_ and validateList_ differ in the level Case
     //for validateList_ we want the ALUGrid Index as ordering, as we want to use it for faces
    VertexListType vertexList_;
  };


  //! contains list of vertices of one level
  //! needed for VertexLevelIterator
  template< class Comm >
  struct ALU3dGridLeafVertexList
  {
    // level vertex iterator list
    typedef typename ALU3dBasicImplTraits< Comm >::VertexType VertexType;
    typedef std::pair< VertexType *, int > ItemType;
    typedef std::vector< ItemType > VertexListType;
    typedef typename VertexListType::iterator IteratorType;

    ALU3dGridLeafVertexList ()
    : up2Date_( false )
    {}

    size_t size () const  { return vertexList_.size(); }

    bool up2Date () const { return up2Date_;  }
    void unsetUp2Date ()  { up2Date_ = false; }

    // make grid walkthrough and calc global size
    template <class GridType>
    void setupVxList (const GridType & grid);

    IteratorType begin () { return vertexList_.begin(); }
    IteratorType end   () { return vertexList_.end(); }

    VertexListType & getItemList() { return vertexList_; }

    int getLevel ( const VertexType &vertex ) const
    {
      const int idx = vertex.getIndex();
      alugrid_assert ( idx >= 0 );
      alugrid_assert ( idx < (int)size());
      const ItemType & p = vertexList_[idx];
      if( p.first == 0 )
        return vertex.level();
      else
        return p.second;
    }

  private:
    bool up2Date_;
    VertexListType vertexList_;
  };



  class ALU3dGridItemList
  {
  public:
    // level vertex iterator list
    typedef std::vector < void * > ItemListType;
    typedef ItemListType :: iterator IteratorType;

    ALU3dGridItemList () : up2Date_(false) {}

    size_t size () const  { return itemList_.size(); }

    bool up2Date () const { return up2Date_;  }
    void unsetUp2Date ()  { up2Date_ = false; }

    void markAsUp2Date() { up2Date_ = true; }

    IteratorType begin () { return itemList_.begin(); }
    IteratorType end   () { return itemList_.end(); }

    ItemListType & getItemList() { return itemList_; }

  private:
    bool up2Date_;
    ItemListType itemList_;
  };

  typedef ALU3dGridItemList ALU3dGridItemListType;

  /////////////////////////////////////////////////////////////////////////
  //  some helper functions
  /////////////////////////////////////////////////////////////////////////

  template< class Comm >
  struct ALU3dGridFaceGetter
  {
    static const typename ALU3dImplTraits< tetra, Comm >::GEOFaceType *
    getFace( const typename ALU3dImplTraits< tetra, Comm >::GEOElementType& elem, int index)
    {
      alugrid_assert (index >= 0 && index < 4);
      return elem.myhface( ElementTopologyMapping< tetra >::dune2aluFace(index) );
    }

    static const typename ALU3dImplTraits< hexa, Comm >::GEOFaceType*
    getFace( const typename ALU3dImplTraits< hexa, Comm >::GEOElementType &elem, int index )
    {
      alugrid_assert (index >= 0 && index < 6);
      return elem.myhface( ElementTopologyMapping< hexa >::dune2aluFace(index) );
    }
  };

} // end namespace Dune

#endif // #ifndef DUNE_ALUGRID_ALU3DINCLUDE_HH
