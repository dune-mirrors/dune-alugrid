// (c) Robert Kloefkorn 2010 

#include "gitter_impl.h"


/////////////////////////////////////////
//  read of data 
/////////////////////////////////////////
void GitterBasis :: Objects :: TetraEmpty :: 
os2VertexData(ObjectStream & os, GatherScatterType & gs , int borderFace )
{
  // only one opposite vertex for tetras 
  gs.setData( os, *myvertex( borderFace ));
}

void GitterBasis :: Objects :: TetraEmpty ::
os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  const vector<int> & edgesNotOnFace = 
    Gitter :: Geometric :: tetra_GEO :: edgesNotOnFace( borderFace );
  const int numEdges = edgesNotOnFace.size();
  assert( numEdges == 3 );
  for(int e = 0; e<numEdges; ++e)
  {
    gs.setData( os, *myhedge1( edgesNotOnFace[e] ) );
  }
}

void GitterBasis :: Objects :: TetraEmpty ::
os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  const vector<int> & facesNotOnFace = 
    Gitter :: Geometric :: tetra_GEO :: facesNotOnFace( borderFace );
  const int numFaces = facesNotOnFace.size();
  assert( numFaces == 3 );
  for (int i = 0; i <numFaces; ++i) 
  {
    gs.setData( os, *myhface3( facesNotOnFace[i] ) );
  }
}

/////////////////////////////////////////
//  writing of data 
/////////////////////////////////////////
void GitterBasis :: Objects :: TetraEmpty ::
VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace )
{
  // only send one vertex
  gs.sendData( os, *myvertex(borderFace) );
}

void GitterBasis :: Objects :: TetraEmpty ::
EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
{
  const vector<int> & edgesNotOnFace = 
    Gitter :: Geometric :: tetra_GEO :: edgesNotOnFace( borderFace );
  const int numEdges = edgesNotOnFace.size();
  assert( numEdges == 3 );
  for(int e=0; e<numEdges; ++e)
  {
    gs.sendData( os, *myhedge1( edgesNotOnFace[e] ) );
  }
}

void GitterBasis :: Objects :: TetraEmpty ::
FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace) 
{
  const vector<int> & facesNotOnFace = 
    Gitter :: Geometric :: tetra_GEO :: facesNotOnFace( borderFace );
  const int numFaces = facesNotOnFace.size();
  assert( numFaces == 3 );
  for (int i = 0; i <numFaces; ++i)
  {
    gs.sendData( os,  *myhface3( facesNotOnFace[i] ) );
  }
}

// declare this element and all parts leaf  
void GitterBasis :: Objects :: TetraEmpty ::
attachleafs() 
{  
  addleaf();
  for (int i = 0; i < 4 ; ++i) myhface3(i)->addleaf();
  for (int i = 0; i < 6 ; ++i) myhedge1(i)->addleaf();
  for (int i = 0; i < 4 ; ++i) myvertex(i)->addleaf();
}

// this element is not leaf anymore 
void GitterBasis :: Objects :: TetraEmpty ::
detachleafs() 
{ 
  removeleaf();
  for (int i = 0; i < 4 ; ++i) myhface3(i)->removeleaf();
  for (int i = 0; i < 6 ; ++i) myhedge1(i)->removeleaf();
  for (int i = 0; i < 4 ; ++i) myvertex(i)->removeleaf();
}

// check that all indices are within range of index manager
void GitterBasis :: Objects :: TetraEmpty ::
resetGhostIndices() 
{  
  // only set indices for macro level ghosts 
  if( this->level() > 0 ) return ;
  
  {
    typedef Gitter :: Geometric :: BuilderIF BuilderIF;
    // only call for ghosts 
    assert( this->isGhost() ); 

    // check my index first 
    resetGhostIndex(_myGrid->indexManager(BuilderIF :: IM_Elements ));
   
    {
      // get index manager of faces 
      IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Faces);
      for (int i = 0; i < 4 ; ++i) myhface3(i)->resetGhostIndex(im);
    }
    {
      // get index manager of edges 
      IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Edges);
      for (int i = 0; i < 6 ; ++i) myhedge1(i)->resetGhostIndex(im);
    }
    {
      // get index manager of vertices 
      IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Vertices);
      for (int i = 0; i < 4 ; ++i) myvertex(i)->resetGhostIndex(im);
    }
  }
}

//ghost tetra gets indices of grid, to which it belongs actually
void GitterBasis :: Objects :: TetraEmpty ::
setIndicesAndBndId (const hface_STI & f, int face_nr) 
{
  // set all items to ghost bnd id 
  setGhostBoundaryIds();
  
  typedef Gitter :: Geometric :: BuilderIF BuilderIF;
    
  typedef Gitter :: Geometric :: vertex_GEO vertex_GEO; 
  typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO; 

  const myhface3_t & face = static_cast<const myhface3_t &> (f); 
  const bndid_t bndid = face.bndId ();

  myhface3_t & myface = *(myhface3(face_nr));

  // set index of face 
  myface.setIndex(_myGrid->indexManager(BuilderIF :: IM_Faces), face.getIndex());
  // set bnd id of face 
  myface.setGhostBndId( bndid );

  IndexManagerType & vxIm = _myGrid->indexManager(BuilderIF :: IM_Vertices);
  IndexManagerType & edIm = _myGrid->indexManager(BuilderIF :: IM_Edges);
  
  for (int i = 0; i < 3; ++i) 
  {
    // make sure we got the right face 
    assert(fabs(myface.myvertex(i)->Point()[0]-
           face.myvertex(i)->Point()[0])<1e-8);
    assert(fabs(myface.myvertex(i)->Point()[1]-
           face.myvertex(i)->Point()[1])<1e-8);
    assert(fabs(myface.myvertex(i)->Point()[2]-
           face.myvertex(i)->Point()[2])<1e-8);

    vertex_GEO * vx = myface.myvertex(i); 
    vx->setIndex( vxIm , face.myvertex(i)->getIndex() );
    vx->setGhostBndId( bndid );
   
    hedge1_GEO * edge = myface.myhedge1(i);
    edge->setIndex( edIm , face.myhedge1(i)->getIndex() );
    edge->setGhostBndId( bndid );
  }
}

//ghost tetra gets indices of grid, to which it belongs actually
void GitterBasis :: Objects :: TetraEmpty ::
setGhostBoundaryIds() 
{
  const bndid_t bndid = Gitter :: hbndseg_STI :: ghost_closure ; 
  
  // value of ghost_closure 
  this->setGhostBndId( bndid );
  for( int i=0; i<4 ; ++i) myhface3(i)->setGhostBndId( bndid );
  for( int i=0; i<6 ; ++i) myhedge1(i)->setGhostBndId( bndid );
  for( int i=0; i<4 ; ++i) myvertex(i)->setGhostBndId( bndid );
}

////////////////////////////////////////////////
// --HexaEmpty  read of data 
////////////////////////////////////////////////
// scatter only on ghosts 
void GitterBasis :: Objects :: HexaEmpty ::
os2VertexData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  const vector<int> & verticesNotOnFace = 
    Gitter :: Geometric :: hexa_GEO :: verticesNotOnFace( borderFace );
  const int numVertices = verticesNotOnFace.size();
  assert( numVertices == 4 );
  for (int i = 0; i <numVertices; ++i) 
  {
    gs.setData( os, *myvertex( verticesNotOnFace[i] ) );
  }
}

// scatter data on ghost edges  
void GitterBasis :: Objects :: HexaEmpty ::
os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  const vector<int> & edgesNotOnFace = 
    Gitter :: Geometric :: hexa_GEO :: edgesNotOnFace( borderFace );
  const int numEdges = edgesNotOnFace.size();
  assert( numEdges == 8 );
  for(int e = 0; e<numEdges; ++e)
  {
    gs.setData( os, *myhedge1( edgesNotOnFace[e] ) );
  }
}

// scatter data on ghost faces 
void GitterBasis :: Objects :: HexaEmpty ::
os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
{
  const vector<int> & facesNotOnFace = 
    Gitter :: Geometric :: hexa_GEO :: facesNotOnFace( borderFace );
  const int numFaces = facesNotOnFace.size();
  assert( numFaces == 5 );
  for (int i = 0; i <numFaces; ++i) 
  {
    gs.setData( os, *myhface4( facesNotOnFace[i] ) );
  }
}

//////////////////////////////////////////
//  writing of data 
//////////////////////////////////////////
void GitterBasis :: Objects :: HexaEmpty ::
VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace )
{
  const vector<int> & verticesNotOnFace = 
    Gitter :: Geometric :: hexa_GEO :: verticesNotOnFace( borderFace );
  const int numVertices = verticesNotOnFace.size();
  assert( numVertices == 4 );
  for (int i = 0; i <numVertices; ++i)
  {
    gs.sendData( os, *myvertex( verticesNotOnFace[i] ) );
  }
}

void GitterBasis :: Objects :: HexaEmpty ::
EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
{
  const vector<int> & edgesNotOnFace = 
    Gitter :: Geometric :: hexa_GEO :: edgesNotOnFace( borderFace );
  const int numEdges = edgesNotOnFace.size();
  assert( numEdges == 8 );
  for(int e=0; e<numEdges; ++e)
  {
    gs.sendData( os, *myhedge1( edgesNotOnFace[e] ) );
  }
}

void GitterBasis :: Objects :: HexaEmpty ::
FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace) 
{
  const vector<int> & facesNotOnFace = 
    Gitter :: Geometric :: hexa_GEO :: facesNotOnFace( borderFace );
  const int numFaces = facesNotOnFace.size();
  assert( numFaces == 5 );
  for (int i = 0; i < numFaces; ++i)
  {
    gs.sendData( os, *myhface4( facesNotOnFace[i] ) );
  }
}

void GitterBasis :: Objects :: HexaEmpty ::
attachleafs() 
{  
  assert(this->leafRefCount()==0);
  addleaf();
  for (int i = 0; i < 6 ; ++i) myhface4(i)->addleaf();
  for (int i = 0; i < 12; ++i) myhedge1(i)->addleaf();
  for (int i = 0; i < 8 ; ++i) myvertex(i)->addleaf();
}

void GitterBasis :: Objects :: HexaEmpty ::
detachleafs() 
{
  assert(this->leafRefCount()==1);
  removeleaf();
  for (int i = 0; i < 6 ; ++i) myhface4(i)->removeleaf();
  for (int i = 0; i < 12; ++i) myhedge1(i)->removeleaf();
  for (int i = 0; i < 8 ; ++i) myvertex(i)->removeleaf();
}

// check that all indices are within range of index manager
void GitterBasis :: Objects :: HexaEmpty ::
resetGhostIndices() 
{  
  // only set indices for macro level ghosts 
  if( this->level() > 0 ) return ;
  
  {
    typedef Gitter :: Geometric :: BuilderIF BuilderIF;
    // only call for ghosts 
    assert( this->isGhost() ); 

    // check my index first 
    resetGhostIndex(_myGrid->indexManager(BuilderIF :: IM_Elements ));
   
    {
      // get index manager of faces 
      IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Faces);
      for (int i = 0; i < 6 ; ++i) myhface4(i)->resetGhostIndex(im);
    }
    {
      // get index manager of edges 
      IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Edges);
      for (int i = 0; i < 12 ; ++i) myhedge1(i)->resetGhostIndex(im);
    }
    {
      // get index manager of vertices 
      IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Vertices);
      for (int i = 0; i < 8 ; ++i) myvertex(i)->resetGhostIndex(im);
    }
  }
}

//ghost hexa gets indices of grid, to which it belongs actually
void GitterBasis :: Objects :: HexaEmpty ::
setIndicesAndBndId (const hface_STI & f, int face_nr)  
{
   // set all items to ghost bnd id
   setGhostBoundaryIds();
  
   typedef Gitter :: Geometric :: BuilderIF BuilderIF;
    
   typedef Gitter :: Geometric :: vertex_GEO vertex_GEO; 
   typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO; 
  
   const myhface4_t & face = static_cast<const myhface4_t &> (f); 
   const bndid_t bndid = face.bndId();
   
   myhface4_t & myface = *(myhface4(face_nr));

   IndexManagerType & vxIm = _myGrid->indexManager(BuilderIF :: IM_Vertices);
   IndexManagerType & edIm = _myGrid->indexManager(BuilderIF :: IM_Edges);

   // set index of face 
   myface.setIndex( _myGrid->indexManager(BuilderIF :: IM_Faces) , face.getIndex ());
   // set bnd id of face 
   myface.setGhostBndId( bndid );
   
   for (int i = 0; i < 4; ++i) 
   {
     // make sure we got the right face 
     assert(fabs(myface.myvertex(i)->Point()[0]-
            face.myvertex(i)->Point()[0])<1e-8);
     assert(fabs(myface.myvertex(i)->Point()[1]-
            face.myvertex(i)->Point()[1])<1e-8);
     assert(fabs(myface.myvertex(i)->Point()[2]-
            face.myvertex(i)->Point()[2])<1e-8);
     
     vertex_GEO * vx = myface.myvertex(i); 
     vx->setIndex(vxIm, face.myvertex(i)->getIndex());
     vx->setGhostBndId( bndid );
     
     hedge1_GEO * edge = myface.myhedge1(i);
     edge->setIndex(edIm, face.myhedge1(i)->getIndex());
     edge->setGhostBndId( bndid );
   }
}


//ghost tetra gets indices of grid, to which it belongs actually
void GitterBasis :: Objects :: HexaEmpty ::
setGhostBoundaryIds() 
{
  const bndid_t bndid = Gitter :: hbndseg_STI :: ghost_closure ; 

  // value of ghost_closure 
  this->setGhostBndId( bndid );
  for( int i=0; i<6 ; ++i) myhface4(i)->setGhostBndId( bndid );
  for( int i=0; i<12; ++i) myhedge1(i)->setGhostBndId( bndid );
  for( int i=0; i<8 ; ++i) myvertex(i)->setGhostBndId( bndid );
}


//////////////////////////////////////////////////////////////////
//
//  --GitterBasisImpl
//
//////////////////////////////////////////////////////////////////
GitterBasisImpl :: GitterBasisImpl () : _macrogitter (0) , _ppv(0) 
{
  _macrogitter = new MacroGitterBasis ( this ) ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

GitterBasisImpl :: GitterBasisImpl (istream & in, ProjectVertex* ppv) 
  : _macrogitter (0) , _ppv( ppv ) 
{
  _macrogitter = new MacroGitterBasis ( this , in) ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

GitterBasisImpl :: GitterBasisImpl (const char * file, 
                                           ProjectVertex* ppv) 
: _macrogitter (0), _ppv( ppv ) 
{
  ifstream in (file) ;
  if (!in) {
    cerr << "  GitterBasisImpl :: GitterBasisImpl (const char *) FEHLER (IGNORIERT) " ;
    cerr << "beim \"Offnen der Datei " << (file ? file : "\"null\"" ) << endl ;
    _macrogitter = new MacroGitterBasis ( this ) ;
  } else {
    _macrogitter = new MacroGitterBasis ( this, in) ;
  }
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

GitterBasisImpl :: ~GitterBasisImpl () {
  delete _macrogitter ;
  return ;
}
GitterBasis :: MacroGitterBasis :: MacroGitterBasis (Gitter * mygrid, istream & in) 
  : _myGrid(mygrid) 
{
  macrogridBuilder (in, _myGrid->vertexProjection() ) ;
  return ;
}

GitterBasis :: MacroGitterBasis :: MacroGitterBasis (Gitter * mygrid) : _myGrid(mygrid) {
  return ;
}

GitterBasis :: VertexGeo * GitterBasis :: MacroGitterBasis :: insert_vertex (double x, double y, double z, int id) {
  return new Objects :: VertexEmptyMacro (x, y, z, id, indexManager(IM_Vertices)) ;
}

GitterBasis :: VertexGeo * GitterBasis :: MacroGitterBasis :: insert_ghostvx (double x, double y, double z, int id) {
  return new Objects :: VertexEmptyMacro (x, y, z, id, indexManager(IM_Vertices)) ;
}

GitterBasis :: hedge1_GEO * GitterBasis :: MacroGitterBasis :: insert_hedge1 (VertexGeo * a, VertexGeo * b) {
  return new Objects :: hedge1_IMPL (0, a, b, indexManager(IM_Edges) ) ;
}

GitterBasis :: hface3_GEO * GitterBasis :: MacroGitterBasis :: insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3]) {
  return new Objects :: hface3_IMPL (0,e[0],s[0],e[1],s[1],e[2],s[2], indexManager(IM_Faces) ) ;
}

GitterBasis :: hface4_GEO * GitterBasis :: MacroGitterBasis :: insert_hface4 (hedge1_GEO *(&e)[4], int (&s)[4]) {
  return new Objects :: hface4_IMPL (0, e[0],s[0],e[1],s[1],e[2],s[2],e[3],s[3], indexManager(IM_Faces) ) ;
}

GitterBasis :: tetra_GEO * GitterBasis :: MacroGitterBasis :: 
insert_tetra (hface3_GEO *(&f)[4], int (&t)[4]) 
{
  return new Objects :: tetra_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3], 
                                    indexManager(IM_Elements) , _myGrid ) ;
}

// inlcudes implementation of MacroGhostTetra and MacroGhostHexa 
#include "ghost_elements.h"

GitterBasis :: periodic3_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic3 (hface3_GEO *(&f)[2], int (&t)[2]) {

  Objects :: periodic3_IMPL * per3 = new Objects :: periodic3_IMPL (0,f[0],t[0],f[1],t[1]) ;
      
  double v[3]; //Verschiebungsvektor (von SW(myhface3(0) zu SW(myhface3(1))
  for (int i = 0; i < 3; i++) {
    v[i] = 0.3333333333*(f[1]->myvertex(0)->Point()[i] +
                         f[1]->myvertex(1)->Point()[i] +
                         f[1]->myvertex(2)->Point()[i] -
                         f[0]->myvertex(0)->Point()[i] -
                         f[0]->myvertex(1)->Point()[i] -
                         f[0]->myvertex(2)->Point()[i]); 
  }

  tetra_GEO * mytetra0 = static_cast<tetra_GEO * > (per3->myneighbour(0).first), //Tetra an Fl 0 (an dieser liegt Ghost0 an)
            * mytetra1 = static_cast<tetra_GEO * > (per3->myneighbour(1).first); //Tetra an Fl 1 ( - " -               1   )

  MacroGhostInfoTetra* allp0 = new MacroGhostInfoTetra(mytetra0, per3->myneighbour(0).second);
  MacroGhostInfoTetra* allp1 = new MacroGhostInfoTetra(mytetra1, per3->myneighbour(1).second);

  MacroGhostTetra * ghost0 = new MacroGhostTetra(*this, allp1, &(*mytetra1), v, -1.0);
  MacroGhostTetra * ghost1 = new MacroGhostTetra(*this, allp0, &(*mytetra0), v,  1.0);
  per3->setGhost(ghost0->getGhost(), 0);
  per3->setGhost(ghost1->getGhost(), 1);
  return per3;    
  //ohne Geister: return new Objects :: periodic3_IMPL (0,f[0],t[0],f[1],t[1]) ;
}

GitterBasis :: periodic4_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic4 (hface4_GEO *(&f)[2], int (&t)[2]) {
  Objects :: periodic4_IMPL * per4 = new Objects :: periodic4_IMPL (0, f [0], t[0], f [1], t[1]) ;
  double v [3]; //Verschiebung von Schwerpunkt(myhface4(0)) zu Schwerpunkt(myhface4(1))
  for (int i = 0; i < 4; i++) {
    v[i] = 0.25*(f[1]->myvertex(0)->Point()[i] +
                 f[1]->myvertex(1)->Point()[i] +
                 f[1]->myvertex(2)->Point()[i] -
                 f[0]->myvertex(0)->Point()[i] -
                 f[0]->myvertex(1)->Point()[i] -
                 f[0]->myvertex(2)->Point()[i]);
  }

  /*
  hexa_GEO * myhexa0 = static_cast<hexa_GEO * > (per4->myneighbour(0).first), //Hexa an Fl 0 (an dieser liegt Ghost0 an)
           * myhexa1 = static_cast<hexa_GEO * > (per4->myneighbour(1).first); //Hexa an Fl 1 ( - " -               1   )

  const Hbnd4IntStoragePoints  allp0 (myhexa0, per4->myneighbour(0).second);
  const Hbnd4IntStoragePoints  allp1 (myhexa1, per4->myneighbour(1).second);
  MacroGhostHexa * ghost0 = new MacroGhostHexa(*this, allp1, &(*myhexa1), v, -1.0);
  MacroGhostHexa * ghost1 = new MacroGhostHexa(*this, allp0, &(*myhexa0), v,  1.0);
  per4->setGhost(ghost0->getGhost(), 0);
  per4->setGhost(ghost1->getGhost(), 1);  
  */
  return per4;   //  return new ObjectsPll :: Periodic4EmptyPllMacro (f [0], t[0], f [1], t[1]) ;  
}

GitterBasis :: hexa_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hexa (hface4_GEO *(&f)[6], int (&t)[6]) 
{
  return new Objects :: hexa_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3],f[4],t[4],f[5],t[5], 
                                   indexManager(IM_Elements), _myGrid ) ;
}

GitterBasis :: hbndseg3_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd3 (hface3_GEO * f, int i, 
              ProjectVertex* ppv, 
              Gitter :: hbndseg_STI :: bnd_t b) 
{
  // the NULL pointer is the pointer to the father which does not exists 
  return new Objects :: hbndseg3_IMPL ( 0, f, i, ppv, b, indexManager(IM_Bnd) ) ;
}

GitterBasis :: hbndseg3_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd3 (hface3_GEO * f, int i, ProjectVertex* ppv, 
              Gitter :: hbndseg_STI :: bnd_t b, MacroGhostInfoTetra* ) 
{
  return insert_hbnd3(f,i,ppv,b); 
}

GitterBasis :: hbndseg4_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd4 (hface4_GEO * f, int i, ProjectVertex* ppv, Gitter :: hbndseg_STI :: bnd_t b) 
{
  return new Objects :: hbndseg4_IMPL ( 0, f, i, ppv, b, indexManager(IM_Bnd) );
}

GitterBasis :: hbndseg4_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd4 (hface4_GEO * f, int i, ProjectVertex* ppv,
              Gitter :: hbndseg_STI :: bnd_t b, MacroGhostInfoHexa* ) 
{
  return insert_hbnd4 (f,i,ppv,b); 
}

void GitterBasisImpl :: printMemUsage ()
{
  typedef GitterBasis :: DuneIndexProvider DuneIndexProvider; 
  typedef GitterBasis :: Objects :: tetra_IMPL tetra_IMPL ; 
  typedef GitterBasis :: Objects :: hexa_IMPL  hexa_IMPL ; 
  typedef GitterBasis :: Objects :: hbndseg3_IMPL hbndseg3_IMPL ; 
  typedef GitterBasis :: Objects :: hbndseg4_IMPL hbndseg4_IMPL ; 
  typedef GitterBasis :: Objects :: hface3_IMPL hface3_IMPL ; 
  typedef GitterBasis :: Objects :: hface4_IMPL hface4_IMPL ; 
  typedef GitterBasis :: Objects :: hedge1_IMPL hedge1_IMPL ; 
  typedef GitterBasis :: Objects :: VertexEmptyMacro VertexEmptyMacro; 
  typedef GitterBasis :: Objects :: VertexEmpty VertexEmpty; 
  typedef Gitter :: Geometric :: VertexGeo VertexGeo; 
  cout << "MyAlloc = " << sizeof(MyAlloc) << "\n";
  cout << "DuneIndexProvider = "<< sizeof(DuneIndexProvider) << "\n";
  cout << "HedgeRule = " << sizeof(Gitter :: Geometric :: Hedge1Rule) <<"\n";
  
  cout << "Tetrasize = " << sizeof(tetra_IMPL) << endl;
  cout << "Hexasize = " << sizeof(hexa_IMPL) << endl;
  cout << "Hface4 = " << sizeof(hface4_IMPL) << endl;
  cout << "Hface3 = " << sizeof(hface3_IMPL) << endl;
  cout << "Hface1 = " << sizeof(hedge1_IMPL) << endl;
  cout << "VertexMacro = " << sizeof(VertexEmptyMacro) << endl;
  cout << "VertexGeo   = " << sizeof(VertexGeo) << endl;
  cout << "Vertex = " << sizeof(VertexEmpty) << endl;
  cout << "Hbnd3  = " << sizeof(hbndseg3_IMPL) << endl;
  cout << "Hbnd4  = " << sizeof(hbndseg4_IMPL) << endl;

  {
    int totalSize = 0; 
    bool simplex = false;
    {
      AccessIterator < helement_STI > :: Handle iter (container ());
      int size = iter.size();
      iter.first(); 
      if( !iter.done() )
      {
        if( iter.item().type() == tetra )
        {
          simplex = true;
          size *= sizeof(tetra_IMPL);
        } 
        else
        {
          size *= sizeof(hexa_IMPL);
        } 
      } 
      totalSize += size;
      cout << "Macro elements: size = " << size/1024/1024 << " MB \n";
    } 
    
    {
      int size = AccessIterator < hbndseg_STI > :: Handle (container ()).size();
      size *= (simplex) ?  sizeof(hbndseg3_IMPL) : sizeof(hbndseg4_IMPL);
      cout << "Macro boundary : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < hface_STI > :: Handle (container ()).size();
      size *= (simplex) ?  sizeof(hface3_IMPL) : sizeof(hface4_IMPL);
      cout << "Macro faces : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < hedge_STI > :: Handle (container ()).size();
      size *= sizeof(hedge1_IMPL);
      cout << "Macro edges : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < vertex_STI > :: Handle (container ()).size();
      size *= sizeof(VertexEmptyMacro);
      cout << "Macro vertices : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    size_t build = container().memUsage();
    cout << "BuilderIF size = " << build/1024/1024 << " MB \n";
    totalSize += build;
    cout << "Overall size = " << totalSize/1024/1024 << " MB \n";
    cout << "\n" ;
  }
}
