// (c) Robert Kloefkorn 2004 - 2005 
#ifndef __GHOSTELEMENTS_INCLUDED__
#define __GHOSTELEMENTS_INCLUDED__

#include "myalloc.h"
#include "gitter_sti.h"
#include "gitter_mgb.h"

#define __DEBUG_GHOST_ELEMENTS__ 

// interface class for macro ghost point
class MacroGhostPoint : public MyAlloc 
{
  public:
    virtual ~MacroGhostPoint () {}

    virtual const double (& getPoint (int i) const )[3] = 0;
    virtual int nop () const = 0;
    virtual void inlineGhostElement(ObjectStream & os) const = 0; 
};
typedef MacroGhostPoint MacroGhostPoint_STI;

template <int points> 
class MacroGhostPointImpl : public MacroGhostPoint
{
    enum { noVx     = HbndIntStoragePoints<points> :: noVx  };
    enum { noFaceVx = HbndIntStoragePoints<points> :: noFaceVx  };
  protected:
    // coords of opp face  
    double _p[points][3];
    // global vertex numbers of hexa
    int _verts[noVx];
    // global numbers of opposite face 
    int _oppVerts[noFaceVx]; 
    // local face number 
    int _fce; 

  public:
    MacroGhostPointImpl(const HbndIntStoragePoints<points> & allp) 
    {
      const double (&p)[points][3]  = allp.getPoints();
      const int (&vertices)[noVx] = allp.getIdents();
      const int (&oppVerts)[noFaceVx] = allp.getOppFaceIdents();
    
      assert(noFaceVx == points);

      for(int vx=0; vx<noFaceVx; ++vx)
      {
        _p[vx][0]     = p[vx][0];
        _p[vx][1]     = p[vx][1];
        _p[vx][2]     = p[vx][2];
        _oppVerts[vx] = oppVerts[vx];
      }

      for(int i=0; i<noVx; ++i) _verts[i]    = vertices[i];

      _fce = allp.getFaceNumber();
    }

    // get coordinate vector of point i 
    virtual const double (& getPoint (int i) const )[3]
    {
      assert(i >= 0 );
      assert(i < noVx);
      return _p[i];
    }

    // write information to stream, used in 
    // BndsegPllBaseXMacroClosure < A > :: packAsBnd
    // gitter_pll_impl.h , line 1481 
    // check also gitter_pll_impl.cc packAsBnd of hexa  
    virtual void inlineGhostElement(ObjectStream & os) const 
    {
      // local face number 
      os.writeObject( _fce );

      // global vertex number of the hexas vertices  
      for(int i=0; i<noVx; ++i) os.writeObject(_verts[i]);
      
      // global vertex numbers of the face not existing on this partition  
      for(int i=0; i<points; ++i) 
      {
        os.writeObject(_oppVerts[i]);
        os.writeObject( _p[i][0] ); 
        os.writeObject( _p[i][1] ); 
        os.writeObject( _p[i][2] ); 
      }
    }; 
   
    // number of coordinates stored 
    virtual int nop () const
    {
      return points;
    }

    // number of interface face (process boundary face)
    int internalFace () const { return _fce; }
    // reference to the 8 verteices of the hexa 
    int (& vertices () )[noVx] { return _verts; }
};
typedef MacroGhostPointImpl<4> MacroGhostPointHexa;
typedef MacroGhostPointImpl<1> MacroGhostPointTetra;


class MacroGhostBuilder : public MacroGridBuilder 
{
  typedef Gitter :: Geometric :: hface4_GEO hface4_GEO;
  typedef Gitter :: Geometric :: hexa_GEO GhostElement_t;
  
  typedef Gitter :: Geometric :: BuilderIF BuilderIF;
  
  typedef  MacroGridBuilder :: vertexMap_t   vertexMap_t; 
  typedef  MacroGridBuilder :: edgeMap_t     edgeMap_t; 
  typedef  MacroGridBuilder :: faceMap_t     faceMap_t; 
  typedef  MacroGridBuilder :: elementMap_t  elementMap_t; 
  typedef  MacroGridBuilder :: hbnd3intMap_t hbnd3intMap_t; 
  typedef  MacroGridBuilder :: hbnd4intMap_t hbnd4intMap_t; 
  typedef  MacroGridBuilder :: faceMap_t    hbnd3Map_t; 
  typedef  MacroGridBuilder :: faceMap_t    hbnd4Map_t; 

  vertexMap_t _existingVertex;   
  edgeMap_t   _existingEdge;

  public:  
    MacroGhostBuilder (BuilderIF & bi) : MacroGridBuilder(bi,false) 
    {
      // create Builder with empty lists 
      this->_initialized = true;
    }

    ~MacroGhostBuilder () 
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
    void InsertNewUniqueVertex (double x, double y, double z, int i) 
    {
      typedef GitterBasis :: MacroGitterBasis SerialMacroGridBuilder_t;
      vertexMap_t :: const_iterator hit = this->_vertexMap.find (i) ;
      if (hit == _vertexMap.end ()) 
      {
        VertexGeo * v = myBuilder ().insert_ghostvx (x,y,z,i) ;
        this->_vertexMap [i] = v ;
      }
    }

    /*
    // insert existing Vertex to vertex map
    // this means, that already exsisting vertices are not created again
    // this is important for Lagrange Elements etc.
    void InsertExistingVertex (VertexGeo * v ) 
    {
      int i = v->ident();
#ifndef NDEBUG
      vertexMap_t :: const_iterator hit = _vertexMap.find (i) ;
      assert( hit == _vertexMap.end () );
#endif
      _existingVertex[i] = v;
      this->_vertexMap [i] = v ;
    }

    // insert already existing edge into the list of this grid builder 
    void InsertExistingHedge1 (hedge1_GEO * h) 
    {
      int l = h->myvertex(0)->ident();
      int r = h->myvertex(1)->ident();
     
      // swap is neccessary
      assert( l < r );
      edgeKey_t key (l,r) ;
#ifndef NDEBUG
      edgeMap_t :: const_iterator hit = _edgeMap.find (key) ;
      assert( hit == _edgeMap.end () ); 
      vertexMap_t :: const_iterator a = _vertexMap.find (l), b = _vertexMap.find (r), end = _vertexMap.end () ;
      assert( a != end );
      assert( b != end ); 
#endif
      _existingEdge[key] = h;
      _edgeMap [key] = h ;
    }
    */

    // delete all elementes and stuff 
    void finalize ()
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
      
      { 
        typedef hbnd3intMap_t :: iterator iterator;
        iterator end = this->_hbnd3Int.end ();
        for (iterator i = this->_hbnd3Int.begin () ; 
             i != end; this->_hbnd3Int.erase(i++)) 
          delete ((hbndseg3_GEO *)(*i).second);
      } 

      { 
        typedef hbnd4intMap_t :: iterator iterator;
        iterator end = this->_hbnd4Int.end ();
        for (iterator i = this->_hbnd4Int.begin () ; 
             i != end; this->_hbnd4Int.erase(i++)) 
          delete ((hbndseg4_GEO *)(*i).second);
      } 

      { 
        typedef hbnd3Map_t :: iterator iterator;
        iterator end = this->_hbnd3Map.end ();
        for (iterator i = this->_hbnd3Map.begin () ; 
             i != end; this->_hbnd3Map.erase(i++)) 
          delete ((hbndseg3_GEO *)(*i).second);
      } 

      { 
        typedef hbnd4Map_t :: iterator iterator;
        iterator end = this->_hbnd4Map.end ();
        for (iterator i = this->_hbnd4Map.begin () ; 
             i != end; this->_hbnd4Map.erase(i++)) 
        {
          delete ((hbndseg4_GEO *)(*i).second);
        }
      } 

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
};

// interface class for macro ghost 
class MacroGhost : public MyAlloc
{
  typedef Gitter :: helement_STI GhostElement_t;

  public:
    virtual ~MacroGhost () {}
    virtual GhostElement_t * getGhost()  = 0;
    virtual const MacroGhostPoint * getGhostPoints () const = 0;
    virtual int ghostFaceNumber () const { return 0; } 
};
typedef MacroGhost MacroGhost_STI;

extern void printHexa(ostream & os , Gitter :: Geometric :: hexa_GEO * item_);

class MacroGhostTetra : public MacroGhost
{
  typedef Gitter :: helement_STI GhostElement_t;
  typedef Gitter :: Geometric :: BuilderIF BuilderIF;

  typedef Gitter :: Geometric :: tetra_GEO GhostTetra_t;
  typedef Gitter :: Geometric :: hface3_GEO hface3_GEO;
  
  MacroGhostBuilder    _mgb; 
  MacroGhostPointTetra _ghPoint;

  GhostTetra_t * _ghost;
public:
  MacroGhostTetra( BuilderIF & bi, const Hbnd3IntStoragePoints & allp, hface3_GEO * face) :
    _mgb(bi) , _ghPoint(allp) , _ghost(0) 
  { 
    MacroGhostBuilder & mgb = _mgb;

    typedef Gitter :: Geometric :: VertexGeo VertexGeo;
    typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO;

    const double (&p)[1][3]  = allp.getPoints();
    const int (&oppVerts)[1] = allp.getOppFaceIdents();

    // here all entities have to be created new, because otherwise 
    // the index generation will fail 
    for(int i=0; i<3; ++i)
    {
      const VertexGeo * vx = face->myvertex(i);
      const double (&p)[3] = vx->Point();
      mgb.InsertNewUniqueVertex(p[0],p[1],p[2],vx->ident());
    }

    const double (&px)[3] = p[0];
    mgb.InsertNewUniqueVertex(px[0],px[1],px[2],oppVerts[0]);

    // InsertUniqueHexa gets the global vertex numbers 
    _ghost = mgb.InsertUniqueTetra ( _ghPoint.vertices() ).first ;
    assert( _ghost );

    int v[3];
    for(int i=0; i<4; ++i)
    {
      const hface3_GEO * myface = _ghost->myhface3(i); 
      for(int vx = 0; vx<3; ++vx) 
        v[vx] = myface->myvertex(vx)->ident();

      mgb.InsertUniqueHbnd3( v , Gitter :: hbndseg :: ghost_closure );
      assert( myface->ref == 2 );
    }
   
#ifndef NDEBUG
    LinearMapping lm (
       _ghost->myvertex(0)->Point(),
       _ghost->myvertex(1)->Point(),
       _ghost->myvertex(2)->Point(),
       _ghost->myvertex(3)->Point()
       );
    assert( lm.det() > 0.0 );
#endif

  }
  
  ~MacroGhostTetra () {
  }
  
  GhostElement_t * getGhost() {
    assert(_ghost); 
    return _ghost; 
  }

  // return local number of fake face 
  int ghostFaceNumber () const { return _ghPoint.internalFace(); } 

  // for storage in PllClosure Elements, if packed, we need the point 
  const MacroGhostPoint * getGhostPoints () const
  {
    return &_ghPoint;
  }
};


// todo: MacroGhostHexa
class MacroGhostHexa : public MacroGhost
{
  typedef Gitter :: helement_STI GhostElement_t;
  typedef Gitter :: Geometric :: VertexGeo VertexGeo;
  typedef Gitter :: Geometric :: hface4_GEO hface4_GEO;
  typedef Gitter :: Geometric :: hbndseg4_GEO hbndseg4_GEO;
  typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO;
  typedef Gitter :: Geometric :: hexa_GEO hexa_GEO;

  typedef Gitter :: Geometric :: BuilderIF BuilderIF;
  typedef hbndseg4_GEO hbnd_seg;
 
  MacroGhostBuilder   _mgb; 
  MacroGhostPointHexa _ghPoint;
  
  hexa_GEO * _ghost;
  
public:
  MacroGhostHexa( BuilderIF & bi, const Hbnd4IntStoragePoints & allp, hface4_GEO * face) :
    _mgb(bi) , _ghPoint(allp) , _ghost(0) 
  { 
    MacroGhostBuilder & mgb = _mgb;
    
    typedef Gitter :: Geometric :: VertexGeo VertexGeo;

    const double (&p)[4][3]  = allp.getPoints();
    const int (&oppVerts)[4] = allp.getOppFaceIdents();

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
    _ghost = mgb.InsertUniqueHexa ( _ghPoint.vertices() ).first ;
    assert( _ghost );

    int v[4];
    for(int i=0; i<6; ++i)
    {
      const hface4_GEO * myface = _ghost->myhface4(i); 
      for(int vx = 0; vx<4; ++vx)
        v[vx] = myface->myvertex(vx)->ident();
      mgb.InsertUniqueHbnd4( v , Gitter :: hbndseg :: ghost_closure );

      assert( myface->ref == 2 );
    }
  }

  // nothing to do here
  ~MacroGhostHexa () {
  }
  
  GhostElement_t * getGhost() { return _ghost; }

  // return local number of fake face 
  int ghostFaceNumber () const { return _ghPoint.internalFace(); } 

  // for storage in PllClosure Elements, if packed, we need the point 
  const MacroGhostPoint * getGhostPoints () const
  {
    return &_ghPoint;
  }
};

#if 0
static void printFace(ostream & os , Gitter :: Geometric :: hface3_GEO * face, int fce )
{
  os << "Face " << fce << endl;
  for(int i=0; i<3; i++)
  {
    os << "Vertex " << i << " = ";
    for(int j=0; j<3; j++)
      os << face->myvertex(i)->Point()[j] << " ";
    os << endl;
  }
  for(int e=0; e<3; e++)
  {
    os << "Edge " << e << "  | ";
    for(int j=0; j<3; j++)
      os << face->myhedge1(e)->myvertex(0)->Point()[j] << " ";
    os << " connect to ";
    for(int j=0; j<3; j++)
      os << face->myhedge1(e)->myvertex(1)->Point()[j] << " ";
    os << endl;
  }
  return ;
}

static void printFace4(ostream & os , Gitter :: Geometric :: hface4_GEO * face, int fce)
{
  os << "Face " << fce << endl;
  for(int i=0; i<4; i++)
  {
    os << "Vertex " << i << " = ";
    for(int j=0; j<3; j++)
      os << face->myvertex(i)->Point()[j] << " ";
    os << endl;
  }
  
  for(int e=0; e<4; e++)
  {
    os << "Edge " << e << "  | ";
    for(int j=0; j<3; j++)
      os << face->myhedge1(e)->myvertex(0)->Point()[j] << " ";
    os << " connect to ";
    for(int j=0; j<3; j++)
      os << face->myhedge1(e)->myvertex(1)->Point()[j] << " ";
    os << endl;
  }

  os<< "**********************************\n";
  return ;
}

static void printTetra(ostream & os , Gitter :: Geometric :: tetra_GEO * item_ )
{
  for(int i=0; i<4; i++)
  {
    os << "Vertex " << i << " = ";
    for(int j=0; j<3; j++)
      os << item_->myvertex(i)->Point()[j] << " ";
    os << endl;
  }

  for(int i=0; i<4; i++)
  {
    os << "Face " << i << " with twist " << item_->twist(i) << endl;
    os << "Edge twists = ";
    for(int j=0; j<3; j++)
      os << item_->myhface3(i)->twist(j) << " " ;
    os << endl;
    os << "Face Vx = ";
    for(int j=0; j<3; j++)
    {
      os << " [";
      for(int k=0; k<3; k++)
        os << item_->myhface3(i)->myvertex(j)->Point()[k] << "," ;
      os << "] ";
    }
    os << endl;
  }

  typedef Gitter :: Geometric :: hface3_GEO hface3_GEO;
  for(int fce=0; fce<4; fce++)
  {
    hface3_GEO * face = item_->myhface3(fce);
    os << "Face " << fce << endl;
    for(int e=0; e<3; e++)
    {
      os << "Edge " << e << "  | ";
      for(int j=0; j<3; j++)
        os << face->myhedge1(e)->myvertex(0)->Point()[j] << " ";
      os << " connect to ";
      for(int j=0; j<3; j++)
        os << face->myhedge1(e)->myvertex(1)->Point()[j] << " ";
      os << endl;
    }
  }

  LinearMapping lm (
      item_->myvertex(0)->Point(),
      item_->myvertex(1)->Point(),
      item_->myvertex(2)->Point(),
      item_->myvertex(3)->Point()
      );
  cout << "det = " << lm.det() << "\n";

  return ;
}

inline void printHexa(ostream & os , Gitter :: Geometric :: hexa_GEO * item_)
{
  for(int i=0; i<8; i++)
  {
    os << "Vertex " << i << " = [";
    for(int j=0; j<2; j++)
      os << item_->myvertex(i)->Point()[j] << ",";
    os << item_->myvertex(i)->Point()[2] << "] \n";
  }

  for(int i=0; i<6; i++)
  {
    os << "Face " << i << " with twist " << item_->twist(i) << endl;
    os << "Edge twists = ";
    for(int j=0; j<4; j++)
      os << item_->myhface4(i)->twist(j) << " " ;
    os << endl;
    os << "Face Vx = ";
    Gitter :: Geometric ::hface4_GEO * aface = item_->myhface4(i);
    for(int j=0; j<4; j++)
    {
      os << " [";
      const double (&p)[3] = aface->myvertex(j)->Point();
      for(int k=0; k<2; k++) os << p[k] << "," ;
      os << p[2] << "] ";
    }
    os << endl;
  }

  typedef Gitter :: Geometric :: hface4_GEO hface4_GEO;
  for(int fce=0; fce<6; fce++)
  {
    hface4_GEO * face = item_->myhface4(fce);
    os << "Face " << fce << endl;
    for(int e=0; e<4; e++)
    {
      os << "Edge " << e << "  | ";
      for(int j=0; j<3; j++)
        os << face->myhedge1(e)->myvertex(0)->Point()[j] << " ";
      os << " connect to ";
      for(int j=0; j<3; j++)
        os << face->myhedge1(e)->myvertex(1)->Point()[j] << " ";
      os << endl;
    }
  }

  double vol = (QuadraturCube3D < VolumeCalc > (TrilinearMapping (
      item_->myvertex(0)->Point(), 
      item_->myvertex(1)->Point(), 
      item_->myvertex(2)->Point(), 
      item_->myvertex(3)->Point(),
      item_->myvertex(4)->Point(), 
      item_->myvertex(5)->Point(), 
      item_->myvertex(6)->Point(), 
      item_->myvertex(7)->Point())).integrate2 (0.0));
  
  cout << "vol  = " << vol << "\n";
  cout.flush();
  //assert( vol > 1e-14 );
  /*
  LinearMapping lm (
      item_->myvertex(0)->Point(),
      item_->myvertex(1)->Point(),
      item_->myvertex(2)->Point(),
      item_->myvertex(3)->Point()
      );
  cout << "det = " << lm.det() << "\n";
  */
  
  return ;
}
#endif


#endif
