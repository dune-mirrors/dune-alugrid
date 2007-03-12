// (c) Robert Kloefkorn 2007
#ifndef GHOSTINFO_H_INCLUDED
#define GHOSTINFO_H_INCLUDED

#include "gitter_sti.h"
#include "serialize.h"

// interface class for macro ghost point
class MacroGhostInfo : public MyAlloc 
{
  public:
    virtual ~MacroGhostInfo () {}

    virtual const double (& getPoint (int i) const )[3] = 0;
    virtual int nop () const = 0;
    virtual void inlineGhostElement(ObjectStream & os) const = 0; 
};

typedef MacroGhostInfo MacroGhostInfo_STI;

// little storage class for points and vertex numbers 
// of transmitted macro elements to become ghosts 
template <int points>
class MacroGhostInfoStorage : public MacroGhostInfo
{
public:
  // number of points of element
  enum { noVx     = (points == 4) ? 8 : 4 };
  // number of all non-internal points 
  enum { noFaceVx = (points == 4) ? 4 : 1 };  
  
protected:
  // coordiante of all non-internal points 
  double _p[points][3]; 

  // vertex idents of all vertices of element 
  int _vx[noVx];

  // vertex idents of all not internal vertices  
  int _vxface[noFaceVx];

  // face number of internal face 
  int _fce;

  // do not allow copying
  MacroGhostInfoStorage(const MacroGhostInfoStorage & );

  MacroGhostInfoStorage() : _fce(-1) {}
public:  
  // destructor 
  virtual ~MacroGhostInfoStorage () {}

  // return reference to _p
  const double (& getPoints () const )[points][3] 
  { 
    assert( _fce >= 0 );
    return _p; 
  }
  
  // return idents of ghost element  
  int (& vertices () )[noVx]
  {
    assert( _fce >= 0 );
    return _vx; 
  }
  
  // return reference to vector with non-internal vertex idents 
  const int (& getOuterVertices () const )[noFaceVx]
  {
    assert( _fce >= 0 );
    return _vxface;
  }

  // return local number of internal face 
  int internalFace () const 
  {
    assert( _fce >=0 );
    return _fce; 
  }

  /////////////////////////////////////
  // interface of MacroGhostInfo_STI 
  ///////////////////////////////////// 
  virtual const double (& getPoint (int i) const )[3] 
  {
    assert( _fce >=0 );
    assert( i>= 0 && i < points );
    return _p[i];
  }

  // return number of non-internal points 
  virtual int nop () const { return points; }

  // write internal data to stream 
  virtual void inlineGhostElement(ObjectStream&) const;

protected:
  // read internal data from stream 
  void readData(ObjectStream&);
};

// macro ghost info for tetras 
class MacroGhostInfoTetra : public MacroGhostInfoStorage<1> 
{
  enum { points = 1 };
public:  
  // create storage by reading data from stream 
  MacroGhostInfoTetra(ObjectStream& os) { this->readData(os); }

  // contructor for tetras 
  MacroGhostInfoTetra(const Gitter:: Geometric :: tetra_GEO *, 
                      const int fce);
};

// macro ghost info for tetras 
class MacroGhostInfoHexa : public MacroGhostInfoStorage<4> 
{
  enum { points = 4 };
public:  
  // create storage by reading data from stream 
  MacroGhostInfoHexa(ObjectStream& os) { this->readData(os); }

  // contructor for tetras 
  MacroGhostInfoHexa(const Gitter:: Geometric :: hexa_GEO * , 
                     const int fce);
};

template<int points>
inline void MacroGhostInfoStorage<points> :: 
inlineGhostElement(ObjectStream & os ) const
{
 // local face number 
  os.writeObject( _fce );

  // global vertex number of the hexas vertices  
  for(int i=0; i<noVx; ++i) os.writeObject( _vx[i] );
  
  // global vertex numbers of the face not existing on this partition  
  for(int i=0; i<noFaceVx; ++i) 
  {
    os.writeObject( _vxface[i] );
    os.writeObject( _p[i][0] ); 
    os.writeObject( _p[i][1] ); 
    os.writeObject( _p[i][2] ); 
  }
}

template<int points>
inline void MacroGhostInfoStorage<points> :: 
readData(ObjectStream & os ) 
{
  // read local face number
  os.readObject ( _fce );

  // read vertices of element
  for(int i=0; i<noVx; ++i)
  {
    os.readObject ( _vx[i] );
  }

  // read vertices of face an coordinates 
  for(int i=0; i<noFaceVx; ++i)
  {
    os.readObject ( _vxface[i] );
    double (&pr) [3] = _p[i];

    os.readObject (pr[0]) ;
    os.readObject (pr[1]) ;
    os.readObject (pr[2]) ;
  }

  assert( _fce >= 0 );
}

inline MacroGhostInfoHexa :: 
MacroGhostInfoHexa(const Gitter :: Geometric :: hexa_GEO * hexa, 
                   const int fce)
{
  assert( points == this->nop() );
  int oppFace = Gitter :: Geometric :: hexa_GEO :: oppositeFace[fce];
  for(int vx=0; vx<points; vx++)
  {
    const Gitter :: Geometric :: VertexGeo * vertex = hexa->myvertex(oppFace,vx);
    this->_vxface[vx] = vertex->ident();
    const double (&p) [3] = vertex->Point();
    this->_p[vx][0] = p[0];
    this->_p[vx][1] = p[1];
    this->_p[vx][2] = p[2];
  }
  
  for(int i=0; i<noVx; i++) 
  {
    this->_vx[i] = hexa->myvertex(i)->ident();
  }
  this->_fce = fce;
}

inline MacroGhostInfoTetra :: 
MacroGhostInfoTetra(const Gitter :: Geometric :: tetra_GEO * tetra, 
                    const int fce)
{
  assert( points == this->nop() );
  const Gitter :: Geometric :: VertexGeo * vertex = tetra->myvertex(fce);
  assert( vertex );
  for(int vx=0; vx<points; ++vx)
  {
    this->_vxface[vx] = vertex->ident();
    const double (&p) [3] = vertex->Point();
    this->_p[vx][0] = p[0];
    this->_p[vx][1] = p[1];
    this->_p[vx][2] = p[2];
  }
  
  for(int i=0; i<noVx; ++i) 
  {
    this->_vx[i] = tetra->myvertex(i)->ident();
  }

  this->_fce = fce;
}

#endif
