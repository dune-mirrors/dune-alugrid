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

class MacroGhostPointTetra : public MacroGhostPoint
{
  protected:
    double _p[3];

  public:
    MacroGhostPointTetra( const double (&p) [3] )
    {
      for(int i=0; i<3; i++) _p[i] = p[i];
    }

    virtual const double (& getPoint (int i) const )[3]
    {
      return _p;
    }

    // write information to stream, used in 
    // BndsegPllBaseXMacroClosure < A > :: packAsBnd
    // gitter_pll_impl.h , line 1481 
    virtual void inlineGhostElement(ObjectStream & os) const
    {
      os.writeObject( _p[0] ); 
      os.writeObject( _p[1] ); 
      os.writeObject( _p[2] ); 
    }; 
    
    virtual int nop () const
    {
      return 1;
    }
};

class MacroGhostPointHexa : public MacroGhostPoint
{
  protected:
    // coords of opp face  
    double _p[4][3];
    // global vertex numbers of hexa
    int _verts[8];
    // global numbers of opposite face 
    int _oppVerts[4]; 
    // local face number 
    int _fce; 

  public:
    MacroGhostPointHexa(const Hbnd4IntStoragePoints & allp) 
    {
      const double (&p)[4][3]  = allp.getPoints();
      const int (&vertices)[8] = allp.getIdents();
      const int (&oppVerts)[4] = allp.getOppFaceIdents();
    
      for(int vx=0; vx<4; ++vx)
      {
        _p[vx][0]     = p[vx][0];
        _p[vx][1]     = p[vx][1];
        _p[vx][2]     = p[vx][2];
        _oppVerts[vx] = oppVerts[vx];
      }

      for(int i=0; i<8; ++i) _verts[i]    = vertices[i];

      _fce = allp.getFaceNumber();
    }

    // get coordinate vector of point i 
    virtual const double (& getPoint (int i) const )[3]
    {
      assert(i >= 0 );
      assert(i < 4);
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
      for(int i=0; i<8; ++i) os.writeObject(_verts[i]);
      
      // global vertex numbers of the face not existing on this partition  
      for(int i=0; i<4; ++i) 
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
      return 4;
    }

    // number of interface face (process boundary face)
    int internalFace () const { return _fce; }
    // reference to the 8 verteices of the hexa 
    int (& vertices () )[8] { return _verts; }
};

class MacroGhostBuilder : public MacroGridBuilder 
{
  typedef Gitter :: Geometric :: hface4_GEO hface4_GEO;
  typedef Gitter :: Geometric :: hexa_GEO GhostElement_t;
  
  typedef Gitter :: Geometric :: BuilderIF BuilderIF;

  public:  
    MacroGhostBuilder (BuilderIF & bi) : MacroGridBuilder(bi,false) 
    {
      // create Builder with empty lists 
      this->_initialized = true;
    }

    ~MacroGhostBuilder () 
    {
      // todo delete all created elements 
      // remove all created elements 
      this->_finalized = true; 
      // if not true then MacroGridBuilder removes
      // elements which leads to error
    }

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
      _edgeMap [key] = h ;
    }

    // insert existing edge into map 
    void InsertExistingHface4 (hface4_GEO * f4)
    {
      int v[4];
      for(int i=0; i<4; ++i) v[i] = f4->myvertex(i)->ident();
      cyclicReorder (v,v+4) ;
      faceKey_t key (v[0],v[1],v[2]) ;
#ifndef NDEBUG
      // f4 should not be in list already 
      faceMap_t :: const_iterator hit = _face4Map.find (key) ;
      assert( hit == _face4Map.end () );
#endif
      _face4Map [key] = f4 ;
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

  typedef Gitter :: Geometric :: tetra_GEO GhostTetra_t;
  typedef Gitter :: Geometric :: VertexGeo VertexGeo;
  typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO;
  typedef Gitter :: Geometric :: hface3_GEO hface3_GEO;
  typedef Gitter :: Geometric :: hbndseg3_GEO hbndseg3_GEO;
  typedef hbndseg3_GEO hbnd_seg;
  
  VertexGeo * _v;
  enum { nedges = 6 };
  hedge1_GEO *(_edges)[nedges];
  enum { nfaces = 4 };
  hface3_GEO *(_faces)[nfaces];
  enum { nhbnd = 3 };
  hbnd_seg  *(_hbnd)[nhbnd];
  GhostTetra_t * _ghost;

  MacroGhostPointTetra _ghPoint;

public:
  MacroGhostTetra (VertexGeo * v, hedge1_GEO *(&edges)[nedges],
          hface3_GEO *(&faces)[nfaces] , hbndseg3_GEO *(&hbnd)[nhbnd] ,
          GhostTetra_t * ghost, const double (&p)[3]) :
          _v(v) , _ghost(ghost) , _ghPoint(p)
  { 
    for(int i=0; i<nedges; i++)
     _edges[i] = edges[i];
    for(int i=0; i<nhbnd; i++)
     _hbnd[i] = static_cast<hbnd_seg *> (hbnd[i]);
    for(int i=0; i<nfaces; i++)
      _faces[i] = faces[i];
  }
  
  ~MacroGhostTetra ()
  { 
    for(int i=nhbnd-1; i>=0; i--)
      if(_hbnd[i]) delete _hbnd[i];
    if(_ghost) delete _ghost;
    for(int i=nfaces-1; i>=0; i--)
      if(_faces[i]) delete _faces[i];
    for(int i=nedges-1; i>=0; i--)
      if(_edges[i]) delete _edges[i];
    if(_v) delete _v;
  }
  
  GhostElement_t * getGhost() { return _ghost; }

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
    typedef Gitter :: Geometric :: VertexGeo VertexGeo;

    const double (&p)[4][3]  = allp.getPoints();
    const int (&vertices)[8] = allp.getIdents();
    const int (&oppVerts)[4] = allp.getOppFaceIdents();

    // we create 8 new points, which are stored in the lists of our
    // internal grid builder 
    for(int i=0; i<4; ++i)
    {
      VertexGeo * vx = face->myvertex(i);
      _mgb.InsertExistingVertex( vx );
    }

    for(int i=0; i<4; ++i)
    {
      hedge1_GEO * h = face->myhedge1(i);
      _mgb.InsertExistingHedge1( h );
    }

    // isnert the face, we need this because we wnat to have the same
    // numbers for the face (Lagrange Elements)
    //logFile << face->getIndex() << " create face \n";
    //_mgb.InsertExistingHface4( face );
    
    for(int i=0; i<4; ++i)
    {
      const double (&px)[3] = p[i];
      _mgb.InsertUniqueVertex(px[0],px[1],px[2],oppVerts[i]);
    }

    // InsertUniqueHexa gets the global vertex numbers 
    _ghost = _mgb.InsertUniqueHexa ( _ghPoint.vertices() ).first ;
    assert( _ghost );

    int v[4];
    for(int i=0; i<6; ++i)
    {
      //if(i == _ghPoint.internalFace()) continue;
      const hface4_GEO * myface = _ghost->myhface4(i); 
      for(int vx = 0; vx<4; ++vx)
        v[vx] = myface->myvertex(vx)->ident();
      _mgb.InsertUniqueHbnd4( v , Gitter :: hbndseg :: ghost_closure );
    }
   
    /*
    logFile << "new hexa is = [";
    for(int i=0 ;i<8; i++) 
    {
      logFile << "(" <<_ghost->myvertex(i)->getIndex() << ", " << _ghost->myvertex(i)->ident() << ") , ";
    }
    logFile << "] \n";
    */

 
#ifndef NDEBUG 
  double vol = (QuadraturCube3D < VolumeCalc > (TrilinearMapping (
      _ghost->myvertex(0)->Point(), 
      _ghost->myvertex(1)->Point(), 
      _ghost->myvertex(2)->Point(), 
      _ghost->myvertex(3)->Point(),
      _ghost->myvertex(4)->Point(), 
      _ghost->myvertex(5)->Point(), 
      _ghost->myvertex(6)->Point(), 
      _ghost->myvertex(7)->Point())).integrate2 (0.0));

    assert( vol > 0.0 );
  
    int count = 0; 
    hface4_GEO * intface = _ghost->myhface4(_ghPoint.internalFace());
    for(int i=0; i<4; i++) 
    {
      if(intface->myvertex(i)->ident() == face->myvertex(i)->ident())
        count ++ ;
    }
    assert(count == 4);
    
    count = 0; 
    int oppFace = hexa_GEO :: oppositeFace[ _ghPoint.internalFace() ];
    //logFile << "created face with twist = "<< _ghost->twist(oppFace) << "\n";
    intface = _ghost->myhface4(oppFace);
    for(int i=0; i<4; i++) 
    {
      int idx = intface->myvertex(i)->ident();
      for( int j=0; j<4 ; j++) 
      {
        if(idx == face->myvertex(j)->ident())
          count ++ ;
      }
    }
    assert(count == 0);
#endif
    //logFile.flush();
  }

  // nothing to do here
  ~MacroGhostHexa () {}
  
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
