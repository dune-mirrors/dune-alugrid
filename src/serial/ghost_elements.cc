// (c) Robert Kloefkorn 2004 - 2005 
#include "ghost_elements.h"

MacroGhostBuilder :: MacroGhostBuilder (BuilderIF & bi) : MacroGridBuilder(bi,(bool) false) 
{
  // create Builder with empty lists 
  this->_initialized = true;
}

// desctructor 
MacroGhostBuilder ::  ~MacroGhostBuilder () 
{
  // remove all faces that already exist from the lists 
  {  
    typedef edgeMap_t :: iterator iterator;
    iterator end = _existingEdge.end();
    for (iterator i = _existingEdge.begin () ; i != end ; ++i )
    {
      edgeKey_t e = (*i).first;
      this->_edgeMap.erase(e);
    }
  }
  
  // remove all faces that already exist from the lists 
  {  
    typedef vertexMap_t :: iterator iterator;
    iterator end = _existingVertex.end();
    for (iterator i = _existingVertex.begin () ; i != end ; ++i )
    {
      vertexKey_t v = (*i).first;
      this->_vertexMap.erase(v);
    }
  }

  finalize();
}

// insert new Vertex without linkagePattern 
void MacroGhostBuilder :: 
InsertNewUniqueVertex (double x, double y, double z, int i) 
{
  typedef GitterBasis :: MacroGitterBasis SerialMacroGridBuilder_t;
  vertexMap_t :: const_iterator hit = this->_vertexMap.find (i) ;
  if (hit == _vertexMap.end ()) 
  {
    VertexGeo * v = myBuilder ().insert_ghostvx (x,y,z,i) ;
    this->_vertexMap [i] = v ;
  }
}

// delete all elementes and stuff 
void MacroGhostBuilder :: 
finalize ()
{
  { 
    typedef elementMap_t :: iterator iterator;
    iterator end = this->_hexaMap.end ();
    for (iterator i = this->_hexaMap.begin () ; 
         i != end; this->_hexaMap.erase(i++)) 
    {  
      delete ((hexa_GEO *)(*i).second);
    }
  } 
  { 
    typedef elementMap_t :: iterator iterator;
    iterator end = this->_tetraMap.end ();
    for (iterator i = this->_tetraMap.begin () ; 
         i != end; this->_tetraMap.erase(i++)) 
    {
      delete ((tetra_GEO *)(*i).second);
    }
  } 
  
  assert( this->_hbnd3Int.empty ());
  assert( this->_hbnd4Int.empty ());

  assert( this->_hbnd3Map.empty ());
  assert( this->_hbnd4Map.empty ());

  // faces 
  {
    typedef faceMap_t :: iterator iterator;
    iterator end = this->_face4Map.end ();
    for (iterator i = this->_face4Map.begin () ; 
         i != end; this->_face4Map.erase(i++)) 
    {
      delete ((hface4_GEO *)(*i).second); 
    }
  } 
  {
    typedef faceMap_t :: iterator iterator;
    iterator end = this->_face3Map.end ();
    for (iterator i = this->_face3Map.begin () ; 
         i != end; this->_face3Map.erase(i++)) 
    {
      delete (hface3_GEO *) (*i).second; 
    }
  } 

  {
    typedef edgeMap_t :: iterator iterator;
    iterator end = this->_edgeMap.end ();
    for (iterator i = this->_edgeMap.begin () ; 
         i != end; this->_edgeMap.erase(i++)) 
    {
      delete (hedge1_GEO *) (*i).second; 
    }
  } 

  {
    typedef vertexMap_t :: iterator iterator;
    iterator end = this->_vertexMap.end ();
    for (iterator i = this->_vertexMap.begin () ; 
         i != end; this->_vertexMap.erase(i++)) 
    {
      delete (VertexGeo *) (*i).second; 
    }
  } 
  this->_finalized = true;
}

////////////////////////////////////////////////////////////////
//
//  --MacroGhostTetra
//
////////////////////////////////////////////////////////////////

MacroGhostTetra ::
MacroGhostTetra( BuilderIF & bi, 
                 MacroGhostInfoTetra * allp, 
                 const hface3_GEO * face) :
  _mgb(bi), 
  _ghInfoPtr( allp ), 
  _ghostPair( (GhostElement_t *)0 , -1) 
{ 
  MacroGhostBuilder & mgb = _mgb;
  MacroGhostInfoTetra& ghInfo = *_ghInfoPtr;

  typedef Gitter :: Geometric :: VertexGeo VertexGeo;
  typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO;

  const double (&p)[1][3]  = ghInfo.getPoints();
  const int (&oppVerts)[1] = ghInfo.getOuterVertices();

  // here all entities have to be created new, because otherwise 
  // the index generation will fail 
  for(int i=0; i<3; ++i)
  {
    const VertexGeo * vx = face->myvertex(i);
#ifndef NDEBUG
    int idx = vx->ident();
    bool found = false;
    for(int j=0; j<4; ++j) 
      if(ghInfo.vertices()[j] == idx) found = true;
    assert( found );
#endif
    const double (&point)[3] = vx->Point();
    mgb.InsertNewUniqueVertex(point[0],point[1],point[2],vx->ident());
  }

#ifndef NDEBUG 
  int idx = oppVerts[0];
  bool found = false;
  for(int j=0; j<4; ++j) 
    if( ghInfo.vertices()[j] == idx) found = true;
  assert( found );
#endif
  const double (&px)[3] = p[0];
  mgb.InsertNewUniqueVertex(px[0],px[1],px[2],oppVerts[0]);

  // InsertUniqueHexa gets the global vertex numbers 
  GhostTetra_t * ghost = mgb.InsertUniqueTetra ( ghInfo.vertices() ).first ;

  // set ghost and number 
  _ghostPair.first = ghost;
  assert( _ghostPair.first );
  _ghostPair.second = ghInfo.internalFace(); 
  assert( _ghostPair.second >= 0 );

  // NOTE: we do not insert boundary faces, because we don't need them
  // here. This is ok because of the hasFaceEmpty class (gitter_sti.h) 
  // which acts as empty boundary. 
}

//alternative Konstruktor fuer die Geister, die an Periodischen
//Raendern haengen
//sign = +/- 1  und ist dafuer da, um den Vektor 
//nicht mit -1 durchmultiplizieren zu muessen fuer anderen Geist
MacroGhostTetra :: 
MacroGhostTetra( BuilderIF & bi, MacroGhostInfoTetra * allp, 
    Gitter::Geometric::tetra_GEO * orig, double (&vec)[3] , double sign) :
  _mgb(bi), 
  _ghInfoPtr(allp), 
  _ghostPair( (GhostElement_t *)0, -1)
{
  MacroGhostInfoTetra& ghInfo = *_ghInfoPtr; 

  MacroGhostBuilder & mgb = _mgb;
  for (int i = 0; i < 4; i++) {
    mgb.InsertNewUniqueVertex(orig->myvertex(i)->Point()[0] + sign*vec[0],
                              orig->myvertex(i)->Point()[1] + sign*vec[1],
                              orig->myvertex(i)->Point()[2] + sign*vec[2],
                              orig->myvertex(i)->ident()   );
  }

  GhostTetra_t * ghost = mgb.InsertUniqueTetra ( ghInfo.vertices() ).first ;
  _ghostPair.first = ghost;
  assert( _ghostPair.first );
  _ghostPair.second = ghInfo.internalFace(); 
  assert( _ghostPair.second >= 0 );
  
  // NOTE: we do not insert boundary faces, because we don't need them
  // here. This is ok because of the hasFaceEmpty class (gitter_sti.h) 
  // which acts as empty boundary. 
}
  
// desctructor deleting _ghInforPtr
MacroGhostTetra :: ~MacroGhostTetra () 
{
  assert( _ghInfoPtr );
  delete _ghInfoPtr;
}

///////////////////////////////////////////////////////////////////
//
//  --MacroGhostHexa
//
///////////////////////////////////////////////////////////////////

// constructor 
MacroGhostHexa :: 
MacroGhostHexa( BuilderIF & bi, MacroGhostInfoHexa* allp, const hface4_GEO * face) :
  _mgb(bi), 
  _ghInfoPtr(allp), 
  _ghostPair( (GhostElement_t *)0 , -1) 
{ 
  MacroGhostBuilder & mgb = _mgb;
  MacroGhostInfoHexa& ghInfo = *_ghInfoPtr;
  
  typedef Gitter :: Geometric :: VertexGeo VertexGeo;

  const double (&p)[4][3]  = ghInfo.getPoints();
  const int (&oppVerts)[4] = ghInfo.getOuterVertices();

  // here all entities have to be created new, because otherwise 
  // the index generation will fail 
  for(int i=0; i<4; ++i)
  {
    const VertexGeo * vx = face->myvertex(i);
    const double (&p)[3] = vx->Point();
    mgb.InsertNewUniqueVertex(p[0],p[1],p[2],vx->ident());
  }

  // isnert the face, we need this because we wnat to have the same
  // numbers for the face (Lagrange Elements)
  for(int i=0; i<4; ++i)
  {
    const double (&px)[3] = p[i];
    mgb.InsertNewUniqueVertex(px[0],px[1],px[2],oppVerts[i]);
  }

  // InsertUniqueHexa gets the global vertex numbers 
  hexa_GEO * ghost = mgb.InsertUniqueHexa ( ghInfo.vertices() ).first ;
  assert( ghost );

  // set ghost values 
  _ghostPair.first  = ghost;
  _ghostPair.second = ghInfo.internalFace();

  // NOTE: we do not insert boundary faces, because we don't need them
  // here. This is ok because of the hasFaceEmpty class (gitter_sti.h) 
  // which acts as empty boundary. 
}

MacroGhostHexa ::~MacroGhostHexa () 
{
  assert( _ghInfoPtr );
  delete _ghInfoPtr;
}

// instantiation 
class MacroGhostBuilder ;
class MacroGhostTetra ;
class MacroGhostHexa ;
