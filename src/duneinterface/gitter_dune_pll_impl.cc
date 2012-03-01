#ifndef GITTER_DUNE_PLL_IMPL_CC_INCLUDED
#define GITTER_DUNE_PLL_IMPL_CC_INCLUDED

#include "gitter_dune_pll_impl.h"

IteratorSTI < Gitter :: helement_STI > * GitterDunePll :: 
leafIterator (const helement_STI *)
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
    TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > (container ()) ;
}

IteratorSTI < Gitter :: helement_STI > * GitterDunePll ::
leafIterator (const IteratorSTI < helement_STI > * p)
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
    TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > >
    (*(const Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
       TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > *) p) ;
}

bool GitterDunePll :: duneNotifyNewGrid ()
{
  LoadBalancer :: DataBase db ;
  return checkPartitioning( db );
}

void GitterDunePll :: duneNotifyMacroGridChanges ()
{
  GitterPll :: notifyMacroGridChanges ();
  rebuildGhostCells ();
}

void GitterDunePll :: duneNotifyGridChanges ()
{
  Gitter :: notifyGridChanges () ;
  duneExchangeDynamicState () ;
  return ;
}


// adapt, no loadBalancing done   
bool GitterDunePll :: adaptWithoutLoadBalancing ()   
{
  __STATIC_myrank = mpAccess ().myrank () ;
  __STATIC_turn ++ ;
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll["<< __STATIC_myrank << "] :: adaptWithoutLB ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;

  int start = clock () ;
  const bool refined = this->refine() ;
  int lap = clock () ;
  this->coarse ();
  
  int end = clock () ;
  if (debugOption (1))
  {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO GitterDunePll["<< __STATIC_myrank << "] :: adaptWithoutLB () [ref (loops)|cse|all] " << u1 << " ("
       << _refineLoops << ") " << u2 << " " << u3 << endl ;
  }

#ifndef NDEBUG
  // make sure every process returns the same value 
  const bool checkRefined = mpAccess().gmax( refined );
  assert( refined == checkRefined );
#endif

  duneNotifyGridChanges () ;
  return refined;
}

// done call notify and loadBalancer  
bool GitterDunePll :: duneAdapt (AdaptRestrictProlongType & arp)   
{
  this->setAdaptRestrictProlongOp(arp);
  bool refined = this->adaptWithoutLoadBalancing();
  this->removeAdaptRestrictProlongOp ();
  return refined;
}

bool GitterDunePll :: duneLoadBalance () 
{
  loadBalancerGridChangesNotify () ;
  return true;
}

// returns true if grid was repartitioned 
bool GitterDunePll :: duneLoadBalance (GatherScatterType & gs, AdaptRestrictProlongType & arp) {
  
  this->setAdaptRestrictProlongOp(arp);
  assert (debugOption (20) ? (cout << "**GitterDunePll :: duneLoadBalance () " << endl, 1) : 1) ;

  LoadBalancer :: DataBase db;
  const bool neu = checkPartitioning( db );

  if (neu) 
  {
    if (mpAccess ().gmax (_ldbMethod)) 
    {
      assert (debugOption (5) ? (cout << "**GitterDunePll :: repartitioning macro grid! " << endl, 1) : 1) ;
      duneRepartitionMacroGrid (db, gs) ;
      notifyMacroGridChanges () ;
    }
  }

  this->removeAdaptRestrictProlongOp ();
  return neu;
}

void GitterDunePll :: duneExchangeDynamicState () 
{
  // Die Methode wird jedesmal aufgerufen, wenn sich der dynamische
  // Zustand des Gitters ge"andert hat: Verfeinerung und alle Situationen
  // die einer "Anderung des statischen Zustands entsprechen. Sie wird in
  // diesem Fall NACH dem Update des statischen Zustands aufgerufen, und
  // kann demnach von einem korrekten statischen Zustand ausgehen. F"ur
  // Methoden die noch h"aufigere Updates erfordern m"ussen diese in der
  // Regel hier eingeschleift werden.
  {
    const int nl = mpAccess ().nlinks () ;
  
#ifndef NDEBUG 
    // if debug mode, then count time 
    const int start = clock () ;
#endif
  
    try 
    {
      typedef Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
         TreeIterator < hface_STI, is_def_true < hface_STI > > > InnerIteratorType;
      typedef Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, is_def_true < hface_STI > > > OuterIteratorType;
                
      vector < ObjectStream > osv (nl) ;
      {
        for (int l = 0 ; l < nl ; l ++) 
        {
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;

          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
          }
      
          OuterIteratorType wo (mof);
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
          }
        }  
      }
    
      // exchange data 
      osv = mpAccess ().exchange (osv) ;
    
      { 
        for (int l = 0 ; l < nl ; l ++ ) 
        {
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
      
          OuterIteratorType wo (mof) ;
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
          }
      
          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
          }
        }
      }
    } 
    catch (Parallel ::  AccessPllException) 
    {
      cerr << "  FEHLER Parallel :: AccessPllException entstanden in: " << __FILE__ << " " << __LINE__ << endl ;
    }
    assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: exchangeDynamicState () used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << endl, 1) : 1 ) ;
  }
}

pair < IteratorSTI < GitterPll :: vertex_STI > *, IteratorSTI < GitterPll :: vertex_STI > *> 
GitterDunePll :: borderIteratorTT (const vertex_STI * v, int link )
{
  // return default vertex iterator 
  return this->iteratorTT(v, link);
}

pair < IteratorSTI < GitterPll :: hedge_STI > *, IteratorSTI < GitterPll :: hedge_STI > *> 
GitterDunePll :: borderIteratorTT (const hedge_STI * e, int link )
{
  // return edge iterator over all edges 
  is_def_true< hedge_STI > * s = 0;
  return this->createEdgeIteratorTT(s, link);
}

pair < IteratorSTI < GitterPll :: hface_STI > *, IteratorSTI < GitterPll :: hface_STI > *> 
GitterDunePll :: borderIteratorTT (const hface_STI * f, int link )
{
  // return face iterator over all faces 
  is_def_true< hface_STI > rule ;
  return this->createFaceIteratorTT( rule , link);
}

pair < IteratorSTI < GitterPll :: hface_STI > *, IteratorSTI < GitterPll :: hface_STI > *> 
GitterDunePll :: leafBorderIteratorTT (const hface_STI * f, int link )
{
  // return face iterator over all faces that are 
  // leaf faces in the DUNE context
  is_leaf_entity < hface_STI > rule;
  return this->createFaceIteratorTT( rule , link);
}

pair < IteratorSTI < GitterPll :: hface_STI > *, IteratorSTI < GitterPll :: hface_STI > *> 
GitterDunePll :: levelBorderIteratorTT (const hface_STI * f, int link , int level)
{
  // return face iterator over faces with given level 
  any_has_level < hface_STI > rule(level);
  return this->createFaceIteratorTT( rule, link);
}

template <class ObjectStreamType, class HItemType> 
void GitterDunePll :: sendSlaves (
    ObjectStreamType & sendBuff, 
    HItemType * fakeItem ,
    GatherScatterType & dataHandle, const int link )
{
  // temporary buffer 
  SmallObjectStream osTmp; 

  pair < IteratorSTI < HItemType > *, IteratorSTI < HItemType > *> 
    a = borderIteratorTT (fakeItem, link ); //ueber alle meine Slave-Knoten 
 
  IteratorSTI < HItemType > & iter = *(a.second);
  for (iter.first (); ! iter.done () ; iter.next ()) 
  {
    HItemType & item = iter.item();

    // gather all data on slaves 
    if ( dataHandle.containsItem(item) ) 
    {
      // write marker that show data is transmitted 
      sendBuff.writeObject( transmittedData );

      // reset read and write position
      osTmp.clear();
      // write data to fake buff to determine size of data package
      dataHandle.sendData(osTmp,item);

      int s = osTmp.size();
      // first write size 
      sendBuff.writeObject(s);
      // then write bytes 
      sendBuff.writeStream(osTmp);
    } 
    else 
    {
      // write noData marker 
      sendBuff.writeObject( noData );
    }
  }

  delete a.first;
  delete a.second;      

  return ;
}

template <class HItemType, class CommMapType>
GitterDunePll :: DataBufferType& 
GitterDunePll :: 
getCommunicationBuffer( HItemType& item, CommMapType& commMap, const int nCommBuff )
{
#ifdef ALUGRID_USE_COMM_BUFFER_IN_ITEM
  // reserve and get comm buffers 
  item.reserveBuffer( nCommBuff );
  return item.commBuffer();
#else 
  DataBufferType& commBuff = commMap[ &item ];
  if( (int) commBuff.size() != nCommBuff ) 
    commBuff.resize( nCommBuff );
  return commBuff;
#endif
}

template <class ObjectStreamType, class HItemType, class CommBuffMapType> 
void GitterDunePll :: unpackOnMaster (
    ObjectStreamType & recvBuff, 
    CommBuffMapType& commBuffMap,
    HItemType * determType,
    GatherScatterType & dataHandle ,
    const int nl, const int link )
{
  int hasdata;

  typedef SmallObjectStream BufferType;
  typedef vector< BufferType > DataBufferType;

  pair < IteratorSTI < HItemType > *, IteratorSTI < HItemType > *> 
    a = borderIteratorTT (determType, link);
 
  IteratorSTI < HItemType > & iter = *(a.first);

  // for all master items 
  for (iter.first (); ! iter.done () ; iter.next ()) 
  {
    HItemType & item = iter.item();
   
    // read data marker 
    recvBuff.readObject(hasdata);
    
    // get comm buffers 
    DataBufferType & data = getCommunicationBuffer( item, commBuffMap, nl + 1 );

    // only gather master data once 
    if ( dataHandle.containsItem( item ) ) 
    {
      // pack master data 
      BufferType & mData = data[ nl ]; 
      // reset read and write position
      mData.clear();
        
      // write master data to fake buffer 
      dataHandle.sendData(mData,item);
    }

    // if data has been send, read data 
    if (hasdata != noData) 
    {
      // pack slave data to tmnp buffer 
      BufferType & slaveBuff = data[link]; 
      // reset read and write position
      slaveBuff.clear();

      int dataSize; 
      recvBuff.readObject(dataSize);
      // read dataSize bytes from recvBuff and write to slaveStream 
      recvBuff.readStream(slaveBuff, dataSize);
    }
  }

  delete a.first;
  delete a.second;

  return ;
}

template <class ObjectStreamType, class HItemType, class CommBuffMapType > 
void GitterDunePll :: sendMaster (
    ObjectStreamType & sendBuff, 
    CommBuffMapType& commBuffMap,
    HItemType * determType,
    GatherScatterType & dataHandle ,
    const int nl , 
    const int myLink )
{
  typedef SmallObjectStream BufferType;
  typedef vector< BufferType > DataBufferType;

  pair < IteratorSTI < HItemType > *, IteratorSTI < HItemType > *> 
    a = borderIteratorTT (determType , myLink ); //ueber alle meine Slave-Knoten
 
  IteratorSTI < HItemType > & iter = *(a.first);

  // create new link vector 
  vector< int > newLink( nl );
  for(int link=0; link<nl ; ++link) 
  {
    newLink[ link ] = link;
  }

  // if myLink == link then write master data
  // instead of data of link 
  // we do not send link i its own data
  newLink[myLink] = nl;

  // for all master items 
  for (iter.first (); ! iter.done () ; iter.next ()) 
  {
    HItemType & item = iter.item();

    // get comm buffer 
    //DataBufferType & dataBuff = item.commBuffer();
    DataBufferType & dataBuff = getCommunicationBuffer( item, commBuffMap, nl + 1);
    
    // scatter on master 
    if ( dataHandle.containsItem( item ) ) 
    {
      for(int link = 0; link<nl; ++link)
      {
        BufferType & localBuff = dataBuff[link];

        // check if stream has been read, if not scatter data 
        // this will unpack data on master only once 
        if( localBuff.validToRead() ) 
        {
          dataHandle.recvData(localBuff, item);
        }
      }
    } 
   
    // pack for slaves 
    {
      // write data marker 
      sendBuff.writeObject(transmittedData);

      for(int link = 0; link<nl; ++link)
      {
        // use new link to send master data to link we are sending for 
        BufferType & localBuff = dataBuff[ newLink[link] ];
        // get size 
        int s = localBuff.size();
        sendBuff.writeObject(s);
        // if buffer size > 0 write hole buffer to stream 
        if( s > 0 ) sendBuff.writeStream( localBuff );
      }
    } 
  }

  delete a.first;
  delete a.second;     

  return ;
}

template <class ObjectStreamType, class HItemType> 
void GitterDunePll :: unpackOnSlaves (
    ObjectStreamType & recvBuff, 
    HItemType * determType,
    GatherScatterType & dataHandle ,
    const int nOtherlinks, const int myLink)
{
  int hasdata;

  pair < IteratorSTI < HItemType > *, IteratorSTI < HItemType > *> 
    a = borderIteratorTT (determType, myLink );

  // get slave iterator 
  IteratorSTI < HItemType > & iter = *(a.second);
  
  for (iter.first (); ! iter.done () ; iter.next ()) 
  {
    // read data marker 
    recvBuff.readObject(hasdata);

    if (hasdata != noData) 
    {
      HItemType & item = iter.item();
      if( dataHandle.containsItem( item ) )
      {
        // for number of recived data, do scatter 
        for(int link = 0; link<nOtherlinks; ++link)
        {
          int s;
          recvBuff.readObject(s);
          if(s > 0) dataHandle.recvData(recvBuff, item );
        }
      }
      else 
      {
        // for number of recived data, do remove  
        for(int link = 0; link<nOtherlinks; ++link)
        {
          int s;
          recvBuff.readObject(s); 
          // if no data for link exists, s == 0
          // otherwise remove s bytes from stream by increasing 
          // read byte counter 
          if(s > 0) recvBuff.removeObject( s );
        }
      }
    }
  }
  delete a.first;
  delete a.second;
}

void GitterDunePll :: sendFaces (
    ObjectStream & sendBuff, 
    IteratorSTI < hface_STI > * iter , 
    GatherScatterType & faceData )
{
  // temporary object buffer  
  SmallObjectStream osTmp; 
  
  for (iter->first () ; ! iter->done () ; iter->next ()) 
  {
    hface_STI & face = iter->item();
    if ( faceData.containsItem( face ) ) 
    {
      sendBuff.writeObject(transmittedData);
      osTmp.clear();
      faceData.sendData(osTmp, face );

      int size = osTmp.size();
      // determin size of data to be able to remove 
      sendBuff.writeObject(size);
      if( size > 0 ) sendBuff.writeStream( osTmp );
    }
    else 
    {
      sendBuff.writeObject(noData);
    }
  }
}

void GitterDunePll :: unpackFaces (
    ObjectStream & recvBuff, 
    IteratorSTI < hface_STI > * iter , 
    GatherScatterType & faceData )
{
  int hasdata;
  for (iter->first () ; ! iter->done () ; iter->next ()) 
  {
    recvBuff.readObject(hasdata);
    if (hasdata != noData) 
    {
      hface_STI & face = iter->item();
      int size; 
      recvBuff.readObject(size); 
      if( size > 0 )
      {
        // if entity is not contained just remove data from stream 
        if ( faceData.containsItem( face ) ) 
          faceData.recvData(recvBuff , face );
        else 
          recvBuff.removeObject( size );
      }
    }
  }
}

////////////////////////////////////////////////////////
//
// communication of higher codim data (vertices,edges,faces)
//
////////////////////////////////////////////////////////
void GitterDunePll :: doBorderBorderComm( 
  vector< ObjectStream > & osvec ,
  GatherScatterType & vertexData , 
  GatherScatterType & edgeData,  
  GatherScatterType & faceData )
{
  const int nl = mpAccess ().nlinks ();
  
  const bool containsVertices = vertexData.contains(3,3);
  const bool containsEdges    = edgeData.contains(3,2);
  const bool containsFaces    = faceData.contains(3,1);

  const bool haveVerticesOrEdges = containsVertices || containsEdges;
   
  assert ((debugOption (5) && containsVertices) ? (cout << "**INFO GitterDunePll :: borderBorderComm (): (containsVertices)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsEdges)    ? (cout << "**INFO GitterDunePll :: borderBorderComm (): (containsEdges)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsFaces)    ? (cout << "**INFO GitterDunePll :: borderBorderComm (): (containsFaces)=true " << endl, 1) : 1) ;
   
  // buffers for vertex and edge master-slave communication
  map< vertex_STI*, DataBufferType > vertexCommMap;
  map< hedge_STI* , DataBufferType > edgeCommMap;

  {
    // gather all data from slaves 
    for (int link = 0; link < nl ; ++link )  
    {
      ObjectStream & sendBuff = osvec[link];
      sendBuff.clear();

      if (containsVertices)
      {
        vertex_STI * determType = 0;
        sendSlaves(sendBuff,determType,vertexData , link);
      }
      
      if (containsEdges) 
      {
        hedge_STI * determType = 0;
        sendSlaves(sendBuff,determType, edgeData , link);
      }
      
      if (containsFaces) 
      {
        hface_STI * determType = 0;
        pair < IteratorSTI < hface_STI > * , IteratorSTI < hface_STI > * >
          iterpair = borderIteratorTT(determType , link );
       
        // pack all faces that we are master on 
        sendFaces( sendBuff, iterpair.first  , faceData ); 
        // pack also all faces that we are not master on 
        sendFaces( sendBuff, iterpair.second , faceData ); 

        delete iterpair.first;
        delete iterpair.second;
      } 
    }
   
    /////////////////////////////////////////////////////
    // den anderen Partitionen die Slave-Daten senden
    /////////////////////////////////////////////////////
    osvec = mpAccess ().exchange (osvec);

    // now get all sended data and store on master item in local buffers
    for (int link = 0; link < nl; ++link) 
    { 
      ObjectStream & recvBuff = osvec[link];
      
      if (containsVertices) 
      {
        vertex_STI * determType = 0;
        unpackOnMaster(recvBuff,vertexCommMap,determType,vertexData,nl,link);
      }

      if (containsEdges) 
      {
        hedge_STI * determType = 0;
        unpackOnMaster(recvBuff,edgeCommMap,determType,edgeData,nl,link);
      }

      if (containsFaces) 
      {
        hface_STI * determType = 0;
        pair < IteratorSTI < hface_STI > * , IteratorSTI < hface_STI > * >
          iterpair = borderIteratorTT( determType , link );

        // first unpack slave data 
        unpackFaces(recvBuff,iterpair.second,faceData);
        // then unpack all master data 
        unpackFaces(recvBuff,iterpair.first ,faceData);

        delete iterpair.first;
        delete iterpair.second;
      }
    }
  }

  // now get all data from the local buffer of the master 
  // and send this data to the slaves (only for vertices and edges)
  if( haveVerticesOrEdges )
  {
    for (int link = 0; link < nl; ++link ) 
    {
      ObjectStream & sendBuff = osvec[link];
      sendBuff.clear();
      
      // write Number of my links 
      sendBuff.writeObject(nl); 

      if (containsVertices) 
      {
        vertex_STI * determType = 0;
        sendMaster(sendBuff,vertexCommMap,determType,vertexData,nl, link );
      }
      
      if (containsEdges) 
      {
        hedge_STI * determType = 0;
        sendMaster(sendBuff,edgeCommMap,determType,edgeData,nl, link );
      }
    }

    // clear buffers to save memory 
    vertexCommMap.clear();
    edgeCommMap.clear();
   
    ///////////////////////////////////////////////////
    // exchange all gathered data 
    ///////////////////////////////////////////////////
    osvec = mpAccess ().exchange (osvec);
   
    // now unpack all data on slave items 
    for (int link = 0; link < nl; ++link) 
    { 
      ObjectStream & recvBuff = osvec[link];
      
      int nOtherlinks;
      recvBuff.readObject(nOtherlinks); // read number of links 

      if (containsVertices) 
      {
        vertex_STI * determType = 0;
        unpackOnSlaves(recvBuff,determType,vertexData, nOtherlinks, link );
      }
      
      if (containsEdges) 
      {
        hedge_STI * determType = 0;
        unpackOnSlaves(recvBuff,determType, edgeData, nOtherlinks, link );
      }
    }

  } // end second loop over vertices and edges 

  return ;
}


// pack element data to stream 
void GitterDunePll :: sendInteriorGhostElementData (
    ObjectStream & sendBuff, 
    IteratorSTI < hface_STI > * iter , 
    GatherScatterType & elementData)
{
#ifndef NDEBUG
  const bool containsElements = elementData.contains(3,0);
  assert( containsElements );
#endif
  const int transmit = 1;
  for (iter->first () ; ! iter->done () ; iter->next ()) 
  {
    hface_STI & face = iter->item(); 
    
    // check ghost leaf 
    pair < ElementPllXIF_t *, int > inner = face.accessInnerPllX () ;

    if ( elementData.containsInterior(face, *(inner.first) ) ) 
    { 
      sendBuff.writeObject(transmit);

      // first interior elements are packed 
      inner.first->writeDynamicState (sendBuff , elementData) ;
    }     
    else 
    {
      sendBuff.writeObject(noData);
    }
  }

  return ;
}

// unpack all data from stream 
void GitterDunePll :: unpackInteriorGhostElementData (
    ObjectStream & recvBuff, 
    IteratorSTI < hface_STI > * iter , 
    GatherScatterType & elementData )
{
#ifndef NDEBUG
  const bool containsElements = elementData.contains(3,0);
  assert( containsElements );
#endif
  
  for (iter->first () ; ! iter->done () ; iter->next ()) 
  {
    int hasdata = 0;        
    recvBuff.readObject(hasdata);

    if( hasdata ) 
    {
      pair < ElementPllXIF_t *, int > p = iter->item ().accessOuterPllX () ;
      p.first->readDynamicState ( recvBuff , elementData);
    }
  }
  return ;
}

// pack all data to stream 
void GitterDunePll :: sendInteriorGhostAllData (
    ObjectStream & sendBuff, 
    IteratorSTI < hface_STI > * iter , 
    GatherScatterType & vertexData , 
    GatherScatterType & edgeData,  
    GatherScatterType & faceData, 
    GatherScatterType & elementData ,
    const bool packInterior ,
    const bool packGhosts )
{
  const bool containsVertices = vertexData.contains(3,3);
  const bool containsEdges    = edgeData.contains(3,2);
  const bool containsFaces    = faceData.contains(3,1);
  
  const bool haveHigherCodimData = containsVertices || 
    containsEdges ||  
    containsFaces ;

  const bool containsElements = elementData.contains(3,0);
  
  pair < ElementPllXIF_t *, int > bnd( ( ElementPllXIF_t * ) 0 , -1);

  // temporary object buffer  
  for (iter->first () ; ! iter->done () ; iter->next ()) 
  {
    hface_STI & face = iter->item(); 
    
    // check ghost leaf 
    pair < ElementPllXIF_t *, int > inner = face.accessInnerPllX () ;

    int interiorLeaf = 0;
    int ghostLeaf = 0;

    if(packInterior) 
    {
      interiorLeaf = (elementData.containsInterior(face, *(inner.first) )) ? 1 : 0;
    }

    if(packGhosts)
    {
      bnd = face.accessOuterPllX () ;
      ghostLeaf = (elementData.containsGhost(face , *(bnd.first))) ? 2 : 0;
    }

    const int transmit = interiorLeaf + ghostLeaf ;
    // transmit = 1 interior, transmit = 2 ghost, transmit = 3 both 
    // if at least one of this possibilities is true then send data
    if ( transmit > 0 ) 
    { 
      sendBuff.writeObject(transmit);

      // first interior elements are packed 
      if( interiorLeaf  > 0 )
      {
        if( haveHigherCodimData )
        {
          if (containsVertices) 
            inner.first->VertexData2os(sendBuff, vertexData, inner.second );
          if (containsEdges)    
            inner.first->EdgeData2os  (sendBuff, edgeData  , inner.second );
          if (containsFaces)    
            inner.first->FaceData2os  (sendBuff, faceData  , inner.second );
        }

        if (containsElements) 
          inner.first->writeDynamicState (sendBuff , elementData) ;
      }

      // then ghost elements 
      if( ghostLeaf > 0 ) 
      {
        if( haveHigherCodimData )
        {
          // get pair < ghost, local face num > 
          Gitter :: ghostpair_STI gpair = bnd.first->getGhost();
          assert( gpair.first );

          if (containsVertices) 
            gpair.first->VertexData2os( sendBuff , vertexData, gpair.second );
          if (containsEdges)    
            gpair.first->EdgeData2os  ( sendBuff , edgeData, gpair.second );
          if (containsFaces)    
            gpair.first->FaceData2os  ( sendBuff , faceData, gpair.second );
        }
        
        if( containsElements ) 
        {
          assert( bnd.first );
          bnd.first->writeDynamicState(sendBuff, elementData );
        }

        // reset bnd pointer 
        bnd.first = 0;
      }
    }     
    else 
    {
      sendBuff.writeObject(noData);
    }
  }

  return ;
}

// unpack all data from stream 
void GitterDunePll :: unpackInteriorGhostAllData (
    ObjectStream & recvBuff, 
    IteratorSTI < hface_STI > * iter , 
    GatherScatterType & vertexData , 
    GatherScatterType & edgeData,  
    GatherScatterType & faceData, 
    GatherScatterType & elementData )
{
  const bool containsVertices = vertexData.contains(3,3);
  const bool containsEdges    = edgeData.contains(3,2);
  const bool containsFaces    = faceData.contains(3,1);
  const bool containsElements = elementData.contains(3,0);
  

  const bool haveHigherCodimData = containsVertices || 
                                   containsEdges ||  
                                   containsFaces ;

  
  for (iter->first () ; ! iter->done () ; iter->next ()) 
  {
    int hasdata;        
    recvBuff.readObject(hasdata);

    if(hasdata != noData)
    {
      // interiorLeaf is true, if on other side ghostLeaf has been packed
      const bool interiorLeaf = (hasdata > 1);

      // ghostLeaf is true if on other side interior has been packed
      const bool ghostLeaf    = (hasdata == 1) || (hasdata == 3);

      // get face 
      hface_STI & face = iter->item ();
      // first unpack ghosts 
      if( ghostLeaf ) 
      {
        pair < ElementPllXIF_t *, int > p = face.accessOuterPllX () ;

        // get pair < ghost, local face num > 
        Gitter :: ghostpair_STI gpair = p.first->getGhost();
        assert( gpair.first );
      
        if( haveHigherCodimData )
        {
          if (containsVertices) 
            gpair.first->os2VertexData( recvBuff , vertexData, gpair.second );
          if (containsEdges)    
            gpair.first->os2EdgeData  ( recvBuff , edgeData, gpair.second );
          if (containsFaces)    
            gpair.first->os2FaceData  ( recvBuff , faceData, gpair.second );
        }

        if (containsElements) 
          p.first->readDynamicState ( recvBuff , elementData);
      }

      // then unpack interior 
      if( interiorLeaf )
      {
        pair < ElementPllXIF_t *, int > pll = face.accessInnerPllX () ;
        pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > 
          p ( (Gitter::helement_STI *) 0, (Gitter::hbndseg_STI *) 0);

        pll.first->getAttachedElement( p );
        assert( p.first );

        if( haveHigherCodimData )
        {
          if (containsVertices) 
            p.first->os2VertexData( recvBuff , vertexData, pll.second );
          if (containsEdges)    
            p.first->os2EdgeData( recvBuff , edgeData, pll.second );
          if (containsFaces)    
            p.first->os2FaceData( recvBuff , faceData, pll.second );
        }

        if (containsElements) 
          elementData.recvData( recvBuff , *(p.first) );
      }
    }
  }
  return ;
}


/////////////////////////////////////////////////////
//
//  interior to ghost communication 
//
/////////////////////////////////////////////////////

void GitterDunePll :: doInteriorGhostComm( 
  vector< ObjectStream > & osvec ,
  GatherScatterType & vertexData , 
  GatherScatterType & edgeData,  
  GatherScatterType & faceData, 
  GatherScatterType & elementData ,
  const CommunicationType commType )
{
  const int nl = mpAccess ().nlinks ();
  
  const bool containsVertices = vertexData.contains(3,3);
  const bool containsEdges    = edgeData.contains(3,2);
  const bool containsFaces    = faceData.contains(3,1);
  const bool containsElements = elementData.contains(3,0);

  const bool haveHigherCodimData = containsVertices || 
                                   containsEdges ||  
                                   containsFaces ;

  const bool containsSomeThing = haveHigherCodimData || containsElements ;

  const bool packInterior = (commType == All_All_Comm) || 
                            (commType == Interior_Ghost_Comm);
  
  const bool packGhosts   = (commType == All_All_Comm) || 
                            (commType == Ghost_Interior_Comm);

  if(!containsSomeThing) 
  {
    cerr << "WARNING: communication called with empty data set, all contains methods returned false! \n";
    return ;
  }
   
  {
    for (int link = 0 ; link < nl ; ++link ) 
    {   
      ObjectStream & sendBuff = osvec[link]; 
      sendBuff.clear();
      
      {
        const hface_STI * determType = 0; // only for type determination 
        pair < IteratorSTI < hface_STI > * , IteratorSTI < hface_STI > * > 
          iterpair = borderIteratorTT( determType , link );

        if(haveHigherCodimData || packGhosts )
        {
          // write all data belong to interior of master faces 
          sendInteriorGhostAllData( sendBuff, iterpair.first , 
                            vertexData, edgeData,
                            faceData, elementData, 
                            packInterior , packGhosts );
        
          // write all data belong to interior of slave faces 
          sendInteriorGhostAllData( sendBuff, iterpair.second , 
                            vertexData, edgeData,
                            faceData, elementData ,
                            packInterior , packGhosts );
        }
        else 
        {
          // write all data belong to interior of master faces 
          sendInteriorGhostElementData( sendBuff, iterpair.first, elementData);
        
          // write all data belong to interior of slave faces 
          sendInteriorGhostElementData( sendBuff, iterpair.second, elementData);
        }

        delete iterpair.first; 
        delete iterpair.second; 
      }
    }

    ///////////////////////////////////////////
    // exchange data 
    ///////////////////////////////////////////
    osvec = mpAccess ().exchange (osvec) ;     
    
    //all ghost cells get new data
    for (int link = 0 ; link < nl ; ++link ) 
    {  
      ObjectStream & recvBuff = osvec[link];

      {
        const hface_STI * determType = 0; // only for type determination 
        pair < IteratorSTI < hface_STI > * , IteratorSTI < hface_STI > * > 
          iterpair = borderIteratorTT( determType , link );

        if(haveHigherCodimData || packGhosts )
        {
          // first unpack slave data, because this has been pack from master
          // first , see above 
          unpackInteriorGhostAllData( recvBuff, iterpair.second , 
                              vertexData, edgeData,
                              faceData, elementData );
         
          // now unpack data sended from slaves to master 
          unpackInteriorGhostAllData( recvBuff, iterpair.first , 
                              vertexData, edgeData,
                              faceData, elementData );
        }
        else 
        {
          // first unpack slave data, because this has been pack from master
          // first , see above 
          unpackInteriorGhostElementData( recvBuff, iterpair.second, elementData );
         
          // now unpack data sended from slaves to master 
          unpackInteriorGhostElementData( recvBuff, iterpair.first, elementData );
        }

        delete iterpair.first;
        delete iterpair.second;
      }
    }
  }

  // end element communication 
  return ;
} 
////////////////////////////////////////////////////////
//
// communicate data
// 
////////////////////////////////////////////////////////
void GitterDunePll :: doCommunication ( 
             GatherScatterType & vertexData , 
             GatherScatterType & edgeData,  
             GatherScatterType & faceData ,
             GatherScatterType & elementData, 
             const CommunicationType commType ) 
{
  const int nl = mpAccess ().nlinks ();

  const bool containsVertices = vertexData.contains(3,3);
  const bool containsEdges    = edgeData.contains(3,2);
  const bool containsFaces    = faceData.contains(3,1);
  const bool containsElements = elementData.contains(3,0);

  const bool haveHigherCodimData = containsVertices || 
    containsEdges ||  
    containsFaces ;

  const bool containsSomeThing = containsElements || haveHigherCodimData;

  if(!containsSomeThing) 
  {
    cerr << "WARNING: communication called with empty data set, all contains methods returned false! \n";
    return ;
  }
   
  assert ((debugOption (5) && containsVertices) ? (cout << "**INFO GitterDunePll :: doCommunication (): (containsVertices)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsEdges)    ? (cout << "**INFO GitterDunePll :: doCommunication (): (containsEdges)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsFaces)    ? (cout << "**INFO GitterDunePll :: doCommunication (): (containsFaces)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsElements) ? (cout << "**INFO GitterDunePll :: doCommunication (): (containsElements)=true " << endl, 1) : 1) ;
   
  // create vector of message buffers 
  // this vector is created here, that the buffer is allocated only once 
  vector < ObjectStream > vec (nl) ;
 
  // if data on entities of higer codim exists
  // then communication if more complicated 
  if ( haveHigherCodimData )
  {
    doBorderBorderComm( vec, vertexData, edgeData, faceData );
  }

  if( commType != Border_Border_Comm ) // otherwise only border border 
  {
    doInteriorGhostComm( vec, vertexData, edgeData, faceData, elementData , commType ); 
  }

  return ;
}

// border border comm 
void GitterDunePll :: borderBorderCommunication ( 
             GatherScatterType & vertexData , 
             GatherScatterType & edgeData,  
             GatherScatterType & faceData ,
             GatherScatterType & elementData )
{
  doCommunication(vertexData,edgeData,faceData,elementData,Border_Border_Comm);
}

// interior ghost comm 
void GitterDunePll :: interiorGhostCommunication ( 
             GatherScatterType & vertexData , 
             GatherScatterType & edgeData,  
             GatherScatterType & faceData ,
             GatherScatterType & elementData )
{
  doCommunication(vertexData,edgeData,faceData,elementData,Interior_Ghost_Comm);
}

// ghost to interior comm 
void GitterDunePll :: ghostInteriorCommunication ( 
             GatherScatterType & vertexData , 
             GatherScatterType & edgeData,  
             GatherScatterType & faceData ,
             GatherScatterType & elementData )
{
  doCommunication(vertexData,edgeData,faceData,elementData,Ghost_Interior_Comm);
}

// all all comm 
void GitterDunePll :: allAllCommunication ( 
             GatherScatterType & vertexData , 
             GatherScatterType & edgeData,  
             GatherScatterType & faceData ,
             GatherScatterType & elementData )
{
  doCommunication(vertexData,edgeData,faceData,elementData,All_All_Comm);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool GitterDunePll :: refine () 
{
  return GitterPll :: refine () ;
}

void GitterDunePll :: coarse () {
  GitterPll :: coarse () ;
}

// rebuild ghost cells 
void GitterDunePll :: rebuildGhostCells() 
{
  const int nl = mpAccess ().nlinks () ;

  try 
  {
    vector < ObjectStream > osv (nl) ;
    
    const hface_STI* determType = 0;

    // pack all elements neighbouring to internal boundary 
    // as ghost elements 
    {
      for (int link = 0 ; link < nl ; ++link ) 
      {
        pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > *> 
              w = levelBorderIteratorTT (determType, link, 0 );
        
        ObjectStream & os = osv[link];
        
        {
          IteratorSTI < hface_STI > & inner = *w.first;
        
          for ( inner.first () ; ! inner.done () ; inner.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = inner.item ().accessInnerPllX () ;
            p.first->packAsGhost(os, p.second) ;
          }
        }

        {
          IteratorSTI < hface_STI > & outer = *w.second;
          for (outer.first () ; ! outer.done () ; outer.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = outer.item ().accessInnerPllX () ;
            p.first->packAsGhost(os, p.second) ;
          }
        }

        delete w.first;
        delete w.second;
      } 
    }
    
    // exchange gathered data 
    osv = mpAccess ().exchange (osv) ;
    
    // unpack all data on internal boundary and create 
    // ghost cells 
    {
      for (int link = 0 ; link < nl ; ++link ) 
      {
        pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > *> 
              w = levelBorderIteratorTT (determType, link, 0 );
        
        ObjectStream & os = osv[link];

        {
          IteratorSTI < hface_STI > & outer = *w.second;
          for (outer.first () ; ! outer.done () ; outer.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = outer.item ().accessOuterPllX () ;
            p.first->insertGhostCell(os, p.second) ;
          }
        }
        {
          IteratorSTI < hface_STI > & inner = *w.first;
          for (inner.first () ; ! inner.done () ; inner.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = inner.item ().accessOuterPllX () ;
            p.first->insertGhostCell(os, p.second) ;
          }
        }

        delete w.first;
        delete w.second;
      } 
    }
  } 
  catch (Parallel ::  AccessPllException) 
  {
    cerr << "  FEHLER Parallel :: AccessPllException entstanden in: " << __FILE__ << " " << __LINE__ << endl ;
  }

  return ;
}

void GitterDunePll :: checkGhostIndices() 
{
  // get number of links 
  const int nl = mpAccess ().nlinks () ;
  
  const hface_STI* determType = 0;
  {
    // for all links check all ghost elements 
    for (int link = 0 ; link < nl ; ++link ) 
    {
      pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > *> 
            w = levelBorderIteratorTT (determType, link , 0);
      
      {
        IteratorSTI < hface_STI > & outer = *w.second;
        for (outer.first () ; ! outer.done () ; outer.next ()) 
        {
          pair < ElementPllXIF_t *, int > p = outer.item ().accessOuterPllX () ;

          // get pair < ghost, local face num > 
          Gitter :: ghostpair_STI gpair = p.first->getGhost();
          assert( gpair.first );

          gpair.first->resetGhostIndices();
        }
      }

      {
        IteratorSTI < hface_STI > & inner = *w.first;
        for (inner.first () ; ! inner.done () ; inner.next ()) 
        {
          pair < ElementPllXIF_t *, int > p = inner.item ().accessOuterPllX () ;

          // get pair < ghost, local face num > 
          Gitter :: ghostpair_STI gpair = p.first->getGhost();
          assert( gpair.first );

          gpair.first->resetGhostIndices();
        }
      }

      // free interators 
      delete w.first;
      delete w.second;
    } 
  } 

  return ;
}

void GitterDunePll :: duneBackup(const char *filename) 
{
  // backup grid, same as in serial case 
  GitterDuneBasis::duneBackup(filename);
}

// wird von Dune verwendet 
void GitterDunePll ::restore (istream & in) 
{
  typedef Gitter :: Geometric :: BuilderIF BuilderIF;
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: restore (istream & = " << in << ") " << endl, 1) : 1) ;
  {
    AccessIterator < hedge_STI > :: Handle ew (container ());
    for (ew.first () ; !ew.done () ; ew.next ()) ew.item ().restore (in) ;
  }
  {
    AccessIterator < hface_STI >:: Handle fw(container());
    for ( fw.first(); !fw.done (); fw.next()) fw.item().restore (in);
  }
  {
    AccessIterator < helement_STI >:: Handle ew(container());
    for ( ew.first(); !ew.done(); ew.next()) ew.item().restore (in);
  }

  // restore indices before ghosts are created 
  // otherwise indices of ghost will be wrong 
  this->restoreIndices (in);
 
#ifndef NDEBUG 
  const int maxIndexBefore = this->indexManager(BuilderIF :: IM_Elements).getMaxIndex();
#endif

  // set ghost indices new for level 0 ghosts 
  checkGhostIndices ();

  // now restore faces and by this ghosts 
  // will be refined 
  {
    AccessIterator < hbndseg_STI > :: Handle bw (container ()) ;
    for (bw.first () ; ! bw.done () ; bw.next ()) bw.item ().restoreFollowFace () ;
  }
  
  // max index should not be largen than before
  assert( (this->indexManager(BuilderIF :: IM_Elements).getMaxIndex() != maxIndexBefore) ?
      (cout << maxIndexBefore << " vor | nach " << this->indexManager(BuilderIF :: IM_Elements).getMaxIndex() << "\n",0) : 1);

  duneNotifyGridChanges () ;
  return ;
}

void GitterDunePll :: duneRestore(const char *fileName) 
{
  assert (debugOption (20) ? 
      (cout << "**INFO GitterDuneBasis :: duneRestore (const char * = \""
            << fileName << "\") " << endl, 1) : 1) ;
                 
  ifstream in (fileName) ;
  if (!in) {
    cerr << "**WARNUNG (IGNORIERT) GitterDunePll :: ";
    cerr <<" duneRestore (const char *, double & ) Fehler beim \"Offnen von < "
         << (fileName ? fileName : "null") << " > " << endl ;
  } 
  else 
  {
    restore(in) ;
  }

  return ;
}

#endif
