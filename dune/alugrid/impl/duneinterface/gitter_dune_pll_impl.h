// (c) Robert Kloefkorn 2004 -- 2005
#ifndef GITTER_DUNE_PLL_IMPL_H_INCLUDED
#define GITTER_DUNE_PLL_IMPL_H_INCLUDED

#include <memory>

#include "gitter_dune_impl.h"

#include "../parallel/gitter_pll_impl.h"
#include "../parallel/gitter_pll_ldb.h"

namespace ALUGrid
{
  class PackUnpackInteriorGhostData ;

  class GitterDunePll
  : public GitterBasisPll,
    public virtual GitterDuneBasis
  {

    virtual IteratorSTI < Gitter::helement_STI > *
      leafIterator (const Gitter::helement_STI *);
    virtual IteratorSTI < Gitter::helement_STI > *
      leafIterator (const IteratorSTI < Gitter::helement_STI > *);

    friend class PackUnpackInteriorGhostData ;

    using GitterDuneBasis :: checkForConformingRefinement;
  protected:
    bool balanceGrid_;

    // enums for communication type
    typedef enum { Border_Border_Comm ,
                   Interior_Ghost_Comm ,
                   Ghost_Interior_Comm ,
                   All_All_Comm  } CommunicationType;

    typedef SmallObjectStream BufferType;
    typedef std::vector< BufferType > DataBufferType;

  public:
    class Communication;

    // select serial geometric realizations
    typedef Gitter::Geometric Geometric;

    // select parallel objects from GitterBasisPll
    typedef GitterBasisPll::ObjectsPll Objects;

    // constructor taking filename containing the macro grid
    GitterDunePll ( const int dim, const bool conformingRefinement, const char * filename, MpAccessLocal &mp, const ProjectVertexPtrPair &ppv = ProjectVertexPtrPair() )
    : GitterBasisPll( dim, filename, mp, ppv ),
      balanceGrid_ ( false )
    {
      // check and set conforming refinement if true
      checkForConformingRefinement( conformingRefinement );
      // build ghost cells after the macro grid has been assembled
      rebuildGhostCells();
    }

    // constructor taking std::istream containing the macro grid
    GitterDunePll ( const int dim, const bool conformingRefinement, std::istream &in, MpAccessLocal &mp, const ProjectVertexPtrPair &ppv = ProjectVertexPtrPair() )
    : GitterBasisPll( dim, in, mp, ppv ),
      balanceGrid_( false )
    {
      // check and set conforming refinement if true
      checkForConformingRefinement( conformingRefinement );
      // build ghost cells after the macro grid has been assembled
      rebuildGhostCells();
    }

    // constructor creating empty grid
    GitterDunePll (const int dim, const bool conformingRefinement, MpAccessLocal &mp)
      : GitterBasisPll (dim, "", mp, ProjectVertexPtrPair() )
      , balanceGrid_ (false)
    {
      // check and set conforming refinement if true
      checkForConformingRefinement( conformingRefinement );
      // build ghost cells after the macro grid has been assembled
      rebuildGhostCells();
    }

    // adapts and witout calling loadBalancer
    bool adaptWithoutLoadBalancing () { return GitterPll::adapt (); }

    // adapts and calls preCoarsening and
    // postRefinement, no loadBalancing done
    bool duneAdapt (AdaptRestrictProlongType & arp);

  public:

    // communication of border data
    void borderBorderCommunication (
           GatherScatterType & vertexData ,
           GatherScatterType & edgeData,
           GatherScatterType & faceData ,
           GatherScatterType & elementData );

    // communication of border data
    Communication interiorGhostCommunication (
           GatherScatterType & vertexData ,
           GatherScatterType & edgeData,
           GatherScatterType & faceData ,
           GatherScatterType & elementData );

    // communication of border data
    Communication ghostInteriorCommunication (
           GatherScatterType & vertexData ,
           GatherScatterType & edgeData,
           GatherScatterType & faceData ,
           GatherScatterType & elementData );

    // communication of border data
    Communication allAllCommunication (
           GatherScatterType & vertexData ,
           GatherScatterType & edgeData,
           GatherScatterType & faceData ,
           GatherScatterType & elementData );

    // return indexmanger
    IndexManagerType & indexManager(int codim)
    {
      return containerPll().indexManager(codim);
    }

    IndexManagerStorageType& indexManagerStorage()
    {
      return containerPll().indexManagerStorage();
    }

    // return indexmanger
    size_t numMacroBndSegments () const
    {
      return containerPll().numMacroBndSegments();
    }

    // restore grid from std::istream, needed to be overloaded
    // because before restoring follow faces, index manager has to be
    // restored
    void restore(std::istream & in);

  public:

    // write grid to vtk file
    void tovtk( const std::string &fn);

    // compress memory of given grid and return new object (holding equivalent information)
    static GitterDunePll* compress( GitterDunePll* grd )
    {
      // only do the backup-restore thing if dlmalloc is enabled
      if( MyAlloc :: ALUGridUsesDLMalloc )
      {
        MpAccessLocal& mpa = grd->mpAccess ();
        // backup stream
        std::stringstream backup;
        // backup grid
        grd->backup( backup );
        delete grd; grd = 0;
        // free allocated memory (only works if all grids are deleted at this point)
        MyAlloc::clearFreeMemory ();
        // restore saved grid
        grd = new GitterDunePll( grd->dimension(), grd->conformingClosureNeeded(), backup, mpa );
        alugrid_assert ( grd );
        grd->restore( backup );

        // make sure every process got here
        mpa.barrier();
      }
      return grd;
    }

    using GitterDuneBasis::restore;
    using GitterDuneBasis::backup;

    // rebuild ghost cells by exchanging bounndary info on macro level
    void rebuildGhostCells();

  private:
    // check that indices of ghost cells are within range of
    // the index managers maxIndex
    void checkGhostIndices();

    // message tag for communication
    enum { transmittedData = 1 , noData = 0 };

    template <class ObjectStreamType, class HItemType>
    void sendSlaves (ObjectStreamType & sendBuff,
        HItemType * determType,
        GatherScatterType & dataHandle , const int link );

    template <class ObjectStreamType, class HItemType, class CommBuffMapType>
    void unpackOnMaster(ObjectStreamType & recvBuff,
        CommBuffMapType& commBufMap,
        HItemType * determType,
        GatherScatterType & dataHandle ,
        const int nl, const int link);

    template <class ObjectStreamType, class HItemType, class CommBuffMapType>
    void sendMaster(ObjectStreamType & sendBuff,
        CommBuffMapType& commBufMap,
        HItemType * determType,
        GatherScatterType & dataHandle ,
        const int nl, const int myLink);

    template <class ObjectStreamType, class HItemType>
    void unpackOnSlaves(ObjectStreamType & recvBuff,
        HItemType * determType,
        GatherScatterType & dataHandle ,
        const int nOtherLinks, const int myLink);

    void sendFaces (ObjectStream & sendBuff,
        IteratorSTI < hface_STI > * iter,
        GatherScatterType & dataHandle );

    void unpackFaces (ObjectStream & recvBuff,
        IteratorSTI < hface_STI > * iter,
        GatherScatterType & dataHandle );

    void sendInteriorGhostAllData (
      ObjectStream & sendBuff,
      IteratorSTI < hface_STI > * iter ,
      GatherScatterType & vertexData ,
      GatherScatterType & edgeData,
      GatherScatterType & faceData,
      GatherScatterType & elementData ,
      const bool packInterior ,
      const bool packGhosts );

    void sendInteriorGhostElementData (
      ObjectStream & sendBuff,
      IteratorSTI < hface_STI > * iter ,
      GatherScatterType & elementData);

    void unpackInteriorGhostAllData (
      ObjectStream & recvBuff,
      IteratorSTI < hface_STI > * iter ,
      GatherScatterType & vertexData ,
      GatherScatterType & edgeData,
      GatherScatterType & faceData,
      GatherScatterType & elementData );

    void unpackInteriorGhostElementData (
      ObjectStream & recvBuff,
      IteratorSTI < hface_STI > * iter ,
      GatherScatterType & elementData );

    // communication of data on border
    void doBorderBorderComm (
        std::vector< ObjectStream > & osvec ,
        GatherScatterType & vertexData ,
        GatherScatterType & edgeData,
        GatherScatterType & faceData );

    // communication of interior data
    void doInteriorGhostComm(
      std::vector< ObjectStream > & osvec ,
      GatherScatterType & vertexData ,
      GatherScatterType & edgeData,
      GatherScatterType & faceData,
      GatherScatterType & elementData ,
      const CommunicationType commType );

    template <class HItemType, class CommMapType>
    DataBufferType&
    getCommunicationBuffer( HItemType&, CommMapType&, const int );

  public:
    std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > *> borderIteratorTT (const vertex_STI *, int);
    std::pair< IteratorSTI < hedge_STI  > *, IteratorSTI < hedge_STI  > *> borderIteratorTT (const hedge_STI  *, int);
    std::pair< IteratorSTI < hface_STI >  *, IteratorSTI < hface_STI  > *> borderIteratorTT  (const hface_STI  *, int);

    std::pair< IteratorSTI < hface_STI >  *, IteratorSTI < hface_STI  > *> leafBorderIteratorTT  (const hface_STI  *, int);
    std::pair< IteratorSTI < hface_STI >  *, IteratorSTI < hface_STI  > *> levelBorderIteratorTT (const hface_STI  *, int link , int level);
  };



  // GitterDunePll::Communication
  // ----------------------------

  class GitterDunePll::Communication
  {
    struct DataHandle;

  public:
    Communication () : exchange_( 0 ) {}

    Communication ( GitterDunePll &grid,
                    GatherScatter &vertexGatherScatter, GatherScatter &edgeGatherScatter, GatherScatter &faceGatherScatter, GatherScatter &elementGatherScatter,
                    GitterDunePll::CommunicationType commType );

    Communication ( Communication &&other );

    ~Communication () { wait(); }

    Communication &operator= ( Communication &&other );

    bool ready () const { return ! bool( exchange_ ); }

    void wait ();

    [[ deprecated ]]
    bool pending () const { return ! ready(); }
  private:
    std::unique_ptr< MpAccessLocal::NonBlockingExchange::DataHandleIF > dataHandle_;
    MpAccessLocal::NonBlockingExchange *exchange_;
    std::vector< ObjectStream > sendOS_;
  };

} // namespace ALUGrid

#endif // #ifndef GITTER_DUNE_PLL_IMPL_H_INCLUDED
