	// (c) bernhard schupp 1997 - 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$
	
/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:39:52  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.1  2004/10/15 09:48:37  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.7  2002/05/24 09:05:31  dedner
 * Vorl"aufig syntaktisch korrekte, d.h. kompilierbare Version
 *
 * Revision 1.6  2002/05/23 16:37:41  dedner
 * Test nach Einbau der Periodischen 4-Raender
 *
 * Revision 1.5  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.4  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

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
    enum ElementRawID {TETRA_RAW=4, HEXA_RAW=8, PERIODIC3_RAW=33, PERIODIC4_RAW=44} ; 
  protected :
    typedef long 		vertexKey_t ;
    typedef pair < int, int > 	edgeKey_t ;
    typedef Key3 < int > 	faceKey_t ;
    typedef Key4 < int > 	elementKey_t ;

    typedef map < vertexKey_t , VertexGeo *, less < vertexKey_t > > 	vertexMap_t ;
    typedef map < edgeKey_t, hedge1_GEO *, less < edgeKey_t > >      	edgeMap_t ;
    typedef map < faceKey_t, void *, less < faceKey_t > >            	faceMap_t ;
    typedef map < elementKey_t, void *, less < elementKey_t > >      	elementMap_t ;
  
    vertexMap_t  _vertexMap ;
    edgeMap_t    _edgeMap ;
    faceMap_t    _face4Map, _face3Map, _hbnd3Map, _hbnd3Int, _hbnd4Map, _hbnd4Int ;
// Anfang - Neu am 23.5.02 (BS)
    elementMap_t _hexaMap, _tetraMap, _periodic3Map, _periodic4Map ;
// Ende - Neu am 23.5.02 (BS)

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
// Anfang - Neu am 23.5.02 (BS)
    virtual pair < periodic4_GEO *, bool > InsertUniquePeriodic4 (int (&)[8]) ;
// Ende - Neu am 23.5.02 (BS)
    virtual pair < hexa_GEO *, bool >      InsertUniqueHexa (int (&)[8]) ;
    virtual bool InsertUniqueHbnd3 (int (&)[3], Gitter :: hbndseg :: bnd_t) ;
    virtual bool InsertUniqueHbnd4 (int (&)[4], Gitter :: hbndseg :: bnd_t) ;
//    virtual void removeHexa (int (&)[8]) ;
//    virtual void removePeriodic3 (int (&)[6]) ;
// Anfang - Neu am 23.5.02 (BS)
//    virtual void removePeriodic4 (int (&)[8]) ;
// Ende - Neu am 23.5.02 (BS)
//    virtual void removeTetra (int (&)[4]) ;
  public :
    static bool debugOption (int) ;
    static void generateRawHexaImage (istream &, ostream &) ;
    static void generateRawTetraImage (istream &, ostream &) ;
    static void cubeHexaGrid (int, ostream &) ;
    MacroGridBuilder (BuilderIF &) ;
    virtual ~MacroGridBuilder () ;
    void inflateMacroGrid (istream &) ;
    void backupMacroGrid (ostream &) ;
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

#endif
