// (c) bernhard schupp 1997 - 1998
// modifications for dune 
// (c) Robert Kloefkorn 2004 - 2005

#ifndef GITTER_MGB_H_INCLUDED
#define GITTER_MGB_H_INCLUDED

#include "key.h"
#include "gitter_sti.h"
#include "ghost_info.h"

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
    // info about ghost element, see ghost_info.h  
    MacroGhostInfoTetra * _ptr;
    // internal face 
    hface3_GEO * _first;
    // twist of face 
    int _second;

  public:  
    // destructor deleting _ptr if not zero 
    ~Hbnd3IntStorage(); 

    // store point and face and twist  
    Hbnd3IntStorage( hface3_GEO * f, int tw, const tetra_GEO * tetra, int fce);
    
    // store point and face and twist  
    Hbnd3IntStorage( hface3_GEO * f, int tw, MacroGhostInfoTetra* p);
    
    // store face and twist and set point to default 
    Hbnd3IntStorage( hface3_GEO * f, int tw ); 

    // release internal MacroGhostInfoTetra pointer
    MacroGhostInfoTetra * release ();
    
    // this two method are just like in pair 
    hface3_GEO * first  () const { return _first;  }
    int          second () const { return _second; }
  };

  // stores a hface4 and the other points needed to build a hexa
  class Hbnd4IntStorage : public MyAlloc 
  {
    // info about ghost element, see ghost_info.h
    MacroGhostInfoHexa * _ptr;
    // internal face 
    hface4_GEO * _first;
    // twist of face 
    int _second;

  public:  
    // destructor deleting _ptr if not zero 
    ~Hbnd4IntStorage (); 

    // store point and face and twist  
    Hbnd4IntStorage( hface4_GEO * f, int tw, const hexa_GEO * hexa, int fce);
    
    // store point and face and twist  
    Hbnd4IntStorage( hface4_GEO * f, int tw, MacroGhostInfoHexa* );
    
    // store face and twist and set point to default 
    Hbnd4IntStorage( hface4_GEO * f, int tw ); 

    // release internal ghost info pointer 
    MacroGhostInfoHexa* release ();

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
    MacroGridBuilder (BuilderIF &, const bool init = true) ;
    MacroGridBuilder (BuilderIF &, Gitter* );
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

    // Vertex projection 
    Gitter* _myGrid;
    ProjectVertex* _ppv; 

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

//- Hbnd3IntStorage 
inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw, const tetra_GEO * tetra, int fce)
 : _ptr(new MacroGhostInfoTetra(tetra,fce))
 , _first(f) , _second(tw)
{
}
    
inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw, MacroGhostInfoTetra *p)
 : _ptr(p) , _first(f) , _second(tw)
{
  assert( _ptr );
}
    
inline MacroGridBuilder :: Hbnd3IntStorage :: 
Hbnd3IntStorage( hface3_GEO * f, int tw )
 : _ptr(0), _first(f) , _second(tw) 
{
}

inline MacroGridBuilder :: Hbnd3IntStorage :: ~Hbnd3IntStorage () 
{
  if( _ptr ) delete _ptr;
}

inline MacroGhostInfoTetra* MacroGridBuilder :: Hbnd3IntStorage :: release ()
{ 
  assert( _ptr );
  MacroGhostInfoTetra* p = _ptr;
  _ptr = 0;
  return p;
}

//- Hbnd4IntStorage 
inline MacroGridBuilder :: Hbnd4IntStorage :: 
Hbnd4IntStorage( hface4_GEO * f, int tw, const hexa_GEO * hexa, int fce)
 : _ptr( new MacroGhostInfoHexa(hexa,fce) ), _first(f) , _second(tw)  
{
}
    
// hface4 storage
inline MacroGridBuilder :: Hbnd4IntStorage :: 
Hbnd4IntStorage( hface4_GEO * f, int tw, MacroGhostInfoHexa* p)
 : _ptr(p) , _first(f) , _second(tw) 
{ 
  assert( _ptr ); 
}
    
inline MacroGridBuilder :: Hbnd4IntStorage :: 
Hbnd4IntStorage( hface4_GEO * f, int tw )
 : _ptr(0) , _first(f) , _second(tw) {}

inline MacroGridBuilder :: Hbnd4IntStorage :: ~Hbnd4IntStorage () 
{
  if( _ptr ) delete _ptr;
}
inline MacroGhostInfoHexa* MacroGridBuilder :: Hbnd4IntStorage :: release() 
{ 
  assert( _ptr );
  MacroGhostInfoHexa* p = _ptr;
  _ptr = 0;
  return p;
}

#endif
