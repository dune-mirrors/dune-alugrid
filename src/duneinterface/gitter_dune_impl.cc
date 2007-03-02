// Robert Kloefkorn (c) 2004 - 2005 
#ifndef GITTER_DUNE_IMPL_CC_INCLUDED
#define GITTER_DUNE_IMPL_CC_INCLUDED

#include "gitter_dune_impl.h"

IteratorSTI < Gitter :: helement_STI > * GitterDuneImpl :: 
leafIterator (const helement_STI *) 
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > (container ()) ;
}

IteratorSTI < Gitter :: helement_STI > * GitterDuneImpl :: 
leafIterator (const IteratorSTI < helement_STI > * p) 
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > 
  (*(const Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > *) p) ;
}

void GitterDuneBasis :: backupIndices (ostream & out)
{
  // backup indices, our index type is hierarchic_index 
  unsigned char indices = hierarchic_index;
  out << indices;

  enum { numOfIndexManager = Gitter :: Geometric :: BuilderIF ::  numOfIndexManager };
  // store max indices 
  for(int i=0; i< numOfIndexManager ; ++i)
    this->indexManager(i).backupIndexSet(out);

  { // backup index of elements 
    AccessIterator <helement_STI> :: Handle ew (container ()) ;
    for (ew.first () ; ! ew.done () ; ew.next ()) ew.item ().backupIndex (out) ;
  }

  // TODO: backup face and edge indices 
  
  {
    // backup index of vertices 
    LeafIterator < vertex_STI > w ( *this );
    for( w->first(); ! w->done() ; w->next () ) w->item().backupIndex(out);
  }

  return ;
}

// go down all children and check index 
inline void GitterDuneBasis :: 
goDownHelement( Gitter::helement_STI & el , vector<bool> & idxcheck)
{
  assert( el.isInterior() );
  typedef Gitter :: helement_STI ElType;
  assert( (static_cast<size_t> (el.getIndex())) < idxcheck.size() );
  // make sue we only visit an element once 
  assert( idxcheck[ el.getIndex() ]  == true );
  idxcheck[ el.getIndex() ] = false;
  for( ElType * ch = el.down() ; ch ; ch = ch->next())
    goDownHelement( *ch , idxcheck );

  return ;
}

void GitterDuneBasis ::restoreIndices (istream & in) 
{
  unsigned char indices = no_index;
  in >> indices;

  // set VERBOSE to 20 and you have the indices value printed 
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: restoreIndices: index flag = " << (int)indices << " file: "
                       << __FILE__ << " line: " << __LINE__ <<") " << endl, 1) : 1) ;

  typedef Gitter :: Geometric :: BuilderIF  BuilderIF;
  enum { numOfIndexManager = BuilderIF :: numOfIndexManager };
  
  // restore dune indices (see backUpIndices method)
  if(indices == hierarchic_index) 
  {
    for(int i=0; i< numOfIndexManager ; ++i)
      this->indexManager(i).restoreIndexSet( in );

    // restore index of elements 
    {
      AccessIterator < helement_STI >:: Handle ew(container());
      for ( ew.first(); !ew.done(); ew.next()) ew.item().restoreIndex (in);
    }
    // restore index of vertices
    {
      LeafIterator < vertex_STI > w ( *this );
      for( w->first(); ! w->done() ; w->next () ) w->item().restoreIndex(in);
    }
    
    // reconstruct holes 

    { 
      /////////////////////////////////
      //  Elements 
      /////////////////////////////////
      {
        // for elements 
        IndexManagerType& elementManager = this->indexManager(BuilderIF :: IM_Elements);
        const int idxsize = elementManager.getMaxIndex();
        
        vector < bool > checkidx ( idxsize );
        for(int i=0; i<idxsize; ++i) checkidx[i] = true;

        AccessIterator < helement_STI >:: Handle ew(container());
        for ( ew.first(); !ew.done(); ew.next())
        {
          goDownHelement( ew.item() , checkidx );
        }

        // all remaining indices are treated as holes 
        for(int i=0; i<idxsize; ++i)
        {
          if(checkidx[i] == true)
          {
            elementManager.freeIndex(i);
          }
        }
      }

      // TODO restore indices of faces and 
      // edges 

      /////////////////////////////////
      //  Vertices  
      /////////////////////////////////
      {
        // for vertices 
        LeafIterator < vertex_STI > w ( *this );
        IndexManagerType& vertexManager = this->indexManager(BuilderIF :: IM_Vertices);
        const int idxsize = vertexManager.getMaxIndex();

        vector < bool > checkidx ( idxsize );
        for(int i=0; i<idxsize; ++i) checkidx[i] = true;

        for( w->first(); ! w->done() ; w->next () )
        {
          assert( (static_cast<size_t> (w->item().getIndex())) < checkidx.size() );
          checkidx[ w->item().getIndex() ] = false;
        }

        // all remaining indices are treated as holes.
        for(int i=0; i<idxsize; ++i)
        {
          if(checkidx[i] == true)
          {
            vertexManager.freeIndex(i);
          }
        }

      }
    }
    return ;
  }

  if(indices == leaf_index) // convert indices to leafindices 
  {
    int idx = 0;
    PureElementLeafIterator < helement_STI > ew(*this);
    for ( ew->first(); !ew->done(); ew->next()) 
    {
      ew->item().setIndex( idx );
      ++idx;
    }
    this->indexManager(0).setMaxIndex ( idx );
    assert (debugOption (20) ? (cout << endl << "**INFO GitterDuneBasis :: restoreIndices: create new leaf indices with size = " << idx << " ! file: "<< __FILE__ << ", line: " << __LINE__ << endl, 1) : 1) ;
    return ;
  }

  cerr<< "**WARNING: GitterDuneBasis :: restoreIndices: indices (id = " << indices << ") not read! file: "<< __FILE__ << ", line: " << __LINE__ << "\n";
  return ;
}

// wird von Dune verwendet 
void GitterDuneBasis :: duneBackup (const char * fileName) 
{
  // diese Methode wird von der Dune Schnittstelle aufgerufen und ruft
  // intern lediglich backup (siehe oben) und backupCMode des Makrogitters
  // auf, allerdings wird hier der path und filename in einer variablen
  // uebergeben 

  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneBackup (const char * = \""
                       << fileName << "\") " << endl, 1) : 1) ;

  ofstream out (fileName) ;
  if (!out) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneBackup (const char *, double) Fehler beim Anlegen von < "
         << (fileName ? fileName : "null") << " >" << endl ;
  }
  else
  {
    FSLock lock (fileName) ;
    Gitter :: backup (out) ;

    this->backupIndices (out) ;

    {
      char *fullName = new char[strlen(fileName)+20];
      if(!fullName)
      {
        cerr << "**WARNUNG GitterDuneBasis :: duneBackup (, const char *, double) :";
        cerr << "couldn't allocate fullName! " << endl;
        abort();
      }
      sprintf(fullName,"%s.macro",fileName);
      ofstream macro (fullName) ;

      if(!macro)
      {
        cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneBackup (const char *, const char *) Fehler beim Anlegen von < "
         << (fullName ? fullName : "null") << " >" << endl ;
      }
      else
      {
        container ().backupCMode (macro) ;
      }
      delete [] fullName;
    }
  }
  return ;
}

// wird von Dune verwendet 
void GitterDuneBasis :: duneRestore (const char * fileName)
{
  // diese Methode wird von der Dune Schnittstelle aufgerufen 
  // diese Methode ruft intern restore auf, hier wird lediglich 
  // der path und filename in einer variablen uebergeben

  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneRestore (const char * = \""
                 << fileName << "\") " << endl, 1) : 1) ;

  ifstream in (fileName) ;
  if (!in) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneRestore (const char *, double & ) Fehler beim \"Offnen von < "
         << (fileName ? fileName : "null") << " > " << endl ;
  } else {
    Gitter :: restore (in) ;
    this->restoreIndices (in);
  }
  return ;
}

int GitterDuneBasis :: preCoarsening  (Gitter::helement_STI & elem)
{
  // if _arp is set then the extrenal preCoarsening is called 
  return (_arp) ? (*_arp).preCoarsening(elem) : 0;
}

int GitterDuneBasis :: postRefinement (Gitter::helement_STI & elem)
{
  // if _arp is set then the extrenal postRefinement is called 
  return (_arp) ? (*_arp).postRefinement(elem) : 0;
}

int GitterDuneBasis :: preCoarsening  (Gitter::hbndseg_STI & bnd)
{
  // if _arp is set then the extrenal preCoarsening is called 
  return (_arp) ? (*_arp).preCoarsening(bnd) : 0;
}

int GitterDuneBasis :: postRefinement (Gitter::hbndseg_STI & bnd)
{
  // if _arp is set then the extrenal postRefinement is called 
  return (_arp) ? (*_arp).postRefinement(bnd) : 0;
}

void GitterDuneBasis ::
setAdaptRestrictProlongOp( AdaptRestrictProlongType & arp )
{
  if(_arp) 
  {
    cerr<< "WARNING: GitterDuneBasis :: setAdaptRestrictProlongOp: _arp not NULL! in:" <<  __FILE__ << " line="<<__LINE__ << endl; 
  }
  _arp = &arp;
}

void GitterDuneBasis :: removeAdaptRestrictProlongOp()
{
  _arp = 0;
}

bool GitterDuneBasis :: refine () {
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: refine ()" << endl, 1) : 1) ;
  bool x = true ;
  {
    leaf_element__macro_element__iterator i (container ()) ;
    for( i.first(); ! i.done() ; i.next()) 
    {
      x &= i.item ().refine () ;
    }
  }
  return x ;
}

void GitterDuneBasis :: coarse() {
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: coarse ()" << endl, 1) : 1) ;
  {
    AccessIterator < helement_STI > :: Handle i (container ()) ;
    for( i.first(); ! i.done() ; i.next()) 
    {
      i.item ().coarse () ; 
    }
  }
  return ;
}

bool GitterDuneBasis :: duneAdapt (AdaptRestrictProlongType & arp) 
{
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneAdapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  const int start = clock () ;

  setAdaptRestrictProlongOp(arp); 
  bool refined = this->refine ();
  if (!refined) {
    cerr << "**WARNUNG (IGNORIERT) Verfeinerung nicht vollst\"andig (warum auch immer)\n" ;
    cerr << "  diese Option ist eigentlich dem parallelen Verfeinerer vorbehalten.\n" ;
    cerr << "  Der Fehler trat auf in " << __FILE__ << " " << __LINE__ << endl ;
  }

  int lap = clock () ;
  this->coarse () ;
  int end = clock () ;
  if (debugOption (1)) {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO GitterDuneBasis :: duneAdapt () [ref|cse|all] " << u1 << " " << u2 << " " << u3 << endl ;
  }
  // sets pointer to zero 
  removeAdaptRestrictProlongOp ();

  return refined;
}

#if 0
void GitterDuneBasis :: ALUcomm (
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

  assert ((debugOption (5) && containsVertices) ? (cout << "**INFO GitterDuneBasis :: ALUcomm (): (containsVertices)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsEdges)    ? (cout << "**INFO GitterDuneBasis :: ALUcomm (): (containsEdges)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsFaces)    ? (cout << "**INFO GitterDuneBasis :: ALUcomm (): (containsFaces)=true " << endl, 1) : 1) ;
  assert ((debugOption (5) && containsElements) ? (cout << "**INFO GitterDuneBasis :: ALUcomm (): (containsElements)=true " << endl, 1) : 1) ;

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

void GitterDunePll :: doInteriorGhostComm( 
  vector< ObjectStream > & osvec ,
  GatherScatterType & vertexData , 
  GatherScatterType & edgeData,  
  GatherScatterType & faceData, 
  GatherScatterType & elementData ,
  const CommunicationType commType )
{
  //Entwurf der periodischen R"ander-Ghost-Kommunikation:
  /*
  it = Iterator "uber alle periodischen R"ander
  for (it = begin; !.it.done(); it++) {
    periodic & per =  it.item()
    osvec[0].clear();
    per.myneighbour(0).gather (osvec[0]);
    per.getGhost(1).scatter (osvec[0]);
    osvec[0].clear();
    per.myneighbour(1).gather(osvec[0]);
    per.getGhost(0).scatter(osvec[0]);

  }
  */
	
  const int nl = mpAccess ().nlinks ();
	    
  const bool containsVertices = vertexData.contains(3,3);
  const bool containsEdges    = edgeData.contains(3,2);
  const bool containsFaces    = faceData.contains(3,1);
  const bool containsElements = elementData.contains(3,0);

  const bool containsSomeThing = containsVertices || 
                                 containsEdges || containsFaces || containsElements ;

  const bool packInterior = (commType == All_All_Comm) || 
                            (commType == Interior_Ghost_Comm);
		        
  const bool packGhosts   = (commType == All_All_Comm) || 
                            (commType == Ghost_Interior_Comm);

  assert( !packGhosts );

  if(!containsSomeThing) 
  {
    cerr << "WARNING: communication called with empty data set, all contains methods returned false! \n";
    return ;
  }
			        
  for (int link = 0 ; link < nl ; ++link ) 
  {   
    ObjectStream & sendBuff = osvec[link]; 
    sendBuff.clear();
						    
    {
      hface_STI * determType = 0; // only for type determination 
      pair < IteratorSTI < hface_STI > * , IteratorSTI < hface_STI > * > 
        iterpair = borderIteratorTT( determType , link );

      // write all data belong to interior of master faces 
      sendInteriorGhostData( sendBuff, iterpair.first , 
                             vertexData, edgeData,
                             faceData, elementData, 
                             packInterior , packGhosts );
      // write all data belong to interior of slave faces 
      sendInteriorGhostData( sendBuff, iterpair.second , 
                             vertexData, edgeData,
                             faceData, elementData ,
                             packInterior , packGhosts );
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
      hface_STI * determType = 0; // only for type determination 
      pair < IteratorSTI < hface_STI > * , IteratorSTI < hface_STI > * > 
        iterpair = borderIteratorTT( determType , link );
      // first unpack slave data, because this has been pack from master
      // first , see above 
      unpackInteriorGhostData( recvBuff, iterpair.second , 
                               vertexData, edgeData,
                               faceData, elementData );
      // now unpack data sended from slaves to master 
      unpackInteriorGhostData( recvBuff, iterpair.first , 
                               vertexData, edgeData,
                               faceData, elementData );
      delete iterpair.first;
      delete iterpair.second;
    }
  }
  // end element communication 
  return ;
}
#endif

#endif
