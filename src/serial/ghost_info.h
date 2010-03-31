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
  // do not copy 
  MacroGhostInfoTetra(const MacroGhostInfoTetra&);
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
  // no copying 
  MacroGhostInfoHexa(const MacroGhostInfoHexa&);
public:  
  // create storage by reading data from stream 
  MacroGhostInfoHexa(ObjectStream& os) { this->readData(os); }

  // contructor for tetras 
  MacroGhostInfoHexa(const Gitter:: Geometric :: hexa_GEO * , 
                     const int fce);
};

#endif
