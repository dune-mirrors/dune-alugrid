/* $Id$
 * $Log$
 * Revision 1.5  2004/11/16 19:26:59  robertk
 * virtual method up for hbndseg, and ghostLevel .
 *
 * Revision 1.4  2004/11/02 17:12:41  robertk
 * Removed duneBackup and duneRestore. Now in GitterDuneImpl.
 *
 * Revision 1.3  2004/10/28 16:04:04  robertk
 * minor changes.
 *
 * Revision 1.2  2004/10/27 15:05:59  robertk
 * index restore and backup changed.
 * now index storeage is done at the end of one gitter storage, because we
 * need the backup and restore methods for data communication aof parallel
 * grid.
 *
 * Revision 1.1  2004/10/25 16:39:53  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.5  2004/10/19 16:29:21  robertk
 * Dune_VertexGeo added which is holding the extra implementations for Dune
 * for example index backup and restore.
 *
 * Revision 1.4  2004/10/19 13:17:58  robertk
 * Vertex index added. neighOuterNormal added. myinteresction added.
 *
 * Revision 1.2  2004/10/15 11:01:36  robertk
 * if not used by Dune the _DUNE_USED_BSGRID_ is not defined and some things
 * are switched off.
 *
 ***/

#ifndef GITTER_STI_H_INCLUDED
#define GITTER_STI_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <utility>
  #include <iostream>
  #include <list>
#else
  #include <pair.h>
  #include <iostream.h>
  #include <list.h>
#endif

#include "myalloc.h"
#include "parallel.h"
  
//hinzugefuegt uwe fuer DUNE
#include "xdrclass.h"

// if DUNE uses this grid the _DUNE_USES_BSGRID_ variable should be defined
// otherwise some dummy are set 
#ifndef _DUNE_USES_BSGRID_ 
  
#include "dummyindexstack.h"  
typedef DummyIndexStack<int> IndexManagerType; 
  
typedef double BSGridVec [3];
typedef BSGridVec BSGridVecType;  
#endif

  
static volatile char RCSId_gitter_sti_h [] = "$Id$" ;

  // Einfacher Referenzenz"ahler mit cast-around-const
  // feature, der zum Z"ahlen der Referenzen auf Fl"achen
  // Kanten und Knoten verwendet wird. Vorteil: Objekte,
  // die einen Z"ahler dieser Klasse enthalten, werden
  // durch Inkrementierung bzw. Dekrementierung des Z"ahlers
  // nicht ver"andert (k"onnen also auch 'const' sein).

class ProjectVertex {
 public:
  virtual int operator()(const double (&p)[3],double (&ret)[3]) const = 0;
};

class Refcount {
#ifndef NDEBUG

  // Der Globale Z"ahler soll helfen, nicht gel"oschte
  // Gitterobjekte oder Iteratorobjekte zu erkennen.
  // (Wird aber nur in den DEBUG-Versionen angelegt.) 

  class Globalcount {
    int _c ;
    public :
      inline Globalcount() ;
     ~Globalcount() ;
      inline void operator ++ (int) const ;
      inline void operator -- (int) const ;
  } ;
  static Globalcount _g ;
#endif 
  int _c ;
  public :
    inline Refcount () ;
    inline ~Refcount () ;
    inline int operator ++ (int) const ;
    inline int operator ++ () const ;
    inline int operator -- (int) const ;
    inline int operator -- () const ;
    inline bool operator ! () const ;
    inline operator int () const ;
} ;

  // Schnittstelle des Iterationsobjekts vgl. Gamma, Helm, Johnson &
  // Vlissides: Design Patterns; Addison Wesley 
  // Die Schnittstellenbeschreibung wird sowohl polymorph als auch
  // in den verschiedenen Schablonen f"ur einfache Iterationsobjekte
  // s.a. Datei 'walk.h' verwendet.

template < class A > class IteratorSTI {
  const IteratorSTI < A > & operator = (const IteratorSTI < A > &) ;
  IteratorSTI (const IteratorSTI < A > &) {}
  public :
    typedef A val_t ;
    IteratorSTI () {}
    virtual ~IteratorSTI () {}
    virtual void first () ;
    virtual void next () ;
    virtual int done () const ;
    virtual int size () ;
    virtual val_t & item () const ;
} ;

  // sobald die const-correctness verbessert wird ...

template < class A > class ConstIteratorSTI {
  const ConstIteratorSTI < A > & operator = (const ConstIteratorSTI < A > &) ;
  ConstIteratorSTI (const ConstIteratorSTI < A > &) {}
  public :
    typedef A val_t ;
    ConstIteratorSTI () {}
    virtual ~ConstIteratorSTI () {}
    virtual void first () ;
    virtual void next () ;
    virtual int done () const ;
    virtual int size () ;
    virtual const val_t & item () const ;
} ;

  // AccessIterator < . > ist eine Schnittstellenschablone, die
  // ausgepr"agt mit vertex, hedge, hface, helement und hbndseg
  // die Schnittstellen des Grobgittercontainers zur Erstellung
  // und Kopie von Iterationsobjekten erzeugt.

template < class A > class AccessIterator {
  public :
    Refcount ref ; 
    virtual IteratorSTI < A > * iterator (const A *) const = 0 ;
    virtual IteratorSTI < A > * iterator (const IteratorSTI < A > *) const = 0 ;
  public :
  
  // Handle ist ein einfaches Iteratorproxy, das ein abstraktes
  // Iterationsobjekt verwendet, um nach aussen die entsprechenden
  // Schnittstellenmethoden zu implementieren. Dabei ist dem Proxy
  // nur bekannt, wie ein Iterator von der entsprechenden Schnittstelle
  // zu bekommen ist, bzw. wie ein bestehender mit einer Schnittstellen-
  // methode kopiert werden kann.
  
    class Handle : public IteratorSTI < A > {
      AccessIterator < A > * _fac ;
      A * _a ;
      IteratorSTI < A > * _w ;
      public :
        inline Handle (AccessIterator < A > &) ;
        inline Handle (const AccessIterator < A > :: Handle &) ;
        inline Handle () ;
        inline ~Handle () ;
        inline const Handle & operator = (const AccessIterator < A > :: Handle &) ;
        inline bool operator == (const AccessIterator < A > :: Handle &) const ;
        inline bool operator < (const AccessIterator < A > :: Handle &) const ;
        inline void first () ;
        inline void next () ;
        inline int done () const ;
        inline int size () ;
        inline A & item () const ;
    } ;
  protected :
    AccessIterator () {}
    virtual ~AccessIterator () { assert (!ref) ; }
} ;

template < class A > class LeafIterator ;
template < class A > class ConstLeafIterator ;

class Gitter {
  Refcount ref ;
  public :
    static inline bool debugOption (int = 0) ;

    typedef Parallel stiExtender_t ;  // parallel.h
   
  // Nachfolgend sind die Iterationsschnittstellen der Knoten,
  // Kanten, Fl"achen, Elemente und Randelemente definiert.
    
  public :
    class vertex : public stiExtender_t :: VertexIF {
      protected :
        vertex () {}
       ~vertex () {}
      public :
        virtual int ident () const = 0 ;
        virtual int level () const = 0 ;
        virtual const double (& Point () const )[3] = 0 ;
        
        // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
        virtual void project(const ProjectVertex &pv) = 0; 
  
  // Extrainteger, damit die Element zu Vertex Zuordnug klappt,
  // wenn die Daten zur Visualisierung mit GRAPE rausgeschrieben
  // werden sollen:
  
  virtual int & vertexIndex () { return (abort (), *(int *)0) ; }
  virtual int   vertexIndex () const { return (abort (), 0) ; }
  
   } ;
    
    class hedge : public stiExtender_t :: EdgeIF {
      protected :
        hedge () {}
       ~hedge () {}
      public :
        virtual hedge * down () = 0 ;
  virtual const hedge * down () const = 0 ;
        virtual hedge  * next () = 0 ;
        virtual const hedge  * next () const = 0 ;
  virtual vertex * innerVertex () = 0 ;
        virtual const vertex * innerVertex () const = 0 ;
        virtual int level () const = 0 ;
        inline  int leaf () const ;
      public :
        virtual bool coarse () = 0 ;
  virtual void backup (ostream &) const = 0 ;
        virtual void restore (istream &) = 0 ;
        // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
        virtual void projectInnerVertex(const ProjectVertex &pv) = 0; 
    } ;
    
    class hface : public stiExtender_t :: FaceIF {
      protected :
        hface () {}
       ~hface () {}
      public :
        virtual hface * down () = 0 ;
        virtual const hface * down () const = 0 ;
  virtual hface * next () = 0 ;
        virtual const hface * next () const = 0 ;
  virtual vertex * innerVertex () = 0 ;
        virtual const vertex * innerVertex () const = 0 ;
        virtual hedge  * innerHedge () = 0 ;
  virtual const hedge  * innerHedge () const = 0 ;
        virtual int level () const = 0 ;
        inline int leaf () const ;
      public :
        virtual bool coarse () = 0 ;
  virtual void backup (ostream &) const = 0 ;
        virtual void restore (istream &) = 0 ;
        // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
        virtual void projectVertex(const ProjectVertex &pv) = 0; 
    } ;

    // class with all extensions for helement 
    class Dune_helement 
    {

#ifdef _DUNE_USES_BSGRID_
      protected: 
        int  _index; // global_index, unique per level but not per processor
        bool _refinedTag; // true if element was refined 
        Dune_helement () : _index (-1) , _refinedTag (true) {}
#endif
      public:
        // reset the _refinedTag to false 
        void resetRefinedTag(); 
        // true if element was refined this adaptation step 
        bool hasBeenRefined () const;
        
        // use only this methods to get and set the _index 
        int getIndex () const;  
        void setIndex (const int index) ; 
        
        virtual void backupIndex (ostream &) const ;   // backup _index  

        // method is virual, because former index set by constructor has to
        // be freeed , means method has to be overloaded for correct work
        virtual void restoreIndex (istream &) ;// retore _index  
        
        // the Dune extentions 

        // calculate outer normal of face face
        virtual void outerNormal(int face, BSGridVecType & normal) 
        {
          cerr << "helement :: outerNormal(..) : in " << __FILE__ << " " <<  __LINE__ << " not overloaded! \n";
          abort();
        }
        
        // calculate outer normal of face face
        virtual void neighOuterNormal(int faceInNeigh, BSGridVecType & normal) 
        {
          cerr << "helement :: neighOuterNormal(..) : in " << __FILE__ << " " <<  __LINE__ << " not overloaded! \n";
          abort();
        }
    };

    class helement : public virtual stiExtender_t :: ElementIF 
        , public Dune_helement 
    {
      protected :
        helement () {}
       ~helement () {}
      public :
        //testweise us
        virtual helement * up () = 0;
        virtual const helement * up () const = 0;
        //us
        virtual helement * down () = 0 ;
  virtual const helement * down () const = 0 ;
        virtual helement * next () = 0 ;
  virtual const helement * next () const = 0 ;
        virtual vertex * innerVertex () = 0 ;
  virtual const vertex * innerVertex () const = 0 ;
        virtual hedge * innerHedge () = 0 ;
  virtual const hedge * innerHedge () const = 0 ;
        virtual hface * innerHface () = 0 ;
  virtual const hface * innerHface () const = 0 ;
        virtual int level () const = 0 ;

        virtual int tagForGlobalRefinement () = 0 ;
        virtual int resetRefinementRequest () = 0 ;
  virtual int tagForBallRefinement (const double (&)[3],double,int) = 0 ;
        virtual int test () const = 0 ;
        inline  int leaf () const ;
      public :
        virtual bool refine () = 0 ;
        virtual bool coarse () = 0 ;
        virtual void backupCMode (ostream &) const = 0 ;
        virtual void backup (ostream &) const = 0 ;
        virtual void restore (istream &) = 0 ;

      public: 
      virtual grid_t type() = 0;
    } ;
  
    
    class hbndseg  
    {
      protected :
        hbndseg () {}
       ~hbndseg () {}
      public :
        typedef enum { none = 0, inflow = 1, outflow = 2, noslip = 3, slip = 4, sym_xy = 5,
                       sym_xz = 6, sym_yz = 7, reflect = 8, fluxtube3d = 9, periodic = 20,
                       closure = 111, undefined = 333 } bnd_t ;
        virtual bnd_t bndtype () const = 0 ;
        
        // for dune 
        virtual hbndseg * up () = 0 ;
  virtual const hbndseg * up () const = 0 ;
  
        virtual hbndseg * down () = 0 ;
  virtual const hbndseg * down () const = 0 ;
        virtual hbndseg * next () = 0 ;
  virtual const hbndseg * next () const = 0 ;
        virtual int level () const = 0 ;
        inline int leaf () const ;
        // for dune 
        virtual int ghostLevel () const = 0 ;
      public :
  virtual void restoreFollowFace () = 0 ;
    } ;
  public :
    typedef hbndseg hbndseg_STI ;
    typedef helement  helement_STI ;
    typedef hface hface_STI ;
    typedef hedge hedge_STI ;
    typedef vertex  vertex_STI ;

  // Die Klassen Internal-*- sind nur daf"ur da, aus einem Element, einer
  // Fl"ache oder einer Kante die inneren geometrischen Objekte, die von
  // dort verwaltet werden 'ans Licht zu f"ordern'. Sie werden ben"otigt,
  // um z.B. von einem Iterator mit Element-Item auf einen Teilbaumiterator
  // der inneren Fl"achen "uberzugehen.

    class InternalVertex {
      public :
        typedef vertex_STI val_t ;
        val_t & operator () (helement_STI & e) const { return * e.innerVertex () ; }
        val_t & operator () (hface_STI & f) const { return * f.innerVertex () ; }
        val_t & operator () (hedge_STI & d) const { return * d.innerVertex () ; }
        val_t & operator () (vertex_STI & v) const { return v ; }
    } ;
    class InternalEdge {
      public :
        typedef hedge_STI val_t ;
        val_t & operator () (helement_STI & e) const { return * e.innerHedge () ; }
        val_t & operator () (hface_STI & f) const { return * f.innerHedge () ; }
        val_t & operator () (hedge_STI & e) const { return e ; }
    } ;
    class InternalFace {
      public :
        typedef hface_STI val_t ;
        val_t & operator () (helement_STI & e) const { return * e.innerHface () ; }
        val_t & operator () (hface_STI & f) const { return f ; }
    } ;
    class InternalBndseg {
      public :
        typedef hbndseg_STI val_t ;
        val_t & operator () (hbndseg_STI & b) const { return b ; }
    } ;
    class InternalElement {
      public :
        typedef helement_STI val_t ;
        val_t & operator () (helement_STI & e) const { return e ; }
    } ;

  public :
  
  // Die allgemeinste Schnittstelle zum Grobgittercontainer enth"alt Methoden zum
  // Anfordern und Kopieren von Iterationsobjekten, die aber nur im Sinne ihres
  // abstrakten Interfaces bekannt sind.
  // Das Makrogitterinterface beerbt erst mal die verschiedenen Auspr"agungen f"ur
  // Schnittstellen zu Anforderung von Iterationsobjekten. Alles ganz abstrakt bis hier.
  
    class Makrogitter : public AccessIterator < vertex_STI >, public AccessIterator < hedge_STI >,
                  public AccessIterator < hface_STI >, public AccessIterator < hbndseg_STI >, 
            public AccessIterator < helement_STI > {
      protected :
        Makrogitter () {}
  virtual ~Makrogitter () ;
      public :
        virtual int iterators_attached () const ;
  virtual void backup (ostream &) const = 0 ;
  virtual void backup (const char*,const char *) const = 0 ;
  virtual void backupCMode (ostream &) const = 0 ;
  virtual void backupCMode (const char *,const char *) const = 0 ;
  
    // Methoden f"ur den Strahlungstransportl"oser
  virtual void sortmacrogrid () {abort();}
    } ;
  public :
    class Geometric {
    
  // Innerhalb des Namensraums Geometric sind zuerst die Klassen mit den
  // Verfeinerungsregeln und dann die Geometriesockelklassen VertexGeo,
  // Hedge1, Hface3, Hface4, Tetra, Hexa, Hbndseg3/4
  // sowie die Polygonverbinder (Schnittstellen) hasFace3/4 definiert.
    
      public :
        class VertexGeo ;
        class hedge1 ;
        class hface4 ;
  class hface3 ;
  class hasFace3 ;
        class hasFace4 ;
  class Tetra ;
        class Hexa ;
  class hbndseg3 ;
        class hbndseg4 ;
      public :
      
  // Die Verfeinerungsregeln sind nur enumerierte Typen, mit Zuweisung
  // Vergleich und Typkonversion, im Falle der Regeln f"ur die Dreiecks-
  // bzw Vierecksfl"ache sind aber auch Methoden n"otig, die eine Regel
  // mit dem Twist der Fl"ache mitdrehen, damit der "Ubergang der
  // Verfeinerung stimmt.
      
        class Hedge1Rule {
          public :
            typedef enum { invalid=0, nosplit=1, iso2 } rule_t ;
    private :
      rule_t _r ;
    public :
      inline Hedge1Rule (int) ;
      inline Hedge1Rule (rule_t = nosplit) ;
      inline bool operator == (rule_t) const ;
      inline operator int () const ;
      inline bool isValid () const ;
      inline Hedge1Rule rotate (int) const ;
        } ;
      
        class Hface3Rule {
          public :
            typedef enum { nosplit=1, e01, e12, e20, iso4 } rule_t ;
    public :
      inline Hface3Rule (int) ;
      inline Hface3Rule (rule_t = nosplit) ;
      inline operator int () const ;
      inline bool operator == (rule_t) const ;
      inline bool isValid () const ;
      inline Hface3Rule rotate (int) const ;
    private :
      rule_t _r ;
        } ;
      
        class Hface4Rule {
          public :
            typedef enum { nosplit=1, iso4, ni02, ni13 } rule_t ;
    public :
      inline Hface4Rule (int) ;
      inline Hface4Rule (rule_t = nosplit) ;
      inline operator int () const ;
      inline bool operator == (rule_t) const ;
      inline bool isValid () const ;
      inline Hface4Rule rotate (int) const ;
    private :
      rule_t _r ;
        } ;
      
        class TetraRule {
          public :
            typedef enum { crs = -1, nosplit = 1, e01, e12, e20, e23, e30, e31, iso8 } rule_t ;
    public :
      inline TetraRule (int) ;
      inline TetraRule (rule_t = nosplit) ;
      inline operator int () const ;
      inline bool operator == (rule_t) const ;
      inline bool isValid () const ;
    private :
      rule_t _r ;
        } ;
      
        class HexaRule {
          public :
            typedef enum { crs = -1, nosplit = 1, iso8 } rule_t ;
    public :
      inline HexaRule (int) ;
      inline HexaRule (rule_t = nosplit) ;
      inline operator int () const ;
      inline bool operator == (rule_t) const ;
      inline bool isValid () const ;
    private :
      rule_t _r ;
        } ;
     
      public :

  // Die Geometriesockelklassen sind die Grundlage zur Implementierung
  // numerischer Verfahren auf den bestimmten Elementtypen, und erlauben
  // alle Man"over, die "uber die geometrische Information verf"ugen
  // m"ussen, wie z.B. Navigation zur Fl"ache, zu den Kanten und Knoten,
  // aber auch Anforderungen an den Nachbarn.

        class hasFace3 : public virtual stiExtender_t :: ElementIF {
    public :
      typedef Hface3Rule balrule_t ;
      virtual bool refineBalance (balrule_t,int) = 0 ;
      virtual bool bndNotifyCoarsen () = 0 ;
          protected :
            hasFace3 () {}
           ~hasFace3 () {}
      inline bool bndNotifyBalance (balrule_t,int) ;

// Schwerpunkt des anliegenden Elements beschaffen:
          public:
            virtual const double (& barycenter () const)[3] {
              static double p [3] = {.0,.0,.0} ;
              return p ;
            }
// Ende
          public:
            virtual int calcSortnr (int,int) {return (abort(),0);}   
            virtual bool isboundary() = 0;      

            virtual helement_STI * mySelf() { return ((helement_STI *) this); }
        } ;

        class hasFace4 : public virtual stiExtender_t :: ElementIF {
    public :
      typedef Hface4Rule balrule_t ;
      virtual bool refineBalance (balrule_t,int) = 0 ;
      virtual bool bndNotifyCoarsen () = 0 ;
    protected :
      hasFace4 () {}
     ~hasFace4 () {}
      inline bool bndNotifyBalance (balrule_t,int) ;

// Schwerpunkt des anliegenden Elements beschaffen:
          public:
            virtual const double (& barycenter () const)[3] {
              static double p [3] = {.0,.0,.0} ;
              return p ;
            }
// Ende
        } ;
     
        class Dune_VertexGeo 
        {
          protected:
            IndexManagerType & _indexmanager;
#ifdef _DUNE_USES_BSGRID_ 
          //protected:
          //  IndexManagerType & _indexmanager;
            int _idx; 
          //  mutable int _wrcnt;
#endif          
          public:
            Dune_VertexGeo (IndexManagerType & im);
            virtual ~Dune_VertexGeo (); 
            inline void backupIndex  (ostream & os, Refcount refcnt ) const;
            inline void restoreIndex (istream & is, Refcount refcnt ) ;
        };
        
        class VertexGeo : public vertex_STI, public MyAlloc 
          , public Dune_VertexGeo 
        {
          public :
            Refcount ref ;
            // VertexGeo is provided for the vertices on lower levels 
            inline VertexGeo (int,double,double,double, VertexGeo & ) ;
            inline VertexGeo (int,double,double,double, IndexManagerType & im ) ;
            inline virtual ~VertexGeo () ;
            inline const double (& Point () const) [3] ;
            inline int level () const ;
            // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
            virtual inline void project(const ProjectVertex &pv) ; 
            
            inline void backup  (ostream & os ) const;
            inline void restore (istream & is ) ;

  // Extramethode zum Rausschreiben der Elementdaten in einfachem
  // Format f"ur die Visualisierung mit GRAPE:
     
      inline int & vertexIndex () ;
      inline int   vertexIndex () const ;
    private :
      double _c [3] ;
      int _lvl ;
#ifndef _DUNE_USES_BSGRID_
      int _idx ;    // Vertexindex zum Datenrausschreiben
#endif              // wird auch fuer Dune verwendet
        } ;
  
        typedef class hedge1 : public hedge_STI, public MyAlloc {
          protected :
            typedef VertexGeo myvertex_t ;
            inline hedge1 (myvertex_t *,myvertex_t *) ;
      inline int postRefinement () ;
            inline int preCoarsening () ;
      inline bool lockedAgainstCoarsening () const ;
          public :
      typedef Hedge1Rule myrule_t ;
            inline virtual ~hedge1 () ;
            Refcount ref ;
      inline myvertex_t * myvertex (int) ;
            inline const myvertex_t * myvertex (int) const ;
            virtual myvertex_t * subvertex (int) = 0 ;
      virtual const myvertex_t * subvertex (int) const = 0 ;
            virtual hedge1 * subedge1 (int) = 0 ;
      virtual const hedge1 * subedge1 (int) const = 0 ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual void refineImmediate (myrule_t) = 0 ;
    private :
      myvertex_t * v0, * v1 ;
        } hedge1_GEO ;
  
  typedef class hface3 : public hface_STI, public MyAlloc {
          public :
      typedef hasFace3  myconnect_t ;
      enum { polygonlength = 3 } ;
            class face3Neighbour {
              pair < hasFace3 *, int > _v, _h ;
              public :
                static const pair < myconnect_t *, int > null ;
                inline face3Neighbour () ;
    inline void operator = (const face3Neighbour &) ;
    inline int complete (const face3Neighbour &) ;
                inline pair < myconnect_t *, int > front () ;
    inline pair < const myconnect_t *, int > front () const ;
                inline pair < myconnect_t *, int > rear () ;
    inline pair < const myconnect_t *, int > rear () const ;
              friend class hface3 ;
            } nb ;
          protected :
            typedef VertexGeo   myvertex_t ;
            typedef hedge1_GEO  myhedge1_t ;
    public :
      typedef Hface3Rule myrule_t ;
          protected :
      inline hface3 (myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
      inline int postRefinement () ;
            inline int preCoarsening () ;
          public :
            inline virtual ~hface3 () ;
            Refcount ref ;
      inline void attachElement (const pair < hasFace3 *, int > &,int) ;
      inline void detachElement (int) ;
    public :
            inline int twist (int) const ;
      inline myvertex_t * myvertex (int) ;
            inline const myvertex_t * myvertex (int) const ;
            inline myhedge1_t * myhedge1 (int) ;
      inline const myhedge1_t * myhedge1 (int) const ;
            virtual hface3 * down () = 0 ;
      virtual const hface3 * down () const = 0 ;
      virtual hface3 * next () = 0 ;
            virtual const hface3 * next () const = 0 ;
      virtual myvertex_t * subvertex (int) = 0 ;
            virtual const myvertex_t * subvertex (int) const = 0 ;
      virtual myhedge1_t * subedge1 (int) = 0 ;
            virtual const myhedge1_t * subedge1 (int) const = 0 ;
      virtual hface3 * subface3 (int) = 0 ;
            virtual const hface3 * subface3 (int) const = 0 ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual bool refine (myrule_t,int) = 0 ;
      virtual void refineImmediate (myrule_t) = 0 ;
          protected :
      myhedge1_t * e [polygonlength] ;
            signed char s [polygonlength] ;

// H"ohere Ordnung: 1. Regel des Elternelements, 2. Nummer in der Reihe der Kinder
//                  3. Nichtkonforme Situation vorne, 4. Ninchtkonforme Situation hinten
//                  bei 3. + 4. ja=1, nein=0
            signed char _parRule, _nChild, _nonv, _nonh ;
// Ende: H"ohere Ordnung

        } hface3_GEO ;

        typedef class hface4 : public hface_STI, public MyAlloc {
          public :
      typedef hasFace4  myconnect_t ;
      enum { polygonlength = 4 } ;
            class face4Neighbour {
              pair < myconnect_t *, int > _v, _h ;
              public :
                static const pair < myconnect_t *, int > null ;
                inline face4Neighbour () ;
    inline void operator = (const face4Neighbour &) ;
    inline int complete (const face4Neighbour &) ;
                inline pair < myconnect_t *, int > front () ;
    inline pair < const myconnect_t *, int > front () const ;
                inline pair < myconnect_t *, int > rear () ;
    inline pair < const myconnect_t *, int > rear () const ;
              friend class hface4 ;
            } nb ;
          protected :
            typedef VertexGeo  myvertex_t ;
            typedef hedge1_GEO  myhedge1_t ;
    public :
      typedef Hface4Rule myrule_t ;
          protected :
      inline hface4 (myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
      inline int postRefinement () ;
            inline int preCoarsening () ;
          public :
            inline virtual ~hface4 () ;
            Refcount ref ;
      inline void attachElement (const pair < hasFace4 *, int > &,int) ;
      inline void detachElement (int) ;
    public :
            inline int twist (int) const ;
      inline myvertex_t * myvertex (int) ;
            inline const myvertex_t * myvertex (int) const ;
            inline myhedge1_t * myhedge1 (int) ;
      inline const myhedge1_t * myhedge1 (int) const ;
            virtual hface4 * down () = 0 ;
      virtual const hface4 * down () const = 0 ;
      virtual hface4 * next () = 0 ;
            virtual const hface4 * next () const = 0 ;
      virtual myvertex_t * subvertex (int) = 0 ;
            virtual const myvertex_t * subvertex (int) const = 0 ;
      virtual myhedge1_t * subedge1 (int) = 0 ;
            virtual const myhedge1_t * subedge1 (int) const = 0 ;
      virtual hface4 * subface4 (int) = 0 ;
            virtual const hface4 * subface4 (int) const = 0 ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual bool refine (myrule_t,int) = 0 ;
      virtual void refineImmediate (myrule_t) = 0 ;

          private :
      myhedge1_t * e [polygonlength] ;
            signed char s [polygonlength] ;
        } hface4_GEO ;
  
  // Geometriesockelklasse des Tetraeders: Vorsicht der Prototyp der dem
  // Tetraeder zugrunde liegt, hat eine nach links (gegen Uhrzeigersinn)
  // umlaufende Numerierung der Knoten auf den Randfl"achen, wenn aus
  // dem Element herausgeblickt wird. Die Konvention f"ur den Hexaeder
  // ist leider genau umgekehrt. Dies sollte beim Aufbau von Pyramiden
  // und Prismen sorgf"altig bedacht werden.
  // Der Prototyp steht in 'gitter_geo.cc'.
  
  typedef class Tetra : public helement_STI, public hasFace3, public MyAlloc {
          protected :
            typedef VertexGeo  myvertex_t ;
            typedef hedge1_GEO myhedge1_t ;
            typedef hface3_GEO myhface3_t ;
      typedef TetraRule  myrule_t ;
            inline Tetra (myhface3_t *, int, myhface3_t *, int, myhface3_t *, int, myhface3_t *, int) ;
            inline int postRefinement () ;
            inline int preCoarsening () ;
          public :
            static const int prototype [4][3] ;
            inline virtual ~Tetra () ;
      inline hface3_GEO * myhface3 (int) ;
            inline const hface3_GEO * myhface3 (int) const ;
            inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
            inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace3 *, int > myneighbour (int) ;
      inline pair < const hasFace3 *, int > myneighbour (int) const ;

      // Dune extension 
      // return pair, first = pointer to face, second = twist of face
      inline pair < hface3_GEO *, int > myintersection (int) ;
      inline pair < const hface3_GEO *, int > myintersection (int) const;
      
            inline int twist (int) const ;
            int test () const ;
    public :
      virtual myrule_t getrule () const = 0 ;
      
      // return rule which was set by request 
      virtual myrule_t requestrule () const = 0 ; 

      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
            int resetRefinementRequest () ;
      int tagForBallRefinement (const double (&)[3],double,int) ;
    virtual bool isboundary() {return false;}
    virtual grid_t type() {return tetra;}

    // Dune extentions 
    
    // calculate outer normal of face face
    virtual void outerNormal (int face , BSGridVecType & normal );
    virtual void neighOuterNormal (int faceInNeigh , BSGridVecType & normal );

    private :
      myhface3_t * f [4] ;
      signed char s [4] ;
        } tetra_GEO ;
  
  // Geometriesockelklasse des periodischen Randelements mit zwei
  // 3-Punkt-Fl"achen.
  
  typedef class Periodic3 : public helement_STI, public hasFace3, public MyAlloc {
          protected :
            typedef VertexGeo  myvertex_t ;
            typedef hedge1_GEO myhedge1_t ;
            typedef hface3_GEO myhface3_t ;
      typedef Hface3Rule myrule_t ;
            inline Periodic3 (myhface3_t *, int, myhface3_t *, int) ;
            inline int postRefinement () ;
            inline int preCoarsening () ;
          public :
            static const int prototype [2][3] ;
            inline virtual ~Periodic3 () ;
      inline hface3_GEO * myhface3 (int) ;
            inline const hface3_GEO * myhface3 (int) const ;
            inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
            inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace3 *, int > myneighbour (int) ;
      inline pair < const hasFace3 *, int > myneighbour (int) const ;
            inline int twist (int) const ;
            int test () const ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
            int resetRefinementRequest () ;
      int tagForBallRefinement (const double (&)[3],double,int) ;
    virtual bool isboundary() {return true;}
    virtual grid_t type() {return tetra;}
    private :
      myhface3_t * f [2] ;
      signed char s [2] ;
        } periodic3_GEO ;

// Anfang - Neu am 23.5.02 (BS)

  // Geometriesockelklasse des periodischen Randelements mit zwei
  // 4-Punkt-Fl"achen.
  
  typedef class Periodic4 : public helement_STI, public hasFace4, public MyAlloc {
          protected :
            typedef VertexGeo  myvertex_t ;
            typedef hedge1_GEO myhedge1_t ;
            typedef hface4_GEO myhface4_t ;
      typedef Hface4Rule myrule_t ;
            inline Periodic4 (myhface4_t *, int, myhface4_t *, int) ;
            inline int postRefinement () ;
            inline int preCoarsening () ;
          public :
            static const int prototype [2][4] ;
            inline virtual ~Periodic4 () ;
      inline hface4_GEO * myhface4 (int) ;
            inline const hface4_GEO * myhface4 (int) const ;
            inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
            inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace4 *, int > myneighbour (int) ;
      inline pair < const hasFace4 *, int > myneighbour (int) const ;
            inline int twist (int) const ;
            int test () const ;
    virtual bool isboundary() {return true;}
    virtual grid_t type() {return hexa;}
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
            int resetRefinementRequest () ;
      int tagForBallRefinement (const double (&)[3],double,int) ;
    private :
      myhface4_t * f [2] ;
      signed char s [2] ;
        } periodic4_GEO ;

// Ende - Neu am 23.5.02 (BS)

  // Der Prototyp f"ur das Hexaederelement bedingt eine im Uhrzeigersinn
  // umlaufende Numerierung der lokalen Knoten einer Aussenfl"ache, falls
  // aus dem Element herausgeschaut wird. Gegensatz zum Tetraeder.
  // Der Prototyp steht in 'gitter_geo.cc'
  
        typedef class Hexa : public helement_STI, public hasFace4, public MyAlloc {
          protected :
            typedef VertexGeo myvertex_t ;
            typedef hedge1_GEO myhedge1_t ;
            typedef hface4_GEO myhface4_t ;
      typedef HexaRule  myrule_t ;
            inline Hexa (myhface4_t *, int, myhface4_t *, int, myhface4_t *, int,
                         myhface4_t *, int, myhface4_t *, int, myhface4_t *, int) ;
            inline int postRefinement () ;
            inline int preCoarsening () ;
          public :
            static const int prototype [6][4] ;
            static const int oppositeFace [6] ;
            inline virtual ~Hexa () ;
      inline hface4_GEO * myhface4 (int) ;
            inline const hface4_GEO * myhface4 (int) const ;
            inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
            inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace4 *, int > myneighbour (int) ;
      inline pair < const hasFace4 *, int > myneighbour (int) const ;
            inline int twist (int) const ;
            int test () const ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
            int resetRefinementRequest () ;
      int tagForBallRefinement (const double (&)[3],double,int) ;
    virtual bool isboundary() {return false;}
    virtual grid_t type() {return hexa;}
    private :
      myhface4_t * f [6] ;
      signed char s [6] ;
        } hexa_GEO ;
  
  // Auch hier ist Vorsicht geboten: Der Protoyp des Dreiecksrandelement
  // numeriert seine Knoten gegen den Uhrzeigersinn, wenn aus dem Randelement
  // auf die Randfl"ache geschaut wird. Das Vierecksrandelement hat die
  // entgegengesetzte Konvention.
  
  typedef class hbndseg3 : public hbndseg_STI, public hasFace3, public MyAlloc {
          public :
            typedef VertexGeo   myvertex_t ;
            typedef hedge1_GEO  myhedge1_t ;
            typedef hface3_GEO  myhface3_t ;
      typedef hface3_GEO  myhface_t ;
      typedef Hface3Rule  myrule_t ;
    protected :
            inline hbndseg3 (myhface3_t *,int,ProjectVertex *) ;
            inline int postRefinement () ;
            inline int preCoarsening () ;
      inline bool lockedAgainstCoarsening () const { return false ; }
          public :
            inline virtual ~hbndseg3 () ;
            inline myrule_t getrule () const ;
      virtual bool refineLikeElement (balrule_t) = 0 ;
            inline myvertex_t * myvertex (int,int) const ;
            inline myhface3_t * myhface3 (int) const ;
            inline int twist (int) const ;
            inline hface3_GEO * subface3 (int,int) const ;
            virtual bool isboundary() {return true;}
    private :
      myhface3_t * _face ;
      int _twist ;
          protected :
            ProjectVertex *projection;
          public:  
        } hbndseg3_GEO ;
  
        typedef class hbndseg4 : public hbndseg_STI, public hasFace4, public MyAlloc {
          public :
            typedef VertexGeo myvertex_t ;
            typedef hedge1_GEO  myhedge1_t ;
            typedef hface4_GEO  myhface4_t ;
      typedef hface4_GEO  myhface_t ;
      typedef Hface4Rule  myrule_t ;
    protected :
            inline hbndseg4 (myhface4_t *,int,ProjectVertex *) ;
            inline int postRefinement () ;
            inline int preCoarsening () ;
      inline bool lockedAgainstCoarsening () const { return false ; }
          public :
            inline virtual ~hbndseg4 () ;
            inline myrule_t getrule () const ;
      virtual bool refineLikeElement (balrule_t) = 0 ;
            inline myvertex_t * myvertex (int,int) const ;
            inline myhface4_t * myhface4 (int) const ;
            inline int twist (int) const ;
            inline hface4_GEO * subface4 (int,int) const ;
    private :
      myhface4_t * _face ;
      int _twist ;
    protected :
            ProjectVertex *projection;

        public:   
        } hbndseg4_GEO ;
  
  class InternalHasFace3 {
          public :
            typedef hasFace3 val_t ;
            val_t * operator () (hasFace3 * x) const { return x ; }
            val_t & operator () (hasFace3 & x) const { return x ; }
        } ;
  
        class InternalHasFace4 {
          public :
            typedef hasFace4 val_t ;
            val_t * operator () (hasFace4 * x) const { return x ; }
            val_t & operator () (hasFace4 & x) const { return x ; }
        } ;
      public :
        class BuilderIF : public Makrogitter {
  
  // BuilderIF ist die Stelle des Makrogitters an der der Builder angreift, wenn das
  // Gitter erbaut werden soll. Der Builder geht direkt mit den Listen um und
  // wendet sich an die Factorymethoden insert_--*-- (), um neue Objekte zu erhalten.
  
          list < VertexGeo * >     _vertexList ;
          list < hedge1_GEO * >    _hedge1List ;
          list < hface4_GEO * >    _hface4List ;
    list < hface3_GEO * >    _hface3List ;
    list < tetra_GEO * >     _tetraList ;
    list < periodic3_GEO * > _periodic3List ;
// Anfang - Neu am 23.5.02 (BS)
          list < periodic4_GEO * > _periodic4List ;
// Ende - Neu am 23.5.02 (BS)
          list < hexa_GEO * >      _hexaList ;
    list < hbndseg3_GEO * >  _hbndseg3List ;
          list < hbndseg4_GEO * >  _hbndseg4List ;
    bool _modified ;
          protected :
            BuilderIF () : _modified (true) {}
            virtual ~BuilderIF () ;
      void generateRawHexaImage (istream &, ostream &) ;
      virtual void macrogridBuilder (istream &) ;
            virtual VertexGeo     * insert_vertex (double, double, double, int,int = 0) = 0 ;
            virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) = 0 ;
      virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) = 0 ;
            virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) = 0 ;
      virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) = 0 ;
      virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) = 0 ;
// Anfang - Neu am 23.5.02 (BS)
      virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) = 0 ;
// Ende - Neu am 23.5.02 (BS)
      virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) = 0 ;
      virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, hbndseg_STI :: bnd_t) = 0 ;
            virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, hbndseg_STI :: bnd_t) = 0 ;
            IteratorSTI < vertex_STI > * iterator (const vertex_STI *) const ;
            IteratorSTI < vertex_STI > * iterator (const IteratorSTI < vertex_STI > *) const ;
            IteratorSTI < hedge_STI >  * iterator (const hedge_STI *) const ;
            IteratorSTI < hedge_STI >  * iterator (const IteratorSTI < hedge_STI > *) const ;
            IteratorSTI < hface_STI >  * iterator (const hface_STI *) const ;
            IteratorSTI < hface_STI >  * iterator (const IteratorSTI < hface_STI > *) const ;
            IteratorSTI < helement_STI > * iterator (const helement_STI *) const ;
            IteratorSTI < helement_STI > * iterator (const IteratorSTI < helement_STI > *) const ;
            IteratorSTI < hbndseg_STI > * iterator (const hbndseg_STI *) const ;
            IteratorSTI < hbndseg_STI > * iterator (const IteratorSTI < hbndseg_STI > *) const ;
    protected :
      IteratorSTI < helement_STI > * pureElementIterator (const helement_STI *) const ;
            IteratorSTI < helement_STI > * pureElementIterator (const IteratorSTI < helement_STI > *) const ;
    public :
      virtual void backup (ostream &) const ;
      virtual void backup (const char*,const char *) const ;
      virtual void backupCMode (ostream &) const ;
      virtual void backupCMode (const char*,const char *) const ;
          friend class MacroGridBuilder ;
        } ;
    } ;
  private :
    IteratorSTI < vertex_STI >   * iterator (const vertex_STI *) ;
    IteratorSTI < vertex_STI >   * iterator (const IteratorSTI < vertex_STI > *) ;
    IteratorSTI < hedge_STI >    * iterator (const hedge_STI *) ;
    IteratorSTI < hedge_STI >    * iterator (const IteratorSTI < hedge_STI > *) ;
    IteratorSTI < hface_STI >    * iterator (const hface_STI *) ;
    IteratorSTI < hface_STI >    * iterator (const IteratorSTI < hface_STI > *) ;
    IteratorSTI < hbndseg_STI >  * iterator (const hbndseg_STI *) ;
    IteratorSTI < hbndseg_STI >  * iterator (const IteratorSTI < hbndseg_STI > *) ;
    IteratorSTI < helement_STI > * iterator (const helement_STI *) ;
    IteratorSTI < helement_STI > * iterator (const IteratorSTI < helement_STI > *) ;
  protected :
    virtual bool refine () ;
    virtual void coarse () ;
    virtual Makrogitter & container () = 0 ;
    virtual const Makrogitter & container () const = 0 ;
    virtual inline int iterators_attached () const ;
    virtual void notifyGridChanges () ;
    virtual void notifyMacroGridChanges () ;
  protected :
    Gitter () {}
    virtual ~Gitter () ;
  public :
    virtual void fullIntegrityCheck () ;
    virtual void printsize () ;
    virtual bool adapt () ;
    virtual void refineGlobal () ;
    virtual void refineBall (const double (&)[3],double,int) ;
    virtual void refineRandom (double) ;
    virtual void backupCMode (ostream &) ;
    virtual void backupCMode (const char*,const char *) ;
    virtual void backup (ostream &) ;
    virtual void backup (const char*,const char *) ;
    virtual void restore (istream &) ;
    virtual void restore (const char*,const char *) ;

  friend class LeafIterator < helement_STI > ;
  friend class LeafIterator < vertex_STI > ;
  friend class LeafIterator < hbndseg_STI > ;
  friend class LeafIterator < hedge_STI > ;
  friend class LeafIterator < hface_STI > ;
} ;

  // "Ausseres Iteratorproxy oder auch einfach ein Smartpointer
  // um von aussen vom Gitter Iterationsobjekte zu bekommen und
  // zu verwalten.

template < class A > class LeafIterator : public MyAlloc {
  IteratorSTI < A > * _w ;
  Gitter * _grd ;
  const A * _a ;
  void * operator new (size_t) { return 0 ; }
  void operator delete (void *) { }
  inline LeafIterator () ;
  public :
    inline LeafIterator (Gitter &) ;
    inline LeafIterator (const LeafIterator < A > & ) ;
    inline ~LeafIterator () ;
    inline IteratorSTI < A > * operator -> () const ;
    inline IteratorSTI < A > & operator * () const ;
} ;

  // Die const-correctness ist leider noch nicht soweit, dass
  // zwischen const und non-const Gitterreferenzen vern"unftig
  // schon an dieser Stelle unterschieden wird.

#if 0
template < class A > class ConstLeafIterator : public MyAlloc {
  ConstIteratorSTI < A > * _w ;
  const Gitter * _grd ;
  const A * _a ;
  void * operator new (size_t) ;
  void operator delete (void *) ;
  inline ConstLeafIterator () ;
  public :
    inline ConstLeafIterator (const Gitter &) ;
    inline ConstLeafIterator (const ConstLeafIterator < A > & ) ;
    inline ~ConstLeafIterator () ;
    inline ConstIteratorSTI < A > * operator -> () const ;
    inline ConstIteratorSTI < A > & operator * () const ;
} ;
#endif

  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //

inline pair < int, int > operator += (pair < int, int> & a, const pair < int, int > & b) {
  return pair < int, int > (a.first += b.first, a.second += b.second) ;
}

/*
inline ostream & operator << (ostream & out, const pair < const int, int > & p) {
  return (out << p.first << " " << p.second << " ") ;
}

inline ostream & operator << (ostream & out, const pair < int, int > & p) {
  return (out << p.first << " " << p.second << " ") ;
}

inline istream & operator >> (istream & in, pair < int, int > & p) {
  return (in >> p.first >> p.second) ;
}
*/
#ifndef NDEBUG
inline Refcount :: Globalcount :: Globalcount () : _c (0) {
  return ;
}

inline void Refcount :: Globalcount :: operator ++ (int) const {
  // ! cast around const
  ++ (int &) _c ;
  return ;
}

inline void Refcount :: Globalcount :: operator -- (int) const {
  -- (int &) _c ;
  return ;
}

inline Refcount :: Refcount () : _c (0) {
  _g ++ ;
  return ;
}

inline Refcount :: ~Refcount () {
  _g -- ;
  return ;
}
#else
inline Refcount :: Refcount () : _c (0) {
  return ;
}

inline Refcount :: ~Refcount () {
  return ;
}
#endif
inline int Refcount :: operator ++ (int) const {
  return ((int &)_c) ++ ;
}

inline int Refcount ::operator ++ () const {
  return ++ (int &) _c ;
}
        
inline int Refcount :: operator -- (int) const {
  return ((int &)_c) -- ;
}
        
inline int Refcount :: operator -- () const {
  return -- (int &) _c ;
}

inline bool Refcount :: operator ! () const {
  return _c ? false : true ;
}
        
inline Refcount :: operator int () const {
  return _c ;
}

template < class A > void ConstIteratorSTI < A > :: first () {
  return ;
}

template < class A > void ConstIteratorSTI < A > :: next () {
  return ;
}

template < class A > int ConstIteratorSTI < A > :: done () const {
  return 1 ;
}

template < class A > int ConstIteratorSTI < A > :: size () {
  return 0 ;
}

template < class A > const A & ConstIteratorSTI < A > :: item () const {
  void * p = (void *)(0) ;
  abort () ;
  return * (const val_t *)p ;
}

template < class A > void IteratorSTI < A > :: first () {
  return ;
}

template < class A > void IteratorSTI < A > :: next () {
  return ;
}

template < class A > int IteratorSTI < A > :: done () const {
  return 1 ;
}

template < class A > int IteratorSTI < A > :: size () {
  return 0 ;
}

template < class A > A & IteratorSTI < A > :: item () const {
  void * p = (void *)(0) ;
  abort() ;
  return *(val_t *)(p) ;
}

template < class A > inline AccessIterator < A > :: Handle :: Handle (AccessIterator < A > & f) : _fac (&f), _w (0), _a (0) {
  _fac->ref ++ ; 
  _w = _fac->iterator (_a) ;
  return ;
}

template < class A > inline AccessIterator < A > :: Handle :: Handle (const AccessIterator < A > :: Handle & p) : _fac (p._fac), _a (0) { 
  _fac ? (_fac->ref ++, _w = _fac->iterator (p._w), 0) : (_w = new IteratorSTI < A > (), 0) ;
  return ;
}

template < class A > inline AccessIterator < A > :: Handle :: Handle () : _fac (0), _w (0), _a (0) {
  _w = new IteratorSTI < A > () ;
  return ;
}

template < class A > inline AccessIterator < A > :: Handle :: ~Handle () {
  _fac ? (_fac->ref --, 0) : 0 ; 
  delete _w ;
  return ;
}

template < class A > inline const typename AccessIterator < A > :: Handle & AccessIterator < A > :: Handle :: operator = (const AccessIterator < A > :: Handle & x) {
   delete _w, _w = 0 ;
   x._fac ? ((_fac ? _fac->ref -- : 0), _w = (_fac = x._fac)->iterator (x._w), _fac->ref ++)
   : ((_fac ? _fac->ref -- : 0), _fac = 0, _w = new IteratorSTI < A > (), 0) ;
   return x ;
}

template < class A > inline bool AccessIterator < A > :: Handle :: operator == (const AccessIterator < A > :: Handle & x) const {
  return (x._fac == _fac) ? ((& x._w->item ()) == (& _w->item ()) ? 1 : 0) : 0 ;
}

template < class A > inline bool AccessIterator < A > :: Handle :: operator < (const AccessIterator < A > :: Handle & x) const {
  return (abort (), false ) ;
}

template < class A > inline void AccessIterator < A > :: Handle :: first () {
  _w->first () ;
  return ;
}

template < class A > inline void AccessIterator < A > :: Handle :: next () {
  _w->next () ;
  return ;
}

template < class A > inline int AccessIterator < A > :: Handle :: done () const {
  return _w->done () ;
}

template < class A > inline int AccessIterator < A > :: Handle :: size () {
  return _w->size () ;
}

template < class A > inline A & AccessIterator < A > :: Handle :: item () const {
  return _w->item () ;
}

inline bool Gitter :: debugOption (int level) {
  return (getenv ("VERBOSE") ? ( atoi (getenv ("VERBOSE")) > level ? true : (level == 0)) : false) ;
}

inline int Gitter :: iterators_attached () const {
  return ref ;
}

inline int Gitter :: hedge :: leaf () const {
  return ! down () ;
}

inline int Gitter :: hface :: leaf () const {
  return ! down () ;
}

inline int Gitter :: helement :: leaf () const {
  return ! down () ;
}

// Dune extensions 
inline void Gitter :: Dune_helement :: resetRefinedTag () {
#ifdef _DUNE_USES_BSGRID_ 
  _refinedTag = false; 
#endif
}

inline bool Gitter :: Dune_helement :: hasBeenRefined () const {
#ifdef _DUNE_USES_BSGRID_ 
  return _refinedTag;
#else 
  return false;
#endif
}

inline int Gitter :: Dune_helement :: getIndex () const {
#ifdef _DUNE_USES_BSGRID_ 
  assert( _index >= 0);
  return _index; 
#else 
  std::cerr << "helement::getIndex () -- ERROR: '_DUNE_USES_BSGRID_' is not defined, so index cannot be used! \n";
  abort();
  return -1;
#endif
}

inline void Gitter :: Dune_helement :: setIndex (const int index) {
#ifdef _DUNE_USES_BSGRID_ 
  _index = index; 
#endif
}

inline void Gitter :: Dune_helement :: backupIndex (ostream & os ) const {
#ifdef _DUNE_USES_BSGRID_ 
  assert(false);
  //os.write( ((const char *) &_index ), sizeof(int) ) ;
#endif
}

inline void Gitter :: Dune_helement :: restoreIndex (istream & is ) {
#ifdef _DUNE_USES_BSGRID_ 
  assert(false);
  //if(_index != -1) _indexmanager.freeIndex( _index );
  //is.read ( ((char *) &_index), sizeof(int) ); 
#endif
}

inline int Gitter :: hbndseg :: leaf () const {
  return ! down () ;
}

inline bool Gitter :: Geometric :: hasFace3 :: bndNotifyBalance (balrule_t,int) {
  return true ;
}

inline bool Gitter :: Geometric :: hasFace4 :: bndNotifyBalance (balrule_t,int) {
  return true ;
}




// #     #                                          #####
// #     #  ######  #####    #####  ######  #    # #     #  ######   ####
// #     #  #       #    #     #    #        #  #  #        #       #    #
// #     #  #####   #    #     #    #####     ##   #  ####  #####   #    #
//  #   #   #       #####      #    #         ##   #     #  #       #    #
//   # #    #       #   #      #    #        #  #  #     #  #       #    #
//    #     ######  #    #     #    ######  #    #  #####   ######   ####

inline Gitter :: Geometric :: VertexGeo :: VertexGeo (int l, double x, double y, double z, IndexManagerType & im) 
  : 
//#ifdef _DUNE_USES_BSGRID_
  Dune_VertexGeo (im) ,    
//#endif
  _lvl (l) {
  _c [0] = x ; _c [1] = y ; _c [2] = z ;
  return ;
}

inline Gitter :: Geometric :: VertexGeo :: VertexGeo (int l, double x, double y, double z, VertexGeo & vx) 
  :
//#ifdef _DUNE_USES_BSGRID_
  Dune_VertexGeo ( vx._indexmanager ) , 
//#endif
  _lvl (l)  {
  _c [0] = x ; _c [1] = y ; _c [2] = z ;
  return ;
}

inline Gitter :: Geometric :: VertexGeo :: ~VertexGeo () {
  assert (ref ? (cerr << "**WARNUNG Vertex-Refcount war " << ref << endl, 1) : 1) ;
  return ;
}

inline const double (& Gitter :: Geometric :: VertexGeo :: Point () const) [3] {
  return _c ;
}

inline int Gitter :: Geometric :: VertexGeo :: level () const {
  return _lvl ;
}

inline void Gitter :: Geometric :: VertexGeo :: project(const ProjectVertex &pv) {
  double p[3] = {_c[0],_c[1],_c[2]};
  if (!pv(p,_c)) {
    cerr << "FEHLER in Gitter :: Geometric :: VertexGeo :: project(const ProjectVertex &pv) " 
         << "keine Randanpassung m\"oglich!" << endl;
    _c[0] = p[0]; _c[1] = p[1]; _c[2] = p[2];
  }
}

inline int & Gitter :: Geometric :: VertexGeo :: vertexIndex () {
  return _idx ;
}

inline int  Gitter :: Geometric :: VertexGeo :: vertexIndex () const {
  return _idx ;
}

inline void Gitter :: Geometric :: VertexGeo :: backup ( ostream & os ) const {
  this->backupIndex(os,ref);
}

inline void Gitter :: Geometric :: VertexGeo :: restore ( istream & is ) {
  this->restoreIndex(is,ref);
}

inline Gitter :: Geometric :: Dune_VertexGeo :: Dune_VertexGeo ( IndexManagerType & im ) : _indexmanager (im)  
{
#ifdef _DUNE_USES_BSGRID_
  _idx = _indexmanager.getIndex(); 
#endif
}

inline Gitter :: Geometric :: Dune_VertexGeo :: ~Dune_VertexGeo () 
{ 
#ifdef _DUNE_USES_BSGRID_
  _indexmanager.freeIndex( _idx );
#endif
}

inline void Gitter :: Geometric :: Dune_VertexGeo :: backupIndex ( ostream & os, Refcount refcnt ) const {
#ifdef _DUNE_USES_BSGRID_
  assert(false);
  /*
  //cout << refcnt << " r|w " << _wrcnt << " \n";
  if(_wrcnt == 0)
  {
    //cout << "write " << _idx << " \n";
    os.write( ((const char *) &_idx ), sizeof(int) ) ;
    _wrcnt = refcnt.operator int ();
    os.write( ((const char *) &_wrcnt ), sizeof(int) ) ;
    _wrcnt = 1;
    return;
  }
 
  _wrcnt ++ ;
  
  if(refcnt == _wrcnt)
    _wrcnt = 0;
  */
#endif
}

inline void Gitter :: Geometric :: Dune_VertexGeo :: restoreIndex ( istream & is , Refcount refcnt ) {
#ifdef _DUNE_USES_BSGRID_ 
  assert(false);
  /*
  // funktioniert noch nicht
  if(_wrcnt == 0)
  {
    //cout << "read " << _idx << " \n";
    is.read ( ((char *) &_idx), sizeof(int) ); 
    is.read ( ((char *) &_wrcnt), sizeof(int) ); 
    return;
  }
  
  if(refcnt == _wrcnt)
    _wrcnt = 0;

  */  
#endif
}

// #     #                                    #    ######
// #     #  ######  #####    ####   ######   ##    #     #  #    #  #       ######
// #     #  #       #    #  #    #  #       # #    #     #  #    #  #       #
// #######  #####   #    #  #       #####     #    ######   #    #  #       #####
// #     #  #       #    #  #  ###  #         #    #   #    #    #  #       #
// #     #  #       #    #  #    #  #         #    #    #   #    #  #       #
// #     #  ######  #####    ####   ######  #####  #     #   ####   ######  ######


inline Gitter :: Geometric :: Hedge1Rule :: Hedge1Rule (int i) : _r ( (rule_t) i ) {
  return ;
}

inline Gitter :: Geometric :: Hedge1Rule :: Hedge1Rule (rule_t r) : _r (r) {
  return ;
}

inline bool Gitter :: Geometric :: Hedge1Rule :: operator == (rule_t r) const {
  return _r == r ;
}

inline Gitter :: Geometric :: Hedge1Rule :: operator int () const {
  return (int) _r ;
}

inline bool Gitter :: Geometric :: Hedge1Rule :: isValid () const {
  return _r == nosplit || _r == iso2 ;
}

inline Gitter :: Geometric :: Hedge1Rule Gitter :: Geometric :: Hedge1Rule :: rotate (int i) const {
  assert (i == 0 || i == 1) ;
  switch (_r) {
    case nosplit :
      return Hedge1Rule (nosplit) ;
    case iso2 :
      return Hedge1Rule (iso2) ;
    default :
      cerr << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
      return Hedge1Rule (nosplit) ;
  }
}

// #     #                                    #
// #     #  ######  #####    ####   ######   ##
// #     #  #       #    #  #    #  #       # #
// #######  #####   #    #  #       #####     #
// #     #  #       #    #  #  ###  #         #
// #     #  #       #    #  #    #  #         #
// #     #  ######  #####    ####   ######  #####

inline Gitter :: Geometric :: hedge1 :: hedge1 (myvertex_t * a, myvertex_t * b) : v0 (a), v1 (b) { 
  v0->ref ++ ; 
  v1->ref ++ ;
  return ;
}

inline Gitter :: Geometric :: hedge1 :: ~hedge1 () {
  assert (ref ? (cerr << "**WARNUNG Kante-Refcount war " << ref << endl, 1) : 1) ;
  v0->ref -- ; 
  v1->ref -- ;
  return ;
}

inline int Gitter :: Geometric :: hedge1 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: hedge1 :: preCoarsening () {
  return 0 ;
}

inline bool Gitter :: Geometric :: hedge1 :: lockedAgainstCoarsening () const {
  return false ;
}

inline Gitter :: Geometric :: VertexGeo * Gitter :: Geometric :: hedge1 :: myvertex (int i) {
  assert (i == 0 || i == 1) ;
  return i == 1 ? v1 : v0 ;
}

inline const Gitter :: Geometric :: hedge1 :: myvertex_t * Gitter :: Geometric :: hedge1 :: myvertex (int i) const {
  assert (i == 0 || i == 1) ;
  return i == 1 ? v1 : v0 ;
}

// #     #                                  #####  ######
// #     #  ######    ##     ####   ###### #     # #     #  #    #  #       ######
// #     #  #        #  #   #    #  #            # #     #  #    #  #       #
// #######  #####   #    #  #       #####   #####  ######   #    #  #       #####
// #     #  #       ######  #       #            # #   #    #    #  #       #
// #     #  #       #    #  #    #  #      #     # #    #   #    #  #       #
// #     #  #       #    #   ####   ######  #####  #     #   ####   ######  ######

inline Gitter :: Geometric :: Hface3Rule :: Hface3Rule (int x) : _r ((rule_t)x) {
  return ;
}

inline Gitter :: Geometric :: Hface3Rule :: Hface3Rule (rule_t r) : _r (r) {
  return ;
}

inline Gitter :: Geometric :: Hface3Rule :: operator int () const {
  return (int) _r ;
}

inline bool Gitter :: Geometric :: Hface3Rule :: operator == (rule_t r) const {
  return r == _r ;
}

inline bool Gitter :: Geometric :: Hface3Rule :: isValid () const {
  return _r == nosplit || _r == iso4 || _r == e01 || _r == e12 || _r == e20 ;
}

inline Gitter :: Geometric :: Hface3Rule Gitter :: Geometric :: Hface3Rule :: rotate (int t) const {
  assert ((-4 < t) && (t < 3)) ;
  switch (_r) {
    case nosplit :
      return Hface3Rule (nosplit) ;
    case e01 :
    case e12 :
    case e20 :
      if (t == 0 || t == -3) {    // twist 0 bzw. -2 : e01 bleibt und e12 <-> e20 event. swappen
        return Hface3Rule (_r == e01 ? e01 : (_r == e12 ? (t == 0 ? e12 : e20) : (t == 0 ? e20 : e12))) ;
      } else if (t == 1 || t == -1) { // twist 1 bzw. -1 : e20 -> e01 (beidesmal)
        return Hface3Rule (_r == e20 ? e01 : (_r == e12 ? (t == 1 ? e20 : e12) : (t == 1 ? e12 : e20))) ;
      } else if (t == 2 || t == -2) { // twist 2 bzw. -3 : e12 -> e01 (beidesmal)
        return Hface3Rule (_r == e12 ? e01 : (_r == e01 ? (t == 2 ? e20 : e12) : (t == 2 ? e12 : e20))) ;
      } else {
        abort () ;
      }
    case iso4 :
      return Hface3Rule (iso4) ;
    default :
      cerr << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
      return Hface3Rule (nosplit) ;
  }
}

// #     #                                 #       ######
// #     #  ######    ##     ####   ###### #    #  #     #  #    #  #       ######
// #     #  #        #  #   #    #  #      #    #  #     #  #    #  #       #
// #######  #####   #    #  #       #####  #    #  ######   #    #  #       #####
// #     #  #       ######  #       #      ####### #   #    #    #  #       #
// #     #  #       #    #  #    #  #           #  #    #   #    #  #       #
// #     #  #       #    #   ####   ######      #  #     #   ####   ######  ######

inline Gitter :: Geometric :: Hface4Rule :: Hface4Rule (int x) : _r ((rule_t)x) {
  return ;
}

inline Gitter :: Geometric :: Hface4Rule :: Hface4Rule (rule_t r) : _r (r) {
  return ;
}

inline Gitter :: Geometric :: Hface4Rule :: operator int () const {
  return (int) _r ;
}

inline bool Gitter :: Geometric :: Hface4Rule :: operator == (rule_t r) const {
  return r == _r ;
}

inline bool Gitter :: Geometric :: Hface4Rule :: isValid () const {
  return _r == nosplit || _r == iso4 /* || _r == ni02 || _r == ni13 */ ;
}

inline Gitter :: Geometric :: Hface4Rule Gitter :: Geometric :: Hface4Rule :: rotate (int t) const {
  switch (_r) {
    case nosplit :
      return Hface4Rule (nosplit) ;
    case iso4 :
      return Hface4Rule (iso4) ;
    default :
      cerr << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
      return Hface4Rule (nosplit) ;
  }
}
 
  //                                         #####
  // #    #  ######    ##     ####   ###### #     #
  // #    #  #        #  #   #    #  #            #
  // ######  #####   #    #  #       #####   #####
  // #    #  #       ######  #       #            #
  // #    #  #       #    #  #    #  #      #     #
  // #    #  #       #    #   ####   ######  #####
  //
  
inline Gitter :: Geometric :: hface3 :: face3Neighbour :: face3Neighbour () : _v (null), _h (null) {
  return ;
}

inline void Gitter :: Geometric :: hface3 :: face3Neighbour :: operator = (const face3Neighbour & n) {
  _v = n._v ;
  _h = n._h ;
  return ;
}

inline int Gitter :: Geometric :: hface3 :: face3Neighbour :: complete (const face3Neighbour & n) {
  return (_v == null ? (_v = n._v, 1) : 0 ) + (_h == null ? (_h = n._h, 1) : 0 ) ;
}

inline pair < Gitter :: Geometric :: hface3 :: myconnect_t *, int > Gitter :: Geometric :: hface3 :: face3Neighbour :: front () {
  assert (!(_v == null)) ;
  return _v ;
}

inline pair < const Gitter :: Geometric :: hface3 :: myconnect_t *, int > Gitter :: Geometric :: hface3 :: face3Neighbour :: front () const {
  assert (!(_v == null)) ;
  return pair < const hasFace3 *, int > (_v.first,_v.second) ;
}

inline pair < Gitter :: Geometric :: hface3 :: myconnect_t *, int > Gitter :: Geometric :: hface3 :: face3Neighbour :: rear () {
  assert (!(_h == null)) ;
  return _h ;
}

inline pair < const Gitter :: Geometric :: hface3 :: myconnect_t *, int > Gitter :: Geometric :: hface3 :: face3Neighbour :: rear () const {
  assert (!(_h == null)) ;
  return pair < const hasFace3 *, int > (_h.first,_h.second) ; ;
}

inline Gitter :: Geometric :: hface3 :: hface3 (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2) {
  assert(e0 && e1 && e2) ;
  (e [0] = e0)->ref ++ ; s [0] = s0 ;
  (e [1] = e1)->ref ++ ; s [1] = s1 ;
  (e [2] = e2)->ref ++ ; s [2] = s2 ;
// H"ohere Ordnung:
  _parRule = _nChild = (signed char) -1 ; // Test.
  _nonv = _nonh = (signed char) 1 ;
// Ende: H"ohere Ordnung
  return ;
}

inline Gitter :: Geometric :: hface3 :: ~hface3 () {
  assert (ref ? (cerr << "**WARNUNG Dreiecksfl\"ache :: Refcount war " << ref << endl, 1) : 1) ;
  e [0] -> ref -- ;
  e [1] -> ref -- ;
  e [2] -> ref -- ;
  return ;
}

inline void Gitter :: Geometric :: hface3 :: attachElement (const pair < myconnect_t *, int > & p, int t) {
// H"ohere Ordnung, bisher: " t < 0 ? nb._h = p : nb._v = p ;"
  t < 0 ? (_nonh = 0, nb._h = p) : (_nonv = 0, nb._v = p) ;
  ref ++ ;
  return ;
}

inline void Gitter :: Geometric :: hface3 :: detachElement (int t) {
// H"ohere Ordnung, bisher: "t < 0 ? nb._h = nb.null : nb._v = nb.null ;"
  t < 0 ? (_nonh = 1, nb._h = nb.null) : (_nonv = 1, nb._v = nb.null) ;
  ref -- ;
  return ;
}

inline int Gitter :: Geometric :: hface3 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface3 :: preCoarsening () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface3 :: twist (int i) const {
  assert (i < 3) ;
  return s [i] ;
}

inline Gitter :: Geometric :: hface3 :: myhedge1_t * Gitter :: Geometric :: hface3 :: myhedge1 (int i) {
  assert (i < 3) ;
  return e [i] ;
}

inline const Gitter :: Geometric :: hface3 :: myhedge1_t * Gitter :: Geometric :: hface3 :: myhedge1 (int i) const {
  assert (i < 3) ;
  return e [i] ;
}

inline Gitter :: Geometric :: hface3 :: myvertex_t * Gitter :: Geometric :: hface3 :: myvertex (int i) {
  assert(0<=i && i < 3) ;
  return myhedge1 (i)->myvertex (s[i]) ;
}

inline const Gitter :: Geometric :: hface3 :: myvertex_t * Gitter :: Geometric :: hface3 :: myvertex (int i) const {
  assert(0<=i && i < 3) ;
  return myhedge1 (i)->myvertex (s[i]) ;
}

//                                        #
// #    #  ######    ##     ####   ###### #    #
// #    #  #        #  #   #    #  #      #    #
// ######  #####   #    #  #       #####  #    #
// #    #  #       ######  #       #      #######
// #    #  #       #    #  #    #  #           #
// #    #  #       #    #   ####   ######      #

inline Gitter :: Geometric :: hface4 :: face4Neighbour :: face4Neighbour () : _v (null), _h (null) {
  return ;
}

inline void Gitter :: Geometric :: hface4 :: face4Neighbour :: operator = (const face4Neighbour & n) {
  _v = n._v ;
  _h = n._h ;
  return ;
}

inline int Gitter :: Geometric :: hface4 :: face4Neighbour :: complete (const face4Neighbour & n) {
  return (_v == null ? (_v = n._v, 1) : 0 ) + (_h == null ? (_h = n._h, 1) : 0 ) ;
}

inline pair < Gitter :: Geometric :: hface4 :: myconnect_t *, int > Gitter :: Geometric :: hface4 :: face4Neighbour :: front () {
  assert (!(_v == null)) ;
  return _v ;
}

inline pair < const Gitter :: Geometric :: hface4 :: myconnect_t *, int > Gitter :: Geometric :: hface4 :: face4Neighbour :: front () const {
  assert (!(_v == null)) ;
  return pair < const myconnect_t *, int > (_v.first,_v.second) ;
}

inline pair < Gitter :: Geometric :: hface4 :: myconnect_t *, int > Gitter :: Geometric :: hface4 :: face4Neighbour :: rear () {
  assert (!(_h == null)) ;
  return _h ;
}

inline pair < const Gitter :: Geometric :: hface4 :: myconnect_t *, int > Gitter :: Geometric :: hface4 :: face4Neighbour :: rear () const {
  assert (!(_h == null)) ;
  return pair < const myconnect_t *, int > (_h.first,_h.second) ; ;
}

inline Gitter :: Geometric :: hface4 :: hface4 (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2, myhedge1_t * e3, int s3) {
  assert(e0 && e1 && e2 && e3) ;
  (e [0] = e0)->ref ++ ; s [0] = s0 ;
  (e [1] = e1)->ref ++ ; s [1] = s1 ;
  (e [2] = e2)->ref ++ ; s [2] = s2 ;
  (e [3] = e3)->ref ++ ; s [3] = s3 ;
  return ;
}

inline Gitter :: Geometric :: hface4 :: ~hface4 () {
  assert (ref ? (cerr << "**WARNUNG Fl\"ache-Refcount war " << ref << endl, 1) : 1) ;
  e [0] -> ref -- ;
  e [1] -> ref -- ;
  e [2] -> ref -- ;
  e [3] -> ref -- ;
  return ;
}

inline void Gitter :: Geometric :: hface4 :: attachElement (const pair < myconnect_t *, int > & p, int t) {
  t < 0 ? nb._h = p : nb._v = p ;
  ref ++ ;
  return ;
}

inline void Gitter :: Geometric :: hface4 :: detachElement (int t) {
  t < 0 ? nb._h = nb.null : nb._v = nb.null ;
  ref -- ;
  return ;
}

inline int Gitter :: Geometric :: hface4 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface4 :: preCoarsening () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface4 :: twist (int i) const {
  assert (i < 4) ;
  return s [i] ;
}

inline Gitter :: Geometric :: hface4 :: myhedge1_t * Gitter :: Geometric :: hface4 :: myhedge1 (int i) {
  assert (i < 4) ;
  return e [i] ;
}

inline const Gitter :: Geometric :: hface4 :: myhedge1_t * Gitter :: Geometric :: hface4 :: myhedge1 (int i) const {
  assert (i < 4) ;
  return e [i] ;
}

inline Gitter :: Geometric :: hface4 :: myvertex_t * Gitter :: Geometric :: hface4 :: myvertex (int i) {
  assert(0<=i && i < 4) ;
  return myhedge1 (i)->myvertex (s[i]) ;
}

inline const Gitter :: Geometric :: hface4 :: myvertex_t * Gitter :: Geometric :: hface4 :: myvertex (int i) const {
  assert(0<=i && i < 4) ;
  return myhedge1 (i)->myvertex (s[i]) ;
}

// #######                                 ######
//    #     ######   #####  #####     ##   #     #  #    #  #       ######
//    #     #          #    #    #   #  #  #     #  #    #  #       #
//    #     #####      #    #    #  #    # ######   #    #  #       #####
//    #     #          #    #####   ###### #   #    #    #  #       #
//    #     #          #    #   #   #    # #    #   #    #  #       #
//    #     ######     #    #    #  #    # #     #   ####   ######  ######

inline Gitter :: Geometric :: TetraRule :: TetraRule (int x) : _r ((rule_t)x) {
  return ;
}

inline Gitter :: Geometric :: TetraRule :: TetraRule (rule_t r) : _r (r) {
  return ;
}

inline Gitter :: Geometric :: TetraRule :: operator int () const {
  return (int) _r ;
}

inline bool Gitter :: Geometric :: TetraRule :: operator == (rule_t r) const {
  return r == _r ;
}

inline bool Gitter :: Geometric :: TetraRule :: isValid () const {
  return _r == crs || _r == nosplit || _r == iso8 || _r == e01 
    || _r == e12 || _r == e20 || _r == e23 || _r == e30 || _r == e31;
}


// #######
//    #     ######   #####  #####     ##
//    #     #          #    #    #   #  #
//    #     #####      #    #    #  #    #
//    #     #          #    #####   ######
//    #     #          #    #   #   #    #
//    #     ######     #    #    #  #    #

inline Gitter :: Geometric :: Tetra :: Tetra (myhface3_t * f0, int t0, myhface3_t * f1,
  int t1, myhface3_t * f2, int t2, myhface3_t * f3, int t3) {
  (f [0] = f0)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 1),(s [1] = t1)) ;
  (f [2] = f2)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 2),(s [2] = t2)) ;
  (f [3] = f3)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 3),(s [3] = t3)) ;
  return ;
}

inline Gitter :: Geometric :: Tetra :: ~Tetra () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  f [2] ->detachElement (s [2]) ;
  f [3] ->detachElement (s [3]) ;
  return ;
}

inline int Gitter :: Geometric :: Tetra :: twist (int i) const {
  assert (i < 4) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Tetra :: myhface3_t * Gitter :: Geometric :: Tetra :: myhface3 (int i) {
  assert (i < 4) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Tetra :: myhface3_t * Gitter :: Geometric :: Tetra :: myhface3 (int i) const {
  assert (i < 4) ;
  return f [i] ;
}

inline Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i, int j) {
  return (twist(i) < 0) ? myhface3(i)->myvertex((7 - j + twist(i)) % 3) : myhface3(i)->myvertex((j + twist(i)) % 3) ;
}

inline const Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i, int j) const {
  return (twist(i) < 0) ? myhface3(i)->myvertex((7 - j + twist(i)) % 3) : myhface3(i)->myvertex((j + twist(i)) % 3) ;
}

inline Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i) {
  assert (0 <= i && i < 4) ;
  return (i < 3) ? myvertex (3,i) : myvertex (2,1) ;
}

inline const Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i) const {
  assert (0 <= i && i < 4) ;
  return (i < 3) ? myvertex (3,i) : myvertex (2,1) ;
}

inline pair < Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Tetra :: myneighbour (int i) 
{
  return twist (i) < 0 ? myhface3 (i)->nb.front () : myhface3 (i)->nb.rear ();
}

inline pair < const Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Tetra :: myneighbour (int i) const {
  return twist (i) < 0 ? pair < const hasFace3 *, int > (myhface3 (i)->nb.front ().first, myhface3 (i)->nb.front ().second)
       : pair < const hasFace3 *, int > (myhface3 (i)->nb.rear ().first, myhface3 (i)->nb.rear ().second) ;
}

inline pair < Gitter :: Geometric :: hface3_GEO *, int > Gitter :: Geometric :: Tetra :: myintersection (int i) 
{
  // return pair, first = pointer to face, second = twist of face
  return pair< Gitter::Geometric::hface3_GEO *,int> (myhface3 (i) ,twist (i));
}

inline pair < const Gitter :: Geometric :: hface3_GEO *, int > Gitter :: Geometric :: Tetra :: myintersection (int i) const
{
  // return pair, first = pointer to face, second = twist of face
  return  pair< const Gitter::Geometric::hface3_GEO * , int > (myhface3 (i) , twist (i) );
}

inline int Gitter :: Geometric :: Tetra :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: Tetra :: preCoarsening () {
  return 0 ;
}

// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

inline Gitter :: Geometric :: Periodic3 :: Periodic3 (myhface3_t * f0, int t0, myhface3_t * f1, int t1) {
  (f [0] = f0)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 1),(s [1] = t1)) ;
  return ;
}

inline Gitter :: Geometric :: Periodic3 :: ~Periodic3 () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  return ;
}

inline int Gitter :: Geometric :: Periodic3 :: twist (int i) const {
  assert (0 <= i && i < 2) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Periodic3 :: myhface3_t * Gitter :: Geometric :: Periodic3 :: myhface3 (int i) {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Periodic3 :: myhface3_t * Gitter :: Geometric :: Periodic3 :: myhface3 (int i) const {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i, int j) {
  assert (0 <= i && i < 2) ;
  return (twist(i) < 0) ? myhface3(i)->myvertex((7 - j + twist(i)) % 3) : myhface3(i)->myvertex((j + twist(i)) % 3) ;
}

inline const Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i, int j) const {
  return (twist(i) < 0) ? myhface3(i)->myvertex((7 - j + twist(i)) % 3) : myhface3(i)->myvertex((j + twist(i)) % 3) ;
}

inline Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i) {
  assert (0 <= i && i < 6) ;
  
    // Der Ausdruck liefert 0-> (0,0)
  //      1-> (0,1)
  //      2-> (0,2)
  //      3-> (1,0)
  //      4-> (1,2)
  //      5-> (1,1)
  return (i < 3) ? myvertex (0,i) : myvertex (1,(6-i)%3) ;
}

inline const Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i) const {
  assert (0 <= i && i < 6) ;
  return (i < 3) ? myvertex (0,i) : myvertex (1,(6-i)%3) ;
}

inline pair < Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Periodic3 :: myneighbour (int i) {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? myhface3 (i)->nb.front () : myhface3 (i)->nb.rear () ;
}

inline pair < const Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Periodic3 :: myneighbour (int i) const {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? pair < const hasFace3 *, int > (myhface3 (i)->nb.front ().first, myhface3 (i)->nb.front ().second)
       : pair < const hasFace3 *, int > (myhface3 (i)->nb.rear ().first, myhface3 (i)->nb.rear ().second) ;
}

inline int Gitter :: Geometric :: Periodic3 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: Periodic3 :: preCoarsening () {
  return 0 ;
}

// Anfang - Neu am 23.5.02 (BS)

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

inline Gitter :: Geometric :: Periodic4 :: Periodic4 (myhface4_t * f0, int t0, myhface4_t * f1, int t1) {
  (f [0] = f0)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 1),(s [1] = t1)) ;
  return ;
}

inline Gitter :: Geometric :: Periodic4 :: ~Periodic4 () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  return ;
}

inline int Gitter :: Geometric :: Periodic4 :: twist (int i) const {
  assert (0 <= i && i < 2) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Periodic4 :: myhface4_t * Gitter :: Geometric :: Periodic4 :: myhface4 (int i) {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Periodic4 :: myhface4_t * Gitter :: Geometric :: Periodic4 :: myhface4 (int i) const {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i, int j) {
  assert (0 <= i && i < 2) ;
  return (twist(i) < 0) ? myhface4(i)->myvertex((9 - j + twist(i)) % 4) : myhface4(i)->myvertex((j + twist(i)) % 4) ;
}

inline const Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i, int j) const {
  assert (0 <= i && i < 2) ;
  return (twist(i) < 0) ? myhface4(i)->myvertex((9 - j + twist(i)) % 4) : myhface4(i)->myvertex((j + twist(i)) % 4) ;
}

inline Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i) { // ok
  assert (0 <= i && i < 8) ;
  return (i < 4) ? myvertex (0, (4 - i) % 4) : myvertex (1, i - 4) ;
}

inline const Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i) const { // ok
  assert (0 <= i && i < 8) ;
  return (i < 4) ? myvertex (0,i) : myvertex (1,(8-i)%4) ;
}

inline pair < Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Periodic4 :: myneighbour (int i) {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? myhface4 (i)->nb.front () : myhface4 (i)->nb.rear () ;
}

inline pair < const Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Periodic4 :: myneighbour (int i) const {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? pair < const hasFace4 *, int > (myhface4 (i)->nb.front ().first, myhface4 (i)->nb.front ().second)
       : pair < const hasFace4 *, int > (myhface4 (i)->nb.rear ().first, myhface4 (i)->nb.rear ().second) ;
}

inline int Gitter :: Geometric :: Periodic4 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: Periodic4 :: preCoarsening () {
  return 0 ;
}

// Ende - Neu am 23.5.02 (BS)

// #     #                         ######
// #     #  ######  #    #    ##   #     #  #    #  #       ######
// #     #  #        #  #    #  #  #     #  #    #  #       #
// #######  #####     ##    #    # ######   #    #  #       #####
// #     #  #         ##    ###### #   #    #    #  #       #
// #     #  #        #  #   #    # #    #   #    #  #       #
// #     #  ######  #    #  #    # #     #   ####   ######  ######


inline Gitter :: Geometric :: HexaRule :: HexaRule (int x) : _r ((rule_t)x) {
  return ;
}

inline Gitter :: Geometric :: HexaRule :: HexaRule (rule_t r) : _r (r) {
  return ;
}

inline Gitter :: Geometric :: HexaRule :: operator int () const {
  return (int) _r ;
}

inline bool Gitter :: Geometric :: HexaRule :: operator == (rule_t r) const {
  return r == _r ;
}

inline bool Gitter :: Geometric :: HexaRule :: isValid () const {
  return _r == crs || _r == nosplit || _r == iso8 ;
}

// #     #
// #     #  ######  #    #    ##
// #     #  #        #  #    #  #
// #######  #####     ##    #    #
// #     #  #         ##    ######
// #     #  #        #  #   #    #
// #     #  ######  #    #  #    #

inline Gitter :: Geometric :: Hexa :: Hexa (myhface4_t * f0, int t0, myhface4_t * f1, int t1, myhface4_t * f2, int t2,
  myhface4_t * f3, int t3, myhface4_t * f4, int t4, myhface4_t * f5, int t5) {
  (f [0] = f0)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 1),(s [1] = t1)) ;
  (f [2] = f2)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 2),(s [2] = t2)) ;
  (f [3] = f3)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 3),(s [3] = t3)) ;
  (f [4] = f4)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 4),(s [4] = t4)) ;
  (f [5] = f5)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 5),(s [5] = t5)) ;
  return ;
}

inline Gitter :: Geometric :: Hexa :: ~Hexa () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  f [2] ->detachElement (s [2]) ;
  f [3] ->detachElement (s [3]) ;
  f [4] ->detachElement (s [4]) ;
  f [5] ->detachElement (s [5]) ;
  return ;
}

inline int Gitter :: Geometric :: Hexa :: twist (int i) const {
  assert (i < 6) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Hexa :: myhface4_t * Gitter :: Geometric :: Hexa :: myhface4 (int i) {
  assert (i < 6) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Hexa :: myhface4_t * Gitter :: Geometric :: Hexa :: myhface4 (int i) const {
  assert (i < 6) ;
  return f [i] ;
}

inline Gitter :: Geometric :: Hexa :: myvertex_t * Gitter :: Geometric :: Hexa :: myvertex (int i, int j) {
  return (twist(i) < 0) ? myhface4(i)->myvertex((9 - j + twist(i)) % 4) : myhface4(i)->myvertex((j + twist(i)) % 4) ;
}

inline const Gitter :: Geometric :: Hexa :: myvertex_t * Gitter :: Geometric :: Hexa :: myvertex (int i, int j) const {
  return (twist(i) < 0) ? myhface4(i)->myvertex((9 - j + twist(i)) % 4) : myhface4(i)->myvertex((j + twist(i)) % 4) ;
}

inline Gitter :: Geometric :: Hexa :: myvertex_t * Gitter :: Geometric :: Hexa :: myvertex (int i) {
  assert (0 <= i && i < 8) ;
  return (i < 4) ? myvertex (0, (4 - i) % 4) : myvertex (1, i - 4) ;
}

inline const Gitter :: Geometric :: Hexa :: myvertex_t * Gitter :: Geometric :: Hexa :: myvertex (int i) const {
  assert (0 <= i && i < 8) ;
  return (i < 4) ? myvertex (0, (4 - i) % 4) : myvertex (1, i - 4) ;
}

inline pair < Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Hexa :: myneighbour (int i) {
  return twist (i) < 0 ? myhface4 (i)->nb.front () : myhface4 (i)->nb.rear () ;
}

inline pair < const Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Hexa :: myneighbour (int i) const {
  return twist (i) < 0 ? pair < const hasFace4 *, int > (myhface4 (i)->nb.front ().first, myhface4 (i)->nb.front ().second)
       : pair < const hasFace4 *, int > (myhface4 (i)->nb.rear ().first, myhface4 (i)->nb.rear ().second) ;
}

inline int Gitter :: Geometric :: Hexa :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: Hexa :: preCoarsening () {
  return 0 ;
}

// #     #                                                  #####
// #     #  #####   #    #  #####    ####   ######   ####  #     #
// #     #  #    #  ##   #  #    #  #       #       #    #       #
// #######  #####   # #  #  #    #   ####   #####   #       #####
// #     #  #    #  #  # #  #    #       #  #       #  ###       #
// #     #  #    #  #   ##  #    #  #    #  #       #    # #     #
// #     #  #####   #    #  #####    ####   ######   ####   #####

inline Gitter :: Geometric :: hbndseg3 :: hbndseg3 (myhface3_t * a, int b, ProjectVertex *ppv) : _face (a), _twist (b), projection(ppv) {
  _face->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this),0), _twist) ;
  return ;
}

inline Gitter :: Geometric :: hbndseg3 :: ~hbndseg3 () {
  _face->detachElement (_twist) ;
  return ;
}

inline int Gitter :: Geometric :: hbndseg3 :: postRefinement () {
  if (projection) {
    myhface3(0)->projectVertex(*projection);
  }
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg3 :: preCoarsening () {
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg3 :: twist (int i) const {
  assert (i == 0) ;
  return _twist ;
}

inline Gitter :: Geometric :: hbndseg3 :: myhface3_t * Gitter :: Geometric :: hbndseg3 :: myhface3 (int i) const {
  assert (i == 0) ;
  return _face ;
}

inline Gitter :: Geometric :: hbndseg3 :: myvertex_t * Gitter :: Geometric :: hbndseg3 :: myvertex (int,int j) const {
  return (twist (0) < 0) ? myhface3 (0)->myvertex ((7 - j + twist (0)) % 3) : myhface3 (0)->myvertex ((j + twist (0)) % 3) ;
}

inline Gitter :: Geometric :: hbndseg3 :: myhface3_t * Gitter :: Geometric :: hbndseg3 :: subface3 (int,int i) const {
  return myhface3 (0)->subface3 (i) ;
}

inline Gitter :: Geometric :: hbndseg3 :: myrule_t Gitter :: Geometric :: hbndseg3 :: getrule () const {
  return myhface3 (0)->getrule () ;
}

// #     #                                                 #
// #     #  #####   #    #  #####    ####   ######   ####  #    #
// #     #  #    #  ##   #  #    #  #       #       #    # #    #
// #######  #####   # #  #  #    #   ####   #####   #      #    #
// #     #  #    #  #  # #  #    #       #  #       #  ### #######
// #     #  #    #  #   ##  #    #  #    #  #       #    #      #
// #     #  #####   #    #  #####    ####   ######   ####       #

inline Gitter :: Geometric :: hbndseg4 :: hbndseg4 (myhface4_t * a, int b, ProjectVertex *ppv) : _face (a), _twist (b), projection(ppv) {
  _face->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this),0), _twist) ;
  return ;
}

inline Gitter :: Geometric :: hbndseg4 :: ~hbndseg4 () {
  _face->detachElement (_twist) ;
  return ;
}

inline int Gitter :: Geometric :: hbndseg4 :: postRefinement () {
  if (projection) {
    myhface4(0)->projectVertex(*projection);
  }
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg4 :: preCoarsening () {
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg4 :: twist (int i) const {
  assert (i == 0) ;
  return _twist ;
}

inline Gitter :: Geometric :: hbndseg4 :: myhface4_t * Gitter :: Geometric :: hbndseg4 :: myhface4 (int i) const {
  assert (i == 0) ;
  return _face ;
}

inline Gitter :: Geometric :: hbndseg4 :: myvertex_t * Gitter :: Geometric :: hbndseg4 :: myvertex (int,int j) const {
  return (twist (0) < 0) ? myhface4 (0)->myvertex ((9 - j + twist (0)) % 4) : myhface4 (0)->myvertex ((j + twist (0)) % 4) ;
}

inline Gitter :: Geometric :: hbndseg4 :: myhface4_t * Gitter :: Geometric :: hbndseg4 :: subface4 (int,int i) const {
  return myhface4 (0)->subface4 (i) ;
}

inline Gitter :: Geometric :: hbndseg4 :: myrule_t Gitter :: Geometric :: hbndseg4 :: getrule () const {
  return myhface4 (0)->getrule () ;
}



#if 0
template < class A > ConstLeafIterator < A > :: ConstLeafIterator () : _grd (0), _w (0) {
  return ;
}

template < class A > ConstLeafIterator < A > :: ConstLeafIterator (const Gitter & g) : _grd (&g), _w (0) {
  _grd->ref ++ ;
  _w = _grd->iterator (_a) ;
  return ;
}

template < class A > ConstLeafIterator < A > :: ConstLeafIterator (const ConstLeafIterator < A > & x) : _grd (x._grd), _w (0) {
  _grd->ref ++ ;
  _w = _grd->iterator (x._w) ;
  return ;
}

template < class A > ConstLeafIterator < A > :: ~ConstLeafIterator () {
  if (_grd) _grd->ref -- ;
  if(_w) delete _w ;
  return ;
}

template < class A > IteratorSTI < A > * ConstLeafIterator < A > :: operator -> () const {
  return _w ;
}

template < class A > IteratorSTI < A > & ConstLeafIterator < A > :: operator * () const {
  return * _w ;
}
#endif


// #                                 ###
// #        ######    ##    ######    #      #####  ######  #####     ##     #####   ####   #####
// #        #        #  #   #         #        #    #       #    #   #  #      #    #    #  #    #
// #        #####   #    #  #####     #        #    #####   #    #  #    #     #    #    #  #    #
// #        #       ######  #         #        #    #       #####   ######     #    #    #  #####
// #        #       #    #  #         #        #    #       #   #   #    #     #    #    #  #   #
// #######  ######  #    #  #        ###       #    ######  #    #  #    #     #     ####   #    #

template < class A > LeafIterator < A > :: LeafIterator () : _grd (0), _w (0) {
  return ;
}

template < class A > LeafIterator < A > :: LeafIterator (Gitter & g) : _grd (&g), _w (0) {
  _grd->ref ++ ;
  _w = _grd->iterator (_a) ;
  return ;
}

template < class A > LeafIterator < A > :: LeafIterator (const LeafIterator < A > & x) : _grd (x._grd), _w (0) {
  _grd->ref ++ ;
  _w = _grd->iterator (x._w) ;
  return ;
}

template < class A > LeafIterator < A > :: ~LeafIterator () {
  if (_grd) _grd->ref -- ;
  if(_w) delete _w ;
  return ;
}

template < class A > IteratorSTI < A > * LeafIterator < A > :: operator -> () const {
  return _w ;
}

template < class A > IteratorSTI < A > & LeafIterator < A > :: operator * () const {
  return * _w ;
}

#endif  // GITTER_STI_H_INCLUDED
