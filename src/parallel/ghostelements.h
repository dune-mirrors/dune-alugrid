// (c) Robert Kloefkorn 2004 - 2005 
#ifndef __GHOSTELEMENTS_INCLUDED__
#define __GHOSTELEMENTS_INCLUDED__

#include "myalloc.h"
#include "gitter_sti.h"

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

// todo: MacroGhostPointHexa

// interface class for macro ghost 
class MacroGhost
{
  typedef Gitter :: helement_STI GhostElement_t;

  public:
    virtual ~MacroGhost () {}
    virtual GhostElement_t * getGhost()  = 0;
    virtual const MacroGhostPoint * getGhostPoints () const = 0;
};
typedef MacroGhost MacroGhost_STI;



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

#endif

#endif
