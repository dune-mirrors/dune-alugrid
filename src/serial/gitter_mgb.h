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
class Hbnd4IntStoragePoints : public MyAlloc
{
  double _p[4][3]; 
  int _vx[8];
  int _vxface[4];

  int _fce;
public:  
  Hbnd4IntStoragePoints();
  Hbnd4IntStoragePoints(const Hbnd4IntStoragePoints & copy );
  Hbnd4IntStoragePoints(const Gitter:: Geometric :: hexa_GEO * hexa, int fce);
  Hbnd4IntStoragePoints(const double (&p)[4][3], const int (&vx)[8] , const int (&vxface)[4], int fce );

  // return reference to _p
  const double (& getPoints () const )[4][3];
  
  const int (& getIdents () const )[8];
  const int (& getFaceIdents () const )[4];

  int getFaceNumber () const ;
};

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
    double _p[3]; 
    hface3_GEO * _first;
    int          _second;
    bool _pInit; // true if p was initialized with a value 
  public:  
    // store point and face and twist  
    Hbnd3IntStorage( hface3_GEO * f, int tw, const double (&p) [3] );
    
    // store face and twist and set point to default 
    Hbnd3IntStorage( hface3_GEO * f, int tw ); 

    // return reference to _p
    const double (& getPoint () const )[3];

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

inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw, const double (&p) [3] )
 : _first(f) , _second(tw) , _pInit(true)
{
  for(int i=0; i<3; i++) _p[i] = p[i];
}
    
inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw )
 : _first(f) , _second(tw) , _pInit(false)
{
  for(int i=0; i<3; i++) _p[i] = -666.0;
}

inline const double (& MacroGridBuilder :: Hbnd3IntStorage :: getPoint () const )[3]
{ 
  assert(_pInit);
  return _p;
}

//********************************************************************8
inline Hbnd4IntStoragePoints :: Hbnd4IntStoragePoints ()
{
  for(int vx=0; vx<4; vx++)
  {
    for(int i=0; i<3; i++) _p[vx][i] = -666.0;
    _vxface[vx] = -1;
  }
  for(int i=0; i<8; i++) _vx[i] = -1;
  _fce = -1;
}

inline Hbnd4IntStoragePoints :: 
Hbnd4IntStoragePoints (const Gitter :: Geometric :: hexa_GEO * hexa, int fce)
{
  int oppFace = Gitter :: Geometric :: hexa_GEO :: oppositeFace[fce];
  for(int vx=0; vx<4; vx++)
  {
    const double (&p) [3] = hexa->myvertex(oppFace,vx)->Point();
    for(int j=0; j<3; j++) _p[vx][j] = p[j];
  }
  for(int i=0; i<8; i++) _vx[i] = hexa->myvertex(i)->ident();
  for(int i=0; i<4; i++) _vxface[i] = hexa->myhface4(oppFace)->myvertex(i)->ident();

  _fce = fce;
}

inline Hbnd4IntStoragePoints :: 
Hbnd4IntStoragePoints (const Hbnd4IntStoragePoints & copy ) 
{
  for(int k=0; k<4; k++)
  {
    for(int j=0; j<3; j++) _p[k][j] = copy._p[k][j];
  }
  for(int i=0; i<8; i++) _vx[i] = copy._vx[i];
  for(int i=0; i<4; i++) _vxface[i] = copy._vxface[i];

  _fce = copy._fce;
}

inline Hbnd4IntStoragePoints :: 
Hbnd4IntStoragePoints (const double (&p)[4][3], const int (&vx)[8], 
    const int (&vxface)[4] , int fce ) 
{
  for(int k=0; k<4; k++)
  {
    for(int j=0; j<3; j++) _p[k][j] = p[k][j];
  }
  for(int i=0; i<8; i++) _vx[i] = vx[i];
  for(int i=0; i<4; i++) _vxface[i] = vxface[i];

  _fce = fce;
}

inline const double (& Hbnd4IntStoragePoints :: getPoints() const )[4][3]
{ 
  return _p;
}

inline const int (& Hbnd4IntStoragePoints :: getIdents() const )[8]
{ 
  return _vx;
}

inline const int (& Hbnd4IntStoragePoints :: getFaceIdents() const )[4]
{ 
  return _vxface;
}

inline int Hbnd4IntStoragePoints :: getFaceNumber () const
{ 
  return _fce;
}

// hface4 storage
inline MacroGridBuilder :: Hbnd4IntStorage :: 
Hbnd4IntStorage( hface4_GEO * f, int tw, const hexa_GEO * hexa, int fce)
 : _p(hexa,fce), _first(f) , _second(tw) , _pInit(true) {}
    
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
