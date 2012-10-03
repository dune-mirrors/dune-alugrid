// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_STI_CC_INCLUDED
#define GITTER_STI_CC_INCLUDED

#include "lock.h"
#include "gitter_sti.h"
#include "walk.h"

#ifndef NDEBUG
#ifdef DEBUG_ALUGRID
Refcount :: Globalcount Refcount :: _g ;

Refcount :: Globalcount :: ~Globalcount () {
  assert (_c ? (cerr << "**WARNING Refcount :: Globalcount :: ~Globalcount() " << _c 
           << " objects have not been removed correctly!" << endl, 1) : 1) ;
  return ;
}
#endif
#endif

typedef Wrapper < AccessIterator < Gitter :: vertex_STI > :: Handle, 
  Gitter :: InternalVertex >              leaf_vertex__macro_vertex__iterator ;

typedef Insert < AccessIterator < Gitter :: hedge_STI > :: Handle, 
  TreeIterator < Gitter :: hedge_STI, is_leaf < Gitter :: hedge_STI > > >   leaf_edge__macro_edge__iterator ;

typedef Insert < AccessIterator < Gitter :: hface_STI > :: Handle, 
  TreeIterator < Gitter :: hface_STI, is_leaf < Gitter :: hface_STI > > >   leaf_face__macro_face__iterator ;

typedef Insert < AccessIterator < Gitter :: hbndseg_STI > :: Handle, 
  TreeIterator < Gitter :: hbndseg_STI, is_leaf < Gitter :: hbndseg_STI> > >  leaf_bnd__macro_bnd__iterator ;

typedef Insert < AccessIterator < Gitter :: helement_STI > :: Handle, 
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > leaf_element__macro_element__iterator ;

IteratorSTI < Gitter :: vertex_STI > * Gitter :: iterator (const Gitter :: vertex_STI *) {

  vector < IteratorSTI < vertex_STI > * > _iterators ;
  {
    _iterators.push_back ( new AccessIterator < vertex_STI > :: Handle (container ())) ;
  }
  Insert < AccessIterator < hedge_STI > :: Handle, 
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > dw (container ()) ;
  _iterators.push_back ( new Wrapper < Insert < AccessIterator < hedge_STI > :: Handle, 
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (dw)) ;
  {
    Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fw (container ()) ;
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fw)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_vertex < helement_STI > > > ew (container ()) ;
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_vertex < helement_STI > > >, InternalVertex > (ew)) ;
  }
  {
    Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fw (container ()) ;
    Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > df (fw) ;
    Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > dif (df) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (dif)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > de (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > die (de) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (die)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >, 
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fie (fe) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fie)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fe) ;
    Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfie (fie) ;
    Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > difie (dfie) ;
    _iterators.push_back (new Wrapper < Insert < Wrapper < Insert < Wrapper < 
  Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (difie)) ;
  }
  return new VectorAlign < vertex_STI > (_iterators) ;
}

IteratorSTI < Gitter :: hedge_STI > * Gitter :: iterator (const hedge_STI * e) 
{
  is_leaf< hedge_STI > rule; 
  return this->createIterator(e, rule);
}

IteratorSTI < Gitter :: hface_STI > * Gitter :: iterator (const hface_STI * f) 
{
  is_leaf< hface_STI > rule; 
  return this->createIterator(f, rule);
}

IteratorSTI < Gitter :: hbndseg_STI > * Gitter :: iterator (const hbndseg_STI * bnd) 
{
  is_leaf <hbndseg_STI> rule;
  return this->createIterator(bnd, rule);
}

IteratorSTI < Gitter :: helement_STI > * Gitter :: iterator (const helement_STI *el) 
{
  is_leaf <helement_STI> rule;
  return this->createIterator(el, rule);
}

//**************************************************************************
// all the level iterators 
//************************************************************************** 
IteratorSTI < Gitter :: vertex_STI > * Gitter :: 
levelIterator (const Gitter :: vertex_STI * a, const any_has_level< vertex_STI > & vhl) 
{
  vector < IteratorSTI < vertex_STI > * > _iterators ;
  {
    _iterators.push_back ( new AccessIterator < vertex_STI > :: Handle (container ())) ;
  }
  Insert < AccessIterator < hedge_STI > :: Handle, 
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > dw (container ()) ;
  _iterators.push_back ( new Wrapper < Insert < AccessIterator < hedge_STI > :: Handle, 
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (dw)) ;
  {
    Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fw (container ()) ;
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fw)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_vertex < helement_STI > > > ew (container ()) ;
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_vertex < helement_STI > > >, InternalVertex > (ew)) ;
  }
  {
    Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fw (container ()) ;
    Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > df (fw) ;
    Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > dif (df) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (dif)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > de (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > die (de) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (die)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >, 
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fie (fe) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fie)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fe) ;
    Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfie (fie) ;
    Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > difie (dfie) ;
    _iterators.push_back (new Wrapper < Insert < Wrapper < Insert < Wrapper < 
  Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (difie)) ;
  }
  return new VectorAlign < vertex_STI > (_iterators) ;
}

// create level edge iterator 
IteratorSTI < Gitter :: hedge_STI > * Gitter :: levelIterator (const hedge_STI * e, const any_has_level<hedge_STI> & ahl) 
{
  any_has_level<hedge_STI> rule(ahl);
  return this->createIterator(e,rule);
}


// create level face iterator 
IteratorSTI < Gitter :: hface_STI > * Gitter :: 
levelIterator (const hface_STI * f , const any_has_level<hface_STI> & ahl ) 
{
  any_has_level< hface_STI > rule(ahl);
  return this->createIterator(f, rule);
}

// create level element iterator 
IteratorSTI < Gitter :: helement_STI > * Gitter :: levelIterator (const helement_STI * el, const any_has_level<helement_STI> & ahl) 
{
  any_has_level <helement_STI> rule(ahl);
  return this->createIterator(el, rule);
}

IteratorSTI < Gitter :: hbndseg_STI > * Gitter :: levelIterator (const hbndseg_STI * bnd, const any_has_level<hbndseg_STI> & ahl) 
{
  any_has_level<hbndseg_STI> rule(ahl);
  return this->createIterator(bnd, rule);
}

//*******************************************
//  other methods on class Gitter 
//*******************************************
void Gitter :: fullIntegrityCheck () {
  const int start = clock() ;
  int count = 0 ;
  leaf_element__macro_element__iterator w (container ()) ;
  for(w.first () ; !w.done () ; w.next ())
    w.item ().test () ? (cerr << "-> fehler gefunden am element : " << count << "\n" << endl, count++) : (count ++) ;
  if (debugOption (3)) {
    float used = (float)(clock () - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO Gitter :: fullIntegrityCheck () used : " << used << " sec." << endl ;
  }
  return ;
}

void Gitter :: printsize () {
  cout << "\n Gitter :: printSize () : \n\n" ;
  if (debugOption (10)) {
    { cout << " - Macro elements .... "  << AccessIterator < helement_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Macro boundary .... " << AccessIterator < hbndseg_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Macro faces ....... " << AccessIterator < hface_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Macro edges ....... "  << AccessIterator < hedge_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Makro vertices .... "  << AccessIterator < vertex_STI > :: Handle (container ()).size() << "\n" ; }
    cout << "\n" ;
  }
  { cout << " - Elements ............ "  << LeafIterator < helement_STI > (*this)->size() << "\n" ;}
  { cout << " - Boundaries .......... " << LeafIterator < hbndseg_STI > (*this)->size() << "\n" ;}
  { cout << " - Faces  .............. " << LeafIterator < hface_STI > (*this)->size() << "\n" ;}
  { cout << " - Edges ............... "  << LeafIterator < hedge_STI > (*this)->size() << "\n" ;}
  { cout << " - Vertices ............ "  << LeafIterator < vertex_STI > (*this)->size() << "\n" ;}
  cout << endl ;
  return ;
}

int nr = 0;
int adaptstep = 0;
bool Gitter :: refine () 
{
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: refine ()" << endl, 1) : 1) ;
  bool x = true ;
  leaf_element__macro_element__iterator i (container ()) ;
  // refine marked elements
  for( i.first(); ! i.done() ; i.next()) x &= i.item ().refine () ;
  std::ostringstream ss;
  int filenr = adaptstep*100+nr;
  ss << "ref-" << ZeroPadNumber(filenr) << ".vtu";
  tovtk(  ss.str() );
  ++nr;
  return  x;
}

/*
int nr = 0;
int adaptstep = 0;
bool Gitter :: refine () 
{
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: refine ()" << endl, 1) : 1) ;
  bool x = true ;
  leaf_element__macro_element__iterator i (container ()) ;
  for( i.first(); ! i.done() ; i.next() ) 
  {
    x &= i.item ().refine () ;
  }
	std::ostringstream ss;
  int filenr = adaptstep*1000+nr;
	ss << "ref-" << ZeroPadNumber(filenr) << ".vtu";
  tovtk(  ss.str() );
  ++nr;
  return  x;
}
*/

bool Gitter :: markNonConform()
{
  bool x = true ;
  leaf_element__macro_element__iterator i (container ()) ;
  // std::cout << "check non conform refinement" << std::endl;
  for( i.first(); ! i.done() ; i.next()) { x &= i.item ().markNonConform () ; }
  return x;
}

void Gitter :: coarse() 
{
  assert (debugOption (20) ? (cout << "**INFO Gitter :: coarse ()" << endl, 1) : 1) ;
  {
    AccessIterator < helement_STI > :: Handle i (container ()) ;
    for( i.first(); ! i.done() ; i.next() ) 
    {
      i.item ().coarse () ; 
    }
  }
}

void Gitter :: tovtk( const std::string &fn ) 
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

  typedef Gitter :: Geometric :: tetra_GEO tetra_GEO ;
  // typedef LeafIterator < Gitter::helement_STI > Iterator;
  typedef LevelIterator < Gitter::helement_STI > Iterator;
  Iterator w (*this,0) ;

  // loop to find vertexList and count cells
  {
    for (w->first () ; ! w->done () ; w->next ())
    {
      tetra_GEO* item = ((tetra_GEO *) &w->item ());
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

    for (w->first () ; ! w->done () ; w->next ())
    {
      vtuFile << w->item ().getIndex() << " ";
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

    for (w->first () ; ! w->done () ; w->next ())
    {
      tetra_GEO* item = ((tetra_GEO *) &w->item ());
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

bool Gitter :: adapt () 
{
  assert (debugOption (20) ? (cout << "**INFO Gitter :: adapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  const int start = clock () ;

  bool x;
  bool refined = true;
  do {
    refined &= refine ();
    // check for conformity
    // if noconform break;
    std::cout << "check non conform refinement" << std::endl;
    x = markNonConform();
  }
  while (!x);  // need something here on required conformity
  ++adaptstep;

  if (!refined) {
    cerr << "**WARNUNG (IGNORIERT) Verfeinerung nicht vollst\"andig (warum auch immer)\n" ;
    cerr << "  diese Option ist eigentlich dem parallelen Verfeinerer vorbehalten.\n" ;
    cerr << "  Der Fehler trat auf in " << __FILE__ << " " << __LINE__ << endl ;
  }
  int lap = clock () ;
  coarse () ;
  int end = clock () ;
  if (debugOption (1)) {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO Gitter :: adapt () [ref|cse|all] " << u1 << " " << u2 << " " << u3 << endl ;
  }
  ++adaptstep;
  return refined;
}

bool Gitter :: adaptWithoutLoadBalancing() {
  return adapt();
}
bool Gitter :: duneAdapt (AdaptRestrictProlongType & arp) 
{
  cerr << "Gitter :: duneAdapt: method not overloaded! in file:"<< __FILE__ << "  line:" << __LINE__<< endl;
  return adapt();
}

template <class ostream_t>
void Gitter :: backupImpl (ostream_t & out) 
{
  // backup edges 
  {
    AccessIterator <hedge_STI> :: Handle fw (container ()) ;
    for (fw.first(); !fw.done(); fw.next()) fw.item ().backup (out) ; 
  }
  // backup faces 
  {
    AccessIterator <hface_STI>::Handle fw (container ()) ;
    for (fw.first () ; ! fw.done () ; fw.next ()) fw.item().backup(out) ; 
  }
  // backup elements 
  {
    AccessIterator <helement_STI> :: Handle ew (container ()) ;
    for (ew.first () ; ! ew.done () ; ew.next ()) ew.item ().backup (out) ; 
  }
  // backup periodic elements 
  {
    AccessIterator <hperiodic_STI> :: Handle ew (container ()) ;
    for (ew.first () ; ! ew.done () ; ew.next ()) ew.item ().backup (out) ; 
  }
  return ;
}

// backup taking std::ostream 
void Gitter :: backup (ostream & out) 
{
  assert (debugOption (20) ? (cout << "**INFO Gitter :: backup (ostream & = " << out << ") " << endl, 1) : 1) ;
  backupImpl( out );
}

// backup taking ObjectStream 
void Gitter :: backup (ObjectStream& out) 
{
  backupImpl( out );
}

template <class istream_t>
void Gitter ::restoreImpl (istream_t & in) 
{
  // restore edges 
  {
    AccessIterator < hedge_STI > :: Handle ew (container ());
    for (ew.first () ; !ew.done () ; ew.next ()) ew.item ().restore (in) ; 
  }
  // restore faces 
  {
    AccessIterator < hface_STI >:: Handle fw(container());
    for ( fw.first(); !fw.done (); fw.next()) fw.item().restore (in); 
  }
  // restore elements 
  {
    AccessIterator < helement_STI >:: Handle ew(container());
    for ( ew.first(); !ew.done(); ew.next()) ew.item().restore (in); 
  }
  // restore periodic elements 
  {
    AccessIterator < hperiodic_STI >:: Handle ew(container());
    for ( ew.first(); !ew.done(); ew.next()) ew.item().restore (in); 
  }
    
  // since the faces have been refined before the elements
  // the boundary faces might not habe benn refined at all
  {
    AccessIterator < hbndseg_STI > :: Handle bw (container ()) ;
    for (bw.first () ; ! bw.done () ; bw.next ()) bw.item ().restoreFollowFace () ; 
  }

  // make parallel grid consistent
  notifyGridChanges () ;
}

// restore taking std::istream 
void Gitter :: restore (istream & in) 
{
  assert (debugOption (20) ? (cout << "**INFO Gitter :: restore (istream & = " << in << ") " << endl, 1) : 1) ;  
  restoreImpl ( in );
}

// restore taking ObjectStream 
void Gitter :: restore (ObjectStream& in) 
{
  // restoreImpl ( in );

  cerr << "Gitter :: restore not implemented for ObjectStream: " << __FILE__ << " " << __LINE__ << endl ;
  abort();
}

void Gitter :: refineGlobal () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: refineGlobal () " << endl, 1) : 1) ;
  const int start = clock () ;
  {leaf_element__macro_element__iterator w (container ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item (). tagForGlobalRefinement () ; }
  adapt () ;
  if (debugOption (2))
    cout << "  Gitter :: refineGlobal () used " 
         << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " sec." << endl ;
  return ;
}

void Gitter :: refineRandom (double p) {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: refineRandom (double = " << p << ") " << endl, 1) : 1) ;
  const int start = clock () ;
  if (p < .0 || p > 1.) {
    cerr << "**WARNUNG (IGNORIERT) Gitter :: refineRandom (double = " << p << ") Argument muss zwischen 0 und 1 liegen" << endl ;
  } else {
    {
       leaf_element__macro_element__iterator w (container ()) ;
       for (w.first () ; ! w.done () ; w.next ()) 
         if( drand48 () < p )  w.item ().tagForGlobalRefinement (); 
    }
    adapt () ;
    if (debugOption (2))
      cout << "**INFO Gitter :: refineRandom () used " 
           << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " sec." << endl ;
  }
  return ;
}

void Gitter :: refineBall (const alucoord_t (&center)[3], double radius, int limit) {
  if (radius < .0) {
    cerr << "**WARNUNG (IGNORIERT) Gitter :: refineBall (center = ?, radius = " 
   << radius << ") Radius darf nicht negativ sein" << endl ;
  } else {
    const int start = clock () ;
    {
       leaf_element__macro_element__iterator w (container ()) ;
       for (w.first () ; ! w.done () ; w.next ())
         w.item (). tagForBallRefinement (center,radius,limit) ;
    }
    adapt () ;
    if (debugOption (2))
      cout << "**INFO Gitter :: refineBall () used " 
           << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " sec." << endl ;
  }
  return ;
}

void Gitter :: notifyGridChanges () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: notifyGridChanges () " << endl, 1) : 1) ;
  return ;
}

void Gitter :: notifyMacroGridChanges () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: notifyMacroGridChanges () " << endl, 1) : 1) ;
  return ;
}

Gitter :: ~Gitter () {
  if (ref)
    cerr << "**WARNUNG (IGNORED) Grid-Reference counter [" << ref << "] not zero a point of removal" << endl ;
  return ;
}

int Gitter :: Makrogitter :: iterators_attached () const {
  return AccessIterator < vertex_STI > :: ref + AccessIterator < hedge_STI > :: ref + 
   AccessIterator < hface_STI > :: ref + AccessIterator < helement_STI > :: ref +
   AccessIterator < hbndseg_STI > :: ref ;
}

Gitter :: Makrogitter :: ~Makrogitter () {
  if (iterators_attached())
    cerr << "**WARNING: (IGNORED) There are still iterators attached to the grid, remove them before removal of the grid to avoid errors! in " << __FILE__ << " " << __LINE__ << endl ;
  return ;
}


#endif
