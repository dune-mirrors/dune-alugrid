// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 

#ifndef GITTER_PLL_IMPL_H_INCLUDED
#define GITTER_PLL_IMPL_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <algorithm>
  #include <vector>
  #include <sstream>
  #include <set>
#else
  #include <algo.h>
  #include <vector.h>
  #include <strstream.h>
  #include <set.h>
#endif

#include "myalloc.h"
#include "gitter_impl.h"
#include "walk.h"
  
#include "gitter_pll_sti.h"
#include "gitter_pll_ldb.h"
#include "ghost_elements.h"

  // Der vector < int > wird als sog. linkagepattern, also als
  // Verbindungsmuster eingesetzt. Die Verbindungsmuster werden
  // nicht in jeder Parallelerweiterung gespeichert sondern in
  // einem zentralen Container im verteilten Grobgitter, dem
  // 'linkagePatternMap' und mit Z"ahlung der Referenzen 
  // verwaltet. Die Methode secondScan () l"oscht dann immer
  // wieder die unreferenzierten Verbindungsmuster aus dem
  // Container. Es gibt "ubrigens kein firstScan () mehr ...

typedef vector < int > linkagePattern_t ;
typedef map < linkagePattern_t, int, less < linkagePattern_t > > linkagePatternMap_t ;

class VertexPllBaseX : public VertexPllXIF, public MyAlloc {
  protected :
    enum { ENDOFSTREAM = -1 } ;
  protected :
    typedef Gitter :: Geometric :: VertexGeo myvertex_t ;
    inline myvertex_t & myvertex () ;
    inline const myvertex_t & myvertex () const ;
  public :
    VertexPllBaseX (myvertex_t &,linkagePatternMap_t &) ;
   ~VertexPllBaseX () ;
    virtual vector < int > estimateLinkage () const ;
    virtual bool setLinkage (vector < int >) ;
    virtual LinkedObject :: Identifier getIdentifier () const ;
    
  protected :
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    
  public :
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void unpackSelf (ObjectStream &, bool) ;
  private :
    static const linkagePattern_t nullPattern ;
    myvertex_t & _v ;
    linkagePatternMap_t & _map ;
    linkagePatternMap_t :: iterator _lpn ;
    map < int, int, less < int > > _moveTo ;
    Refcount _ref ;
} ;

class EdgePllBaseX : public EdgePllXIF, public MyAlloc {
  protected :
    typedef Gitter :: Geometric :: hedge1_GEO myhedge1_t ;
    inline myhedge1_t & myhedge1 () ;
    inline const myhedge1_t & myhedge1 () const ;
  public :
    inline EdgePllBaseX (myhedge1_t &) ;
    inline ~EdgePllBaseX () ;
    virtual vector < int > estimateLinkage () const ;
    virtual LinkedObject :: Identifier getIdentifier () const ;
    virtual void getRefinementRequest (ObjectStream &) const ;
    virtual bool setRefinementRequest (ObjectStream &) ;
  public :
    virtual bool lockAndTry () ;
    virtual bool unlockAndResume (bool) ;
    virtual bool lockedAgainstCoarsening () const ;
  public :
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void unpackSelf (ObjectStream &, bool) ;
  private :
    myhedge1_t & _edge ;
    bool _lockCRS ;
} ;

class EdgePllBaseXMacro : public EdgePllBaseX {
  protected :
    enum { ENDOFSTREAM = -1 } ;
  public :
    EdgePllBaseXMacro (myhedge1_t &) ;
   ~EdgePllBaseXMacro () ;
    virtual vector < int > estimateLinkage () const ;
    virtual LinkedObject :: Identifier getIdentifier () const ;

  protected :
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}

  public :
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void unpackSelf (ObjectStream &, bool) ;
  private :
    map < int, int, less < int > > _moveTo ;
    Refcount _ref ;
} ;

template < class A > class FacePllBaseX : public FacePllXIF, public MyAlloc {
  protected :
    typedef A myhface_t ;
    inline myhface_t & myhface () ;
    inline const myhface_t & myhface () const ;
  public :
    inline FacePllBaseX (myhface_t &) ;
    inline ~FacePllBaseX () {}
    virtual vector < int > estimateLinkage () const ;
    virtual LinkedObject :: Identifier getIdentifier () const ;
    virtual vector < int > checkParallelConnectivity () const ;
  public :
    virtual pair < ElementPllXIF_t *, int > accessOuterPllX () ;
    virtual pair < const ElementPllXIF_t *, int > accessOuterPllX () const ;
    virtual pair < ElementPllXIF_t *, int > accessInnerPllX () ;
    virtual pair < const ElementPllXIF_t *, int > accessInnerPllX () const ;
  public :
    virtual void writeStaticState (ObjectStream &) const ;
    virtual void readStaticState (ObjectStream &) ;
  public :
    virtual bool ldbUpdateGraphEdge (LoadBalancer :: DataBase &) ;
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void unpackSelf (ObjectStream &, bool) ;
  private :
    myhface_t & _face ;
} ;

template < class A > class FacePllBaseXMacro : public FacePllBaseX < A > {
  enum { ENDOFSTREAM = -1 } ;
  public :
    typedef typename FacePllBaseX < A > :: myhface_t myhface_t ;
    inline FacePllBaseXMacro (myhface_t &) ;
    inline ~FacePllBaseXMacro () ;
    virtual vector < int > estimateLinkage () const ;
    virtual LinkedObject :: Identifier getIdentifier () const ;

  protected :
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}

  public :
    virtual bool ldbUpdateGraphEdge (LoadBalancer :: DataBase &) ;
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void unpackSelf (ObjectStream &, bool) ;
  private :
    map < int, int, less < int > > _moveTo ;
    Refcount _ref ;
} ;

class ElementPllBaseX : public ElementPllXIF, public MyAlloc {
  protected :
    enum { ENDOFSTREAM = -1 } ;
    
  // Alle Methoden in dieser Klasse sind Dummies und erzeugen
  // Laufzeitfehler. Sie m"ussen von abgeleiteten Klassen 
  // mit den richtigen Inhalten "uberschrieben werden.

  public :
    pair < ElementPllXIF_t *, int > accessOuterPllX (const pair < ElementPllXIF_t *, int > &, int) ;
    pair < const ElementPllXIF_t *, int > accessOuterPllX (const pair < const ElementPllXIF_t *, int > &, int) const ;
    pair < ElementPllXIF_t *, int > accessInnerPllX (const pair < ElementPllXIF_t *, int > &, int) ;
    pair < const ElementPllXIF_t *, int > accessInnerPllX (const pair < const ElementPllXIF_t *, int > &, int) const ;
  public :
    void writeStaticState (ObjectStream &, int) const ;
    void readStaticState (ObjectStream &, int) ;
    virtual void writeDynamicState (ObjectStream &, int) const  = 0 ;
    void readDynamicState (ObjectStream &, int) ;

    virtual void writeDynamicState (ObjectStream &, GatherScatterType &) const {} ;
    void readDynamicState (ObjectStream &, GatherScatterType &) {}
  public :
    int ldbVertexIndex () const ;
    int & ldbVertexIndex () ;
    bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) ;
  protected :
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}
  public :
    void attach2 (int) ;
    void unattach2 (int) ;
    bool packAll (vector < ObjectStream > &) ;
    void packAsBnd (int,int,ObjectStream &) const ;
    void unpackSelf (ObjectStream &,bool) ;
    bool erasable () const ;
  public :
    void getRefinementRequest (ObjectStream &) ;
    bool setRefinementRequest (ObjectStream &) ;
  public :
    bool lockAndTry () ;
    bool unlockAndResume (bool) ;
} ;

class TetraPllXBase : public ElementPllBaseX {
  public :
    typedef Gitter :: Geometric :: tetra_GEO mytetra_t ;
    inline mytetra_t & mytetra () ;
    inline const mytetra_t & mytetra () const ;
  public :
    inline TetraPllXBase (mytetra_t &) ;
    inline ~TetraPllXBase () {}

    // method to get internal tetra located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p );

  public :
    void writeDynamicState (ObjectStream &, int) const ;
    void writeDynamicState (ObjectStream &, GatherScatterType &) const ;
  private :
    mytetra_t & _tetra ;
} ;

class TetraPllXBaseMacro : public TetraPllXBase {
  public :
    TetraPllXBaseMacro (mytetra_t &) ;
   ~TetraPllXBaseMacro () ;
  protected :
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}
  public :
    virtual void writeStaticState (ObjectStream &, int) const ;
    virtual int ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) ;
  public :
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual bool dunePackAll (vector < ObjectStream > &, GatherScatterType &) ;
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    virtual void unpackSelf (ObjectStream &, bool) ;
    virtual void duneUnpackSelf (ObjectStream &, GatherScatterType &, bool) ;
    virtual bool erasable () const ;
    
    // method to get internal tetra located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p );

  private :
    int _ldbVertexIndex ;
    map < int, int, less < int > > _moveTo ;
    bool _erasable ;
    double _center [3] ;
} ;

// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

class Periodic3PllXBase : public ElementPllBaseX {
  public :
    typedef Gitter :: Geometric :: periodic3_GEO myperiodic3_t ;
    inline myperiodic3_t & myperiodic3 () ;
    inline const myperiodic3_t & myperiodic3 () const ;
  public :
    inline Periodic3PllXBase (myperiodic3_t &) ;
    inline ~Periodic3PllXBase () {}
  public :
    void writeDynamicState (ObjectStream &, int) const ;
    void writeDynamicState (ObjectStream &, GatherScatterType &) const { assert(false); abort(); };
  private :
    myperiodic3_t & _periodic3 ;
} ;

class Periodic3PllXBaseMacro : public Periodic3PllXBase {
  public :
    Periodic3PllXBaseMacro (myperiodic3_t &) ;
   ~Periodic3PllXBaseMacro () ;
  protected:
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}
  public :
    virtual void writeStaticState (ObjectStream &, int) const ;
    virtual int ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) ;
  public :
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    virtual void unpackSelf (ObjectStream &, bool) ;
    virtual bool erasable () const ;
  private :
    int _ldbVertexIndex ;
    map < int, int, less < int > > _moveTo ;
    bool _erasable ;
    double _center [3] ;
} ;

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

class Periodic4PllXBase : public ElementPllBaseX {
  public :
    typedef Gitter :: Geometric :: periodic4_GEO myperiodic4_t ;
    inline myperiodic4_t & myperiodic4 () ;
    inline const myperiodic4_t & myperiodic4 () const ;
  public :
    inline Periodic4PllXBase (myperiodic4_t &) ;
    inline ~Periodic4PllXBase () {}
  public :
    void writeDynamicState (ObjectStream &, int) const ;
    void writeDynamicState (ObjectStream &, GatherScatterType &) const { assert(false); abort(); };
  private :
    myperiodic4_t & _periodic4 ;
} ;

class Periodic4PllXBaseMacro : public Periodic4PllXBase {
  public :
    Periodic4PllXBaseMacro (myperiodic4_t &) ;
   ~Periodic4PllXBaseMacro () ;
  protected:
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}
  public :
    virtual void writeStaticState (ObjectStream &, int) const ;
    virtual int ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) ;
  public :
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    virtual void unpackSelf (ObjectStream &, bool) ;
    virtual bool erasable () const ;
  private :
    int _ldbVertexIndex ;
    map < int, int, less < int > > _moveTo ;
    bool _erasable ;
    double _center [3] ;
} ;

// #     #
// #     #  ######  #    #    ##
// #     #  #        #  #    #  #
// #######  #####     ##    #    #
// #     #  #         ##    ######
// #     #  #        #  #   #    #
// #     #  ######  #    #  #    #

class HexaPllBaseX : public ElementPllBaseX {
  protected :
    typedef Gitter :: Geometric :: hexa_GEO myhexa_t ;
    inline myhexa_t & myhexa () ;
    inline const myhexa_t & myhexa () const ;
  public :
    inline HexaPllBaseX (myhexa_t &) ;
    inline ~HexaPllBaseX () {}
    void writeDynamicState (ObjectStream &, int) const ;
    void writeDynamicState (ObjectStream &, GatherScatterType &) const {};
    
    // method to get internal hexa located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);

  private :
    myhexa_t & _hexa ;
} ;

class HexaPllBaseXMacro : public HexaPllBaseX {
  public :
    HexaPllBaseXMacro (myhexa_t &) ;
   ~HexaPllBaseXMacro () ;
    virtual void writeStaticState (ObjectStream &, int) const ;
  public :
    virtual int ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) ;
  public :
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    virtual void unpackSelf (ObjectStream &, bool) ;
    virtual bool erasable () const ;

    // pack and unpack funtions for dune 
    virtual bool dunePackAll (vector < ObjectStream > &, GatherScatterType &) ;
    virtual void duneUnpackSelf (ObjectStream &, GatherScatterType &, bool) ;
    
    // method to get internal hexa located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);

  protected :
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}
  private :
    int _ldbVertexIndex ;
    map < int, int, less < int > > _moveTo ;
    bool _erasable ;
    double _center [3] ;
} ;

class BndsegPllBaseX : public ElementPllBaseX {
  public :
    void writeDynamicState (ObjectStream &, int) const { abort () ; }
    void writeDynamicState (ObjectStream &, GatherScatterType &) const { assert(false); abort(); };
    pair < ElementPllXIF_t *, int > accessOuterPllX (const pair < ElementPllXIF_t *, int > &, int) ;
    pair < const ElementPllXIF_t *, int > accessOuterPllX (const pair < const ElementPllXIF_t *, int > &, int) const ;
    pair < ElementPllXIF_t *, int > accessInnerPllX (const pair < ElementPllXIF_t *, int > &, int) ;
    pair < const ElementPllXIF_t *, int > accessInnerPllX (const pair < const ElementPllXIF_t *, int > &, int) const ;

} ;

template < class A > class BndsegPllBaseXMacro : public BndsegPllBaseX {
  protected :
    typedef A                       myhbnd_t ;
    typedef typename A :: myhface_t myhface_t ;
    typedef typename A :: balrule_t balrule_t ;
    inline myhbnd_t & myhbnd () ;
    inline const myhbnd_t & myhbnd () const ;
  public :
    inline BndsegPllBaseXMacro (myhbnd_t &) ;
  public :
    virtual int ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
  public :
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    
    // method to get internal bnd located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);

  private :
    myhbnd_t & _hbnd ;
} ;

template < class A > class BndsegPllBaseXClosure : public BndsegPllBaseX {
  protected :
    typedef A                       myhbnd_t ;
    typedef typename A :: myhface_t myhface_t ;
    typedef typename A :: balrule_t balrule_t ;
    inline myhbnd_t & myhbnd () ;
    inline const myhbnd_t & myhbnd () const ;
  public :
    inline BndsegPllBaseXClosure (myhbnd_t &) ;
   ~BndsegPllBaseXClosure () {}
    void readDynamicState (ObjectStream &, int) ;

    void readDynamicState (ObjectStream &, GatherScatterType &);
    
    void getRefinementRequest (ObjectStream &) ;
    bool setRefinementRequest (ObjectStream &) ;
  public :
    bool lockAndTry () ;
    bool unlockAndResume (bool) ;
  public :
    virtual void notifyBalance (balrule_t,int) ;
    virtual bool lockedAgainstCoarsening () const ;
    
    // method to get internal bnd located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);
    
  private :
    myhbnd_t & _hbnd ;
    double _center [3] ;
    balrule_t _rul ;
    bool _lockCRS ;

    int _ghostLevel;
  public:
    const double (& barycenter () const)[3] { return _center ; }
    inline int ghostLevel () const { return _ghostLevel; }

    // to be revised (works for the moment )
    virtual Gitter::helement_STI * getGhost () 
    { 
      //assert(false);
      //cout << "getGhost \n";
      return myhbnd().getGhost(); 
    }
} ;

template < class A > class BndsegPllBaseXMacroClosure : public BndsegPllBaseXClosure < A > {
  public :
    typedef A                       myhbnd_t ;
    typedef typename A :: myhface_t myhface_t ;
    inline BndsegPllBaseXMacroClosure (myhbnd_t &) ;
    inline BndsegPllBaseXMacroClosure (myhbnd_t &, const MacroGhostPoint * _ghp) ;
    virtual void readStaticState (ObjectStream &, int) ;
  public :
    virtual int   ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
  public :
    virtual void packAsBnd (int,int,ObjectStream &) const ;
  private :
    int _extGraphVertexIndex ;
    const MacroGhostPoint * _ghPoint; 
} ;

class GitterBasisPll : public Gitter :: Geometric, public GitterPll {
  public :  
    class ObjectsPll : public GitterBasis :: Objects {
      public :
        class VertexPllImplMacro : public VertexEmptyMacro {
    public :
      typedef VertexPllBaseX mypllx_t ;
          public :
            VertexPllImplMacro (double,double,double,int,linkagePatternMap_t &,  
                                IndexManagerType &im) ;
     ~VertexPllImplMacro () ;
            virtual VertexPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const VertexPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
    // friend class mypllx_t ;
    // ### Goettingen ###
          friend class VertexPllBaseX;
        } ;
      public :

        class Hedge1EmptyPll : public Hedge1Empty {
    protected :
      inline bool lockedAgainstCoarsening () const ;
    public :
      typedef EdgePllBaseX mypllx_t ;
      inline Hedge1EmptyPll (myvertex_t *,myvertex_t *) ;
      virtual EdgePllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const EdgePllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t _pllx ;
    // friend mypllx_t ;
    // ### Goettingen ###
          friend class EdgePllBaseX;
  } ;
  typedef Hedge1Top < Hedge1EmptyPll > hedge1_IMPL ;
      public :
        class Hedge1EmptyPllMacro : public hedge1_IMPL {
    public :
      typedef EdgePllBaseXMacro mypllx_t ;
      inline Hedge1EmptyPllMacro (myvertex_t *,myvertex_t *, IndexManagerType & im) ;
     ~Hedge1EmptyPllMacro () ;
      virtual EdgePllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const EdgePllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
      
      friend class EdgePllBaseXMacro;
  } ;

      public :
        class Hface3EmptyPll : public Hface3Empty {
    protected :
      typedef hedge1_IMPL inneredge_t ;
    public :
      typedef FacePllBaseX < hface3_GEO > mypllx_t ;
      inline Hface3EmptyPll (myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
      virtual FacePllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const FacePllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t _pllx ;
    //    friend mypllx_t ;
    // ### Goettingen ###
    friend class FacePllBaseX < hface3_GEO >;
  } ;
        typedef Hface3Top < Hface3EmptyPll > hface3_IMPL ;
  
        class Hface3EmptyPllMacro : public hface3_IMPL {
    public :
      typedef FacePllBaseXMacro < hface3_GEO > mypllx_t ;
      Hface3EmptyPllMacro (myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int, IndexManagerType & im) ;
     ~Hface3EmptyPllMacro () ;
      virtual FacePllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const FacePllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
    // friend mypllx_t ;
    // ### Goettingen ###
    friend class FacePllBaseXMacro < hface3_GEO >;
  } ;

      public :
        class Hface4EmptyPll : public Hface4Empty {
    protected :
      typedef hedge1_IMPL inneredge_t ;
    public :
      typedef FacePllBaseX < hface4_GEO > mypllx_t ;
      inline Hface4EmptyPll (myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
      virtual FacePllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const FacePllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
    private:
      mypllx_t _pllx ;
      // friend mypllx_t ;
      // ### Goettingen ###
      friend class FacePllBaseX < hface4_GEO >;
  } ;
        typedef Hface4Top < Hface4EmptyPll > hface4_IMPL ;
  
        class Hface4EmptyPllMacro : public hface4_IMPL {
    public :
      typedef FacePllBaseXMacro < hface4_GEO > mypllx_t ;
      Hface4EmptyPllMacro (myhedge1_t *,int,myhedge1_t *,int,
          myhedge1_t *,int,myhedge1_t *,int, IndexManagerType & im) ;
     ~Hface4EmptyPllMacro () ;
      virtual FacePllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const FacePllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
    // friend mypllx_t ;
    // ### Goettingen ###
    friend class FacePllBaseXMacro < hface4_GEO >;
  } ;
      public :

        class TetraEmptyPll : public TetraEmpty {
    protected :
      typedef hedge1_IMPL inneredge_t ;
      typedef hface3_IMPL innerface_t ;
    public :
      typedef TetraPllXBase mypllx_t ;
      inline TetraEmptyPll (myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int, Gitter *) ;
           ~TetraEmptyPll () {}
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t &accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
          private :
      mypllx_t _pllx ;

      friend class TetraTop < TetraEmptyPll >;
  } ;
  typedef TetraTop < TetraEmptyPll > tetra_IMPL ;

        class TetraEmptyPllMacro : public tetra_IMPL {
    public :
      typedef TetraPllXBaseMacro mypllx_t ;
      TetraEmptyPllMacro (myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,
            myhface3_t *,int, IndexManagerType &, Gitter * ) ;
     ~TetraEmptyPllMacro () ;
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
    // friend mypllx_t ;
    // ### Goettingen ###
    friend class TetraPllXBaseMacro;
  } ;
  // Neu >
      public :
        class Periodic3EmptyPll : public Periodic3Empty {
    protected :
      typedef hedge1_IMPL inneredge_t ;
      typedef hface3_IMPL innerface_t ;
    public :
      typedef Periodic3PllXBase mypllx_t ;
      inline Periodic3EmptyPll (myhface3_t *,int,myhface3_t *,int) ;
     ~Periodic3EmptyPll () {}
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t &accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
          private :
      mypllx_t _pllx ;
  } ;
  typedef Periodic3Top < Periodic3EmptyPll > periodic3_IMPL ;
  
  class Periodic3EmptyPllMacro : public periodic3_IMPL {
    public :
      typedef Periodic3PllXBaseMacro mypllx_t ;
      Periodic3EmptyPllMacro (myhface3_t *,int,myhface3_t *,int) ;
     ~Periodic3EmptyPllMacro () ;
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
    // friend mypllx_t ;
    // ### Goettingen ###
    friend class Periodic3PllXBaseMacro;
  } ;
  // < Neu

// Anfang - Neu am 23.5.02 (BS)

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

      public :
        class Periodic4EmptyPll : public Periodic4Empty {
    protected :
      typedef hedge1_IMPL inneredge_t ;
      typedef hface4_IMPL innerface_t ;
    public :
      typedef Periodic4PllXBase mypllx_t ;
      inline Periodic4EmptyPll (myhface4_t *,int,myhface4_t *,int) ;
     ~Periodic4EmptyPll () {}
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t &accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
          private :
      mypllx_t _pllx ;
  } ;
  typedef Periodic4Top < Periodic4EmptyPll > periodic4_IMPL ;
  
  class Periodic4EmptyPllMacro : public periodic4_IMPL {
    public :
      typedef Periodic4PllXBaseMacro mypllx_t ;
      Periodic4EmptyPllMacro (myhface4_t *,int,myhface4_t *,int) ;
     ~Periodic4EmptyPllMacro () ;
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
    // friend mypllx_t ;
    // ### Goettingen ###
    friend class Periodic4PllXBaseMacro;
  } ;

// Ende - Neu am 23.5.02 (BS)

      public :
        class HexaEmptyPll : public HexaEmpty {
    protected :
      typedef hedge1_IMPL inneredge_t ;
      typedef hface4_IMPL innerface_t ;
    public :
      typedef HexaPllBaseX mypllx_t ;
      inline HexaEmptyPll (myhface4_t *,int,myhface4_t *,int,
                           myhface4_t *,int,myhface4_t *,int,
                           myhface4_t *,int,myhface4_t *,int,
                           Gitter* gitter) ;
            inline ~HexaEmptyPll () {}
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
          private :
      mypllx_t _pllx ;
  } ;
  typedef HexaTop < HexaEmptyPll > hexa_IMPL ;

        class HexaEmptyPllMacro : public hexa_IMPL {
    public :
      typedef HexaPllBaseXMacro mypllx_t ;
      HexaEmptyPllMacro (myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
              myhface4_t *,int, IndexManagerType & im, Gitter * gitter ) ;
     ~HexaEmptyPllMacro () ;
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
  
  // Hier, und auch beim Tetraeder besteht die M"oglichkeit das refine
  // auf dem Grobgitterelement zu "uberschreiben, um gegebenenfalls noch
  // andere Funktionen mit aufzuklemmen:  
  // virtual int refine () ;
    
    private :
      mypllx_t * _pllx ;
    // friend mypllx_t ;
    // ### Goettingen ###
    friend class HexaPllBaseXMacro;
  } ;
  
  // Die Randelemente des verteilten Gitters werden aus Templates 
  // in 'gitter_hexa_top_pll.h' und 'gitter_tetra_top_pll.h' erzeugt
  // indem diese die Randelementklassen des sequentiellen Verfahrens
  // "ubernehmen und mit passenden Extendern anreichern.
  
    } ;

  public :
    class MacroGitterBasisPll : public MacroGitterPll, public GitterBasis :: MacroGitterBasis {
      protected :
        linkagePatternMap_t _linkagePatterns ;
        set < int, less < int > > secondScan () ;
      protected :
        int iterators_attached () const ;
        
        virtual VertexGeo     * insert_vertex (double,double,double,int,int = 0) ;
        virtual VertexGeo     * insert_ghostvx (const double (&p)[3]) ;
       
        // insert hbnd_int without ghost hexa 
        virtual hbndseg4_GEO  * insert_hbnd4  (hface4_GEO *, int,Gitter :: hbndseg_STI :: bnd_t) ;
        // insert hbnd_int with ghost hexa 
        virtual hbndseg4_GEO  * insert_hbnd4  (hface4_GEO *, int,Gitter :: hbndseg_STI :: bnd_t,const double (&p)[4][3]) ;
        
        // normal insert hbnd3 version
  virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int,Gitter :: hbndseg_STI :: bnd_t) ;
        // version that get point and create ghost macro 
  virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int,Gitter :: hbndseg_STI :: bnd_t, const double (&p)[3]) ;
        // version that created internal boundary on ghost elements 
  virtual hbndseg3_GEO  * insert_hbnd3_ghost  (hface3_GEO *, int) ;
        virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) ;
        virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) ;
  virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) ;
        virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) ;
  virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) ;

  // insertes a new ghost tetra which is stored inside the MacroGhostTetra 
  virtual MacroGhostTetra    * insert_ghosttetra (hface3_GEO *, int , const double (&p)[3]) ;
  // insertes new ghost hexa which is strored inside the MacroGhostHexa 
  virtual MacroGhostHexa     * insert_ghosthexa (hface4_GEO *, int , const double (&p)[4][3]) ;
  
  virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) ;
// Anfang - Neu am 23.5.02 (BS)
  virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) ;
// Ende - Neu am 23.5.02 (BS)
        IteratorSTI < vertex_STI > * iterator (const vertex_STI *) const ;
        IteratorSTI < vertex_STI > * iterator (const IteratorSTI < vertex_STI > *) const ;
        IteratorSTI < hedge_STI > * iterator (const hedge_STI *) const ;
        IteratorSTI < hedge_STI > * iterator (const IteratorSTI < hedge_STI > *) const ;
        IteratorSTI < hface_STI > * iterator (const hface_STI *) const ;
        IteratorSTI < hface_STI > * iterator (const IteratorSTI < hface_STI > *) const ;
        IteratorSTI < helement_STI > * iterator (const helement_STI *) const ;
        IteratorSTI < helement_STI > * iterator (const IteratorSTI < helement_STI > *) const ;
        IteratorSTI < hbndseg_STI > * iterator (const hbndseg_STI *) const ;
        IteratorSTI < hbndseg_STI > * iterator (const IteratorSTI < hbndseg_STI > *) const ;
      public :
        MacroGitterBasisPll (Gitter * , istream &) ;
        MacroGitterBasisPll (Gitter * ) ;
       ~MacroGitterBasisPll () ;

       // Dune index management 
       IndexManagerType & indexManager(int codim) 
       {
         return MacroGitterBasis::indexManager(codim);
       }
    } ;
  protected :
    MpAccessLocal & _mpaccess ;
    MacroGitterPll * _macrogitter ;
  public :
    virtual inline Makrogitter & container () ;
    virtual inline const Makrogitter & container () const ;
  protected :
    virtual inline MpAccessLocal & mpAccess () ;
    virtual inline const MpAccessLocal & mpAccess () const ;
    GitterBasisPll (MpAccessLocal & ) ;

  public :
    virtual inline MacroGitterPll & containerPll () ;
    virtual inline const MacroGitterPll & containerPll () const ;

    GitterBasisPll (const char *, MpAccessLocal &) ;
    virtual ~GitterBasisPll () ;
// sp"ater   virtual void adapt () ;
} ;


  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //

inline VertexPllBaseX :: myvertex_t & VertexPllBaseX :: myvertex () {
  return _v ;
}

inline const VertexPllBaseX :: myvertex_t & VertexPllBaseX :: myvertex () const {
  return _v ;
}

inline EdgePllBaseX :: EdgePllBaseX (myhedge1_t & e) : _edge (e), _lockCRS (false) {
  return ;
}

inline EdgePllBaseX :: ~EdgePllBaseX () {
#ifndef NDEBUG
  // Falls die nachfolgende Situation eintritt, ist massiv was faul im
  // parallelen Vergr"oberungsalgorithmus: Eine Kante, die gegen Ver-
  // gr"oberung gesperrt war, ist gel"oscht worden. Bestenfalls h"atten
  // die Kinder gel"oscht werden d"urfen, aber nur falls der lock auf-
  // gehoben wird.

  if (_lockCRS) {
    cerr << "**FEHLER (FATAL) in Datei " << __FILE__ << " Zeile " << __LINE__ << endl ;
    abort () ;
  }
#endif
  return ;
}

inline EdgePllBaseX :: myhedge1_t & EdgePllBaseX :: myhedge1 () {
  return _edge ;
}

inline const EdgePllBaseX :: myhedge1_t & EdgePllBaseX :: myhedge1 () const {
  return _edge ;
}

template < class A > inline FacePllBaseX < A > :: FacePllBaseX (myhface_t & f) : _face (f) {
  return ;
}

template < class A > inline typename FacePllBaseX < A > :: myhface_t & FacePllBaseX < A > :: myhface () {
  return _face ;
}

template < class A > inline const typename FacePllBaseX < A > :: myhface_t & FacePllBaseX < A > :: myhface () const {
  return _face ;
}
template < class A > vector < int > FacePllBaseX < A > :: estimateLinkage () const {
  return (abort (), vector < int > ()) ;
}

template < class A > typename LinkedObject :: Identifier FacePllBaseX < A > :: getIdentifier () const {
  return (abort (), LinkedObject :: Identifier ()) ;
}

template < class A > vector < int >  FacePllBaseX < A > :: checkParallelConnectivity () const {
  vector < int > v (A :: polygonlength + 1) ;
  int i ;
  for (i = 0 ; i < A :: polygonlength ; i ++)
    v [i] = myhface ().myvertex (0)->ident () ;
  v [i] = myhface ().level () ;
  return v ;
}

template < class A > pair < ElementPllXIF_t *, int > FacePllBaseX < A > :: accessOuterPllX () {
  return myhface ().nb.front ().first->accessPllX ().accessOuterPllX (pair < ElementPllXIF_t *, int > (& myhface ().nb.rear ().first->accessPllX (),myhface ().nb.rear ().second), myhface ().nb.front ().second) ;
}

template < class A > pair < const ElementPllXIF_t *, int > FacePllBaseX < A > :: accessOuterPllX () const {
  return myhface ().nb.front ().first->accessPllX ().accessOuterPllX (pair < const ElementPllXIF_t *, int > (& myhface ().nb.rear ().first->accessPllX (), myhface ().nb.rear ().second), myhface ().nb.front ().second) ;
}

template < class A > pair < ElementPllXIF_t *, int > FacePllBaseX < A > :: accessInnerPllX () {
  return myhface ().nb.front ().first->accessPllX ().accessInnerPllX (pair < ElementPllXIF_t *, int > (& myhface ().nb.rear ().first->accessPllX (), myhface ().nb.rear ().second), myhface ().nb.front ().second) ;
}

template < class A > pair < const ElementPllXIF_t *, int > FacePllBaseX < A > :: accessInnerPllX () const {
  return myhface ().nb.front ().first->accessPllX ().accessInnerPllX (pair < const ElementPllXIF_t *, int > (& myhface ().nb.rear ().first->accessPllX (), myhface ().nb.rear ().second), myhface ().nb.front ().second) ;
}

template < class A > void FacePllBaseX < A > :: writeStaticState (ObjectStream &) const {
  abort () ;
  return;
}

template < class A > void FacePllBaseX < A > :: readStaticState (ObjectStream &) {
  abort () ;
  return ;
}

template < class A > bool FacePllBaseX < A > :: ldbUpdateGraphEdge (LoadBalancer :: DataBase & db) {
  return (abort (), false) ;
}

template < class A > void FacePllBaseX < A > :: unattach2 (int) {
  abort () ;
  return ;
}

template < class A > void FacePllBaseX < A > :: attach2 (int) {
  abort () ;
  return ;
}

template < class A > bool FacePllBaseX < A > :: packAll (vector < ObjectStream > &) {
  return (abort (), false) ;
}

template < class A > void FacePllBaseX < A > :: unpackSelf (ObjectStream &, bool ) {
  abort () ;
  return ;
}

template < class A > inline FacePllBaseXMacro < A > :: FacePllBaseXMacro (myhface_t & f) : FacePllBaseX < A > (f), _moveTo (), _ref () {
  return ;
}

template < class A > inline FacePllBaseXMacro < A > :: ~FacePllBaseXMacro () {
  assert (0 == _moveTo.size ()) ;
  return ;
}

template < class A > vector < int > FacePllBaseXMacro < A > :: estimateLinkage () const {

  // Diese Methode sch"atzt den Verbindungsstern der Grobgitterfl"ache,
  // indem sie die Schnittmenge der Verbindungssterne der anliegenden
  // Grobgitterknoten bildet. Je besser die Sch"atzung, desto schneller
  // arbeitet das Identifikationsmodul. Falls keine Sch"atzung m"oglich
  // ist, kann man auch einfach aller log. Teilgiternummern in einem 
  // Vektor zur"uckgeben. Dann geht die Identifikation eben langsam.

  vector < int > t1, t2, est ;
  vector < int > l0 = this->myhface ().myhedge1 (0)->accessPllX ().estimateLinkage () ;
  vector < int > l1 = this->myhface ().myhedge1 (1)->accessPllX ().estimateLinkage () ;
  vector < int > l2 = this->myhface ().myhedge1 (2)->accessPllX ().estimateLinkage () ;
  vector < int > l3 = this->myhface ().myhedge1 (A :: polygonlength == 3 ? 2 : 3)->accessPllX ().estimateLinkage () ;
  set_intersection (l0.begin (), l0.end (), l1.begin (), l1.end (), back_inserter (t1), less < int > ()) ;
  set_intersection (l2.begin (), l2.end (), l3.begin (), l3.end (), back_inserter (t2), less < int > ()) ;
  set_intersection (t1.begin (), t1.end (), t2.begin (), t2.end (), back_inserter (est), less < int > ()) ;
  return est ;
}

template < class A > LinkedObject :: Identifier FacePllBaseXMacro < A > :: getIdentifier () const {

  // Diese Methode erzeugt den Schl"ussel einer Grobgitterfl"ache im Auftrag
  // des Identifikationsmoduls 'gitter_pll_idn.cc'. Der Schl"ussel besteht
  // aus drei Integer Werten 'key.h', die aus der globelen Ausrichtung der
  // Fl"ache folgen und auch f"ur Fl"achen mit mehr als drei Knoten korrekt
  // (d.h. gleiche Fl"ache in versch. Teilgittern -> gleicher Schl"ussel) sind.

  return LinkedObject :: Identifier (this->myhface ().myvertex (0)->ident (), 
      this->myhface ().myvertex (1)->ident (), this->myhface ().myvertex (2)->ident ()) ;
}

template < class A > bool FacePllBaseXMacro < A > :: ldbUpdateGraphEdge (LoadBalancer :: DataBase & db) {
  
  // Diese Methode erzeugt eine Kante im Graphen f"ur die Berechnung
  // der Neupartitionierung, der sie das Gewicht der Anzahl aller feinsten
  // Fl"achen "uber der verwalteten Grobgitterfl"ache gibt.
  
  const typename myhface_t :: myconnect_t * mycon1 = this->myhface().nb.front().first;
  const typename myhface_t :: myconnect_t * mycon2 = this->myhface().nb.rear ().first;

  if(mycon1 && mycon2)
  {
    db.edgeUpdate ( LoadBalancer :: GraphEdge 
      (((const typename myhface_t :: myconnect_t *)this->myhface ().nb.front ().first)->accessPllX ().ldbVertexIndex (),
       ((const typename myhface_t :: myconnect_t *)this->myhface ().nb.rear ().first)->accessPllX ().ldbVertexIndex (),
      TreeIterator < typename Gitter :: hface_STI, is_leaf < Gitter :: hface_STI > > (this->myhface ()).size ())) ;
  }
  return true ;
}

template < class A > void FacePllBaseXMacro < A > :: unattach2 (int i) {

  // Diese Methode bindet die Fl"ache von einer Zuweisung zu einem neuen
  // Teilgitter ab. D.h. der Eintrag in der Zuweisungsliste wird gel"oscht,
  // und dann wird die M"oglichkeit an die anliegenden Kanten weitervermittelt.

  assert (_moveTo.find (i) != _moveTo.end ()) ;
  if ( -- _moveTo [i] == 0) _moveTo.erase (i) ;
  {for (int j = 0 ; j < A :: polygonlength ; j ++) 
    this->myhface ().myhedge1 (j)->accessPllX ().unattach2 (i) ;}
  return ;
}

template < class A > void FacePllBaseXMacro < A > :: attach2 (int i) {
  map < int, int, less < int > > :: iterator pos = _moveTo.find (i) ;
  if (pos == _moveTo.end ()) {
    _moveTo.insert (pair < const int, int > (i,1)) ;
  } else {
    (*pos).second ++ ;
  }
  {for (int j = 0 ; j < A :: polygonlength ; j ++) 
    this->myhface ().myhedge1 (j)->accessPllX ().attach2 (i) ;}
  return ;  
}

template < class A > bool FacePllBaseXMacro < A > :: packAll (vector < ObjectStream > & osv) {

  // Die Methode packAll () verpackt die Fl"ache auf alle Datenstr"ome,
  // die zu Teilgittern f"uhren, an die sie zugewiesen wurde mit attach2 ().
  // Ausserdem geht die Methode noch an die anliegenden Elemente (Randelemente)
  // "uber.

  bool action = false ;
  for (map < int, int, less < int > > :: const_iterator i = _moveTo.begin () ; i != _moveTo.end () ; i ++) {
    int j = (*i).first ;
    assert ((osv.begin () + j) < osv.end ()) ;
    
    if (A :: polygonlength == 4) osv [j].writeObject (MacroGridMoverIF :: FACE4) ;
    else if (A :: polygonlength == 3) osv [j].writeObject (MacroGridMoverIF :: FACE3) ;
    else abort () ;
    
    {
      for (int k = 0 ; k < A :: polygonlength ; k ++)
        osv [j].writeObject (this->myhface ().myvertex (k)->ident ()) ;
    }
    try {
    
  // Sicherheitshalber testen, ob das ENDOFSTREAM Tag nicht auch
  // mit einer Verfeinerungsregel identisch ist - sonst gibt's
  // nachher beim Auspacken nur garbage.
    
      assert (! typename myhface_t :: myrule_t (ENDOFSTREAM).isValid ()) ;
    
  // Der ganze Baum der Verfeinerungsregeln wird jetzt in den
  // Stringstream geschrieben (anstelle einer Datei) und dann
  // in den Datenstrom als 'integer' hineinkonvertiert.
    
      strstream_t s ;
      this->myhface ().backup (s) ;
      {for (int c = s.get () ; ! s.eof () ; c = s.get ()) osv [j].writeObject (c) ;}
      osv [j].writeObject (ENDOFSTREAM) ;
      
      inlineData (osv [j]) ;

    } catch (ObjectStream :: OutOfMemoryException) {
      cerr << "**FEHLER (FATAL) ObjectStream :: OutOfMemoryException aufgetreten in " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    try {
    
  // Wenn die Fl"ache auf den j. Strom des Lastverschiebers
  // geschrieben wurde, dann mu"ussen auch die anliegenden
  // Elemente daraufhin untersucht werden, ob sie sich nicht
  // als Randelemente dorthin schreiben sollen - das tun sie
  // aber selbst.
    
      this->myhface ().nb.front ().first->accessPllX ().packAsBnd (this->myhface ().nb.front ().second, j, osv [j]) ;
      this->myhface ().nb.rear  ().first->accessPllX ().packAsBnd (this->myhface ().nb.rear  ().second, j, osv [j]) ;
    } catch (Parallel :: AccessPllException) {
      cerr << "**FEHLER (FATAL) AccessPllException aufgetreten in " << __FILE__ << " " << __LINE__ << ". Ende." << endl ;
      abort () ;
    }
    action = true ;
  }
  return action ;
}

template < class A > void FacePllBaseXMacro < A > :: unpackSelf (ObjectStream & os, bool i) {

  // Die Methode wird eine Fl"ache aus dem Datenstrom rekonstruieren,
  // die der MacrogridBuilder angelegt hat, oder sie wird nur die
  // Verfeinerungsregeln aus dem Strom nehmen, falls schon vorher
  // eine identische Fl"ache bestand. Dies wird vom MacrogridBuilder
  // durch die 'bool' Variable mitgeteilt. Trotzdem k"onnen auch auf
  // einen bestehenden Fl"achenbaum durch die Lastverschiebung neue
  // Daten aufgebracht werden - dies ist dann hier zu realisieren.

  strstream_t s ;
  int c ;
  try {
    for (os.readObject (c) ; c != ENDOFSTREAM ; os.readObject (c)) s.put (char(c)) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  if (i) {
  
  // Sobald der Stringstream mit den 'byte' Verfeinerungsregeln
  // voll ist, kann mit dem normalen restore der ganze Fl"achen-
  // baum wieder hochgezogen werden. Analog zur Wiederherstellung
  // aus einer Datei.
  
    this->myhface ().restore (s) ;
    assert (!s.eof ()) ;

    xtractData (os) ;
  }
  return ;
}

inline TetraPllXBase :: TetraPllXBase (mytetra_t & t) : _tetra (t) {
  return ;
}

inline TetraPllXBase :: mytetra_t & TetraPllXBase :: mytetra () {
  return _tetra ;
}

inline const TetraPllXBase :: mytetra_t & TetraPllXBase :: mytetra () const {
  return _tetra ;
}

inline void TetraPllXBase :: getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
{
  p.first  = & mytetra();
  p.second = 0;
}

inline void TetraPllXBaseMacro :: getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
{
  p.first  = & mytetra();
  p.second = 0;
}

// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

inline Periodic3PllXBase :: Periodic3PllXBase (myperiodic3_t & p) : _periodic3 (p) {
  return ;
}

inline Periodic3PllXBase :: myperiodic3_t & Periodic3PllXBase :: myperiodic3 () {
  return _periodic3 ;
}

inline const Periodic3PllXBase :: myperiodic3_t & Periodic3PllXBase :: myperiodic3 () const {
  return _periodic3 ;
}
  
// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

inline Periodic4PllXBase :: Periodic4PllXBase (myperiodic4_t & p) : _periodic4 (p) {
  return ;
}

inline Periodic4PllXBase :: myperiodic4_t & Periodic4PllXBase :: myperiodic4 () {
  return _periodic4 ;
}

inline const Periodic4PllXBase :: myperiodic4_t & Periodic4PllXBase :: myperiodic4 () const {
  return _periodic4 ;
}

inline HexaPllBaseX :: HexaPllBaseX (myhexa_t & h) : _hexa (h) {
  return ;
}

inline HexaPllBaseX :: myhexa_t & HexaPllBaseX :: myhexa () {
  return _hexa ;
}

inline const HexaPllBaseX :: myhexa_t & HexaPllBaseX :: myhexa () const {
  return _hexa ;
}

inline void HexaPllBaseX :: getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p)
{
  p.first  = & myhexa();
  p.second = 0;
}

inline void HexaPllBaseXMacro :: getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p)
{
  p.first  = & myhexa();
  p.second = 0;
}

template < class A > inline BndsegPllBaseXMacro < A > :: 
BndsegPllBaseXMacro (myhbnd_t & b) : _hbnd (b) {
  return ;
}

template < class A > inline typename BndsegPllBaseXMacro < A > :: myhbnd_t & BndsegPllBaseXMacro < A > :: myhbnd () {
  return _hbnd ;
}

template < class A > inline const typename BndsegPllBaseXMacro < A > :: myhbnd_t & BndsegPllBaseXMacro < A > :: myhbnd () const {
  return _hbnd ;
}

template < class A > int BndsegPllBaseXMacro < A > ::ldbVertexIndex () const {
  return -1 ;
}

template < class  A > int & BndsegPllBaseXMacro < A > :: ldbVertexIndex () {
  return (abort (), *(int *)0) ;
}

template < class A > void BndsegPllBaseXMacro < A > :: packAsBnd (int fce, int who, ObjectStream & os) const {
  assert (!fce) ;
  if (myhface_t :: polygonlength == 3) os.writeObject (MacroGridMoverIF :: HBND3EXT) ;
  else if (myhface_t :: polygonlength == 4) os.writeObject (MacroGridMoverIF :: HBND4EXT) ;
  else abort () ;
  os.writeObject (myhbnd ().bndtype ()) ;
  {for (int i = 0 ; i < myhface_t :: polygonlength ; i++) os.writeObject (myhbnd ().myvertex (fce,i)->ident ()) ; }
  return ;
}

template < class A > inline void BndsegPllBaseXMacro < A > :: 
getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
{
  p.first  = 0;
  p.second = & myhbnd (); 
  return ;
}

template < class A > inline void BndsegPllBaseXClosure < A > :: 
getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
{
  p.first  = 0;
  p.second = & myhbnd (); 
  return ;
}

template < class A > inline BndsegPllBaseXClosure < A > :: BndsegPllBaseXClosure (myhbnd_t & b) : _hbnd (b), _lockCRS (false) , _ghostLevel (-1) {
  return ;
}

template < class A > inline typename BndsegPllBaseXClosure < A > :: myhbnd_t & BndsegPllBaseXClosure < A > :: myhbnd () {
  return _hbnd ;
}

template < class A > inline const typename BndsegPllBaseXClosure < A > :: myhbnd_t & BndsegPllBaseXClosure < A > :: myhbnd () const {
  return _hbnd ;
}

template < class A > inline void BndsegPllBaseXClosure < A > :: notifyBalance (balrule_t r,int) {
  _rul = r ;
  return ;
}

template < class A > void BndsegPllBaseXClosure < A > :: getRefinementRequest (ObjectStream & os) {
  os.writeObject (int (_rul)) ;
  _rul = balrule_t :: nosplit ;
  return ;
}

template < class A > bool BndsegPllBaseXClosure < A > :: setRefinementRequest (ObjectStream & os) {

  // Die Methode schickt die Regel aus dem Nachbargitter in das
  // eigene Teilgitter hinein. Sie liefert "true" immer dann, wenn
  // sich das eigene Gitter g"andert hat -> f"uhrt zu einer weiteren
  // Iteration des parallelen refine ().

  int i ;
  try {
    os.readObject (i) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) BndsegPllBaseXClosure :: setRefinementRequest (..)\n" ;
    cerr << "  EOF gelesen. In " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  balrule_t r (i)  ;
  if (r == balrule_t :: nosplit) {
    return false ;
  } else {
    if (myhbnd ().getrule () == r) {
      return false ;
    } else {
      if (myhbnd ().refineLikeElement (r)) {
        // Verfeinerung erfolgreich
        return true ;
      } else {
      
        // Verfeinerung verhindert irgendwo im Gitter. Dies ist ein Vorbehalt
        // f"ur den parallelen anisotropen Verfeinerungsalgorithmus. Daher
        // sollte die Situation im isotropen Fall nicht auftreten.
      
        cerr << "**FEHLER (FATAL, weil nicht vorgesehen): Verfeinerung wurde\n" ;
        cerr << "  verhindert am inneren Rand. In " << __FILE__ << " " << __LINE__ << endl ;
        abort () ;
      }
    }
  }
  return (abort (), false) ;
}

template < class A > void BndsegPllBaseXClosure < A > :: readDynamicState (ObjectStream & os, GatherScatterType & gs ) {
  gs.recvData( os , myhbnd () );
  return ;
}

template < class A > void BndsegPllBaseXClosure < A > :: readDynamicState (ObjectStream & os, int) {
  try {

#ifdef _DUNE_USES_ALU3DGRID_
    // read the real level of ghost 
    os.readObject( _ghostLevel );
   
#ifdef __USE_INTERNAL_FACES__  
    double p[3];
    for(int i=0; i<myhbnd().dimVx(); i++)
    {
      os.readObject ( p[0] ) ;
      os.readObject ( p[1] ) ;
      os.readObject ( p[2] ) ;
      
      myhbnd (). setOppPoint ( i , p ) ;
    }
#endif

#endif
    
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }

  return ;
}

template < class A > bool BndsegPllBaseXClosure < A > :: lockAndTry () {
  _lockCRS = true ;
  return myhbnd ().bndNotifyCoarsen () ;
}

template < class A > bool BndsegPllBaseXClosure < A > :: lockedAgainstCoarsening () const {
  return _lockCRS ;
}

template < class A > bool BndsegPllBaseXClosure < A > :: unlockAndResume (bool r) {
  _lockCRS = false ;
  bool x ;
  if (r) {
    x = myhbnd ().bndNotifyCoarsen () ;
  }
  else {
    x = false ;
  }
  return x ;
}

template < class A > inline BndsegPllBaseXMacroClosure < A > :: BndsegPllBaseXMacroClosure (myhbnd_t & b)
  : BndsegPllBaseXClosure < A > (b), _extGraphVertexIndex (-1) , _ghPoint (0) {
  return ;
}

template < class A > inline BndsegPllBaseXMacroClosure < A > :: 
BndsegPllBaseXMacroClosure (myhbnd_t & b, const MacroGhostPoint * ghp)
  : BndsegPllBaseXClosure < A > (b), _extGraphVertexIndex (-1) , _ghPoint (ghp) {
  return ;
}

template < class A > int BndsegPllBaseXMacroClosure < A > :: ldbVertexIndex () const {
  return _extGraphVertexIndex ;
}

template < class A > int & BndsegPllBaseXMacroClosure < A > :: ldbVertexIndex () {
  return _extGraphVertexIndex ;
}

template < class A > void BndsegPllBaseXMacroClosure < A > :: readStaticState (ObjectStream & os, int) {
  try {
    os.readObject (_extGraphVertexIndex) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  assert (_extGraphVertexIndex >= 0) ;
  return ;
}

template < class A > void BndsegPllBaseXMacroClosure < A > :: 
packAsBnd (int fce, int who, ObjectStream & os) const {
  assert (!fce) ;
  assert (this->myhbnd ().bndtype () == Gitter :: hbndseg :: closure) ;
  if (myhface_t :: polygonlength == 3) os.writeObject (MacroGridMoverIF :: HBND3INT) ;
  else if (myhface_t :: polygonlength == 4) os.writeObject (MacroGridMoverIF :: HBND4INT) ;
  else abort () ;
  os.writeObject (this->myhbnd ().bndtype ()) ;
  {
    for (int i = 0 ; i < myhface_t :: polygonlength ; i++) 
      os.writeObject (this->myhbnd ().myvertex (fce,i)->ident ()) ; 
  }

  if(_ghPoint) // is stored ghost point exists
  {
    os.writeObject ( 1 ); // 1 == no point transmitted 
    // the third vertex is the new vertex, see insert_ghosttetra
    for(int j=0; j<_ghPoint->nop(); j++) // j = 1 for tetra and 4 for hexa
    {
      const double (&p)[3] = _ghPoint->getPoint(j);
      for(int i=0; i<3; i++) os.writeObject ( p[i] ) ;
    }
  }
  else 
  {
    os.writeObject ( 0 ); // 0 == no point transmitted 
  }
   
  return ;
}

GitterBasisPll :: ObjectsPll :: Hedge1EmptyPll :: Hedge1EmptyPll (VertexGeo * a, VertexGeo * b) :
  GitterBasis :: Objects :: Hedge1Empty (a,b), _pllx (*this) {
  return ;
}

inline bool GitterBasisPll :: ObjectsPll :: Hedge1EmptyPll :: lockedAgainstCoarsening () const {
 return accessPllX ().lockedAgainstCoarsening () ;
}

inline GitterBasisPll :: ObjectsPll :: Hface3EmptyPll :: Hface3EmptyPll (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2) :
  GitterBasis :: Objects :: Hface3Empty (e0,s0,e1,s1,e2,s2), _pllx (*this) {
  return ;
}

inline GitterBasisPll :: ObjectsPll :: Hface4EmptyPll :: Hface4EmptyPll 
  (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2, myhedge1_t * e3, int s3) :
  GitterBasis :: Objects :: Hface4Empty (e0,s0,e1,s1,e2,s2,e3,s3), _pllx (*this) {
  return ;
}

inline GitterBasisPll :: ObjectsPll :: TetraEmptyPll :: TetraEmptyPll (myhface3_t * f0, int t0, myhface3_t * f1, int t1, myhface3_t * f2, int t2, myhface3_t * f3, int t3, Gitter * mygrid)
  : GitterBasis :: Objects :: TetraEmpty (f0,t0,f1,t1,f2,t2,f3,t3,mygrid), _pllx (*this) {
  return ;
}

  // Neu >
  
inline GitterBasisPll :: ObjectsPll :: Periodic3EmptyPll :: Periodic3EmptyPll (myhface3_t * f0, int t0, myhface3_t * f1, int t1) 
  : GitterBasis :: Objects :: Periodic3Empty (f0,t0,f1,t1), _pllx (*this) {
  return ;
}
  // < Neu

// Anfang - Neu am 23.5.02 (BS)

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

inline GitterBasisPll :: ObjectsPll :: Periodic4EmptyPll :: Periodic4EmptyPll (myhface4_t * f0, int t0, myhface4_t * f1, int t1) 
  : GitterBasis :: Objects :: Periodic4Empty (f0,t0,f1,t1), _pllx (*this) {
  return ;
}

// Ende - Neu am 23.5.02 (BS)

inline GitterBasisPll :: ObjectsPll :: HexaEmptyPll :: 
HexaEmptyPll (myhface4_t * f0, int t0, myhface4_t * f1, int t1, 
              myhface4_t * f2, int t2, myhface4_t * f3, int t3, 
              myhface4_t * f4, int t4, myhface4_t * f5, int t5,
              Gitter* gitter) :
  GitterBasis::Objects::HexaEmpty(f0,t0,f1,t1,f2,t2,f3,t3,f4,t4,f5,t5,gitter),
  _pllx (*this) {
  return ;
}

inline int GitterBasisPll :: MacroGitterBasisPll :: iterators_attached () const {
  return GitterBasis :: MacroGitterBasis :: iterators_attached () + MacroGitterPll :: iterators_attached () ;
}

inline MpAccessLocal & GitterBasisPll :: mpAccess () {
  return _mpaccess ;
}

inline const MpAccessLocal & GitterBasisPll :: mpAccess () const {
  return _mpaccess ;
}

inline GitterBasisPll :: Makrogitter & GitterBasisPll :: container () {
  return * _macrogitter ;
}

inline const GitterBasisPll :: Makrogitter & GitterBasisPll :: container () const {
  return * _macrogitter ;
}

inline GitterBasisPll :: MacroGitterPll & GitterBasisPll :: containerPll () {
  return * _macrogitter ;
}

inline const GitterBasisPll :: MacroGitterPll & GitterBasisPll :: containerPll () const {
  return * _macrogitter ;
}
#endif
