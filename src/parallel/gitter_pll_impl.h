// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 

#ifndef GITTER_PLL_IMPL_H_INCLUDED
#define GITTER_PLL_IMPL_H_INCLUDED

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

class VertexPllBaseX : public VertexPllXIF_t, public MyAlloc {
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

template < class A > 
class EdgePllBaseX : public A 
{
  protected :
    typedef A myhedge1_t ;
    inline myhedge1_t & myhedge1 () { return *this; }
    inline const myhedge1_t & myhedge1 () const { return *this; }

  public :
    typedef typename A :: myvertex_t myvertex_t;

    inline EdgePllBaseX (myvertex_t * a, myvertex_t * b) ;
    ~EdgePllBaseX () ;
    virtual vector < int > estimateLinkage () const ;
    virtual LinkedObject :: Identifier getIdentifier () const ;
    virtual void getRefinementRequest (ObjectStream &) const ;
    virtual bool setRefinementRequest (ObjectStream &) ;
    virtual bool lockAndTry () ;
    virtual bool unlockAndResume (bool) ;
    virtual bool lockedAgainstCoarsening () const ;
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    virtual bool packAll (vector < ObjectStream > &) ;
    virtual void unpackSelf (ObjectStream &, bool) ;
} ;

template < class A > 
class EdgePllBaseXMacro : public A 
{
  public :
    typedef typename A :: myhedge1_t  myhedge1_t;
    typedef typename A :: myvertex_t myvertex_t;
    inline EdgePllBaseXMacro(myvertex_t *,myvertex_t *) ;
   ~EdgePllBaseXMacro () ;
    virtual vector < int > estimateLinkage () const ;
    virtual LinkedObject :: Identifier getIdentifier () const ;
  protected :
    using A :: myhedge1;
    using A :: EDGE1;
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

template < class A > class FacePllBaseX : public A 
{
  protected :
    typedef A myhface_t ;
    typedef typename A :: myconnect_t myconnect_t ;
    typedef typename A :: myhedge1_t myhedge1_t;
      
  public :
    inline FacePllBaseX(myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
    inline FacePllBaseX(myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t*,int) ;
    inline ~FacePllBaseX () {}

    inline myhface_t & myhface () { return *this; }
    inline const myhface_t & myhface () const { return *this; }

    virtual vector < int > estimateLinkage () const ;
    virtual LinkedObject :: Identifier getIdentifier () const ;
    virtual vector < int > checkParallelConnectivity () const ;
    virtual pair < ElementPllXIF_t *, int > accessOuterPllX () ;
    virtual pair < const ElementPllXIF_t *, int > accessOuterPllX () const ;
    virtual pair < ElementPllXIF_t *, int > accessInnerPllX () ;
    virtual pair < const ElementPllXIF_t *, int > accessInnerPllX () const ;
} ;

template < class A > class FacePllBaseXMacro : public A 
{
  public :
    // some typedefs 
    typedef typename A :: myhface_t  myhface_t;
    typedef typename A :: myhedge1_t myhedge1_t;

    // constructor for hface3 
    inline FacePllBaseXMacro(int l, myhedge1_t * e0, int s0, myhedge1_t * e1, int s1,
                                    myhedge1_t * e2, int s2) ;
    // constructor for hface4 
    inline FacePllBaseXMacro(int l, myhedge1_t * e0, int s0, myhedge1_t * e1, int s1,
                                    myhedge1_t * e2, int s2, myhedge1_t * e3, int s3) ; 
    // destructor only checking move-to
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

/*
template < class A >
class ElementPllBaseX : public A 
{
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
*/

template < class A >
class TetraPllXBase : public A {
  public :
    typedef A mytetra_t ;
    typedef typename A :: myhface3_t myhface3_t;
  protected:  
    inline TetraPllXBase(myhface3_t *f0, int s0, myhface3_t *f1, int s1, 
                         myhface3_t *f2, int s2, myhface3_t *f3, int s3) 
        : A(f0, s0, f1, s1, f2, s2, f3, s3 ) {}

    inline mytetra_t& mytetra() { return *this; }
    inline const mytetra_t& mytetra() const { return *this; }
  public :
    inline ~TetraPllXBase () {}

    // method to get internal tetra located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p );

  public :
    void writeDynamicState (ObjectStream &, int) const ;
    void writeDynamicState (ObjectStream &, GatherScatterType &) const ;
} ;

template < class A >
class TetraPllXBaseMacro : public A 
{
  public :
   ~TetraPllXBaseMacro () ;
  protected :
    using A :: TETRA ;
    using A :: HBND3INT ;
    typedef A mytetra_t ;
    inline mytetra_t& mytetra() { return *this; }
    inline const mytetra_t& mytetra() const { return *this; }

    typedef typename A :: myhface3_t myhface3_t;
    TetraPllXBaseMacro(int l, myhface3_t *f0, int s0, myhface3_t *f1, int s1, 
                       myhface3_t *f2, int s2, myhface3_t *f3, int s3);

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
    // pack ghost information 
    virtual void packAsGhost(ObjectStream &,int) const ;
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    virtual void unpackSelf (ObjectStream &, bool) ;
    virtual void duneUnpackSelf (ObjectStream &, GatherScatterType &, bool) ;
    virtual bool erasable () const ;
    
    // method to get internal tetra located behind this parallel interface 
    //virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p );
  protected:
    void packAsBndNow (int, ObjectStream &) const;
    
  private :
    alucoord_t _center [3] ;
    map < int, int, less < int > > _moveTo ;
    int _ldbVertexIndex ;
    bool _erasable ;
} ;

// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

class Periodic3PllXBase : public ElementPllXIF_t {
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
    alucoord_t _center [3] ;
    map < int, int, less < int > > _moveTo ;
    int _ldbVertexIndex ;
    bool _erasable ;
} ;

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

class Periodic4PllXBase : public ElementPllXIF_t {
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
    alucoord_t _center [3] ;
    map < int, int, less < int > > _moveTo ;
    int _ldbVertexIndex ;
    bool _erasable ;
} ;

// #     #
// #     #  ######  #    #    ##
// #     #  #        #  #    #  #
// #######  #####     ##    #    #
// #     #  #         ##    ######
// #     #  #        #  #   #    #
// #     #  ######  #    #  #    #

template < class A > 
class HexaPllBaseX : public A 
{
  protected :
    typedef typename A :: myhface4_t myhface4_t;
    typedef A  myhexa_t ;
    inline myhexa_t & myhexa () { return *this; }
    inline const myhexa_t & myhexa () const { return *this; }

    inline HexaPllBaseX(myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                        myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                        myhface4_t *f4, int s4, myhface4_t *f5, int s5)
        : A(f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5) {}
  public :
    void writeDynamicState (ObjectStream &, int) const ;
    void writeDynamicState (ObjectStream &, GatherScatterType &) const ;
 
    // method to get internal hexa located behind this parallel interface 
    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);
} ;

template < class A > 
class HexaPllBaseXMacro : public A 
{
  protected:
    using A :: HEXA ;
    using A :: HBND4INT ;

    typedef A  myhexa_t ;
    inline myhexa_t & myhexa () { return *this; }
    inline const myhexa_t & myhexa () const { return *this; }

    typedef typename A :: myhface4_t myhface4_t;
    HexaPllBaseXMacro(int l, myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                             myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                             myhface4_t *f4, int s4, myhface4_t *f5, int s5) ;
  public :
   ~HexaPllBaseXMacro () ;
    virtual void writeStaticState (ObjectStream &, int) const ;
    virtual int ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) ;
    virtual void attach2 (int) ;
    virtual void unattach2 (int) ;
    
    virtual bool packAll (vector < ObjectStream > &) ;
    // pack ghost information 
    virtual void packAsGhost(ObjectStream &,int) const ;
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    virtual void unpackSelf (ObjectStream &, bool) ;
    virtual bool erasable () const ;

    // pack and unpack funtions for dune 
    virtual bool dunePackAll (vector < ObjectStream > &, GatherScatterType &) ;
    virtual void duneUnpackSelf (ObjectStream &, GatherScatterType &, bool) ;
    
    // method to get internal hexa located behind this parallel interface 
    //virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);

  protected :
    virtual void inlineData (ObjectStream &) throw (ObjectStream :: EOFException) {}
    virtual void xtractData (ObjectStream &) throw (ObjectStream :: EOFException) {}

    void packAsBndNow (int, ObjectStream &) const;
  protected:
    alucoord_t _center [3] ;
    map < int, int, less < int > > _moveTo ;
    int _ldbVertexIndex ;
    bool _erasable ;
} ;

class BndsegPllBaseX : public ElementPllXIF_t 
{
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
    void writeDynamicState (ObjectStream &, GatherScatterType &) const;
    
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
    int _ghostLevel;
    int _ghostLeaf;
    balrule_t _rul ;

  public:
    inline int ghostLevel () const { return _ghostLevel; }
    inline bool ghostLeaf () const { return (_ghostLevel == myhbnd().level()) && (_ghostLeaf==1); }

    typedef Gitter :: ghostpair_STI ghostpair_STI;
    // to be revised (works for the moment )
    virtual ghostpair_STI getGhost () { return myhbnd().getGhost(); }
    virtual const ghostpair_STI getGhost () const { return myhbnd().getGhost(); }
} ;

template < class A > class BndsegPllBaseXMacroClosure : public BndsegPllBaseXClosure < A > {
  public :
    typedef A                       myhbnd_t ;
    typedef typename A :: myhface_t myhface_t ;
    inline BndsegPllBaseXMacroClosure (myhbnd_t &) ;
    inline BndsegPllBaseXMacroClosure (myhbnd_t &, const MacroGhostInfo_STI* ) ;
    virtual void readStaticState (ObjectStream &, int) ;
  public :
    virtual int   ldbVertexIndex () const ;
    virtual int & ldbVertexIndex () ;
  public :
    virtual void packAsBnd (int,int,ObjectStream &) const ;
    
    // unpack ghost information and insert ghost cell 
    virtual void insertGhostCell(ObjectStream &,int);

  private :
    const MacroGhostInfo_STI * _ghInfo; 
    int _extGraphVertexIndex ;
} ;

class GitterBasisPll : public Gitter :: Geometric, public GitterPll
{
public :
  class ObjectsPll : public GitterBasis :: Objects
  {
  public :
    ///////////////////////////////////////////////////////////////
    // --VertexImpl
    ///////////////////////////////////////////////////////////////
    class VertexPllImplMacro : public VertexEmptyMacro 
    {
    public :
      typedef VertexPllBaseX mypllx_t ;
    public :
      VertexPllImplMacro (double,double,double,int,IndexManagerStorageType&, linkagePatternMap_t &) ;
     ~VertexPllImplMacro () ;
      virtual VertexPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const VertexPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
      friend class VertexPllBaseX;
    } ;

    ///////////////////////////////////////////////////////////////
    // --EdgeImpl
    ///////////////////////////////////////////////////////////////
    class Hedge1EmptyPll : public EdgePllBaseX < Hedge1Empty >
    {
    public :
      inline Hedge1EmptyPll (myvertex_t * a, myvertex_t * b) 
        : EdgePllBaseX < Hedge1Empty > (a, b) {}
    };
    typedef Hedge1Top < Hedge1EmptyPll > hedge1_IMPL ;

    class Hedge1EmptyPllMacro : public EdgePllBaseXMacro < hedge1_IMPL >
    {
    public :
      inline Hedge1EmptyPllMacro (myvertex_t *v0, myvertex_t *v1)
        : EdgePllBaseXMacro < hedge1_IMPL >(v0 ,v1) {}
    };

    ///////////////////////////////////////////////////////////////
    // --FaceImpl
    ///////////////////////////////////////////////////////////////
    class Hface3EmptyPll : public FacePllBaseX< Hface3Empty >
    {
    public :
      // we need to change this typedef here 
      typedef hedge1_IMPL inneredge_t ;

      // constructor 
      inline Hface3EmptyPll (myhedge1_t *e0, int s0, myhedge1_t *e1, int s1, myhedge1_t *e2, int s2)
        : FacePllBaseX< Hface3Empty >( e0, s0, e1, s1, e2, s2 ) {}
    } ;
    typedef Hface3Top < Hface3EmptyPll > hface3_IMPL ;

    class Hface3EmptyPllMacro : public FacePllBaseXMacro< hface3_IMPL >
    {
      typedef FacePllBaseXMacro< hface3_IMPL > Base_t ;
    public :
      Hface3EmptyPllMacro (myhedge1_t * e0, int s0, myhedge1_t *e1,int s1, myhedge1_t *e2, int s2) ;
    };

    class Hface4EmptyPll : public FacePllBaseX< Hface4Empty >
    {
    public :
      // we need to change this typedef here 
      typedef hedge1_IMPL inneredge_t ;

      // constructor 
      inline Hface4EmptyPll (myhedge1_t *e0, int s0, myhedge1_t *e1, int s1, 
                             myhedge1_t *e2, int s2, myhedge1_t *e3, int s3)
        : FacePllBaseX< Hface4Empty >(e0,s0, e1,s1, e2,s2, e3,s3) {} 
    } ;
    typedef Hface4Top < Hface4EmptyPll > hface4_IMPL ;
  
    class Hface4EmptyPllMacro : public FacePllBaseXMacro< hface4_IMPL >
    {
      typedef  FacePllBaseXMacro< hface4_IMPL > Base_t ;
    public :
      Hface4EmptyPllMacro (myhedge1_t *e0, int s0, myhedge1_t *e1, int s1,
                           myhedge1_t *e2, int s2, myhedge1_t *e3, int s3) ;
    } ;

public :
    ///////////////////////////////////////////////////////////////
    // --TetraImpl
    ///////////////////////////////////////////////////////////////
    class TetraEmptyPll : public TetraPllXBase< TetraEmpty >
    {
    protected :
      typedef hedge1_IMPL inneredge_t ;
      typedef hface3_IMPL innerface_t ;
      typedef TetraEmpty :: balrule_t balrule_t;
    public :
      inline TetraEmptyPll (myhface3_t *f0, int s0, myhface3_t *f1, int s1, 
                            myhface3_t *f2, int s2, myhface3_t *f3, int s3) 
        : TetraPllXBase< TetraEmpty >(f0, s0, f1, s1, f2, s2, f3, s3 ) {}
      virtual ElementPllXIF & accessPllX () throw (Parallel :: AccessPllException) { return *this; }
      virtual const ElementPllXIF & accessPllX () const throw (Parallel :: AccessPllException) { return *this; }
    } ;
    typedef TetraTop < TetraEmptyPll > tetra_IMPL ;

    class TetraEmptyPllMacro : public TetraPllXBaseMacro< tetra_IMPL >
    {
    public :
      inline TetraEmptyPllMacro (myhface3_t *f0, int s0, myhface3_t *f1, int s1, 
                                 myhface3_t *f2, int s2, myhface3_t *f3, int s3) 
        : TetraPllXBaseMacro< tetra_IMPL >(0, f0, s0, f1, s1, f2, s2, f3, s3 ) {} // 0 == level 0
      virtual ElementPllXIF & accessPllX () throw (Parallel :: AccessPllException) { return *this; }
      virtual const ElementPllXIF & accessPllX () const throw (Parallel :: AccessPllException) { return *this; }
    } ;

   /////////////////////////////////
   // Periodic 3 
   /////////////////////////////////
    class Periodic3EmptyPll : public Periodic3Empty
    {
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
  
    class Periodic3EmptyPllMacro : public periodic3_IMPL
    {
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

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

    class Periodic4EmptyPll : public Periodic4Empty
    {
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
  
    class Periodic4EmptyPllMacro : public periodic4_IMPL
    {
    public :
      typedef Periodic4PllXBaseMacro mypllx_t ;
      Periodic4EmptyPllMacro (myhface4_t *,int,myhface4_t *,int) ;
     ~Periodic4EmptyPllMacro () ;
      virtual ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
      virtual const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
      virtual void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
    private :
      mypllx_t * _pllx ;
      friend class Periodic4PllXBaseMacro;
    } ;

    ///////////////////////////////////////////////////////////////
    // --HexaImpl
    ///////////////////////////////////////////////////////////////
    class HexaEmptyPll : public HexaPllBaseX< HexaEmpty >
    {
    protected :
      typedef hedge1_IMPL inneredge_t ;
      typedef hface4_IMPL innerface_t ;
      typedef HexaEmpty :: balrule_t balrule_t;
    public :
      inline HexaEmptyPll (myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                           myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                           myhface4_t *f4, int s4, myhface4_t *f5, int s5)
        : HexaPllBaseX< HexaEmpty >(f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5) {}
      virtual ElementPllXIF & accessPllX () throw (Parallel :: AccessPllException) { return *this; }
      virtual const ElementPllXIF & accessPllX () const throw (Parallel :: AccessPllException) { return *this; }
    } ;
    typedef HexaTop < HexaEmptyPll > hexa_IMPL ;

    class HexaEmptyPllMacro : public HexaPllBaseXMacro< hexa_IMPL >
    {
    public :
      inline HexaEmptyPllMacro(myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                               myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                               myhface4_t *f4, int s4, myhface4_t *f5, int s5)
        : HexaPllBaseXMacro< hexa_IMPL >(0, f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5) {}
      virtual ElementPllXIF & accessPllX () throw (Parallel :: AccessPllException) { return *this; }
      virtual const ElementPllXIF & accessPllX () const throw (Parallel :: AccessPllException) { return *this; }
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
        
        virtual VertexGeo     * insert_vertex  (double,double,double,int) ;
        virtual VertexGeo     * insert_ghostvx (double,double,double,int) ;
       
        // insert hbnd_int without ghost hexa 
        virtual hbndseg4_GEO  * insert_hbnd4  (hface4_GEO *, int, Gitter :: hbndseg_STI :: bnd_t) ;
        // insert hbnd_int without ghost hexa 
        virtual hbndseg4_GEO  * insert_hbnd4_ghost (hface4_GEO *, int) ;
        // insert hbnd_int with ghost hexa 
        virtual hbndseg4_GEO  * insert_hbnd4  (hface4_GEO *, int, Gitter :: hbndseg_STI :: bnd_t, MacroGhostInfoHexa* );
        
        // normal insert hbnd3 version
        virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter :: hbndseg_STI :: bnd_t) ;
        // version that get point and create ghost macro 
        virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter :: hbndseg_STI :: bnd_t, MacroGhostInfoTetra* ) ;
        // version that created internal boundary on ghost elements 
        virtual hbndseg3_GEO  * insert_hbnd3_ghost  (hface3_GEO *, int) ;
        virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) ;
        hedge1_GEO    * insert_hedge1_twist (VertexGeo *,int , VertexGeo * , int ) ;
        virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) ;
        virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) ;
        virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) ;
        virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) ;

        virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) ;
        virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) ;

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
    } ;
  protected :
    MpAccessLocal & _mpaccess ;
    MacroGitterPll* _macrogitter ;
    ProjectVertex*  _ppv;
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

    GitterBasisPll (const string, MpAccessLocal &, ProjectVertex* ) ;

    virtual ~GitterBasisPll () ;

    virtual ProjectVertex* vertexProjection() const { return _ppv; }
    
    virtual void printMemUsage();
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

/////////////////////////////////////////////////////
//  --EdgePllBaseX
/////////////////////////////////////////////////////
template < class A >
inline EdgePllBaseX< A > :: EdgePllBaseX( myvertex_t* a, myvertex_t *b )
  : A( a, b ) 
{
}

template < class A >
inline EdgePllBaseX< A > :: ~EdgePllBaseX()
{
#ifndef NDEBUG
  // Falls die nachfolgende Situation eintritt, ist massiv was faul im
  // parallelen Vergr"oberungsalgorithmus: Eine Kante, die gegen Ver-
  // gr"oberung gesperrt war, ist gel"oscht worden. Bestenfalls h"atten
  // die Kinder gel"oscht werden d"urfen, aber nur falls der lock auf-
  // gehoben wird.

  if( myhedge1().isSet( myhedge1_t::flagLock ) )
  {
   cerr << "**FEHLER (FATAL) in Datei " << __FILE__ << " Zeile " << __LINE__ << endl ;
    abort () ;
  }
#endif
}

template < class A > 
inline bool EdgePllBaseX< A > :: lockedAgainstCoarsening () const
{
  return myhedge1().isSet( myhedge1_t::flagLock );
}

template < class A >
inline vector < int > EdgePllBaseX< A > :: estimateLinkage () const {
  return (abort (), vector < int > ()) ;
}

template < class A >
inline LinkedObject :: Identifier EdgePllBaseX< A > :: getIdentifier () const {
  return (abort (), LinkedObject :: Identifier  ()) ;
}

template < class A >
inline void EdgePllBaseX< A > :: getRefinementRequest (ObjectStream & os) const {
  os.writeObject (int(myhedge1 ().getrule ())) ;
  return ;
}

template < class A >
inline bool EdgePllBaseX< A > :: setRefinementRequest (ObjectStream & os) {
  int i ;
  try {
    os.readObject (i) ;
  } catch (ObjectStream :: EOFException) {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  typedef typename myhedge1_t :: myrule_t  myrule_t;
  return myrule_t (i) == myrule_t :: nosplit ? 
    false : (myhedge1 ().refineImmediate (myrule_t (i)), true) ;
}

template < class A >
inline void EdgePllBaseX< A > :: unattach2 (int) {
  abort () ;
  return ;
}

template < class A >
inline void EdgePllBaseX< A > :: attach2 (int) {
  abort () ;
  return ;
}

template < class A >
bool EdgePllBaseX< A > :: packAll (vector < ObjectStream > &) {
  return (abort (), false) ;
}

template < class A >
inline void EdgePllBaseX< A > :: unpackSelf (ObjectStream &,bool) {
  abort () ;
  return ;
}

template < class A >
bool EdgePllBaseX< A > :: lockAndTry ()
{
  myhedge1().set( myhedge1_t::flagLock );
  return myhedge1().coarse () ;
}

template < class A >
inline bool EdgePllBaseX< A > :: unlockAndResume (bool r)
{
  myhedge1().unset( myhedge1_t::flagLock );
  bool x ;
  if (r) {
    x = myhedge1().coarse () ;
  }
  else {
    x = false ;
  }
  return x ;
}

template < class A >
inline EdgePllBaseXMacro< A > :: EdgePllBaseXMacro(myvertex_t * a, myvertex_t * b) :
  A(0, a, b), _moveTo(), _ref()
{
}

template < class A >
inline EdgePllBaseXMacro< A > :: ~EdgePllBaseXMacro()
{
  assert (0 == _moveTo.size ()) ;
}

//////////////////////////////////////////////////////////
//  --FacePllBaseX
//////////////////////////////////////////////////////////
template < class A > inline FacePllBaseX < A > :: FacePllBaseX 
    (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2) 
  : A( e0, s0, e1, s1, e2, s2 )
{
  return ;
}

template < class A > inline FacePllBaseX < A > :: 
FacePllBaseX (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, 
              myhedge1_t * e2, int s2, myhedge1_t * e3, int s3 ) 
  : A( e0, s0, e1, s1, e2, s2, e3, s3 )
{
  return ;
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

template < class A > pair < ElementPllXIF_t *, int > FacePllBaseX < A > :: accessInnerPllX () 
{
  typedef pair < myconnect_t * ,int > myconnectpair_t;
  myconnectpair_t front = myhface ().nb.front ();
  assert( front.first );
  myconnectpair_t rear  = myhface ().nb.rear  ();
  assert( rear.first );
  return front.first->accessPllX ().accessInnerPllX (
            pair < ElementPllXIF_t *, int > (& rear.first->accessPllX (), rear.second), 
            front.second) ;
}

template < class A > pair < const ElementPllXIF_t *, int > FacePllBaseX < A > :: accessInnerPllX () const {
  typedef pair < const myconnect_t * ,int > constmyconnectpair_t;
  constmyconnectpair_t front = myhface ().nb.front ();
  assert( front.first );
  constmyconnectpair_t rear  = myhface ().nb.rear  ();
  assert( rear.first );
  return front.first->accessPllX ().accessInnerPllX (
            pair < const ElementPllXIF_t *, int > (& rear.first->accessPllX (), rear.second), 
            front.second) ;
  //return myhface ().nb.front ().first->accessPllX ().accessInnerPllX (pair < const ElementPllXIF_t *, int > (& myhface ().nb.rear ().first->accessPllX (), myhface ().nb.rear ().second), myhface ().nb.front ().second) ;
}

///////////////////////////////////////////////////////////////////
//
//  --TetraPllXBase
//
///////////////////////////////////////////////////////////////////

template < class A >
inline void TetraPllXBase< A > :: getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
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

inline Periodic3PllXBase :: Periodic3PllXBase (myperiodic3_t & p) : _periodic3 (p) 
{
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

template < class A >
inline void HexaPllBaseX< A > :: getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p)
{
  p.first  = & myhexa();
  p.second = 0;
}

template < class A > inline BndsegPllBaseXMacro < A > :: 
BndsegPllBaseXMacro (myhbnd_t & b) : _hbnd (b) 
{
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

template < class A > inline BndsegPllBaseXClosure < A > :: BndsegPllBaseXClosure (myhbnd_t & b) 
  : _hbnd (b), _ghostLevel (-1), _ghostLeaf(0) 
{
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

template < class A > inline bool BndsegPllBaseXClosure < A > :: lockAndTry ()
{
  myhbnd().set( myhbnd_t::flagLock );
  return myhbnd().bndNotifyCoarsen();
}

template < class A > inline bool BndsegPllBaseXClosure < A > :: lockedAgainstCoarsening () const
{
  return myhbnd().isSet( myhbnd_t::flagLock );
}

template < class A > inline bool BndsegPllBaseXClosure < A > :: unlockAndResume (bool r)
{
  myhbnd().unset( myhbnd_t::flagLock );
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
  : BndsegPllBaseXClosure < A > (b)
  , _ghInfo (0) 
  , _extGraphVertexIndex (-1)
{
  return ;
}

template < class A > inline BndsegPllBaseXMacroClosure < A > :: 
BndsegPllBaseXMacroClosure (myhbnd_t & b, const MacroGhostInfo_STI* ghinfo)
  : BndsegPllBaseXClosure < A > (b)
  , _ghInfo (ghinfo) 
  , _extGraphVertexIndex (-1)
{
  return ;
}

template < class A > inline int BndsegPllBaseXMacroClosure < A > :: ldbVertexIndex () const {
  return _extGraphVertexIndex ;
}

template < class A > inline int & BndsegPllBaseXMacroClosure < A > :: ldbVertexIndex () {
  return _extGraphVertexIndex ;
}

inline GitterBasisPll :: ObjectsPll :: Periodic3EmptyPll :: 
Periodic3EmptyPll (myhface3_t * f0, int t0, myhface3_t * f1, int t1) 
  : GitterBasis :: Objects :: Periodic3Empty (f0,t0,f1,t1), _pllx (*this) {
  return ;
}

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

inline GitterBasisPll :: ObjectsPll :: Periodic4EmptyPll :: 
Periodic4EmptyPll (myhface4_t * f0, int t0, myhface4_t * f1, int t1) 
  : GitterBasis :: Objects :: Periodic4Empty (f0,t0,f1,t1), _pllx (*this) {
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
