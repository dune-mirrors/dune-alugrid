#ifndef GITTER_DUNE_PLL_IMPL_CC_INCLUDED
#define GITTER_DUNE_PLL_IMPL_CC_INCLUDED

#include "gitter_dune_pll_impl.h"
#include "gitter_dune_pll_mgb.cc"

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
  assert (debugOption (20) ? (cout << "**GitterDunePll :: duneNotifyNewGrid () " << endl, 1) : 1) ;
  const int np = mpAccess ().psize () ;
  LoadBalancer :: DataBase db ;
  {
    AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphEdge (db) ;
  }
  {
    AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphVertex (db) ;
  }
  bool neu = false ;
  {
    // Kriterium, wann eine Lastneuverteilung vorzunehmen ist:
    // 
    // load  - eigene ElementLast
    // mean  - mittlere ElementLast
    // nload - Lastverh"altnis

    double load = db.accVertexLoad () ;
    vector < double > v (mpAccess ().gcollect (load)) ;
    double mean = accumulate (v.begin (), v.end (), 0.0) / double (np) ;

    for (vector < double > :: iterator i = v.begin () ; i != v.end () ; i ++)
      neu |= (*i > mean ? (*i > (_ldbOver * mean) ? true : false) : (*i < (_ldbUnder * mean) ? true : false)) ;
  }
  return neu;
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
  bool refined = this->refine() ;
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
  const int np = mpAccess ().psize () ;
  LoadBalancer :: DataBase db ;
  {
    AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphEdge (db) ;
  }
  {
    AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphVertex (db) ;
  }
  bool neu = false ;
  {
    // Kriterium, wann eine Lastneuverteilung vorzunehmen ist:
    // 
    // load  - eigene ElementLast
    // mean  - mittlere ElementLast
    // nload - Lastverh"altnis
  
    double load = db.accVertexLoad () ;
    vector < double > v (mpAccess ().gcollect (load)) ;
    double mean = accumulate (v.begin (), v.end (), 0.0) / double (np) ;

    for (vector < double > :: iterator i = v.begin () ; i != v.end () ; i ++)
      neu |= (*i > mean ? (*i > (_ldbOver * mean) ? true : false) : (*i < (_ldbUnder * mean) ? true : false));
  }

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
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
          }
      
          OuterIteratorType wo (mof);
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessInnerPllX () ;
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
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
          }
      
          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessOuterPllX () ;
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

template <class ObjectStreamType, class HItemType> 
void GitterDunePll :: unpackOnMaster (
    ObjectStreamType & recvBuff, 
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
    
    item.reserveBuffer( nl + 1 );
    DataBufferType & data = item.commBuffer();

    // only gather master data once 
    if ( dataHandle.containsItem( item ) ) 
    {
      // pack master data 
      BufferType & mData = data[nl]; 
      mData.clear();
        
      // write master data to fake buffer 
      dataHandle.sendData(mData,item);
    }

    // if data has been send, read data 
    if (hasdata != noData) 
    {
      // pack slave data to tmnp buffer 
      BufferType & v = data[link]; 
      v.clear();

      int dataSize; 
      recvBuff.readObject(dataSize);
      recvBuff.readStream(v,dataSize);
    }
  }
  delete a.first;
  delete a.second;

  return ;
}

template <class ObjectStreamType, class HItemType> 
void GitterDunePll :: sendMaster (
    ObjectStreamType & sendBuff, 
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
    DataBufferType & dataBuff = item.commBuffer();
    
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
        unpackOnMaster(recvBuff,determType,vertexData,nl,link);
      }

      if (containsEdges) 
      {
        hedge_STI * determType = 0;
        unpackOnMaster(recvBuff,determType,edgeData,nl,link);
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
        sendMaster(sendBuff,determType,vertexData,nl, link );
      }
      
      if (containsEdges) 
      {
        hedge_STI * determType = 0;
        sendMaster(sendBuff,determType,edgeData,nl, link );
      }
    }
   
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
    pair < ElementPllXIF_t *, int > inner = face.accessPllX ().accessInnerPllX () ;

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
      pair < ElementPllXIF_t *, int > p = iter->item ().accessPllX ().accessOuterPllX () ;
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
    pair < ElementPllXIF_t *, int > inner = face.accessPllX ().accessInnerPllX () ;

    int interiorLeaf = 0;
    int ghostLeaf = 0;

    if(packInterior) 
    {
      interiorLeaf = (elementData.containsInterior(face, *(inner.first) )) ? 1 : 0;
    }

    if(packGhosts)
    {
      bnd = face.accessPllX ().accessOuterPllX () ;
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
        pair < ElementPllXIF_t *, int > p = face.accessPllX ().accessOuterPllX () ;

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
        pair < ElementPllXIF_t *, int > pll = face.accessPllX ().accessInnerPllX () ;
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

bool GitterDunePll :: refine () {
  assert (debugOption (5) ? (cout << "**INFO GitterDunePll :: refine () " << endl, 1) : 1) ;
  const int nl = mpAccess ().nlinks () ;
  bool state = false ;
  vector < vector < hedge_STI * > > innerEdges (nl), outerEdges (nl) ;
  vector < vector < hface_STI * > > innerFaces (nl), outerFaces (nl) ;
  {
    // Erst die Zeiger auf alle Fl"achen und Kanten mit paralleler
    // Mehrdeutigkeit sichern, da die LeafIteratorTT < . > nach dem 
    // Verfeinern auf gitter nicht mehr stimmen werden. Die Technik
    // ist zul"assig, da keine mehrfache Verfeinerung entstehen kann.
  
    {
      for (int l = 0 ; l < nl ; l ++) 
      {
        //cout << "refinepll \n";
        LeafIteratorTT < hface_STI > fw (*this,l) ;
        LeafIteratorTT < hedge_STI > dw (*this,l) ;

        // reserve memory first 
        outerFaces[l].reserve(fw.outer().size());
        innerFaces[l].reserve(fw.inner().size());
        
        for (fw.outer ().first () ; ! fw.outer().done () ; fw.outer ().next ())
          outerFaces [l].push_back (& fw.outer ().item ()) ;
        for (fw.inner ().first () ; ! fw.inner ().done () ; fw.inner ().next ())
          innerFaces [l].push_back (& fw.inner ().item ()) ;

        // reserve memory first 
        outerEdges[l].reserve(dw.outer().size());
        innerEdges[l].reserve(dw.inner().size());
        
        for (dw.outer ().first () ; ! dw.outer().done () ; dw.outer ().next ())
          outerEdges [l].push_back (& dw.outer ().item ()) ;
        for (dw.inner ().first () ; ! dw.inner ().done () ; dw.inner ().next ())
          innerEdges [l].push_back (& dw.inner ().item ()) ;
      }
    }
    // jetzt normal verfeinern und den Status der Verfeinerung
    // [unvollst"andige / vollst"andige Verfeinerung] sichern.
    
    __STATIC_phase = 1 ;
    
    state = Gitter :: refine () ;
       
    // Phase des Fl"achenausgleichs an den Schnittfl"achen des
    // verteilten Gitters. Weil dort im sequentiellen Fall pseudorekursive
    // Methodenaufrufe vorliegen k"onnen, muss solange iteriert werden,
    // bis die Situation global station"ar ist.
  
    __STATIC_phase = 2 ;
  
    bool repeat (false) ;
    _refineLoops = 0 ;
    do {
      repeat = false ;
      {
        vector < ObjectStream > osv (nl) ;
        try {
          for (int l = 0 ; l < nl ; l ++) 
          {
            {
              vector < hface_STI * > :: const_iterator iEnd = outerFaces[l].end () ;
              for (vector < hface_STI * > :: const_iterator i = outerFaces [l].begin () ;
                   i != iEnd; (*i ++)->accessPllX ().accessOuterPllX ().first->getRefinementRequest (osv [l])) ; 
            }
            {
              vector < hface_STI * > :: const_iterator iEnd = innerFaces[l].end () ;
              for (vector < hface_STI * > :: const_iterator i = innerFaces [l].begin () ;
                   i != iEnd; (*i ++)->accessPllX ().accessOuterPllX ().first->getRefinementRequest (osv [l])) ; 
            }
          }
        } 
        catch (Parallel :: AccessPllException) 
        {
          cerr << "**FEHLER (FATAL) AccessPllException in " << __FILE__ << " " << __LINE__ << endl ; abort () ;
        }
  
        // exchange data 
        osv = mpAccess ().exchange (osv) ;
  
        try 
        {
          for (int l = 0 ; l < nl ; l ++) 
          {
            {
              vector < hface_STI * > :: const_iterator iEnd = innerFaces[l].end () ;
              for (vector < hface_STI * > :: const_iterator i = innerFaces [l].begin () ;
                i != iEnd; repeat |= (*i ++)->accessPllX ().accessOuterPllX ().first->setRefinementRequest (osv [l])) ; 
            }
            {
              vector < hface_STI * > :: const_iterator iEnd = outerFaces[l].end () ; 
              for (vector < hface_STI * > :: const_iterator i = outerFaces [l].begin () ;
                i != iEnd; repeat |= (*i ++)->accessPllX ().accessOuterPllX ().first->setRefinementRequest (osv [l])) ; 
            }
          }
        } 
        catch (Parallel :: AccessPllException) 
        {
          cerr << "**FEHLER (FATAL) AccessPllException in " << __FILE__ << " " << __LINE__ << endl ; abort () ;
        }
      }

      _refineLoops ++ ;
    } while (mpAccess ().gmax (repeat ? 1 : 0)) ;

    // Jetzt noch die Kantensituation richtigstellen, es gen"ugt ein Durchlauf,
    // weil die Verfeinerung einer Kante keine Fernwirkungen hat. Vorsicht: Die
    // Kanten sind bez"uglich ihrer Identifikation sternf"ormig organisiert, d.h.
    // es muss die Verfeinerungsinformation einmal am Eigent"umer gesammelt und
    // dann wieder zur"ucktransportiert werden, eine einfache L"osung, wie bei
    // den Fl"achen (1/1 Beziehung) scheidet aus.

    __STATIC_phase = 3 ;

    {
      vector < ObjectStream > osv (nl) ;
      {
        for (int l = 0 ; l < nl ; l ++) 
        {
          vector < hedge_STI * > :: const_iterator iEnd = outerEdges[l].end () ;
          for (vector < hedge_STI * > :: const_iterator i = outerEdges [l].begin () ;
            i != iEnd; (*i ++)->accessPllX ().getRefinementRequest (osv [l])) ;
        }
      }
      
      // exchange data 
      osv = mpAccess ().exchange (osv) ;
      
      {
        for (int l = 0 ; l < nl ; l ++)
        {
          vector < hedge_STI * > :: const_iterator iEnd = innerEdges[l].end () ;
          for (vector < hedge_STI * > :: const_iterator i = innerEdges [l].begin () ;
            i != iEnd; (*i ++)->accessPllX ().setRefinementRequest (osv [l])) ;
        }
      }
    } // ~vector < ObjectStream > ... 
     
    {
      vector < ObjectStream > osv (nl) ;
      {
        for (int l = 0 ; l < nl ; l ++)
        {
          vector < hedge_STI * > :: const_iterator iEnd = innerEdges[l].end () ;
          for (vector < hedge_STI * > :: const_iterator i = innerEdges [l].begin () ;
            i != iEnd; (*i ++)->accessPllX ().getRefinementRequest (osv [l])) ;
        }
      }
      
      // exchange data 
      osv = mpAccess ().exchange (osv) ;
      
      {
        for (int l = 0 ; l < nl ; l ++)
        {
          vector < hedge_STI * > :: const_iterator iEnd = outerEdges [l].end () ;
          for (vector < hedge_STI * > :: const_iterator i = outerEdges [l].begin () ;
            i != iEnd; (*i ++)->accessPllX ().setRefinementRequest (osv [l])) ;
        }
      }
    }   // ~vector < ObjectStream > ... 
  }
  
  __STATIC_phase = -1 ;
  
  return state ;
}

void GitterDunePll :: coarse () {
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: coarse () " << endl, 1) : 1) ;
  const int nl = mpAccess ().nlinks () ;

  {
    vector < vector < hedge_STI * > > innerEdges (nl), outerEdges (nl) ;
    vector < vector < hface_STI * > > innerFaces (nl), outerFaces (nl) ;
  
    for (int l = 0 ; l < nl ; l ++) {
    
      // Zun"achst werden f"ur alle Links die Zeiger auf Gitterojekte mit
      // Mehrdeutigkeit gesichert, die an der Wurzel einer potentiellen
      // Vergr"oberungsoperation sitzen -> es sind die Knoten in der Hierarchie,
      // deren Kinder alle Bl"atter sind. Genau diese Knoten sollen gegen"uber
      // der Vergr"oberung blockiert werden und dann die Vergr"oberung falls
      // sie zul"assig ist, sp"ater durchgef"uhrt werden (pending) ;
    
      AccessIteratorTT < hface_STI > :: InnerHandle mfwi (containerPll (),l) ;
      AccessIteratorTT < hface_STI > :: OuterHandle mfwo (containerPll (),l) ;
      AccessIteratorTT < hedge_STI > :: InnerHandle mdwi (containerPll (),l) ;
      AccessIteratorTT < hedge_STI > :: OuterHandle mdwo (containerPll (),l) ;
      
      // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
      // Fl"achen "uber den Grobgitterfl"achen. In den Elementen passiert erstmal
      // nichts, solange nicht mit mehrfachen Grobgitterelementen gearbeitet wird.
      
      Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, childs_are_leafs < hface_STI > > > fwi (mfwi) ;
      Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, childs_are_leafs < hface_STI > > > fwo (mfwo) ;
      
      // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
      // Kanten "uber den Grobgitterkanten.
      
      Insert < AccessIteratorTT < hedge_STI > :: InnerHandle, 
        TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dwi (mdwi) ;
      Insert < AccessIteratorTT < hedge_STI > :: OuterHandle, 
        TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dwo (mdwo) ;

      // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
      // Kanten "uber den Grobgitterfl"achen. Diese Konstruktion wird beim Tetraeder-
      // gitter notwendig, weil dort keine Aussage der Form:
      //

      Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > > efi (mfwi) ;
      Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > > efo (mfwo) ;
      Wrapper < Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > eifi (efi) ;
      Wrapper < Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > eifo (efo) ;
      Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
      TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dfi (eifi) ;
        Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
          TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
      TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dfo (eifo) ;

      // Die 'item ()' Resultatwerte (Zeiger) werden in Vektoren gesichert, weil die
      // Kriterien die zur Erzeugung der Iteratoren angewendet wurden (Filter) nach
      // einer teilweisen Vergr"oberung nicht mehr g"ultig sein werden, d.h. die 
      // Iterationsobjekte "andern w"ahrend der Vergr"oberung ihre Eigenschaften.
      // Deshalb werden sie auch am Ende des Blocks aufgegeben. Der Vektor 'cache'
      // ist zul"assig, weil kein Objekt auf das eine Referenz im 'cache' vorliegt
      // beseitigt werden kann. Sie sind alle ein Niveau darunter.

      // reserve memory first 
      innerEdges[l].reserve(fwi.size() + dwi.size() + dfi.size());
      outerEdges[l].reserve(fwo.size() + dwo.size() + dfo.size());
        
      for (fwi.first () ; ! fwi.done () ; fwi.next ()) innerFaces [l].push_back (& fwi.item ()) ;
      for (fwo.first () ; ! fwo.done () ; fwo.next ()) outerFaces [l].push_back (& fwo.item ()) ;
      for (dwo.first () ; ! dwo.done () ; dwo.next ()) outerEdges [l].push_back (& dwo.item ()) ;
      for (dfo.first () ; ! dfo.done () ; dfo.next ()) outerEdges [l].push_back (& dfo.item ()) ;
      for (dwi.first () ; ! dwi.done () ; dwi.next ()) innerEdges [l].push_back (& dwi.item ()) ;
      for (dfi.first () ; ! dfi.done () ; dfi.next ()) innerEdges [l].push_back (& dfi.item ()) ;
    }

    try 
    {
      // Erstmal alles was mehrdeutig ist, gegen die drohende Vergr"oberung sichern.
      // Danach werden sukzessive die Fl"achenlocks aufgehoben, getestet und
      // eventuell vergr"obert, dann das gleiche Spiel mit den Kanten.

      for (int l = 0 ; l < nl ; l ++) 
      {
        {
          vector < hedge_STI * > :: iterator iEnd = outerEdges [l].end () ;
          for (vector < hedge_STI * > :: iterator i = outerEdges [l].begin () ;
               i != iEnd; (*i ++)->accessPllX ().lockAndTry ()) ; 
        }
        {
          vector < hedge_STI * > :: iterator iEnd = innerEdges [l].end () ;
          for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ;
               i != iEnd; (*i ++)->accessPllX ().lockAndTry ()) ; 
        }
        {
          vector < hface_STI * > :: iterator iEnd = outerFaces [l].end () ;
          for (vector < hface_STI * > :: iterator i = outerFaces [l].begin () ;
               i != iEnd; (*i ++)->accessPllX ().accessOuterPllX ().first->lockAndTry ()) ; 
        }
        {
          vector < hface_STI * > :: iterator iEnd = innerFaces [l].end () ;
          for (vector < hface_STI * > :: iterator i = innerFaces [l].begin () ;
               i != iEnd; (*i ++)->accessPllX ().accessOuterPllX ().first->lockAndTry ()) ; 
        }
      }
      
      // Gitter :: coarse () ist elementorientiert, d.h. die Vergr"oberung auf Fl"achen und
      // Kanten wird nur durch Vermittlung eines sich vergr"obernden Knotens in der Element-
      // hierarchie angestossen. In allen gegen Vergr"oberung 'gelockten' Fl"achen und Kanten
      // wird die angeforderte Operation zur"uckgewiesen, um erst sp"ater von aussen nochmals
      // angestossen zu werden.
      
      __STATIC_phase = 4 ;
      
      Gitter :: coarse () ;
      
    } 
    catch (Parallel :: AccessPllException) 
    {
      cerr << "**FEHLER (FATAL) AccessPllException beim Vergr\"obern der Elementhierarchie oder\n" ;
      cerr << "  beim locken der Fl\"achen- bzw. Kantenb\"aume aufgetreten. In " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    
    try {
    
      // Phase des Fl"achenausgleichs des verteilten Vergr"oberungsalgorithmus
      // alle Schnittfl"achenpaare werden daraufhin untersucht, ob eine
      // Vergr"oberung in beiden Teilgittern durchgef"uhrt werden darf,
      // wenn ja, wird in beiden Teilgittern vergr"obert und der Vollzug
      // getestet.
  
      __STATIC_phase = 5 ;
    
      vector < vector < int > > clean (nl) ;
      {
        vector < vector < int > > inout (nl) ;
        {
          for (int l = 0 ; l < nl ; l ++)
          {
            // reserve memory first 
            inout[l].reserve( outerFaces [l].size() );
            // get end iterator 
            vector < hface_STI * > :: iterator iEnd = outerFaces [l].end () ;
            for (vector < hface_STI * > :: iterator i = outerFaces [l].begin () ; 
                 i != iEnd; i ++)
            {
              inout [l].push_back ((*i)->accessPllX ().accessOuterPllX ().first->lockAndTry ()) ;
            }
          }
        }

        // exchange data 
        inout = mpAccess ().exchange (inout) ;
        
        {
          for (int l = 0 ; l < nl ; l ++) 
          {
            clean [l] = vector < int > (innerFaces [l].size (), long (true)) ;
            vector < int > :: iterator j = clean [l].begin (), k = inout [l].begin () ;
            vector < hface_STI * > :: iterator iEnd = innerFaces [l].end () ;
            for (vector < hface_STI * > :: iterator i = innerFaces [l].begin () ; 
                 i != iEnd; i ++, j++, k++) 
            {
              assert (j != clean [l].end ()) ; assert (k != inout [l].end ()) ;
              (*j) &= (*k) && (*i)->accessPllX ().accessOuterPllX ().first->lockAndTry () ;
            }
          } 
        }
      }
      
      {
        vector < vector < int > > inout (nl) ;
        {
          for (int l = 0 ; l < nl ; l ++) 
          {
            // reserve memory first 
            inout[l].reserve( innerFaces [l].size() );

            vector < int > :: iterator j = clean [l].begin () ;
            vector < hface_STI * > :: iterator iEnd = innerFaces [l].end () ;
            for (vector < hface_STI * > :: iterator i = innerFaces [l].begin () ; 
                 i != iEnd; i ++, j++) 
            {
              inout [l].push_back (*j) ;
              (*i)->accessPllX ().accessOuterPllX ().first->unlockAndResume (bool (*j)) ;
            }
          }     
        }
      
        // exchange data 
        inout = mpAccess ().exchange (inout) ;
      
        {
          for (int l = 0 ; l < nl ; l ++) 
          {
            vector < int > :: iterator j = inout [l].begin () ;
            vector < hface_STI * > :: iterator iEnd = outerFaces [l].end () ;
            for (vector < hface_STI * > :: iterator i = outerFaces [l].begin () ; 
                 i != iEnd; i ++, j++) 
            {
              assert (j != inout [l].end ()) ;
              (*i)->accessPllX ().accessOuterPllX ().first->unlockAndResume (bool (*j)) ;
            }
          }     
        }
      }
    } 
    catch (Parallel :: AccessPllException) 
    {
      cerr << "**FEHLER (FATAL) AccessPllException beim Vergr\"obern der Fl\"achenb\"aume\n" ;
      cerr << "  aufgetreten. In " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    
    try 
    {
    
      // Phase des Kantenausgleichs im parallelen Vergr"oberungsalgorithmus:
  
      __STATIC_phase  = 6 ;
    
      // Weil hier jede Kante nur eindeutig auftreten darf, muss sie in einem
      // map als Adresse hinterlegt werden, dann k"onnen die verschiedenen
      // Refcounts aus den verschiedenen Links tats"achlich global miteinander
      // abgemischt werden. Dazu werden zun"achst alle eigenen Kanten auf ihre
      // Vergr"oberbarkeit hin untersucht und dieser Zustand (true = vergr"oberbar
      // false = darf nicht vergr"obert werden) im map 'clean' hinterlegt. Dazu
      // kommt noch ein zweiter 'bool' Wert, der anzeigt ob die Kante schon ab-
      // schliessend vergr"obert wurde oder nicht. 
    
      map < hedge_STI *, pair < bool, bool >, less < hedge_STI * > > clean ;
      
      {
        for (int l = 0 ; l < nl ; l ++)
        {
          vector < hedge_STI * > :: iterator iEnd = innerEdges [l].end () ;
          for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ; 
               i != iEnd; i ++)
          {
            if (clean.find (*i) == clean.end ()) 
            {
              clean [*i] = pair < bool, bool > ((*i)->accessPllX ().lockAndTry (), true) ;
            }
          }
        }
      }
      
      {
        vector < vector < int > > inout (nl) ;
        {
          for (int l = 0 ; l < nl ; l ++)
          {
            // reserve memory first 
            inout[l].reserve( outerEdges [l].size() );
            // get end iterator 
            vector < hedge_STI * > :: iterator iEnd = outerEdges [l].end () ;
            for (vector < hedge_STI * > :: iterator i = outerEdges [l].begin () ; 
                 i != iEnd; i ++)
            {
              inout [l].push_back ((*i)->accessPllX ().lockAndTry ()) ;
            }
          }
        }
        
        // exchange data 
        inout = mpAccess ().exchange (inout) ;
        
        {
          for (int l = 0 ; l < nl ; l ++) 
          {
            vector < int > :: const_iterator j = inout [l].begin () ;
            // get end iterator 
            vector < hedge_STI * > :: iterator iEnd = innerEdges [l].end () ;
            for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ; 
                 i != iEnd; i ++, j++) 
            {
              assert (j != inout [l].end ()) ;
              assert (clean.find (*i) != clean.end ()) ;
              if (*j == false) clean [*i] = pair < bool, bool > (false, clean[*i].second) ; 
            }
          }
        }
      }
      
      {
        vector < vector < int > > inout (nl) ;
        {
          for (int l = 0 ; l < nl ; l ++) 
          {
            // reserve memory first 
            inout[l].reserve( innerEdges [l].size() );
            // get end iterator 
            vector < hedge_STI * > :: iterator iEnd = innerEdges [l].end () ;
            for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ; 
                 i != iEnd; i ++) 
            {
              assert (clean.find (*i) != clean.end ()) ;
              pair < bool, bool > & a = clean [*i] ;
              inout [l].push_back (a.first) ;
              if (a.second) 
              {
                // Wenn wir hier sind, kann die Kante tats"achlich vergr"obert werden, genauer gesagt,
                // sie wird es auch und der R"uckgabewert testet den Vollzug der Aktion. Weil aber nur
                // einmal vergr"obert werden kann, und die Iteratoren 'innerEdges [l]' aber eventuell
                // mehrfach "uber eine Kante hinweglaufen, muss diese Vergr"oberung im map 'clean'
                // vermerkt werden. Dann wird kein zweiter Versuch unternommen.
              
                a.second = false ;
#ifndef NDEBUG
                bool b = 
#endif
                  (*i)->accessPllX ().unlockAndResume (a.first) ;
                assert (b == a.first) ;
              }
            }
          }
        }
        
        // exchange data 
        inout = mpAccess ().exchange (inout) ;
        
        {
          for (int l = 0 ; l < nl ; l ++) 
          {
            vector < int > :: iterator j = inout [l].begin () ;
            // get end iterator 
            vector < hedge_STI * > :: iterator iEnd = outerEdges [l].end () ;
            for (vector < hedge_STI * > :: iterator i = outerEdges [l].begin () ; 
                 i != iEnd; i ++, j++) 
            {
              assert (j != inout [l].end ()) ;
      
              // Selbe Situation wie oben, aber der Eigent"umer der Kante hat mitgeteilt, dass sie
              // vergr"obert werden darf und auch wird auf allen Teilgebieten also auch hier. Der
              // Vollzug der Vergr"oberung wird durch den R"uckgabewert getestet.
            
#ifndef NDEBUG
              bool b = 
#endif
                (*i)->accessPllX ().unlockAndResume (bool (*j)) ;
              assert (b == bool (*j)) ;
            }
          }
        }
      }
    } 
    catch (Parallel :: AccessPllException) 
    {
      cerr << "**FEHLER (FATAL) AccessPllException beim Vergr\"obern der Kantenb\"aume\n" ;
      cerr << "  aufgetreten. In " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
  }
  
  __STATIC_phase = -1 ;
  
  return ;
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
            pair < ElementPllXIF_t *, int > p = inner.item ().accessPllX ().accessInnerPllX () ;
            p.first->packAsGhost(os, p.second) ;
          }
        }

        {
          IteratorSTI < hface_STI > & outer = *w.second;
          for (outer.first () ; ! outer.done () ; outer.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = outer.item ().accessPllX ().accessInnerPllX () ;
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
            pair < ElementPllXIF_t *, int > p = outer.item ().accessPllX ().accessOuterPllX () ;
            p.first->insertGhostCell(os, p.second) ;
          }
        }
        {
          IteratorSTI < hface_STI > & inner = *w.first;
          for (inner.first () ; ! inner.done () ; inner.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = inner.item ().accessPllX ().accessOuterPllX () ;
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
          pair < ElementPllXIF_t *, int > p = outer.item ().accessPllX ().accessOuterPllX () ;

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
          pair < ElementPllXIF_t *, int > p = inner.item ().accessPllX ().accessOuterPllX () ;

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
