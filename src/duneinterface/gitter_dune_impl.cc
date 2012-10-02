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

//int nr = 0;
//int adaptstep = 0;
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
    for( i.first(); ! i.done() ; i.next()) { x &= i.item ().markNonConform () ; }
    std::ostringstream ss;
    int filenr = adaptstep*100+nr;
    ss << "ref-" << ZeroPadNumber(filenr) << ".vtu";
    tovtk(  ss.str() );
    ++nr;
    // break;
    if (x) break;
  }
  while (1);  // need something here on required conformity
  ++adaptstep;
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

void GitterDuneBasis :: tovtk( const std::string &fn) 
{
  // openfile
  std::ofstream vtuFile;
  vtuFile.open( fn.c_str() );
    
  // header info
  vtuFile << "<?xml version=\"1.0\"?>" << std::endl;
  vtuFile << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
  vtuFile << "  <UnstructuredGrid>" << std::endl;

  // vertex list
  typedef std::vector< double > Vertex;
  typedef std::map< int, std::pair<int,Vertex> > VertexList;
  VertexList vertexList;

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
	    Vertex v(3);
	    for (int k=0;k<3;++k) 
	      v[k] = item->myvertex(i)->Point()[k];
	    vertexList[ item->myvertex(i)->getIndex() ] = make_pair(-1,v);
	  }
	++nCells;
      }
  }

  vtuFile << "    <Piece NumberOfPoints=\"" << vertexList.size() << "\" "
	  << "NumberOfCells=\"" << nCells << "\">" << std::endl;

  // cell data
  {
    vtuFile << "      <CellData Scalars=\"cell-id\">" << std::endl;
    vtuFile << "        <DataArray type=\"Float32\" Name=\"cell-id\" NumberOfComponents=\"1\">" << std::endl;
    vtuFile << "          ";

    typedef Objects :: tetra_IMPL tetra_IMPL ;
    LeafIterator < Gitter::helement_STI > w (*this) ;
    for (w->first () ; ! w->done () ; w->next ())
      {
	tetra_IMPL* item = ((tetra_IMPL *) &w->item ());
	vtuFile << item->getIndex() << " ";
      }

    vtuFile << std::endl;
    vtuFile << "        </DataArray>" << std::endl;
    vtuFile << "      </CellData>" << std::endl;
  }

  // points info
  {
    vtuFile << "      <Points>" << std::endl;
    vtuFile << "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">" << std::endl;

    const VertexList::iterator end = vertexList.end();
    int nr=0;
    for( VertexList::iterator i = vertexList.begin(); i != end; ++i, ++nr )
      {
	vtuFile << "          " << (*i).second.second[ 0 ] << " " << (*i).second.second[ 1 ] << " " << (*i).second.second[ 2 ] << std::endl;
	(*i).second.first = nr;
      }

    vtuFile << "        </DataArray>" << std::endl;
    vtuFile << "      </Points>" << std::endl;
  }

  // cell info
  {
    vtuFile << "      <Cells>" << std::endl;
    // connectivity
    vtuFile << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << std::endl;
    vtuFile << "         ";

    typedef Objects :: tetra_IMPL tetra_IMPL ;
    LeafIterator < Gitter::helement_STI > w (*this) ;
    for (w->first () ; ! w->done () ; w->next ())
      {
	tetra_IMPL* item = ((tetra_IMPL *) &w->item ());
	for (int i=0;i<4;++i)
	  {
	    vtuFile << " " << vertexList[item->myvertex(i)->getIndex()].first;
	  }
      }
    vtuFile << std::endl;
    vtuFile << "        </DataArray>" << std::endl;

    // offsets
    vtuFile << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << std::endl;
    vtuFile << "         ";

    for( int i = 0; i < nCells; ++i )
      {
	vtuFile << " " << (i+1)*4;
      }
    vtuFile << std::endl;

    vtuFile << "        </DataArray>" << std::endl;

    // cell type
    vtuFile << "        <DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">" << std::endl;
    vtuFile << "         ";

    for( int i = 0; i < nCells; ++i )
      {
	vtuFile << " " << 10; // 10 for tetrahedra
      }
    vtuFile << std::endl;

    vtuFile << "        </DataArray>" << std::endl;
  }

  vtuFile << "      </Cells>" << std::endl;
  vtuFile << "    </Piece>" << std::endl;
  vtuFile << "  </UnstructuredGrid>" << std::endl;
  vtuFile << "</VTKFile>" << std::endl;

  vtuFile.close();
  std::cout << "data written to " << fn << std::endl;
}

#endif
