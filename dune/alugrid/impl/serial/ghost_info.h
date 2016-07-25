// (c) Robert Kloefkorn 2007
#ifndef GHOSTINFO_H_INCLUDED
#define GHOSTINFO_H_INCLUDED

#include "gitter_sti.h"
#include "serialize.h"

namespace ALUGrid
{
  // interface class for macro ghost point
  class MacroGhostInfo
  : public MyAlloc
  {
    public:
      virtual ~MacroGhostInfo () {}

      virtual const alucoord_t (& getPoint (int i) const )[3] = 0;
      virtual int nop () const = 0;
      virtual void inlineGhostElement(ObjectStream & os) const = 0;
      virtual SimplexTypeFlag simplexTypeFlag () const { return SimplexTypeFlag(); }

      void print( std::ostream& os );
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

    static const signed char invalidFace = -111;

  protected:
    // coordiante of all non-internal points
    alucoord_t _p[points][3];  // 24 or 96 bytes

    // global vertex ids (ident) of all vertices of element
    int _vx[noVx]; // 16 or 32 bytes

    // vertex idents of all not internal vertices
    int _vxface[noFaceVx]; // 4 or 16 bytes

    // face number of internal face
    signed char _fce; // 1 byte

    // simplex flag info for bisection
    SimplexTypeFlag _simplexTypeFlag; // 1 byte

    // do not allow copying
    MacroGhostInfoStorage(const MacroGhostInfoStorage & );

    MacroGhostInfoStorage() : _fce( invalidFace ), _simplexTypeFlag() {}
  public:
    // destructor
    virtual ~MacroGhostInfoStorage () {}

    // return reference to _p
    const alucoord_t (& getPoints () const )[points][3]
    {
      alugrid_assert ( _fce != invalidFace );
      return _p;
    }

    // return idents of ghost element
    int (& vertices () )[noVx]
    {
      alugrid_assert ( _fce != invalidFace );
      return _vx;
    }

    // return reference to vector with non-internal vertex idents
    const int (& getOuterVertices () const )[noFaceVx]
    {
      alugrid_assert ( _fce != invalidFace );
      return _vxface;
    }

    // return local number of internal face
    int internalFace () const
    {
      alugrid_assert ( _fce != invalidFace );
      return _fce < 0 ? (-_fce) - 1 : _fce;
    }

    // int orientation () const = 0; // { return _fce < 0 ? 1 : 0; }
    SimplexTypeFlag simplexTypeFlag () const { return _simplexTypeFlag; }

    /////////////////////////////////////
    // interface of MacroGhostInfo_STI
    /////////////////////////////////////
    virtual const alucoord_t (& getPoint (int i) const )[3]
    {
      alugrid_assert ( _fce != invalidFace );
      alugrid_assert ( i>= 0 && i < points );
      return _p[i];
    }

    // return number of non-internal points
    virtual int nop () const { return points; }

  protected:
    // write internal data to stream
    void doInlineGhostElement(ObjectStream&) const;

    // read internal data from stream
    void doReadData(ObjectStream&);
  };


  // macro ghost info for tetras
  class MacroGhostInfoTetra
  : public MacroGhostInfoStorage< 1 >
  {
    typedef MacroGhostInfoStorage< 1 > BaseType;
    enum { points = 1 };
    // do not copy
    MacroGhostInfoTetra(const MacroGhostInfoTetra&);
  public:
    // create storage by reading data from stream
    explicit MacroGhostInfoTetra(ObjectStream& os)
    {
      readData(os);
    }

    // constructor for tetras
    MacroGhostInfoTetra(const Gitter:: Geometric :: tetra_GEO *,
                        const int fce);

    // write ghost information to stream
    static void writeGhostInfo( ObjectStream& os,
                                const int fce,
                                const Gitter:: Geometric :: tetra_GEO& );

    // write internal data to stream
    virtual void inlineGhostElement(ObjectStream& os) const
    {
      BaseType::doInlineGhostElement( os );
      // additionally write bisection type flag
      _simplexTypeFlag.write( os );
    };

  protected:
    // read internal data from stream
    void readData(ObjectStream& os)
    {
      BaseType::doReadData( os );
      // read bisection type flag
      _simplexTypeFlag.read( os );
    }
  };

  // macro ghost info for tetras
  class MacroGhostInfoHexa
  : public MacroGhostInfoStorage< 4 >
  {
  protected:
    typedef MacroGhostInfoStorage< 4 > BaseType;
    enum { points = 4 };
    // no copying
    MacroGhostInfoHexa(const MacroGhostInfoHexa&);

    // read internal data from stream
    void readData(ObjectStream& os)
    {
      BaseType::doReadData( os );
    }

  public:
    // create storage by reading data from stream
    explicit MacroGhostInfoHexa(ObjectStream& os)
    {
      readData(os);
    }

    // constructor for tetras
    MacroGhostInfoHexa(const Gitter:: Geometric :: hexa_GEO * ,
                       const int fce);

    // write ghost information to stream
    static void writeGhostInfo( ObjectStream& os,
                                const int fce,
                                const Gitter:: Geometric :: hexa_GEO& );

    // write internal data to stream
    virtual void inlineGhostElement(ObjectStream& os) const
    {
      BaseType::doInlineGhostElement( os );
    }
  };

} // namespace ALUGrid

#endif // #ifndef GHOSTINFO_H_INCLUDED
