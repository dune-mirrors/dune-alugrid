// (c) Robert Kloefkorn 2004 -- 2005 
#ifndef GITTER_DUNE_PLL_IMPL_H_INCLUDED
#define GITTER_DUNE_PLL_IMPL_H_INCLUDED

#include "gitter_dune_impl.h"
 
#include "gitter_pll_impl.h"
#include "gitter_pll_ldb.h"
  
class GitterDunePll : public GitterBasisPll , public virtual GitterDuneBasis
{
 
  virtual IteratorSTI < Gitter :: helement_STI > * 
    leafIterator (const Gitter :: helement_STI *) ;
  virtual IteratorSTI < Gitter :: helement_STI > * 
    leafIterator (const IteratorSTI < Gitter :: helement_STI > *) ;
  
protected:  
  bool balanceGrid_;

  // enums for communication type
  typedef enum { Border_Border_Comm , 
                 Interior_Ghost_Comm , 
                 Ghost_Interior_Comm , 
                 All_All_Comm  } CommunicationType ; 

  typedef SmallObjectStream BufferType;
  typedef vector< BufferType > DataBufferType;

public:
  typedef Gitter :: Geometric Geometric;
  typedef GitterDuneImpl :: Objects  Objects;

  
  GitterDunePll (const char * filename , 
                 MpAccessLocal &mp, 
                 ProjectVertex* ppv = 0 ) 
    : GitterBasisPll (filename, mp, ppv) 
    , balanceGrid_ (false) 
  {
#ifndef NDEBUG
    __STATIC_myrank = mp.myrank(); 
#endif
    // if grid is created from backup, then restore ghost cells 
    rebuildGhostCells();
  }

  GitterDunePll (MpAccessLocal &mp) 
    : GitterBasisPll ("", mp, 0) 
    , balanceGrid_ (false) 
  {
#ifndef NDEBUG
    __STATIC_myrank = mp.myrank(); 
#endif
    // if grid is created from backup, then restore ghost cells 
    rebuildGhostCells();
  }

  ~GitterDunePll () {
  }

  // refine alle leaf elements 
  bool refine (); 

  // coarse all leaf elements if possible 
  void coarse ();

  // adapts and witout calling loadBalancer  
  bool adaptWithoutLoadBalancing ();

  // adapts and calls preCoarsening and
  // postRefinement, no loadBalancing done   
  bool duneAdapt (AdaptRestrictProlongType & arp);

  // return true if grid has to be balanced again 
  bool duneNotifyNewGrid ();
  bool duneNotifyNewGrid ( LoadBalancer::DataBase& );

  bool duneLoadBalance () ; // call loadBalancer 
  bool duneLoadBalance (GatherScatterType & , AdaptRestrictProlongType & arp ) ; // call loadBalancer a

  void duneRepartitionMacroGrid (LoadBalancer :: DataBase &, GatherScatterType & gs) ;
  void repartitionMacroGrid (LoadBalancer :: DataBase &) ;
 
  // notifyMacroGridChanges for dune
  void duneNotifyMacroGridChanges (); 
  
  // notifyGridChanges for dune
  void duneNotifyGridChanges (); 
  
  // exchange changed elements  
  void duneExchangeDynamicState ();
  
  // communication of border data 
  void borderBorderCommunication (
         GatherScatterType & vertexData ,
         GatherScatterType & edgeData,
         GatherScatterType & faceData ,
         GatherScatterType & elementData );

  // communication of border data 
  void interiorGhostCommunication (
         GatherScatterType & vertexData ,
         GatherScatterType & edgeData,
         GatherScatterType & faceData ,
         GatherScatterType & elementData );

  // communication of border data 
  void ghostInteriorCommunication (
         GatherScatterType & vertexData ,
         GatherScatterType & edgeData,
         GatherScatterType & faceData ,
         GatherScatterType & elementData );

  // communication of border data 
  void allAllCommunication (
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

  // restore parallel grid from before
  virtual void duneRestore (const char*) ;
  // backup current grid status 
  virtual void duneBackup (const char*) ;

private:
  // restore grid from istream, needed to be overloaded 
  // because before restoring follow faces, index manager has to be
  // restored 
  virtual void restore(istream & in);

  // rebuild ghost cells by exchanging bounndary info on macro level 
  void rebuildGhostCells();
  
  // check that indices of ghost cells are within range of
  // the index managers maxIndex  
  void checkGhostIndices();
  
  // communication of data 
  void doCommunication(
         GatherScatterType & vertexData ,
         GatherScatterType & edgeData,
         GatherScatterType & faceData ,
         GatherScatterType & elementData ,
         const CommunicationType commType);

  // message tag for communication 
  enum { transmittedData = 1 , noData = 0 };

  template <class ObjectStreamType, class HItemType>
  void sendSlaves (ObjectStreamType & sendBuff,
      HItemType * determType,
      GatherScatterType & dataHandle , const int link ) ; 
    
  template <class ObjectStreamType, class HItemType, class CommBuffMapType>
  void unpackOnMaster(ObjectStreamType & recvBuff,
      CommBuffMapType& commBufMap,
      HItemType * determType,
      GatherScatterType & dataHandle , 
      const int nl, const int link) ; 
    
  template <class ObjectStreamType, class HItemType, class CommBuffMapType>
  void sendMaster(ObjectStreamType & sendBuff,
      CommBuffMapType& commBufMap,
      HItemType * determType,
      GatherScatterType & dataHandle , 
      const int nl, const int myLink) ; 
    
  template <class ObjectStreamType, class HItemType>
  void unpackOnSlaves(ObjectStreamType & recvBuff,
      HItemType * determType,
      GatherScatterType & dataHandle , 
      const int nOtherLinks, const int myLink) ; 

  void sendFaces (ObjectStream & sendBuff,
      IteratorSTI < hface_STI > * iter, 
      GatherScatterType & dataHandle ) ; 
    
  void unpackFaces (ObjectStream & recvBuff,
      IteratorSTI < hface_STI > * iter, 
      GatherScatterType & dataHandle ) ; 
    
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
      vector< ObjectStream > & osvec ,
      GatherScatterType & vertexData ,
      GatherScatterType & edgeData,
      GatherScatterType & faceData );

  // communication of interior data 
  void doInteriorGhostComm(
    vector< ObjectStream > & osvec ,
    GatherScatterType & vertexData ,
    GatherScatterType & edgeData,
    GatherScatterType & faceData,
    GatherScatterType & elementData ,
    const CommunicationType commType );

  template <class HItemType, class CommMapType> 
  DataBufferType& 
  getCommunicationBuffer( HItemType&, CommMapType&, const int ); 

public:
  pair < IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > *> borderIteratorTT (const vertex_STI *, int) ;
  pair < IteratorSTI < hedge_STI  > *, IteratorSTI < hedge_STI  > *> borderIteratorTT (const hedge_STI  *, int) ;
  pair < IteratorSTI < hface_STI >  *, IteratorSTI < hface_STI  > *> borderIteratorTT  (const hface_STI  *, int) ;
  
  pair < IteratorSTI < hface_STI >  *, IteratorSTI < hface_STI  > *> leafBorderIteratorTT  (const hface_STI  *, int) ;
  pair < IteratorSTI < hface_STI >  *, IteratorSTI < hface_STI  > *> levelBorderIteratorTT (const hface_STI  *, int link , int level) ;
};
#endif
