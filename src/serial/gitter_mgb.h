// (c) bernhard schupp 1997 - 1998
// modifications for dune 
// (c) Robert Kloefkorn 2004 - 2005

#ifndef GITTER_MGB_H_INCLUDED
#define GITTER_MGB_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <vector>
  #include <functional>
  #include <utility>
  #include <map>
  #include <algorithm>
#else
  #include <vector.h>
  #include <function.h>
  #include <pair.h>
  #include <map.h>
  #include <algo.h>
#endif

#include "key.h"
#include "gitter_sti.h"

// little storage class for points and vertex numbers 
// of transmitted macro elements to become ghosts 
template <int points>
class HbndIntStoragePoints : public MyAlloc
{
public:
  enum { noVx     = (points == 4) ? 8 : 4 };
  enum { noFaceVx = (points == 4) ? 4 : 1 };  
  
private:
  double _p[points][3]; 
  int _vx[noVx];
  int _vxface[noFaceVx];

  int _fce;
public:  
  HbndIntStoragePoints();
  HbndIntStoragePoints(const HbndIntStoragePoints & copy );
  // constructor for hexas 
  HbndIntStoragePoints(const Gitter:: Geometric :: hexa_GEO * hexa  , int fce);
  // contructor for tetras 
  HbndIntStoragePoints(const Gitter:: Geometric :: tetra_GEO * tetra, int fce);
  HbndIntStoragePoints(const double (&p)[points][3], const int (&vx)[noVx] , const int (&vxface)[noFaceVx], int fce );

  // return reference to _p
  const double (& getPoints () const )[points][3]
  {
    return _p;
  }
  
  const int (& getIdents () const )[noVx]
  {
    return _vx; 
  }
  
  const int (& getOppFaceIdents () const )[noFaceVx]
  {
    return _vxface;
  }

  int getFaceNumber () const 
  {
    return _fce; 
  }
};

typedef HbndIntStoragePoints<4> Hbnd4IntStoragePoints;
typedef HbndIntStoragePoints<1> Hbnd3IntStoragePoints;

template < class RandomAccessIterator > inline int cyclicReorder (RandomAccessIterator begin, RandomAccessIterator end) {
  RandomAccessIterator middle = min_element (begin,end) ;
  int pos = middle == begin ? 0 : (rotate (begin,middle,end), (end - middle)) ;
  if (*(begin + 1) < *(end - 1)) return pos ;
  else {
    reverse (begin,end) ;
    rotate (begin,end - 1,end) ;
    return - pos - 1 ;
  }
}

class MacroGridBuilder : protected Gitter :: Geometric {
  
  protected:  
  // stores a hface3 and the other point needed to build a tetra  
  class Hbnd3IntStorage : public MyAlloc 
  {
    Hbnd3IntStoragePoints _p;
    hface3_GEO * _first;
    int          _second;
    bool _pInit; // true if p was initialized with a value 
  public:  
    // store point and face and twist  
    Hbnd3IntStorage( hface3_GEO * f, int tw, const tetra_GEO * tetra, int fce);
    
    // store point and face and twist  
    Hbnd3IntStorage( hface3_GEO * f, int tw, const Hbnd3IntStoragePoints &p);
    
    // store face and twist and set point to default 
    Hbnd3IntStorage( hface3_GEO * f, int tw ); 

    const Hbnd3IntStoragePoints & getPoints () const;
    
    // this two method are just like in pair 
    hface3_GEO * first  () const { return _first;  }
    int          second () const { return _second; }
  };

  // stores a hface4 and the other points needed to build a hexa
  class Hbnd4IntStorage : public MyAlloc 
  {
    Hbnd4IntStoragePoints _p;
    hface4_GEO * _first;
    int          _second;
    bool _pInit; // true if p was initialized with a value 

  public:  
    // store point and face and twist  
    Hbnd4IntStorage( hface4_GEO * f, int tw, const hexa_GEO * hexa, int fce);
    
    // store point and face and twist  
    Hbnd4IntStorage( hface4_GEO * f, int tw, const Hbnd4IntStoragePoints & p);
    
    // store face and twist and set point to default 
    Hbnd4IntStorage( hface4_GEO * f, int tw ); 

    const Hbnd4IntStoragePoints & getPoints () const;

    // this two method are just like in pair 
    hface4_GEO * first  () const { return _first;  }
    int          second () const { return _second; }
  };

  protected :
    enum ElementRawID {TETRA_RAW=4, HEXA_RAW=8, PERIODIC3_RAW=33, PERIODIC4_RAW=44} ; 
  protected :
    typedef long    vertexKey_t ;
    typedef pair < int, int >   edgeKey_t ;
    typedef Key3 < int >  faceKey_t ;
    typedef Key4 < int >  elementKey_t ;

    typedef map < vertexKey_t , VertexGeo *, less < vertexKey_t > >     vertexMap_t ;
    typedef map < edgeKey_t,    hedge1_GEO *, less < edgeKey_t > >      edgeMap_t ;
    typedef map < faceKey_t,    void *, less < faceKey_t > >            faceMap_t ;
    typedef map < elementKey_t, void *, less < elementKey_t > >         elementMap_t ;
  
    typedef map < faceKey_t, Hbnd3IntStorage *, less < faceKey_t > > hbnd3intMap_t ;
    typedef map < faceKey_t, Hbnd4IntStorage *, less < faceKey_t > > hbnd4intMap_t ;
    
    vertexMap_t  _vertexMap ;
    edgeMap_t    _edgeMap ;
    
    faceMap_t    _face4Map, _face3Map, _hbnd3Map, _hbnd4Map; // _hbnd3Int,_hbnd4Int
    
    // new type here, so we dont have to cast to void *
    hbnd3intMap_t _hbnd3Int;
    hbnd4intMap_t _hbnd4Int; 
    
    elementMap_t _hexaMap, _tetraMap, _periodic3Map, _periodic4Map ;
    
    inline BuilderIF & myBuilder () ;
    inline const BuilderIF & myBuilder () const ;
    void removeElement (const elementKey_t &) ;
  public :
    virtual pair < VertexGeo *, bool >     InsertUniqueVertex (double, double, double, int) ;
    virtual pair < hedge1_GEO *, bool >    InsertUniqueHedge1 (int,int) ;
    virtual pair < hface3_GEO *, bool >    InsertUniqueHface3 (int (&)[3]) ;
    virtual pair < hface4_GEO *, bool >    InsertUniqueHface4 (int (&)[4]) ;
    virtual pair < tetra_GEO *, bool >     InsertUniqueTetra (int (&)[4]) ;
    virtual pair < periodic3_GEO *, bool > InsertUniquePeriodic3 (int (&)[6]) ;

    virtual pair < periodic4_GEO *, bool > InsertUniquePeriodic4 (int (&)[8]) ;
    virtual pair < hexa_GEO *, bool >      InsertUniqueHexa (int (&)[8]) ;
    
    virtual bool InsertUniqueHbnd3 (int (&)[3], Gitter :: hbndseg :: bnd_t) ;
    virtual bool InsertUniqueHbnd4 (int (&)[4], Gitter :: hbndseg :: bnd_t) ;

  public :
    static bool debugOption (int) ;
    static void generateRawHexaImage (istream &, ostream &) ;
    static void generateRawTetraImage (istream &, ostream &) ;
    static void cubeHexaGrid (int, ostream &) ;
    MacroGridBuilder (BuilderIF &, bool init = true) ;
    virtual ~MacroGridBuilder () ;
    void inflateMacroGrid (istream &) ;
    void backupMacroGrid (ostream &) ;

    // former constructor 
    void initialize ();
    // former destructor 
    void finalize ();
  protected:  
    bool _initialized;
    bool _finalized;
  private :
    BuilderIF & _mgb ;
} ;


//
//    #    #    #  #          #    #    #  ######
//    #    ##   #  #          #    ##   #  #
//    #    # #  #  #          #    # #  #  #####
//    #    #  # #  #          #    #  # #  #
//    #    #   ##  #          #    #   ##  #
//    #    #    #  ######     #    #    #  ######
//
inline Gitter :: Geometric :: BuilderIF & MacroGridBuilder :: myBuilder () {
  return _mgb ;
}

inline const Gitter :: Geometric :: BuilderIF & MacroGridBuilder :: myBuilder () const {
  return _mgb ;
}

inline bool MacroGridBuilder :: debugOption (int level) {
  return (getenv ("VERBOSE_MGB") ? ( atoi (getenv ("VERBOSE_MGB")) > level ? true : (level == 0)) : false) ;
}

//********************************************************************8
template <int points> 
inline HbndIntStoragePoints<points> :: HbndIntStoragePoints ()
{
  for(int vx=0; vx<points; vx++)
  {
    for(int i=0; i<3; i++) _p[vx][i] = -666.0;
    _vxface[vx] = -1;
  }
  for(int i=0; i<noVx; i++) _vx[i] = -1;
  _fce = -1;
}

template<int points>
inline HbndIntStoragePoints<points> :: 
HbndIntStoragePoints (const Gitter :: Geometric :: hexa_GEO * hexa, int fce)
{
  // dont call for points == 1
  assert( points == 4 );
  int oppFace = Gitter :: Geometric :: hexa_GEO :: oppositeFace[fce];
  for(int vx=0; vx<points; vx++)
  {
    const Gitter :: Geometric :: VertexGeo * vertex = hexa->myvertex(oppFace,vx);
    _vxface[vx] = vertex->ident();
    const double (&p) [3] = vertex->Point();
    _p[vx][0] = p[0];
    _p[vx][1] = p[1];
    _p[vx][2] = p[2];
  }
  
  for(int i=0; i<noVx; i++) 
  {
    _vx[i] = hexa->myvertex(i)->ident();
  }
  _fce = fce;
}

template<int points>
inline HbndIntStoragePoints<points> :: 
HbndIntStoragePoints (const Gitter :: Geometric :: tetra_GEO * tetra, int fce)
{
  assert( points == 1 );
  const Gitter :: Geometric :: VertexGeo * vertex = tetra->myvertex(fce);
  for(int vx=0; vx<points; vx++)
  {
    _vxface[vx] = vertex->ident();
    const double (&p) [3] = vertex->Point();
    _p[vx][0] = p[0];
    _p[vx][1] = p[1];
    _p[vx][2] = p[2];
  }
  
  for(int i=0; i<noVx; i++) 
  {
    _vx[i] = tetra->myvertex(i)->ident();
  }

  _fce = fce;
}

template <int points>
inline HbndIntStoragePoints<points> :: 
HbndIntStoragePoints (const HbndIntStoragePoints<points> & copy ) 
{
  for(int k=0; k<points; k++)
  {
    _p[k][0] = copy._p[k][0];
    _p[k][1] = copy._p[k][1];
    _p[k][2] = copy._p[k][2];
  }
  for(int i=0; i<noVx; i++) _vx[i] = copy._vx[i];
  for(int i=0; i<noFaceVx; i++) _vxface[i] = copy._vxface[i];

  _fce = copy._fce;
}

template <int points>
inline HbndIntStoragePoints<points> :: 
HbndIntStoragePoints (const double (&p)[points][3], const int (&vx)[noVx], 
    const int (&vxface)[noFaceVx] , int fce ) 
{
  for(int k=0; k<points; ++k)
  {
    _p[k][0]   = p[k][0];
    _p[k][1]   = p[k][1];
    _p[k][2]   = p[k][2];
  }

  for(int k=0; k<noFaceVx; ++k)
  {
    _vxface[k] = vxface[k];
  }

  for(int i=0; i<noVx; ++i) _vx[i] = vx[i];

  _fce = fce;
}

//- Hbnd3IntStorage 
inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw, const tetra_GEO * tetra, int fce)
 : _p(tetra,fce) , _first(f) , _second(tw) , _pInit(true)
{
}
    
inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw, const Hbnd3IntStoragePoints &p)
 : _p(p) , _first(f) , _second(tw) , _pInit(true)
{
}
    
inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw )
 : _p(), _first(f) , _second(tw) , _pInit(false)
{
}

inline const Hbnd3IntStoragePoints & MacroGridBuilder :: Hbnd3IntStorage :: getPoints () const
{ 
  assert(_pInit);
  return _p;
}

//- Hbnd4IntStorage 
inline MacroGridBuilder :: Hbnd4IntStorage :: 
Hbnd4IntStorage( hface4_GEO * f, int tw, const hexa_GEO * hexa, int fce)
 : _p(hexa,fce), _first(f) , _second(tw) , _pInit(true) 
 {
 }
    
// hface4 storage
inline MacroGridBuilder :: Hbnd4IntStorage :: 
Hbnd4IntStorage( hface4_GEO * f, int tw, const Hbnd4IntStoragePoints & p)
 : _p(p) , _first(f) , _second(tw) , _pInit(true) {}
    
inline MacroGridBuilder :: Hbnd4IntStorage :: 
Hbnd4IntStorage( hface4_GEO * f, int tw )
 : _p() , _first(f) , _second(tw) , _pInit(false) {}

inline const Hbnd4IntStoragePoints & MacroGridBuilder :: Hbnd4IntStorage :: getPoints() const
{ 
  assert(_pInit);
  return _p;
}

#endif
