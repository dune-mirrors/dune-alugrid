// (c) bernhard schupp 1997 - 1998
// modification for Dune Interface
// (c) Robert Kloefkorn 2004 - 2005
#ifndef GITTER_TETRA_TOP_PLL_H_INCLUDED
#define GITTER_TETRA_TOP_PLL_H_INCLUDED

#include "../serial/parallel.h"
#include "gitter_pll_impl.h"
#include "../serial/gitter_tetra_top.h"

namespace ALUGrid
{

  class MacroGhost;

    // Nachfolgend steht ein Template zum Aufbohren der Randklassen f"ur die
    // physikalischen R"ander aus einem seriellen Gitter zur Verwendung in
    // einem parallelen Gitter.

  template < class A, class MX > class Hbnd3PllExternal : public Hbnd3Top < A > {
    public :
      typedef MX mypllx_t ;
    protected :
      typedef typename A :: myhface3_t myhface3_t ;
      typedef typename A :: bnd_t     bnd_t ;
    public :
      inline Hbnd3PllExternal (myhface3_t *, int, const bnd_t bt) ;
      inline ~Hbnd3PllExternal () ;
      ElementPllXIF_t & accessPllX () ;
      const ElementPllXIF_t & accessPllX () const ;
      void detachPllXFromMacro () ;
    private :
      mypllx_t * _mxt ;
  } ;

  template < class A, class X, class MX > class Hbnd3PllInternal {
    public :

      //***************************************************************
      //  HbndPll
      //***************************************************************
      // for example: A  = GitterBasis :: Objects :: Hbnd3Default
      //              X  = BndsegPllBaseXClosure < A >
      //              MX = MacroBndsegPllBaseXClosure < A >
      class HbndPll : public A {
        public :
          typedef X mypllx_t ;
        protected :

          typedef Gitter :: ghostpair_STI  ghostpair_STI;
          typedef Gitter :: GhostChildrenInfo GhostChildrenInfo_t;
          typedef Gitter :: helement_STI helement_STI;
          typedef typename GitterBasisImpl::Objects::tetra_IMPL GhostTetra_t;
          typedef typename A :: myhface3_t myhface3_t ;
          typedef typename A :: balrule_t balrule_t ;
          typedef typename A :: bnd_t     bnd_t ;

          inline HbndPll (myhface3_t *, int);
          ~HbndPll () {}
          virtual bool bndNotifyBalance (balrule_t,int) ;
          virtual bool lockedAgainstCoarsening () const ;
        public :
          bnd_t bndtype () const ;
          ElementPllXIF_t & accessPllX () ;
          const ElementPllXIF_t & accessPllX () const ;
          void detachPllXFromMacro () ;

        private :
          mypllx_t _ext ;

        protected :
          // ghost element behind pllx bnd, can be pointer to null
          mutable ghostpair_STI _ghostPair;

          // refine ghost if face is refined and ghost is not zero
          void splitGhost (GhostChildrenInfo_t & );

          // mark all children for coarsening and call coarse on elem
          void removeDescendents( helement_STI & elem );
          // coarse ghost if face is coarsened
          void coarseGhost ();

          // set ghost pointer, use this method otherwise all constructors
          // have to be changed
          void setGhost ( const ghostpair_STI & gpair );

        public:
          // return ghost pointer
          const ghostpair_STI & getGhost () const;

          // Schwerpunkt des anliegenden Elements beschaffen:
          inline int ghostLevel () const ;
      } ;
      typedef class HbndPll micro_t ;

      //****************************************************************
      //  HbndPllMacro
      //****************************************************************
    public :
      class HbndPllMacro : public Hbnd3Top < micro_t > {
        public :
          typedef MX mypllx_t ;
        protected :
          typedef typename A :: myhface3_t myhface3_t ;
          typedef typename A :: balrule_t  balrule_t ;
          typedef typename A :: bnd_t     bnd_t ;
          typedef typename Gitter :: Geometric :: BuilderIF BuilderIF;

          virtual bool bndNotifyBalance (balrule_t,int) ;
          virtual bool lockedAgainstCoarsening () const ;
        public :
          HbndPllMacro (myhface3_t *,int, const bnd_t bt ,
                        BuilderIF& , MacroGhostInfoTetra* ) ;
          HbndPllMacro (myhface3_t *,int, const bnd_t bt, BuilderIF& ) ;
         ~HbndPllMacro () ;
          ElementPllXIF_t & accessPllX () ;
          const ElementPllXIF_t & accessPllX () const ;
          void detachPllXFromMacro () ;
          inline int ghostLevel () const ;

          // return mxt's ldbVertexIndex, otherwise the default is returned which is wrong
          int ldbVertexIndex() const { alugrid_assert ( _mxt ); return _mxt->ldbVertexIndex(); }
          int master() const { alugrid_assert ( _mxt ); return _mxt->master(); }
          // call mxt's setLoadBalanceVertexIndex (otherwise default impl is called which is wrong)
          void setLoadBalanceVertexIndex ( int ldbVx ) { alugrid_assert ( _mxt ); _mxt->setLoadBalanceVertexIndex( ldbVx ); }
          void setMaster ( int master ) { alugrid_assert ( _mxt ); _mxt->setMaster( master ); }

          virtual const MacroGhostInfo_STI* buildGhostCell(ObjectStream& os, int fce);

        private :
          mypllx_t * _mxt ;
          BuilderIF& _mgb;
          MacroGhost * _gm;
      } ;
      typedef class HbndPllMacro macro_t ;
  } ;


  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //
  template < class A, class MX > inline Hbnd3PllExternal < A, MX > ::
  Hbnd3PllExternal (myhface3_t * f, int t, const bnd_t bt)
      : Hbnd3Top < A > (0,f,t,bt), _mxt (new MX (*this)) {
    this->restoreFollowFace () ;
    return ;
  }

  template < class A, class MX > inline Hbnd3PllExternal < A, MX > :: ~Hbnd3PllExternal () {
    delete _mxt ;
    _mxt = 0 ;
    return ;
  }

  template < class A, class MX > ElementPllXIF_t & Hbnd3PllExternal < A, MX > :: accessPllX () {
    alugrid_assert (_mxt) ;
    return * _mxt ;
  }

  template < class A, class MX > const ElementPllXIF_t & Hbnd3PllExternal < A, MX > :: accessPllX () const {
    alugrid_assert (_mxt) ;
    return * _mxt ;
  }

  template < class A, class MX > void Hbnd3PllExternal < A, MX > :: detachPllXFromMacro () {
    delete _mxt ;
    _mxt = 0 ;
    return ;
  }

  template < class A, class X, class MX > inline Hbnd3PllInternal < A, X, MX > :: HbndPll ::
  HbndPll (myhface3_t * f, int t )
    : A (f,t), _ext (*this), _ghostPair( (helement_STI *) 0, -1) {
    return ;
  }

  template < class A, class X, class MX > typename Hbnd3PllInternal < A, X, MX > :: HbndPll :: bnd_t Hbnd3PllInternal < A, X, MX > :: HbndPll ::  bndtype () const {
    return Gitter :: hbndseg_STI :: closure ;
  }

  template < class A, class X, class MX > ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPll ::
  accessPllX () {
    return _ext ;
  }

  template < class A, class X, class MX > const ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPll ::
  accessPllX () const {
    return _ext ;
  }

  template < class A, class X, class MX > void Hbnd3PllInternal < A, X, MX > :: HbndPll :: detachPllXFromMacro () {
    abort () ;
    return ;
  }

  template < class A, class X, class MX >
  bool Hbnd3PllInternal < A, X, MX > :: HbndPll :: bndNotifyBalance (balrule_t r, int w)
  {
    _ext.notifyBalance (r,w) ;
    return true ;
  }

  template < class A, class X, class MX >
  bool Hbnd3PllInternal < A, X, MX > :: HbndPll :: lockedAgainstCoarsening () const {
    return _ext.lockedAgainstCoarsening () ;
  }

  template < class A, class X, class MX >
  inline int Hbnd3PllInternal < A, X, MX > :: HbndPll ::  ghostLevel () const {
    return _ext.ghostLevel () ;
  }

  template < class A, class X, class MX >
  inline const Gitter :: ghostpair_STI &
  Hbnd3PllInternal < A, X, MX > :: HbndPll :: getGhost () const
  {
    return _ghostPair;
  }

  //***************************************************************************************
  //  --HbndPllMacro
  //***************************************************************************************
  template < class A, class X, class MX >
  Hbnd3PllInternal < A, X, MX > :: HbndPllMacro ::
  HbndPllMacro (myhface3_t * f, int t,
                const bnd_t bt,
                BuilderIF& mgb ,
                MacroGhostInfoTetra* ghInfo)
   : Hbnd3Top < micro_t > (0,f,t,bt)
   , _mxt(0)
   , _mgb(mgb)
   , _gm( new MacroGhostTetra( _mgb , ghInfo, f ) )
  {
    alugrid_assert ( _gm );
    this->setGhost ( _gm->getGhost() );
    _mxt = new MX (*this, _gm->getGhostInfo() );

    this->restoreFollowFace () ;
    return ;
  }

  template < class A, class X, class MX >
  Hbnd3PllInternal < A, X, MX > :: HbndPllMacro ::
  HbndPllMacro (myhface3_t * f, int t,
                const bnd_t bt,
                BuilderIF& mgb )
   : Hbnd3Top < micro_t > (0,f,t,bt)
   , _mxt ( new MX (*this) )
   , _mgb(mgb)
   , _gm( 0 )
  {
    alugrid_assert ( _mxt );
    this->restoreFollowFace () ;
    return ;
  }

  template < class A, class X, class MX > Hbnd3PllInternal < A, X, MX > :: HbndPllMacro ::
  ~HbndPllMacro () {
    if(_gm) delete _gm;
    _gm = 0;
    delete _mxt ;
    _mxt = 0 ;
    return ;
  }

  template < class A, class X, class MX > ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: accessPllX () {
    alugrid_assert (_mxt) ;
    return * _mxt ;
  }

  template < class A, class X, class MX > const ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: accessPllX () const {
    alugrid_assert (_mxt) ;
    return * _mxt ;
  }

  template < class A, class X, class MX > void Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: detachPllXFromMacro () {
    delete _mxt ;
    _mxt = 0 ;
    return ;
  }

  template < class A, class X, class MX >
  bool Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: bndNotifyBalance (balrule_t r, int w)
  {
    alugrid_assert (_mxt) ;
    _mxt->notifyBalance (r,w) ;
    return true ;
  }

  template < class A, class X, class MX > bool Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: lockedAgainstCoarsening () const
  {
    alugrid_assert (_mxt) ;
    return _mxt->lockedAgainstCoarsening () ;
  }

  template < class A, class X, class MX > inline int Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: ghostLevel () const
  {
    return this->level () ;
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_TETRA_TOP_PLL_H_INCLUDED
