// (c) bernhard schupp 1997 - 1998

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

static volatile char RCSId_gitter_mgb_h [] = "$Id$" ;

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
  protected :
  class Hbnd3IntStorage
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

    hface3_GEO * first  () const { return _first;  }
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
    typedef map < faceKey_t,    Hbnd3IntStorage *, less < faceKey_t > > hbndintMap_t ;
    typedef map < elementKey_t, void *, less < elementKey_t > >         elementMap_t ;
  
    vertexMap_t  _vertexMap ;
    edgeMap_t    _edgeMap ;
    
    faceMap_t    _face4Map, _face3Map, _hbnd3Map, _hbnd4Map, _hbnd4Int ;
    hbndintMap_t _hbnd3Int; // new type here, so we dont have to cast to void *
    // todo here: same thing for hbnd4int 
    
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

#endif
