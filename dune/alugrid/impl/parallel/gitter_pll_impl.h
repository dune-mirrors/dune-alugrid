// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface
// (c) Robert Kloefkorn 2004 - 2005
#ifndef GITTER_PLL_IMPL_H_INCLUDED
#define GITTER_PLL_IMPL_H_INCLUDED

#include "../serial/myalloc.h"
#include "../serial/gitter_impl.h"
#include "../serial/walk.h"

#include "gitter_pll_sti.h"
#include "gitter_pll_ldb.h"
#include "../serial/ghost_elements.h"

namespace ALUGrid
{

  // Der std::vector< int > wird als sog. linkagepattern, also als
  // Verbindungsmuster eingesetzt. Die Verbindungsmuster werden
  // nicht in jeder Parallelerweiterung gespeichert sondern in
  // einem zentralen Container im verteilten Grobgitter, dem
  // 'linkagePatternMap' und mit Z"ahlung der Referenzen
  // verwaltet. Die Methode secondScan () l"oscht dann immer
  // wieder die unreferenzierten Verbindungsmuster aus dem
  // Container. Es gibt "ubrigens kein firstScan () mehr ...

  template < class A >
  class VertexPllBaseX : public A
  {
    protected :
      typedef A myvertex_t;
      typedef typename A::moveto_t moveto_t;
      inline myvertex_t & myvertex () { return *this; }
      inline const myvertex_t & myvertex () const { return *this; }

      using A::VERTEX;
      void doClearLinkage ();
    public :
      typedef typename A :: ElementLinkage_t ElementLinkage_t ;

      VertexPllBaseX (double,double,double,int,IndexManagerStorageType&);
     ~VertexPllBaseX ();

      virtual void checkAndAddLinkage( const int rank ) ;
      virtual std::vector< int > estimateLinkage () const;
      virtual bool setLinkage ( const std::vector< int >& );
      virtual bool setLinkageSorted ( const std::vector< int >& );
      virtual int linkagePosition () const { return (*_lpn).second; }
      virtual void clearLinkage ();
      virtual LinkedObject::Identifier getIdentifier () const;

    protected :
      bool doSetLinkageSorted ( const std::vector< int >& );
      inline linkagePatternMap_t& linkagePatterns () { return this->indexManagerStorage().linkagePatterns();  }
      const linkagePatternMap_t& linkagePatterns () const { return this->indexManagerStorage().linkagePatterns();  }
      bool doPackLink (const int link, ObjectStream& os );
    public :
      virtual void attach2 (int);
      virtual void unattach2 (int);
      virtual bool packAll  (std::vector< ObjectStream > &);
      virtual moveto_t* moveToMap() { return _moveTo; }
      virtual void unpackSelf (ObjectStream &, bool);

    protected :
      static const linkagePattern_t nullPattern;
      linkagePatternMap_t::iterator _lpn;
      moveto_t*  _moveTo;

      ElementLinkage_t _elements ;
      typedef std::set< int > elementset_t ;

    public:
      virtual bool insertLinkedElements( const elementset_t& elements )
      {
        return _elements.insertElementLinkage( elements );
      }

      const ElementLinkage_t& linkedElements() const { return _elements ; }
  };

  template < class A >
  class EdgePllBaseX : public A
  {
    protected :
      typedef A myhedge_t;
      inline myhedge_t & myhedge () { return *this; }
      inline const myhedge_t & myhedge () const { return *this; }

    public :
      typedef typename A::myvertex_t myvertex_t;

      inline EdgePllBaseX (myvertex_t * a, myvertex_t * b);
      ~EdgePllBaseX ();
      virtual void getRefinementRequest (ObjectStream &) const;
      virtual bool setRefinementRequest (ObjectStream &);
      virtual bool lockAndTry ();
      virtual bool unlockAndResume (bool);
      virtual bool lockedAgainstCoarsening () const;
      virtual void attach2 (int);
      virtual void unattach2 (int);
  };

  template < class A >
  class EdgePllBaseXMacro : public A
  {
    public :
      typedef typename A::myhedge_t  myhedge_t;
      typedef typename A::myvertex_t myvertex_t;
      typedef typename A::moveto_t   moveto_t;

      inline EdgePllBaseXMacro(myvertex_t *,myvertex_t *);
     ~EdgePllBaseXMacro ();
      virtual std::vector< int > estimateLinkage () const;
      virtual LinkedObject::Identifier getIdentifier () const;
    protected :
      using A::myhedge;
      using A::EDGE1;

      bool doPackLink (const int link, ObjectStream& os );
    public :
      virtual void attach2 (int);
      virtual void unattach2 (int);
      virtual bool packAll (std::vector< ObjectStream > &);
      virtual void unpackSelf (ObjectStream &, bool);
      virtual moveto_t* moveToMap() { return _moveTo; }
    protected :
      moveto_t*  _moveTo;
  };

  template < class A > class FacePllBaseX : public A
  {
    protected :
      typedef A myhface_t;
      typedef typename A::myconnect_t myconnect_t;
      typedef typename A::myhedge_t myhedge_t;

    public :
      inline FacePllBaseX(myhedge_t *,int,myhedge_t *,int,myhedge_t *,int);
      inline FacePllBaseX(myhedge_t *,int,myhedge_t *,int,myhedge_t *,int,myhedge_t*,int);
      inline ~FacePllBaseX () {}

      inline myhface_t & myhface () { return *this; }
      inline const myhface_t & myhface () const { return *this; }

      virtual std::vector< int > checkParallelConnectivity () const;
      virtual std::pair< ElementPllXIF_t *, int > accessOuterPllX ();
      virtual std::pair< const ElementPllXIF_t *, int > accessOuterPllX () const;
      virtual std::pair< ElementPllXIF_t *, int > accessInnerPllX ();
      virtual std::pair< const ElementPllXIF_t *, int > accessInnerPllX () const;
  };

  template < class A > class FacePllBaseXMacro : public A
  {
    public :
      // some typedefs
      typedef typename A::myhface_t  myhface_t;
      typedef typename A::myhedge_t myhedge_t;
      typedef typename A::moveto_t   moveto_t;

      // constructor for hface3
      inline FacePllBaseXMacro(int l, myhedge_t * e0, int s0, myhedge_t * e1, int s1,
                                      myhedge_t * e2, int s2);
      // constructor for hface4
      inline FacePllBaseXMacro(int l, myhedge_t * e0, int s0, myhedge_t * e1, int s1,
                                      myhedge_t * e2, int s2, myhedge_t * e3, int s3);
      // destructor only checking move-to
      inline ~FacePllBaseXMacro ();

      virtual std::vector< int > estimateLinkage () const;
      virtual LinkedObject::Identifier getIdentifier () const;

    protected :
      using A::myhface;
      bool doPackLink (const int link, ObjectStream& os );

    public :
      virtual bool ldbUpdateGraphEdge (LoadBalancer::DataBase &, const bool );
      virtual void attach2 (int);
      virtual void unattach2 (int);
      virtual bool packAll (std::vector< ObjectStream > &);
      virtual void unpackSelf (ObjectStream &, bool);
      virtual moveto_t* moveToMap() { return _moveTo; }
    protected :
      moveto_t*  _moveTo;
  };

  template < class A >
  class TetraPllXBase : public A {
    public :
      typedef A mytetra_t;
      typedef typename A::myhface3_t myhface3_t;
    protected:
      inline TetraPllXBase(myhface3_t *f0, int s0, myhface3_t *f1, int s1,
                           myhface3_t *f2, int s2, myhface3_t *f3, int s3)
          : A(f0, s0, f1, s1, f2, s2, f3, s3 ) {}

      inline mytetra_t& mytetra() { return *this; }
      inline const mytetra_t& mytetra() const { return *this; }
    public :
      inline ~TetraPllXBase () {}

      // method to get internal tetra located behind this parallel interface
      virtual void getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p );

    public :
      void writeDynamicState (ObjectStream &, int) const;
      void writeDynamicState (ObjectStream &, GatherScatterType &) const;
  };

  template< class A >
  class TetraPllXBaseMacro
  : public A
  {
    public :
     ~TetraPllXBaseMacro ();
    protected :
      using A::TETRA;
      using A::HBND3INT;
      using A::myneighbour;
      using A::flagLock;
      using A::isSet;
      using A::unset;
      using A::set;
      using A::master;

      typedef A mytetra_t;
      inline mytetra_t& mytetra() { return *this; }
      inline const mytetra_t& mytetra() const { return *this; }

      typedef typename A::myhface3_t myhface3_t;
      typedef typename A::myneighbour_t myneighbour_t;
      typedef typename A::vertexelementlinkage_t vertexelementlinkage_t ;

      TetraPllXBaseMacro(int l, myhface3_t *f0, int s0, myhface3_t *f1, int s1,
                         myhface3_t *f2, int s2, myhface3_t *f3, int s3, SimplexTypeFlag elementType);
    public :
      virtual int ldbVertexIndex () const;
      virtual void writeStaticState (ObjectStream &, int) const ;
      // overload firstLdbVertexIndex from hasFacePllXIF since it only makes sense here
      virtual int firstLdbVertexIndex() const { return ldbVertexIndex(); }
      virtual void setLoadBalanceVertexIndex ( const int );
      virtual bool ldbUpdateGraphVertex (LoadBalancer::DataBase &, GatherScatter* );
      virtual void computeVertexLinkage( vertexelementlinkage_t& ) ;
    public :
      virtual bool erasable () const;
      virtual void attachElement2 ( const int, const int );
      virtual void attach2 (int);
      virtual void unattach2 (int);
      virtual bool packAll (std::vector< ObjectStream > &);
      virtual bool dunePackAll (std::vector< ObjectStream > &, GatherScatterType &);
      // pack ghost information
      virtual void packAsGhost(ObjectStream &,int) const;
      virtual void packAsBnd (int,int,ObjectStream &, const bool) const;
      virtual void unpackSelf (ObjectStream &, bool);
      virtual void duneUnpackSelf (ObjectStream &, const bool, GatherScatterType* );
      virtual int moveTo () const { return _moveTo; }
      virtual void computeBaryCenter( alucoord_t (&center)[3] ) const ;
    protected:
      bool doPackLink (const int link, ObjectStream& os, GatherScatterType* );
      bool doPackAll (std::vector< ObjectStream > &, GatherScatterType * );
      void doUnpackSelf (ObjectStream &, const bool, GatherScatterType* );
      void packAsBndNow (int, ObjectStream &, const bool ) const;

    private :
      // link number corresponding to rank where element is moved to
      int _moveTo;
      // globally unique element number
      int _ldbVertexIndex;
  };

  // ######                                                           #####
  // #     #  ######  #####      #     ####   #####      #     ####  #     #
  // #     #  #       #    #     #    #    #  #    #     #    #    #       #
  // ######   #####   #    #     #    #    #  #    #     #    #       #####
  // #        #       #####      #    #    #  #    #     #    #            #
  // #        #       #   #      #    #    #  #    #     #    #    # #     #
  // #        ######  #    #     #     ####   #####      #     ####   #####

  template < class A >
  class Periodic3PllXBase : public A
  {
    public :
      typedef typename A::myhface3_t myhface3_t;
      typedef A myperiodic_t;

      using A::PERIODIC3;
      using A::HBND3INT;

      Periodic3PllXBase( myhface3_t* f0,int s0, myhface3_t *f1,int s1)
        : A( f0, s0, f1, s1 ) {}

      inline myperiodic_t & myperiodic () { return *this; }
      inline const myperiodic_t & myperiodic () const { return *this; }

      // method to get internal periodic located behind this parallel interface
      virtual void getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p );
    public :
      void writeDynamicState (ObjectStream &, int) const;

      // access interior element and write data
      void writeDynamicState (ObjectStream &, GatherScatterType &) const
      {
        alugrid_assert ( false );
        abort();
      }
  };

  template < class A >
  class Periodic3PllXBaseMacro : public A
  {
    public :
      typedef typename A::myhface3_t myhface3_t;
      typedef typename A::const_myneighbour_t const_myneighbour_t;
      typedef A myperiodic_t;
      typedef Gitter::hface_STI hface_STI;

      using A::PERIODIC3;
      using A::HBND3INT;
      using A::myneighbour;
      using A::myhface3;
      using A::flagLock;
      using A::isSet;
      using A::unset;
      using A::set;

      Periodic3PllXBaseMacro (int, myhface3_t* f0,int s0, myhface3_t *f1,int s1,
                              const Gitter::hbndseg_STI::bnd_t (&bt)[2] );
     ~Periodic3PllXBaseMacro ();

    protected:
      inline myperiodic_t & myperiodic () { return *this; }
      inline const myperiodic_t & myperiodic () const { return *this; }
      bool doPackLink (const int link, ObjectStream& os );
    public :
      virtual void attachPeriodic( const int destination );
      virtual std::pair<int,int> insideLdbVertexIndex() const;
      virtual int otherLdbVertexIndex( const int faceIndex ) const;
    public :
      virtual void attach2 (int);
      virtual void unattach2 (int);
      virtual bool packAll (std::vector< ObjectStream > &);
      virtual void packAsBnd (int,int,ObjectStream &, const bool) const;
      virtual void unpackSelf (ObjectStream &, bool);
      virtual bool erasable () const
      {
        alugrid_assert ( ! isSet( flagLock ) == ( _moveTo >= 0 ) );
        return ! isSet( flagLock );
      }
    private :
      int _moveTo;
  };

  // ######                                                          #
  // #     #  ######  #####      #     ####   #####      #     ####  #    #
  // #     #  #       #    #     #    #    #  #    #     #    #    # #    #
  // ######   #####   #    #     #    #    #  #    #     #    #      #    #
  // #        #       #####      #    #    #  #    #     #    #      #######
  // #        #       #   #      #    #    #  #    #     #    #    #      #
  // #        ######  #    #     #     ####   #####      #     ####       #

  template < class A >
  class Periodic4PllXBase : public A {
    public :
      typedef typename A::myhface4_t myhface4_t;
      typedef A myperiodic_t;

      using A::PERIODIC4;
      using A::HBND4INT;

      Periodic4PllXBase( myhface4_t* f0, int s0, myhface4_t *f1, int s1)
        : A( f0, s0, f1, s1 ) {}

    protected:
      inline myperiodic_t & myperiodic () { return *this; }
      inline const myperiodic_t & myperiodic () const { return *this; }
    public :
      inline ~Periodic4PllXBase () {}
    public :
      // method to get internal periodic located behind this parallel interface
      virtual void getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p );

      void writeDynamicState (ObjectStream &, int) const;
      void writeDynamicState (ObjectStream &, GatherScatterType &) const
      {
        alugrid_assert ( false ); abort();
      }
  };

  template < class A >
  class Periodic4PllXBaseMacro : public A
  {
    public :
      typedef typename A::myhface4_t    myhface4_t;
      typedef typename A::const_myneighbour_t const_myneighbour_t;
      typedef A myperiodic_t;
      typedef Gitter::hface_STI hface_STI;

      Periodic4PllXBaseMacro (int, myhface4_t* f0,int s0, myhface4_t *f1,int s1,
                              const Gitter::hbndseg_STI::bnd_t (&bt)[2] );
      ~Periodic4PllXBaseMacro ();

      using A::PERIODIC4;
      using A::HBND4INT;
      using A::myneighbour;
      using A::myhface4;
      using A::flagLock;
      using A::isSet;
      using A::unset;
      using A::set;


    protected:
      inline myperiodic_t & myperiodic () { return *this; }
      inline const myperiodic_t & myperiodic () const { return *this; }
      bool doPackLink (const int link, ObjectStream& os );
    public :
      virtual void attachPeriodic( const int destination );
      virtual std::pair<int,int> insideLdbVertexIndex() const;
      virtual int otherLdbVertexIndex( const int faceIndex ) const;
    public :
      virtual void attach2 (int);
      virtual void unattach2 (int);
      virtual bool packAll (std::vector< ObjectStream > &);
      virtual void packAsBnd (int,int,ObjectStream &, const bool) const;
      virtual void unpackSelf (ObjectStream &, bool);
      virtual bool erasable () const;
    private :
      int _moveTo;
  };

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
      typedef typename A::myhface4_t myhface4_t;
      typedef A  myhexa_t;
      inline myhexa_t & myhexa () { return *this; }
      inline const myhexa_t & myhexa () const { return *this; }

      inline HexaPllBaseX(myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                          myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                          myhface4_t *f4, int s4, myhface4_t *f5, int s5)
          : A(f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5) {}
    public :
      void writeDynamicState (ObjectStream &, int) const;
      void writeDynamicState (ObjectStream &, GatherScatterType &) const;

      // method to get internal hexa located behind this parallel interface
      virtual void getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);
  };

  template < class A >
  class HexaPllBaseXMacro : public A
  {
    protected:
      using A::HEXA;
      using A::HBND4INT;
      using A::myneighbour;
      using A::flagLock;
      using A::isSet;
      using A::unset;
      using A::set;
      using A::master;

      typedef A  myhexa_t;
      inline myhexa_t & myhexa () { return *this; }
      inline const myhexa_t & myhexa () const { return *this; }

      typedef typename A::myhface4_t myhface4_t;
      typedef typename A::myneighbour_t  myneighbour_t;
      typedef typename A::vertexelementlinkage_t vertexelementlinkage_t ;

      HexaPllBaseXMacro(int l, myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                               myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                               myhface4_t *f4, int s4, myhface4_t *f5, int s5);
    public :
     ~HexaPllBaseXMacro ();
      virtual int ldbVertexIndex () const;
      virtual void writeStaticState (ObjectStream &, int) const ;
      // overload firstLdbVertexIndex from hasFacePllXIF since it only makes sense here
      virtual int firstLdbVertexIndex() const { return ldbVertexIndex(); }
      virtual void setLoadBalanceVertexIndex ( const int );
      virtual bool ldbUpdateGraphVertex (LoadBalancer::DataBase &, GatherScatter* );
      virtual void computeVertexLinkage( vertexelementlinkage_t& ) ;
    public:
      virtual void attachElement2 ( const int, const int );
      virtual void attach2 (int);
      virtual void unattach2 (int);

      virtual bool packAll (std::vector< ObjectStream > &);
      // pack ghost information
      virtual void packAsGhost(ObjectStream &,int) const;
      virtual void packAsBnd (int,int,ObjectStream &, const bool) const;
      virtual void unpackSelf (ObjectStream &, bool);
      virtual bool erasable () const;

      // pack and unpack funtions for dune
      virtual bool dunePackAll (std::vector< ObjectStream > &, GatherScatterType &);
      virtual void duneUnpackSelf (ObjectStream &, const bool, GatherScatterType* );
      virtual int moveTo () const { return _moveTo; }
      virtual void computeBaryCenter( alucoord_t (&center)[3] ) const ;
    protected :
      bool doPackLink (const int link, ObjectStream& os, GatherScatterType* );
      bool doPackAll( std::vector< ObjectStream > &, GatherScatterType * );
      void doUnpackSelf (ObjectStream &, const bool, GatherScatterType* );
      void packAsBndNow (int, ObjectStream &, const bool ) const;
    protected:
      // link number corresponding to rank where element is moved to
      int _moveTo;
      // globally unique element number
      int _ldbVertexIndex;
  };

  class BndsegPllBaseX : public ElementPllXIF_t
  {
    public :
      void writeDynamicState (ObjectStream &, int) const { abort (); }
      void writeDynamicState (ObjectStream &, GatherScatterType &) const { alugrid_assert (false); abort(); }
      std::pair< ElementPllXIF_t *, int > accessOuterPllX (const std::pair< ElementPllXIF_t *, int > &, int);
      std::pair< const ElementPllXIF_t *, int > accessOuterPllX (const std::pair< const ElementPllXIF_t *, int > &, int) const;
      std::pair< ElementPllXIF_t *, int > accessInnerPllX (const std::pair< ElementPllXIF_t *, int > &, int);
      std::pair< const ElementPllXIF_t *, int > accessInnerPllX (const std::pair< const ElementPllXIF_t *, int > &, int) const;
  };

  template < class A > class BndsegPllBaseXMacro : public BndsegPllBaseX {
    protected :
      typedef A                       myhbnd_t;
      typedef typename A::myhface_t myhface_t;
      typedef typename A::balrule_t balrule_t;
      inline myhbnd_t & myhbnd ();
      inline const myhbnd_t & myhbnd () const;
    public :
      inline BndsegPllBaseXMacro (myhbnd_t &);
      virtual void packAsBnd (int,int,ObjectStream &, const bool) const;

      // method to get internal bnd located behind this parallel interface
      virtual void getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);

    private :
      myhbnd_t & _hbnd;
  };

  template < class A > class BndsegPllBaseXClosure : public BndsegPllBaseX {
    protected :
      typedef A                       myhbnd_t;
      typedef typename A::myhface_t myhface_t;
      typedef typename A::balrule_t balrule_t;
      inline myhbnd_t & myhbnd ();
      inline const myhbnd_t & myhbnd () const;
    public :
      inline BndsegPllBaseXClosure (myhbnd_t &);
      ~BndsegPllBaseXClosure () {}

      void readDynamicState (ObjectStream &, int);
      void readDynamicState (ObjectStream &, GatherScatterType &);
      void writeDynamicState (ObjectStream &, GatherScatterType &) const;
      using BndsegPllBaseX::writeDynamicState;

      void getRefinementRequest (ObjectStream &);
      bool setRefinementRequest (ObjectStream &);
    public :
      bool lockAndTry ();
      bool unlockAndResume (bool);
    public :
      virtual void notifyBalance (balrule_t,int);
      virtual bool lockedAgainstCoarsening () const;

      // method to get internal bnd located behind this parallel interface
      virtual void getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p);

    private :
      myhbnd_t & _hbnd;
      int  _ghostLevel;
      bool _ghostLeaf;
      balrule_t _rul;

    public:
      inline int ghostLevel () const { return _ghostLevel; }
      inline bool ghostLeaf () const { return (_ghostLevel == myhbnd().level()) && _ghostLeaf; }

      typedef Gitter::ghostpair_STI ghostpair_STI;
      // to be revised (works for the moment )
      virtual ghostpair_STI getGhost () { return myhbnd().getGhost(); }
      virtual const ghostpair_STI getGhost () const { return myhbnd().getGhost(); }
  };

  template < class A > class BndsegPllBaseXMacroClosure : public BndsegPllBaseXClosure < A > {
    public :
      typedef A                       myhbnd_t;
      typedef typename A::myhface_t myhface_t;
      inline BndsegPllBaseXMacroClosure (myhbnd_t &);
      inline BndsegPllBaseXMacroClosure (myhbnd_t &, const MacroGhostInfo_STI* );
    public :
      virtual int  ldbVertexIndex () const;
      virtual int master () const
      {
        // alugrid_assert ( _master != this->myhbnd().myvertex(0,0)->indexManagerStorage ().myrank() );
        return _master;
      }
      virtual void readStaticState (ObjectStream &, int) ;
      virtual void setLoadBalanceVertexIndex ( const int );
      virtual void setMaster ( const int master ) {
        _master = master;
      }
    public :
      virtual void packAsBnd (int,int,ObjectStream &, const bool) const;

      // unpack ghost information and insert ghost cell
      virtual void insertGhostCell(ObjectStream &,int);

    private :
      const MacroGhostInfo_STI * _ghInfo;
      int _ldbVertexIndex;
      int _master;
  };

  class GitterBasisPll : public Gitter::Geometric, public GitterPll
  {
  public :
    class ObjectsPll : public GitterBasis::Objects
    {
    public :
      ///////////////////////////////////////////////////////////////
      // --VertexImpl
      ///////////////////////////////////////////////////////////////
      class VertexPllImplMacro : public VertexPllBaseX< VertexEmptyMacro >
      {
      public :
        VertexPllImplMacro (double x, double y, double z, int i, IndexManagerStorageType& ims, linkagePatternMap_t & map)
          : VertexPllBaseX< VertexEmptyMacro >( x, y, z, i, ims )
        {
          alugrid_assert ( &map == &ims.linkagePatterns() );
        }
        virtual VertexPllXIF_t & accessPllX () { return *this; }
        virtual const VertexPllXIF_t & accessPllX () const { return *this; }
      };

      ///////////////////////////////////////////////////////////////
      // --EdgeImpl
      ///////////////////////////////////////////////////////////////
      class Hedge1EmptyPll : public EdgePllBaseX < Hedge1Empty >
      {
      public :
        inline Hedge1EmptyPll (myvertex_t * a, myvertex_t * b)
          : EdgePllBaseX < Hedge1Empty > (a, b) {}
      };
      typedef Hedge1Top < Hedge1EmptyPll > hedge1_IMPL;

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
        typedef hedge1_IMPL inneredge_t;

        // constructor
        inline Hface3EmptyPll (myhedge_t *e0, int s0, myhedge_t *e1, int s1, myhedge_t *e2, int s2)
          : FacePllBaseX< Hface3Empty >( e0, s0, e1, s1, e2, s2 ) {}
      };
      typedef Hface3Top < Hface3EmptyPll > hface3_IMPL;

      class Hface3EmptyPllMacro : public FacePllBaseXMacro< hface3_IMPL >
      {
        typedef FacePllBaseXMacro< hface3_IMPL > Base_t;
      public :
        Hface3EmptyPllMacro (myhedge_t * e0, int s0, myhedge_t *e1,int s1, myhedge_t *e2, int s2);
      };

      class Hface4EmptyPll : public FacePllBaseX< Hface4Empty >
      {
      public :
        // we need to change this typedef here
        typedef hedge1_IMPL inneredge_t;

        // constructor
        inline Hface4EmptyPll (myhedge_t *e0, int s0, myhedge_t *e1, int s1,
                               myhedge_t *e2, int s2, myhedge_t *e3, int s3)
          : FacePllBaseX< Hface4Empty >(e0,s0, e1,s1, e2,s2, e3,s3) {}
      };
      typedef Hface4Top < Hface4EmptyPll > hface4_IMPL;

      class Hface4EmptyPllMacro : public FacePllBaseXMacro< hface4_IMPL >
      {
        typedef  FacePllBaseXMacro< hface4_IMPL > Base_t;
      public :
        Hface4EmptyPllMacro (myhedge_t *e0, int s0, myhedge_t *e1, int s1,
                             myhedge_t *e2, int s2, myhedge_t *e3, int s3);
      };

  public :
      ///////////////////////////////////////////////////////////////
      // --TetraImpl
      ///////////////////////////////////////////////////////////////
      class TetraEmptyPll : public TetraPllXBase< TetraEmpty >
      {
      protected :
        typedef hedge1_IMPL inneredge_t;
        typedef hface3_IMPL innerface_t;
        typedef TetraEmpty::balrule_t balrule_t;
      public :
        inline TetraEmptyPll (myhface3_t *f0, int s0, myhface3_t *f1, int s1,
                              myhface3_t *f2, int s2, myhface3_t *f3, int s3)
          : TetraPllXBase< TetraEmpty >(f0, s0, f1, s1, f2, s2, f3, s3 ) {}
        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };
      typedef TetraTop < TetraEmptyPll > tetra_IMPL;

      class TetraEmptyPllMacro : public TetraPllXBaseMacro< tetra_IMPL >
      {
      public :
        inline TetraEmptyPllMacro (myhface3_t *f0, int s0, myhface3_t *f1, int s1,
                                   myhface3_t *f2, int s2, myhface3_t *f3, int s3, SimplexTypeFlag elementType)
          : TetraPllXBaseMacro< tetra_IMPL >(0, f0, s0, f1, s1, f2, s2, f3, s3, elementType) {} // 0 == level 0
        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };

     /////////////////////////////////
     // Periodic 3
     /////////////////////////////////
     class Periodic3EmptyPll : public Periodic3PllXBase< Periodic3Empty >
     {
      protected :
        typedef hedge1_IMPL inneredge_t;
        typedef hface3_IMPL innerface_t;
      public :
        inline Periodic3EmptyPll (myhface3_t * f0, int s0, myhface3_t *f1, int s1 )
          : Periodic3PllXBase< Periodic3Empty >( f0, s0, f1, s1 ) {}
        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };
      typedef Periodic3Top < Periodic3EmptyPll > periodic3_IMPL;

      class Periodic3EmptyPllMacro : public Periodic3PllXBaseMacro< periodic3_IMPL >
      {
      public :
        Periodic3EmptyPllMacro (myhface3_t* f0, int s0, myhface3_t* f1, int s1,
                                const Gitter:: hbndseg_STI::bnd_t (&bt)[2] )
          : Periodic3PllXBaseMacro< periodic3_IMPL >( 0, f0, s0, f1, s1, bt ) {}
        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };

  // ######                                                          #
  // #     #  ######  #####      #     ####   #####      #     ####  #    #
  // #     #  #       #    #     #    #    #  #    #     #    #    # #    #
  // ######   #####   #    #     #    #    #  #    #     #    #      #    #
  // #        #       #####      #    #    #  #    #     #    #      #######
  // #        #       #   #      #    #    #  #    #     #    #    #      #
  // #        ######  #    #     #     ####   #####      #     ####       #

      class Periodic4EmptyPll : public Periodic4PllXBase< Periodic4Empty >
      {
      protected :
        typedef hedge1_IMPL inneredge_t;
        typedef hface4_IMPL innerface_t;
      public :
        inline Periodic4EmptyPll (myhface4_t* f0, int s0, myhface4_t* f1, int s1)
          : Periodic4PllXBase< Periodic4Empty >( f0, s0, f1, s1 ) {}

        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };
      typedef Periodic4Top < Periodic4EmptyPll > periodic4_IMPL;

      class Periodic4EmptyPllMacro : public Periodic4PllXBaseMacro< periodic4_IMPL >
      {
      public :
        Periodic4EmptyPllMacro (myhface4_t* f0, int s0, myhface4_t* f1, int s1,
                                const Gitter:: hbndseg_STI::bnd_t (&bt)[2] )
          : Periodic4PllXBaseMacro< periodic4_IMPL >( 0, f0, s0, f1, s1, bt ) {}

        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };

      ///////////////////////////////////////////////////////////////
      // --HexaImpl
      ///////////////////////////////////////////////////////////////
      class HexaEmptyPll : public HexaPllBaseX< HexaEmpty >
      {
      protected :
        typedef hedge1_IMPL inneredge_t;
        typedef hface4_IMPL innerface_t;
        typedef HexaEmpty::balrule_t balrule_t;
      public :
        inline HexaEmptyPll (myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                             myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                             myhface4_t *f4, int s4, myhface4_t *f5, int s5)
          : HexaPllBaseX< HexaEmpty >(f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5) {}
        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };
      typedef HexaTop < HexaEmptyPll > hexa_IMPL;

      class HexaEmptyPllMacro : public HexaPllBaseXMacro< hexa_IMPL >
      {
      public :
        inline HexaEmptyPllMacro(myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                                 myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                                 myhface4_t *f4, int s4, myhface4_t *f5, int s5)
          : HexaPllBaseXMacro< hexa_IMPL >(0, f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5) {}
        virtual ElementPllXIF & accessPllX () { return *this; }
        virtual const ElementPllXIF & accessPllX () const { return *this; }
      };

      // Die Randelemente des verteilten Gitters werden aus Templates
      // in 'gitter_hexa_top_pll.h' und 'gitter_tetra_top_pll.h' erzeugt
      // indem diese die Randelementklassen des sequentiellen Verfahrens
      // "ubernehmen und mit passenden Extendern anreichern.
    };

    public:
      class MacroGitterBasisPll
      : public MacroGitterPll,
        public GitterBasis::MacroGitterBasis
      {
        protected :
          linkagePatternMap_t& _linkagePatterns;
          void secondScan( std::set< int >& );
          void clearLinkagePattern();
        protected :
          int iterators_attached () const;

          virtual VertexGeo     * insert_vertex  (double,double,double,int);
          virtual VertexGeo     * insert_ghostvx (double,double,double,int);

          // insert hbnd_int without ghost hexa
          virtual hbndseg4_GEO  * insert_hbnd4  (hface4_GEO *, int, Gitter::hbndseg_STI::bnd_t);
          // insert hbnd_int with ghost hexa
          virtual hbndseg4_GEO  * insert_hbnd4  (hface4_GEO *, int, Gitter::hbndseg_STI::bnd_t, MacroGhostInfoHexa* );

          // normal insert hbnd3 version
          virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter::hbndseg_STI::bnd_t);
          // version that get point and create ghost macro
          virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter::hbndseg_STI::bnd_t, MacroGhostInfoTetra* );
          // version that created internal boundary on ghost elements
          virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *);
          hedge1_GEO    * insert_hedge1_twist (VertexGeo *,int , VertexGeo * , int );
          virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]);
          virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]);
          virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]);
          virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4], SimplexTypeFlag);

          virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2], const Gitter::hbndseg_STI::bnd_t (&)[2] );
          virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2], const Gitter::hbndseg_STI::bnd_t (&)[2] );

          using GitterBasis::MacroGitterBasis::iterator;
        public :
          MacroGitterBasisPll (const int, GitterBasisPll *, const ProjectVertexPtrPair&, std::istream & );
          MacroGitterBasisPll (const int, GitterBasisPll *, const ProjectVertexPtrPair& );
         ~MacroGitterBasisPll ();
      }; // end MacroGitterBasisPll

    protected :
      MpAccessLocal & _mpaccess;
      MacroGitterPll* _macrogitter;
    public :
      virtual inline Makrogitter & container ();
      virtual inline const Makrogitter & container () const;
      virtual inline MpAccessLocal & mpAccess ();
      virtual inline const MpAccessLocal & mpAccess () const;
    protected :
      GitterBasisPll (const int dim, MpAccessLocal & );
      virtual void dumpInfo() const { container().dumpInfo( _mpaccess.myrank() ); }

    public :
      virtual inline MacroGitterPll & containerPll ();
      virtual inline const MacroGitterPll & containerPll () const;

      GitterBasisPll ( const int dim, const std::string &, MpAccessLocal &, const ProjectVertexPtrPair& );
      GitterBasisPll ( const int dim, std::istream &in, MpAccessLocal &, const ProjectVertexPtrPair& );

      virtual ~GitterBasisPll ();

      virtual void printMemUsage();
  };


    //
    //    #    #    #  #          #    #    #  ######
    //    #    ##   #  #          #    ##   #  #
    //    #    # #  #  #          #    # #  #  #####
    //    #    #  # #  #          #    #  # #  #
    //    #    #   ##  #          #    #   ##  #
    //    #    #    #  ######     #    #    #  ######
    //

  /////////////////////////////////////////////////////
  //  --EdgePllBaseX
  /////////////////////////////////////////////////////
  template < class A >
  inline EdgePllBaseX< A >::EdgePllBaseX( myvertex_t* a, myvertex_t *b )
    : A( a, b )
  {
  }

  template < class A >
  inline EdgePllBaseX< A >::~EdgePllBaseX()
  {
#ifdef ALUGRIDDEBUG
    // Falls die nachfolgende Situation eintritt, ist massiv was faul im
    // parallelen Vergr"oberungsalgorithmus: Eine Kante, die gegen Ver-
    // gr"oberung gesperrt war, ist gel"oscht worden. Bestenfalls h"atten
    // die Kinder gel"oscht werden d"urfen, aber nur falls der lock auf-
    // gehoben wird.

    if( myhedge().isSet( myhedge_t::flagLock ) )
    {
      std::cerr << "**FEHLER (FATAL) in Datei " << __FILE__ << " Zeile " << __LINE__ << std::endl;
      abort ();
    }
#endif
  }

  template < class A >
  inline bool EdgePllBaseX< A >::lockedAgainstCoarsening () const
  {
    return myhedge().isSet( myhedge_t::flagLock );
  }

  template < class A >
  inline void EdgePllBaseX< A >::getRefinementRequest (ObjectStream & os) const
  {
    os.put( char(myhedge ().getrule ()) );
    return;
  }

  template < class A >
  inline bool EdgePllBaseX< A >::setRefinementRequest (ObjectStream & os) {
    char i;
    try {
      i = os.get();
    }
    catch( ObjectStream::EOFException& )
    {
      std::cerr << "ERROR (fatal): EdgePllBaseX< A >::setRefinementRequest EOF encountered." << std::endl;
      abort();
    }
    typedef typename myhedge_t::myrule_t  myrule_t;
    return myrule_t (i) == myrule_t::nosplit ?
      false : (myhedge ().refineImmediate (myrule_t (i)), true);
  }

  template < class A >
  inline void EdgePllBaseX< A >::unattach2 (int) {
    abort ();
    return;
  }

  template < class A >
  inline void EdgePllBaseX< A >::attach2 (int) {
    abort ();
    return;
  }

  template < class A >
  bool EdgePllBaseX< A >::lockAndTry ()
  {
    myhedge().set( myhedge_t::flagLock );
    return myhedge().coarse ();
  }

  template < class A >
  inline bool EdgePllBaseX< A >::unlockAndResume (bool r)
  {
    myhedge().unset( myhedge_t::flagLock );
    return ( r ) ? myhedge().coarse () : false;
  }

  template < class A >
  inline EdgePllBaseXMacro< A >::EdgePllBaseXMacro(myvertex_t * a, myvertex_t * b) :
    A(0, a, b), _moveTo( 0 )
  {
  }

  template < class A >
  inline EdgePllBaseXMacro< A >::~EdgePllBaseXMacro()
  {
    alugrid_assert ( _moveTo == 0 );
    //alugrid_assert (0 == _moveTo.size ());
  }

  //////////////////////////////////////////////////////////
  //  --FacePllBaseX
  //////////////////////////////////////////////////////////
  template < class A > inline FacePllBaseX < A >::FacePllBaseX
      (myhedge_t * e0, int s0, myhedge_t * e1, int s1, myhedge_t * e2, int s2)
    : A( e0, s0, e1, s1, e2, s2 )
  {
    return;
  }

  template < class A > inline FacePllBaseX < A >::
  FacePllBaseX (myhedge_t * e0, int s0, myhedge_t * e1, int s1,
                myhedge_t * e2, int s2, myhedge_t * e3, int s3 )
    : A( e0, s0, e1, s1, e2, s2, e3, s3 )
  {
    return;
  }

  template < class A > std::vector< int >  FacePllBaseX < A >::checkParallelConnectivity () const {
    std::vector< int > v (A::polygonlength + 1);
    int i;
    for (i = 0; i < A::polygonlength; ++i )
      v [i] = myhface ().myvertex (0)->ident ();
    v [i] = myhface ().level ();
    return v;
  }

  template < class A > std::pair< ElementPllXIF_t *, int > FacePllBaseX < A >::accessOuterPllX () {
    return myhface ().nb.front ().first->accessPllX ().accessOuterPllX (std::pair< ElementPllXIF_t *, int > (& myhface ().nb.rear ().first->accessPllX (),myhface ().nb.rear ().second), myhface ().nb.front ().second);
  }

  template < class A > std::pair< const ElementPllXIF_t *, int > FacePllBaseX < A >::accessOuterPllX () const {
    return myhface ().nb.front ().first->accessPllX ().accessOuterPllX (std::pair< const ElementPllXIF_t *, int > (& myhface ().nb.rear ().first->accessPllX (), myhface ().nb.rear ().second), myhface ().nb.front ().second);
  }

  template < class A > std::pair< ElementPllXIF_t *, int > FacePllBaseX < A >::accessInnerPllX ()
  {
    typedef std::pair< myconnect_t * ,int > myconnectpair_t;
    myconnectpair_t front = myhface ().nb.front ();
    alugrid_assert ( front.first );
    myconnectpair_t rear  = myhface ().nb.rear  ();
    alugrid_assert ( rear.first );
    return front.first->accessPllX ().accessInnerPllX (
              std::pair< ElementPllXIF_t *, int > (& rear.first->accessPllX (), rear.second),
              front.second);
  }

  template < class A > std::pair< const ElementPllXIF_t *, int > FacePllBaseX < A >::accessInnerPllX () const {
    typedef std::pair< const myconnect_t * ,int > constmyconnectpair_t;
    constmyconnectpair_t front = myhface ().nb.front ();
    alugrid_assert ( front.first );
    constmyconnectpair_t rear  = myhface ().nb.rear  ();
    alugrid_assert ( rear.first );
    return front.first->accessPllX ().accessInnerPllX (
              std::pair< const ElementPllXIF_t *, int > (& rear.first->accessPllX (), rear.second),
              front.second);
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  --TetraPllXBase
  //
  ///////////////////////////////////////////////////////////////////

  template < class A >
  inline void TetraPllXBase< A >::getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
  {
    p.first  = & mytetra();
    p.second = 0;
  }

  template < class A >
  inline void Periodic3PllXBase< A >::getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
  {
    p.first  = & myperiodic();
    p.second = 0;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  --HexaPllXBase
  //
  ///////////////////////////////////////////////////////////////////

  template < class A >
  inline void HexaPllBaseX< A >::getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p)
  {
    p.first  = & myhexa();
    p.second = 0;
  }

  template < class A >
  inline void Periodic4PllXBase< A >::getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
  {
    p.first  = & myperiodic();
    p.second = 0;
  }

  template < class A > inline BndsegPllBaseXMacro < A >::
  BndsegPllBaseXMacro (myhbnd_t & b) : _hbnd (b)
  {
  }

  template < class A > inline typename BndsegPllBaseXMacro < A >::myhbnd_t & BndsegPllBaseXMacro < A >::myhbnd () {
    return _hbnd;
  }

  template < class A > inline const typename BndsegPllBaseXMacro < A >::myhbnd_t & BndsegPllBaseXMacro < A >::myhbnd () const {
    return _hbnd;
  }

  template < class A > void BndsegPllBaseXMacro < A >::
  packAsBnd (int fce, int who, ObjectStream & os, const bool ghostCellsEnabled ) const
  {
    alugrid_assert (!fce);
    if (myhface_t::polygonlength == 3) os.writeObject (MacroGridMoverIF::HBND3EXT);
    else if (myhface_t::polygonlength == 4) os.writeObject (MacroGridMoverIF::HBND4EXT);
    else abort ();
    os.writeObject (myhbnd ().bndtype ());
    {
      for (int i = 0; i < myhface_t::polygonlength; ++i)
        os.writeObject ( myhbnd ().myvertex (fce,i)->ident () );
    }

    const typename ProjectVertex::ProjectionType projectionType = myhbnd().projectionType();
    os.put( projectionType );
    // only segment projection needs to be backed up
    if( projectionType == ProjectVertex::segment )
    {
      myhbnd ().projection().backup( os );
    }
    return;
  }

  template < class A > inline void BndsegPllBaseXMacro < A >::
  getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
  {
    p.first  = 0;
    p.second = & myhbnd ();
    return;
  }

  template < class A > inline void BndsegPllBaseXClosure < A >::
  getAttachedElement ( std::pair< Gitter::helement_STI* , Gitter::hbndseg_STI * > & p )
  {
    p.first  = 0;
    p.second = & myhbnd ();
    return;
  }

  template < class A > inline BndsegPllBaseXClosure < A >::BndsegPllBaseXClosure (myhbnd_t & b)
    : _hbnd (b), _ghostLevel (-1), _ghostLeaf( false )
  {
    return;
  }

  template < class A > inline typename BndsegPllBaseXClosure < A >::myhbnd_t & BndsegPllBaseXClosure < A >::myhbnd () {
    return _hbnd;
  }

  template < class A > inline const typename BndsegPllBaseXClosure < A >::myhbnd_t & BndsegPllBaseXClosure < A >::myhbnd () const {
    return _hbnd;
  }

  template < class A > inline void BndsegPllBaseXClosure < A >::notifyBalance (balrule_t r,int) {
    _rul = r;
    return;
  }

  template < class A > inline bool BndsegPllBaseXClosure < A >::lockAndTry ()
  {
    myhbnd().set( myhbnd_t::flagLock );
    return myhbnd().bndNotifyCoarsen();
  }

  template < class A > inline bool BndsegPllBaseXClosure < A >::lockedAgainstCoarsening () const
  {
    return myhbnd().isSet( myhbnd_t::flagLock );
  }

  template < class A > inline bool BndsegPllBaseXClosure < A >::unlockAndResume (bool r)
  {
    myhbnd().unset( myhbnd_t::flagLock );
    bool x;
    if (r) {
      x = myhbnd ().bndNotifyCoarsen ();
    }
    else {
      x = false;
    }
    return x;
  }

  template < class A > inline BndsegPllBaseXMacroClosure < A >::BndsegPllBaseXMacroClosure (myhbnd_t & b)
    : BndsegPllBaseXClosure < A > (b)
    , _ghInfo (0)
    , _ldbVertexIndex (-2)
    , _master( -2 )
  {
  }

  template < class A > inline BndsegPllBaseXMacroClosure < A >::
  BndsegPllBaseXMacroClosure (myhbnd_t & b, const MacroGhostInfo_STI* ghinfo)
    : BndsegPllBaseXClosure < A > (b)
    , _ghInfo (ghinfo)
    , _ldbVertexIndex (-2)
    , _master( -2 )
  {
  }

  template < class A > inline int BndsegPllBaseXMacroClosure < A >::ldbVertexIndex () const
  {
    alugrid_assert ( _ldbVertexIndex != -2 );
    alugrid_assert ( _ldbVertexIndex >= 0 );
    return _ldbVertexIndex;
  }

  template < class A > inline void BndsegPllBaseXMacroClosure < A >::setLoadBalanceVertexIndex ( const int ldbVx )
  {
    alugrid_assert ( ldbVx >= 0 );
    _ldbVertexIndex = ldbVx;
  }

  inline int GitterBasisPll::MacroGitterBasisPll::iterators_attached () const
  {
    return GitterBasis::MacroGitterBasis::iterators_attached () + MacroGitterPll::iterators_attached ();
  }

  inline MpAccessLocal & GitterBasisPll::mpAccess ()
  {
    return _mpaccess;
  }

  inline const MpAccessLocal & GitterBasisPll::mpAccess () const
  {
    return _mpaccess;
  }

  inline GitterBasisPll::Makrogitter & GitterBasisPll::container ()
  {
    return * _macrogitter;
  }

  inline const GitterBasisPll::Makrogitter & GitterBasisPll::container () const
  {
    return * _macrogitter;
  }

  inline GitterBasisPll::MacroGitterPll & GitterBasisPll::containerPll ()
  {
    return * _macrogitter;
  }

  inline const GitterBasisPll::MacroGitterPll & GitterBasisPll::containerPll () const
  {
    return * _macrogitter;
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_PLL_IMPL_H_INCLUDED
