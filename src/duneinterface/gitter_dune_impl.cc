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
    Gitter :: backup (out) ;

    backupIndices ( out ) ;

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
  return (_arp) ? (*_arp).preCoarsening( bnd ) : 0;
}

int GitterDuneBasis :: postRefinement (Gitter::hbndseg_STI & bnd)
{
  // if _arp is set then the extrenal postRefinement is called 
  return (_arp) ? (*_arp).postRefinement( bnd ) : 0;
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

int nr = 0;
bool GitterDuneBasis :: refine () {
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: refine ()" << endl, 1) : 1) ;
  bool x = true ;
  leaf_element__macro_element__iterator i (container ()) ;
  do
  {
    x = true ;
    // refine marked elements
    for( i.first(); ! i.done() ; i.next()) x &= i.item ().refine () ;
    // check for conformity
    // if noconform break;
    std::cout << "check non conform refinement" << std::endl;
    x = true ;
    for( i.first(); ! i.done() ; i.next()) { std::cout << "***" << std::endl; x &= i.item ().markNonConform () ; }
	  std::ostringstream ss;
	  ss << "ref-" << nr << ".vtk";
    tovtk(  ss.str().c_str() );
    ++nr;
    break;
    if (x) break;
  }
  while (1);  // need something here on required conformity
  return  x;
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

void GitterDuneBasis :: tovtk( const char *fn) 
{
  // openfile
  std::ofstream vtkFile;
  vtkFile.open( fn );
    
  // header info
  vtkFile << "# vtk DataFile Version 2.0" << std::endl;
  vtkFile << "Unstructured Grid" << std::endl;
  vtkFile << "ASCII" << std::endl;
  vtkFile << "DATASET UNSTRUCTURED_GRID" << std::endl;

  // vertex list
  typedef std::vector< double > Vertex;
  std::map< int, Vertex > vertexList;

  int nCells = 0;

  // loop to find vertexList and count cells
  {
    typedef Objects :: tetra_IMPL tetra_IMPL ;
    LeafIterator < Gitter::helement_STI > w (*this) ;
    for (w->first () ; ! w->done () ; w->next ())
      {
      
	tetra_IMPL* item = ((tetra_IMPL *) &w->item ());

	for (int i=0;i<4;++i)
	  {
	    Vertex v ( item->myvertex(i)->Point(), item->myvertex(i)->Point() + sizeof( item->myvertex(i)->Point() ) / sizeof( double ) );
	    vertexList[ item->myvertex(i)->getIndex() ]
	      = v;
	  }

	++nCells;
      }
  }

  // points info
  {
    vtkFile << "POINTS " << vertexList.size() << " double" << std::endl;
    for( unsigned int i = 0; i < vertexList.size(); ++i )
      {
	vtkFile << vertexList[ i ][ 0 ]
		<< " " << vertexList[ i ][ 1 ]
		<< " " << vertexList[ i ][ 2 ] << std::endl;
      }
  }

  // cell info
  {
    vtkFile << "CELLS " << nCells << " " << 5*nCells << std::endl;

    typedef Objects :: tetra_IMPL tetra_IMPL ;
    LeafIterator < Gitter::helement_STI > w (*this) ;
    for (w->first () ; ! w->done () ; w->next ())
      {
      	tetra_IMPL* item = ((tetra_IMPL *) &w->item ());

	vtkFile << 4;

	for (int i=0;i<4;++i)
	  {
	    vtkFile << " " << item->myvertex(i)->getIndex();
	  }

	vtkFile << std::endl;
      }
  }

  // cell type info
  {
    vtkFile << "CELL_TYPES " << nCells << std::endl;

    for( int i = 0; i < nCells; ++i )
      {
	vtkFile << 10 << std::endl; // 10 for a tetrahedron
      }
  }

  // cell data
  {
    vtkFile << "CELL_DATA " << nCells << std::endl;
    vtkFile << "SCALARS cell-id double 1" << std::endl;
    vtkFile << "LOOKUP_TABLE default" << std::endl;

    typedef Objects :: tetra_IMPL tetra_IMPL ;
    LeafIterator < Gitter::helement_STI > w (*this) ;
    for (w->first () ; ! w->done () ; w->next ())
      {
      	tetra_IMPL* item = ((tetra_IMPL *) &w->item ());

	vtkFile << item->getIndex() << std::endl;
      }
  }

  vtkFile.close();
  std::cout << "data written to " << fn << std::endl;
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
