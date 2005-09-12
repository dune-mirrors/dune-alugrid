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

    virtual int nop () const
    {
      return 1;
    }
};

class MacroGhostPointHexa : public MacroGhostPoint
{
  protected:
    double _p[4][3];

  public:
    MacroGhostPointHexa( const double (&p)[4][3] )
    {
      for(int vx=0; vx<4; vx++)
        for(int i=0; i<3; i++) _p[vx][i] = p[vx][i];
    }

    virtual const double (& getPoint (int i) const )[3]
    {
      assert(i >= 0 );
      assert(i < 4);
      return _p[i];
    }

    virtual int nop () const
    {
      return 4;
    }
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
};



// interface class for macro ghost 
class MacroGhost
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
  typedef Gitter :: Geometric :: hexa_GEO hexa_GEO;

  typedef Gitter :: Geometric :: BuilderIF BuilderIF;
  typedef hbndseg4_GEO hbnd_seg;
 
  MacroGhostBuilder   _mgb; 
  MacroGhostPointHexa _ghPoint;
  
  hexa_GEO * _ghost;
  
  int _internalFace;
  
public:
  MacroGhostHexa( BuilderIF & bi, const Hbnd4IntStoragePoints & allp, hface4_GEO * face) :
    _mgb(bi) , _ghPoint(allp.getPoints()) , _ghost(0) , _internalFace (-1)
  { 
    typedef Gitter :: Geometric :: VertexGeo VertexGeo;

    const double (&p)[4][3]  = allp.getPoints();
    const int (&vertices)[8] = allp.getIdents();
    const int (&oppVerts)[4] = allp.getFaceIdents();

    int vxface[4];
    for(int i=0; i<4; i++)
    {
      VertexGeo * vx = face->myvertex(i);
      const double (&px)[3] = vx->Point();
      vxface[i] = vx->ident();
      _mgb.InsertUniqueVertex(px[0],px[1],px[2],vx->ident());
    }
    
    for(int i=0; i<4; i++)
    {
      const double (&px)[3] = p[i];
      _mgb.InsertUniqueVertex(px[0],px[1],px[2],oppVerts[i]);
    }

    // InsertUniqueHexa doesn't accept const int & 
    int v[8]; 
    for(int i=0; i<8; i++) v[i] = vertices[i];
    _ghost = _mgb.InsertUniqueHexa (v).first ;
    
  double vol = (QuadraturCube3D < VolumeCalc > (TrilinearMapping (
      _ghost->myvertex(0)->Point(), 
      _ghost->myvertex(1)->Point(), 
      _ghost->myvertex(2)->Point(), 
      _ghost->myvertex(3)->Point(),
      _ghost->myvertex(4)->Point(), 
      _ghost->myvertex(5)->Point(), 
      _ghost->myvertex(6)->Point(), 
      _ghost->myvertex(7)->Point())).integrate2 (0.0));
  
  
    _internalFace = allp.getFaceNumber();
#ifndef NDEBUG 
    int count = 0; 
    hface4_GEO * intface = _ghost->myhface4(_internalFace);
    for(int i=0; i<4; i++) 
    {
      if(intface->myvertex(i)->ident() == face->myvertex(i)->ident())
        count ++ ;
    }
    assert(count == 4);
#endif
    assert( _internalFace >= 0 );

    cout << _internalFace << " int face \n";     
    if( vol < 1e-10 ) 
    {
      cout << "vol  = " << vol << " on p = " << __STATIC_myrank << "\n";
      for(int i=0 ;i<4; i++) 
        cout << "face ident(" << i << ") = " << vxface[i] << "\n";
      for(int i=0 ;i<4; i++) 
        cout << "opp face ident(" << i << ") = " << oppVerts[i] << "\n";

      cout.flush();
      printHexa(cout,_ghost);
    }
  }
  
  ~MacroGhostHexa () {}
  
  GhostElement_t * getGhost() { return _ghost; }

  int ghostFaceNumber () const { return _internalFace; } 

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
