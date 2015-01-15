// (c) Robert Kloefkorn 2004 - 2005
#ifndef GHOSTELEMENTS_H_INCLUDED
#define GHOSTELEMENTS_H_INCLUDED

#include "myalloc.h"
#include "ghost_info.h"
#include "gitter_sti.h"
#include "gitter_mgb.h"

namespace ALUGrid
{

  // macro ghost builder to construct macro ghost
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

    // do not copy
    MacroGhostBuilder(const MacroGhostBuilder& );
  public:
    // constructor
    MacroGhostBuilder (BuilderIF & bi);

    // desctructor
    ~MacroGhostBuilder ();

    // insert new Vertex without linkagePattern
    bool InsertNewUniqueVertex (double x, double y, double z, int i) ;

    // empty all lists
    void finalize () ;
  };

  // interface class for macro ghost
  class MacroGhost : public MyAlloc
  {
    typedef Gitter :: ghostpair_STI ghostpair_STI;

    public:
      virtual ~MacroGhost () {}
      virtual ghostpair_STI getGhost() = 0;
      virtual const MacroGhostInfo_STI * getGhostInfo () const = 0;
      virtual int ghostFaceNumber () const { return 0; }
  };
  // interface typedef
  typedef MacroGhost MacroGhost_STI;

  class MacroGhostTetra : public MacroGhost
  {
    typedef Gitter :: helement_STI GhostElement_t;
    typedef Gitter :: ghostpair_STI ghostpair_STI;
    typedef Gitter :: Geometric :: BuilderIF BuilderIF;

    typedef Gitter :: Geometric :: tetra_GEO  GhostTetra_t;
    typedef Gitter :: Geometric :: tetra_GEO  tetra_GEO ;
    typedef Gitter :: Geometric :: hface3_GEO hface3_GEO;
    typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO;
    typedef Gitter :: Geometric :: VertexGeo  VertexGeo;

    // store info about ghost element as pointer
    MacroGhostInfoTetra * _ghInfoPtr;

    // ghost pair
    ghostpair_STI _ghostPair;

    // copy constructor (prohibited)
    MacroGhostTetra(const MacroGhostTetra& );
  public:
    MacroGhostTetra( BuilderIF & bi,
                     MacroGhostInfoTetra * allp,
                     const hface3_GEO * face);

    //alternative Konstruktor fuer die Geister, die an Periodischen
    //Raendern haengen
    //sign = +/- 1  und ist dafuer da, um den Vektor
    //nicht mit -1 durchmultiplizieren zu muessen fuer anderen Geist
    MacroGhostTetra( BuilderIF & bi, MacroGhostInfoTetra * allp,
        Gitter::Geometric::tetra_GEO * orig, alucoord_t (&vec)[3] , double sign) ;

    // desctructor deleting ghost element and _ghInforPtr
    virtual ~MacroGhostTetra () ;

    ghostpair_STI getGhost()
    {
      alugrid_assert (_ghostPair.first);
      return _ghostPair;
    }

    // return local number of fake face
    int ghostFaceNumber () const
    {
      alugrid_assert (_ghostPair.second >= 0);
      return _ghostPair.second;
    }

    // for storage in PllClosure Elements, if packed, we need the point
    const MacroGhostInfo_STI * getGhostInfo () const
    {
      alugrid_assert ( _ghInfoPtr );
      return _ghInfoPtr;
    }
  };

  // todo: MacroGhostHexa
  class MacroGhostHexa : public MacroGhost
  {
    typedef Gitter :: helement_STI GhostElement_t;
    typedef Gitter :: ghostpair_STI ghostpair_STI;

    typedef Gitter :: Geometric :: VertexGeo VertexGeo;
    typedef Gitter :: Geometric :: hface4_GEO hface4_GEO;
    typedef Gitter :: Geometric :: hbndseg4_GEO hbndseg4_GEO;
    typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO;
    typedef Gitter :: Geometric :: hexa_GEO hexa_GEO;

    typedef Gitter :: Geometric :: BuilderIF BuilderIF;
    typedef hbndseg4_GEO hbnd_seg;

    // pointer to ghost info
    MacroGhostInfoHexa* _ghInfoPtr;

    // ghost pair
    ghostpair_STI _ghostPair;

    // copy constructor (prohibited)
    MacroGhostHexa (const MacroGhostHexa& );
  public:
    // constructor
    MacroGhostHexa( BuilderIF & bi,
                    MacroGhostInfoHexa* allp, const hface4_GEO * face);

    // desctructor deleting ghost element and _ghInforPtr
    virtual ~MacroGhostHexa () ;

    ghostpair_STI getGhost()
    {
      alugrid_assert ( _ghostPair.first );
      return _ghostPair;
    }

    // return local number of fake face
    int ghostFaceNumber () const
    {
      alugrid_assert ( _ghostPair.second >= 0 );
      return _ghostPair.second;
    }

    // for storage in PllClosure Elements, if packed, we need the point
    const MacroGhostInfo_STI * getGhostInfo () const
    {
      alugrid_assert ( _ghInfoPtr );
      return _ghInfoPtr;
    }
  };

} // namespace ALUGrid

#endif // #ifndef GHOSTELEMENTS_H_INCLUDED
