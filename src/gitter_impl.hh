//  Version f"ur DUNE
//
	// (c) bernhard schupp 1997 - 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$
	
/* $Id$
 * $Log$
 * Revision 1.1  2004/10/15 09:48:37  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.11  2002/05/23 16:37:41  dedner
 * Test nach Einbau der Periodischen 4-Raender
 *
 * Revision 1.10  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.9  2002/02/25 07:44:47  dedner
 * Atmosphaere als konstanter Wert auf Flaeche gespeichert
 *
 * Revision 1.7  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef GITTER_IMPL_H_INCLUDED
#define GITTER_IMPL_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <fstream>
  #include <vector>
  #include <utility>
#else
  #include <fstream.h>
  #include <vector.h>
  #include <pair.h>
#endif

#include "gitter_sti.hh"
#include "gitter_hexa_top.hh"
#include "gitter_tetra_top.hh"

static volatile char RCSId_gitter_impl_h [] = "$Id$" ;

class GitterBasis : public virtual Gitter, public Gitter :: Geometric {
  public :
    class Objects {
      public :
        class VertexEmpty : public VertexGeo {
          public :
            inline VertexEmpty (int, double, double, double) ;
	   ~VertexEmpty () {}
            virtual inline int ident () const ;
        } ;

        class VertexEmptyMacro : public VertexEmpty {
          public :
            inline VertexEmptyMacro (double, double, double, int) ;
	   ~VertexEmptyMacro () {}
            virtual inline int ident () const ;
          private :
            int _idn ;
        } ;

        class Hbnd3Default : public hbndseg3_GEO {
          protected :
            inline Hbnd3Default (myhface3_t *, int, ProjectVertex *) ;
	   ~Hbnd3Default () {}
          public :
            virtual inline bnd_t bndtype () const ;
	} ;
	typedef Hbnd3Top < Hbnd3Default > hbndseg3_IMPL ;

        class Hbnd4Default : public hbndseg4_GEO {
          protected :
            inline Hbnd4Default (myhface4_t *, int, ProjectVertex *) ;
	   ~Hbnd4Default () {}
          public :
            virtual inline bnd_t bndtype () const ;
	} ;
        typedef Hbnd4Top < Hbnd4Default > hbndseg4_IMPL ;

        class Hedge1Empty : public hedge1_GEO {
          protected :
            typedef VertexEmpty innervertex_t ;
            inline Hedge1Empty (myvertex_t *,myvertex_t *) ;
	   ~Hedge1Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectInnerVertex(const ProjectVertex &pv) ; 
        } ;
	
        typedef Hedge1Top < Hedge1Empty > hedge1_IMPL ;

        class Hface3Empty : public hface3_GEO {
          protected :
            typedef VertexEmpty 	innervertex_t ;
            typedef hedge1_IMPL		inneredge_t ;
            inline Hface3Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int) ;
	   ~Hface3Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertex &pv) ; 
	} ;
        typedef Hface3Top < Hface3Empty > hface3_IMPL ;

        class Hface4Empty : public hface4_GEO {
          protected :
            typedef VertexEmpty innervertex_t ;
            typedef hedge1_IMPL     inneredge_t ;
            inline Hface4Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int,myhedge1_t *,int) ;
	   ~Hface4Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertex &pv) ; 
        } ;
        typedef Hface4Top < Hface4Empty > hface4_IMPL ;

        class TetraEmpty : public tetra_GEO {
	  protected :
            typedef hface3_IMPL innerface_t ;
            typedef hedge1_IMPL inneredge_t ;
            typedef VertexEmpty innervertex_t ;
            inline TetraEmpty (myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int) ;
           ~TetraEmpty () {}
	  public :
	} ;
	typedef TetraTop < TetraEmpty > tetra_IMPL ;
	
	class Periodic3Empty : public periodic3_GEO {
	  protected :
            typedef hface3_IMPL innerface_t ;
            typedef hedge1_IMPL inneredge_t ;
            typedef VertexEmpty innervertex_t ;
	    inline Periodic3Empty (myhface3_t *,int,myhface3_t *,int) ;
	   ~Periodic3Empty () {}
	  public:
	} ;
	typedef Periodic3Top < Periodic3Empty > periodic3_IMPL ;

// Anfang - Neu am 23.5.02 (BS)
	class Periodic4Empty : public periodic4_GEO {
	  protected :
            typedef hface4_IMPL innerface_t ;
            typedef hedge1_IMPL inneredge_t ;
            typedef VertexEmpty innervertex_t ;
	    inline Periodic4Empty (myhface4_t *,int,myhface4_t *,int) ;
	   ~Periodic4Empty () {}
	  public:
	} ;
	typedef Periodic4Top < Periodic4Empty > periodic4_IMPL ;
// Ende - Neu am 23.5.02 (BS)

        class HexaEmpty : public hexa_GEO {
          protected :
            typedef hface4_IMPL innerface_t ;
            typedef hedge1_IMPL inneredge_t ;
            typedef VertexEmpty innervertex_t ;
            inline HexaEmpty (myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int) ;
           ~HexaEmpty () {}
        } ;
        typedef HexaTop < HexaEmpty > hexa_IMPL ;
    } ;
  public :
    class MacroGitterBasis : public virtual BuilderIF {
      protected :
        virtual inline VertexGeo     * insert_vertex (double, double, double, int,int = 0) ;
        virtual inline hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) ;
	virtual inline hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) ;
        virtual inline hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) ;
	virtual inline hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter :: hbndseg_STI :: bnd_t) ;
        virtual inline hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, Gitter :: hbndseg_STI :: bnd_t) ;
	virtual inline tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) ;
	virtual inline periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) ;
// Anfang - Neu am 23.5.02 (BS)
	virtual inline periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) ;
// Ende - Neu am 23.5.02 (BS)
        virtual inline hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) ;
      public :
        inline MacroGitterBasis (istream &) ;
        inline MacroGitterBasis () ;
      	virtual ~MacroGitterBasis () {}

        IndexManagerType & indexManager(int codim) 
        { 
          assert((codim >= 0) && (codim < 4));
          return _indexmanager[codim];
        }
        
      private: 
        // index provider, for every codim one 
        IndexManagerType _indexmanager[4];
    } ;
} ;

class GitterBasisImpl : public GitterBasis {
  MacroGitterBasis * _macrogitter ;
  public:
   //us fuer Globalmethode levelwalk
  inline Makrogitter & container () ;
  inline const Makrogitter & container () const ;
  public :
     IndexManagerType & indexManager(int codim) 
     { 
       return _macrogitter->indexManager(codim);
     }
        
    inline GitterBasisImpl () ;
    inline GitterBasisImpl (istream &) ;
    inline GitterBasisImpl (const char *) ;
    inline ~GitterBasisImpl () ;
} ;


	//
	//    #    #    #  #          #    #    #  ######
	//    #    ##   #  #          #    ##   #  #
	//    #    # #  #  #          #    # #  #  #####
	//    #    #  # #  #          #    #  # #  #
	//    #    #   ##  #          #    #   ##  #
	//    #    #    #  ######     #    #    #  ######
	//


inline GitterBasis :: Objects :: VertexEmpty :: VertexEmpty (int l, double x, double y, double z)
  : GitterBasis :: VertexGeo (l,x,y,z) {
  return ;
}

inline int GitterBasis :: Objects :: VertexEmpty :: ident () const {
  cerr << "**FEHLER (FATAL) vertex :: ident () nur f\"ur level-0 Vertices zul\"assig " << endl ;
  return (abort (), -1) ;
}

inline GitterBasis :: Objects :: VertexEmptyMacro :: VertexEmptyMacro (double x,double y,double z,int i) 
	: GitterBasis :: Objects :: VertexEmpty (0,x,y,z), _idn (i) {
  return ;
}

inline int GitterBasis :: Objects :: VertexEmptyMacro :: ident () const {
  return _idn ;
}

inline GitterBasis :: Objects :: Hedge1Empty :: Hedge1Empty (myvertex_t * a, myvertex_t * b) 
  : Gitter :: Geometric :: hedge1_GEO (a,b) {
  return ;
}

inline void GitterBasis :: Objects :: Hedge1Empty :: projectInnerVertex(const ProjectVertex &pv) {
  if (innerVertex()) {
    assert(!leaf());
    innerVertex()->project(pv);
  }
}

inline GitterBasis :: Objects :: Hface3Empty :: Hface3Empty (myhedge1_t *e0, int s0, 
  myhedge1_t *e1, int s1, myhedge1_t *e2, int s2) : Gitter :: Geometric :: hface3_GEO (e0, s0, e1, s1, e2, s2) {
  return ;
}

inline void GitterBasis :: Objects :: Hface3Empty :: projectVertex(const ProjectVertex &pv) {
  assert(!leaf());
  for (int e = 0; e < polygonlength; e++)
    myhedge1(e)->projectInnerVertex(pv);
  if (innerVertex())
    innerVertex()->project(pv);
}

inline GitterBasis :: Objects :: Hface4Empty :: Hface4Empty (myhedge1_t *e0, int s0, 
  myhedge1_t *e1, int s1, myhedge1_t *e2, int s2, myhedge1_t *e3, int s3)
  : Gitter :: Geometric :: hface4_GEO (e0, s0, e1, s1, e2, s2, e3, s3) {
  return ;
}

inline void GitterBasis :: Objects :: Hface4Empty :: projectVertex(const ProjectVertex &pv) {
  for (int e = 0; e < polygonlength; e++)
    myhedge1(e)->projectInnerVertex(pv);
  if (innerVertex())
    innerVertex()->project(pv);
}

inline GitterBasis :: Objects :: Hbnd3Default :: Hbnd3Default (myhface3_t * f, int i, ProjectVertex *ppv) : Gitter :: Geometric :: hbndseg3_GEO (f, i, ppv) {
  return ;
}

inline Gitter :: hbndseg_STI :: bnd_t GitterBasis :: Objects :: Hbnd3Default :: bndtype () const {
  return undefined ;
}

inline GitterBasis :: Objects :: Hbnd4Default :: Hbnd4Default (myhface4_t * f, int i, ProjectVertex *ppv) : Gitter :: Geometric :: hbndseg4_GEO (f, i,ppv) {
  return ;
}

inline Gitter :: hbndseg_STI :: bnd_t GitterBasis :: Objects :: Hbnd4Default :: bndtype () const {
  return undefined ;
}

inline GitterBasis :: Objects :: TetraEmpty :: TetraEmpty (myhface3_t * f0, int t0, myhface3_t * f1, int t1,
  myhface3_t * f2, int t2, myhface3_t * f3, int t3) : Gitter :: Geometric :: Tetra (f0, t0, f1, t1, f2, t2, f3, t3) {
  return ;
}

inline GitterBasis :: Objects :: Periodic3Empty :: Periodic3Empty (myhface3_t * f0, int t0, myhface3_t * f1, int t1) 
  : Gitter :: Geometric :: Periodic3 (f0, t0, f1, t1) {
  return ;
}

// Anfang - Neu am 23.5.02 (BS)
inline GitterBasis :: Objects :: Periodic4Empty :: Periodic4Empty (myhface4_t * f0, int t0, myhface4_t * f1, int t1) 
  : Gitter :: Geometric :: Periodic4 (f0, t0, f1, t1) {
  return ;
}
// Ende - Neu am 23.5.02 (BS)

inline GitterBasis :: Objects :: HexaEmpty :: HexaEmpty (myhface4_t * f0, int t0, myhface4_t * f1, int t1,
  myhface4_t * f2, int t2, myhface4_t * f3, int t3, myhface4_t * f4, int t4, myhface4_t * f5, int t5)
  : Gitter :: Geometric :: hexa_GEO (f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5) {
  return ;
}

inline GitterBasisImpl :: GitterBasisImpl () : _macrogitter (0) {
  _macrogitter = new MacroGitterBasis () ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

inline GitterBasisImpl :: GitterBasisImpl (istream & in) : _macrogitter (0) {
  _macrogitter = new MacroGitterBasis (in) ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

inline GitterBasisImpl :: GitterBasisImpl (const char * file) : _macrogitter (0) {
  ifstream in (file) ;
  if (!in) {
    cerr << "  GitterBasisImpl :: GitterBasisImpl (const char *) FEHLER (IGNORIERT) " ;
    cerr << "beim \"Offnen der Datei " << (file ? file : "\"null\"" ) << endl ;
    _macrogitter = new MacroGitterBasis () ;
  } else {
    _macrogitter = new MacroGitterBasis (in) ;
  }
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

inline GitterBasisImpl :: ~GitterBasisImpl () {
  delete _macrogitter ;
  return ;
}

inline Gitter :: Makrogitter & GitterBasisImpl :: container () {
  return * _macrogitter ;
}

inline const Gitter :: Makrogitter & GitterBasisImpl :: container () const {
  return * _macrogitter ;
}

inline GitterBasis :: MacroGitterBasis :: MacroGitterBasis (istream & in) {
  macrogridBuilder (in) ;
  return ;
}

inline GitterBasis :: MacroGitterBasis :: MacroGitterBasis () {
  return ;
}

inline GitterBasis :: VertexGeo * GitterBasis :: MacroGitterBasis :: insert_vertex (double x, double y, double z, int id,int) {
  return new Objects :: VertexEmptyMacro (x, y, z, id) ;
}

inline GitterBasis :: hedge1_GEO * GitterBasis :: MacroGitterBasis :: insert_hedge1 (VertexGeo * a, GitterBasis :: VertexGeo * b) {
  return new Objects :: hedge1_IMPL (0, a, b) ;
}

inline GitterBasis :: hface3_GEO * GitterBasis :: MacroGitterBasis :: insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3]) {
  return new Objects :: hface3_IMPL (0,e[0],s[0],e[1],s[1],e[2],s[2]) ;
}

inline GitterBasis :: hface4_GEO * GitterBasis :: MacroGitterBasis :: insert_hface4 (hedge1_GEO *(&e)[4], int (&s)[4]) {
  return new Objects :: hface4_IMPL (0, e[0],s[0],e[1],s[1],e[2],s[2],e[3],s[3]) ;
}

inline GitterBasis :: tetra_GEO * GitterBasis :: MacroGitterBasis :: insert_tetra (hface3_GEO *(&f)[4], int (&t)[4]) {
  return new Objects :: tetra_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3], _indexmanager[0] ) ;
}

inline GitterBasis :: periodic3_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic3 (hface3_GEO *(&f)[2], int (&t)[2]) {
  return new Objects :: periodic3_IMPL (0,f[0],t[0],f[1],t[1]) ;
}

// Anfang - Neu am 23.5.02 (BS)
inline GitterBasis :: periodic4_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic4 (hface4_GEO *(&f)[2], int (&t)[2]) {
  return new Objects :: periodic4_IMPL (0,f[0],t[0],f[1],t[1]) ;
}
// Ende - Neu am 23.5.02 (BS)

inline GitterBasis :: hexa_GEO * GitterBasis :: MacroGitterBasis :: insert_hexa (hface4_GEO *(&f)[6], int (&t)[6]) {
  return new Objects :: hexa_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3],f[4],t[4],f[5],t[5]) ;
}

inline GitterBasis :: hbndseg3_GEO * GitterBasis :: MacroGitterBasis :: insert_hbnd3 (hface3_GEO * f, int i, Gitter :: hbndseg_STI :: bnd_t b) {
  return new Objects :: hbndseg3_IMPL (0,f,i,NULL) ;
}

inline GitterBasis :: hbndseg4_GEO * GitterBasis :: MacroGitterBasis :: insert_hbnd4 (hface4_GEO * f, int i, Gitter :: hbndseg_STI :: bnd_t b) {
  return new Objects :: hbndseg4_IMPL (0,f,i,NULL) ;
}

#endif	//	GITTER_IMPL_H_INCLUDED
