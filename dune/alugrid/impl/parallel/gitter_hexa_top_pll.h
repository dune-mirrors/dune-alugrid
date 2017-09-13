// (c) bernhard schupp 1997 - 1998
// modification for Dune Interface
// (c) Robert Kloefkorn 2004 - 2005
#ifndef GITTER_HEXA_TOP_PLL_H_INCLUDED
#define GITTER_HEXA_TOP_PLL_H_INCLUDED

#include "../serial/parallel.h"
#include "../serial/gitter_hexa_top.h"
#include "gitter_pll_impl.h"

namespace ALUGrid
{

  template< class A, class MX >
  class Hbnd4PllExternal
  : public Hbnd4Top< A >
  {
    public :
      typedef MX mypllx_t;
    protected :
      typedef typename A::myhface4_t myhface4_t;
      typedef typename A::bnd_t     bnd_t;
    public :
      inline Hbnd4PllExternal (myhface4_t *, int, const bnd_t bt);
      inline ~Hbnd4PllExternal ();
      ElementPllXIF_t & accessPllX ();
      const ElementPllXIF_t & accessPllX () const;
      void detachPllXFromMacro ();
    private :
      mypllx_t * _mxt;
  };

  template < class A, class X, class MX > class Hbnd4PllInternal {
    public :

      //***************************************************************
      //  HbndPll
      //***************************************************************
      // for example: A = GitterBasis::Objects::Hbnd4Default
      class HbndPll : public A {
        public :
          typedef X mypllx_t;
        protected :

          typedef Gitter::ghostpair_STI ghostpair_STI;
          typedef Gitter::helement_STI helement_STI;
          typedef Gitter::GhostChildrenInfo GhostChildrenInfo_t;
          typedef typename GitterBasisImpl::Objects::hexa_IMPL GhostHexa_t;
          typedef typename A::myhface4_t myhface4_t;
          typedef typename A::balrule_t  balrule_t;
          typedef typename A::bnd_t     bnd_t;

          inline HbndPll (myhface4_t *, int);
         ~HbndPll () {}
          virtual bool bndNotifyBalance (balrule_t,int);
          virtual bool lockedAgainstCoarsening () const;

        public :
          bnd_t bndtype () const;
          ElementPllXIF_t & accessPllX ();
          const ElementPllXIF_t & accessPllX () const;
          void detachPllXFromMacro ();
        private :
          mypllx_t _ext;

        protected :
          // ghost element behind pllx bnd, can be pointer to null
          mutable ghostpair_STI _ghostPair;

          // refine ghost if face is refined and ghost is not zero
          void splitGhost ( GhostChildrenInfo_t & );

          // mark children for coarsening and call coarse
          void removeDescendents ( helement_STI & );

          // coarse ghost if face is coarsened
          void coarseGhost ();

          // set ghost pointer, use this method otherwise all constructors
          // have to be changed
          void setGhost (const ghostpair_STI & gpair);
        public:
          // return ghost pointer
          ghostpair_STI & getGhost () const;

        public:
          // for dune
          inline int ghostLevel () const;
      };
      typedef class HbndPll micro_t;

      // NOTE: ghost element support is missing.
      // the necessary changes are similar to the changes in
      // gitter_tetra_top*
    public :
      class HbndPllMacro : public Hbnd4Top < micro_t > {
        public :
          typedef MX mypllx_t;
        protected :
          typedef typename A::myhface4_t myhface4_t;
          typedef typename A::balrule_t  balrule_t;
          typedef typename A::bnd_t     bnd_t;
          typedef typename Gitter::Geometric::BuilderIF BuilderIF;

          virtual bool bndNotifyBalance (balrule_t,int);
          virtual bool lockedAgainstCoarsening () const;
        public :
          HbndPllMacro (myhface4_t *,int,
                        const bnd_t bt,
                        BuilderIF & ,
                        MacroGhostInfoHexa* );
          HbndPllMacro (myhface4_t *,int,
                        const bnd_t bt,
                        BuilderIF & );

          ~HbndPllMacro ();
          ElementPllXIF_t & accessPllX ();
          const ElementPllXIF_t & accessPllX () const;
          void detachPllXFromMacro ();

          // builds ghost cell if not exists
          virtual const MacroGhostInfo_STI* buildGhostCell(ObjectStream& os, int fce);
          // for dune
          inline int ghostLevel () const;

          // overload ldbVertexIndex, otherwise the default is return which is wrong in this case
          int ldbVertexIndex () const { alugrid_assert ( _mxt ); return _mxt->ldbVertexIndex(); }
          int master() const { alugrid_assert ( _mxt ); return _mxt->master(); }
          // call mxt's setLoadBalanceVertexIndex (otherwise default impl is called which is wrong)
          void setLoadBalanceVertexIndex ( int ldbVx ) { alugrid_assert ( _mxt ); _mxt->setLoadBalanceVertexIndex( ldbVx ); }
          void setMaster ( int master ) { alugrid_assert ( _mxt ); _mxt->setMaster( master ); }

        private :
          mypllx_t * _mxt;
          BuilderIF & _mgb;
          MacroGhost * _gm;
      };
      typedef class HbndPllMacro macro_t;
  };


    //
    //    #    #    #  #          #    #    #  ######
    //    #    ##   #  #          #    ##   #  #
    //    #    # #  #  #          #    # #  #  #####
    //    #    #  # #  #          #    #  # #  #
    //    #    #   ##  #          #    #   ##  #
    //    #    #    #  ######     #    #    #  ######
    //

  template < class A, class MX > inline Hbnd4PllExternal < A, MX >::
  Hbnd4PllExternal (myhface4_t * f, int t, const bnd_t bt)
    : Hbnd4Top < A > (0,f,t,bt), _mxt (new MX (*this))
  {
    this->restoreFollowFace ();
    return;
  }

  template < class A, class MX > inline Hbnd4PllExternal < A, MX >::~Hbnd4PllExternal () {
    delete _mxt;
    _mxt = 0;
    return;
  }

  template < class A, class MX > ElementPllXIF_t & Hbnd4PllExternal < A, MX >::accessPllX () {
    alugrid_assert (_mxt);
    return * _mxt;
  }

  template < class A, class MX > const ElementPllXIF_t & Hbnd4PllExternal < A, MX >::accessPllX () const {
    alugrid_assert (_mxt);
    return * _mxt;
  }

  template < class A, class MX > void Hbnd4PllExternal < A, MX >::detachPllXFromMacro () {
    delete _mxt;
    _mxt = 0;
    return;
  }

  template < class A, class X, class MX > inline Hbnd4PllInternal < A, X, MX >::HbndPll::
  HbndPll (myhface4_t * f, int t) : A (f,t), _ext (*this) , _ghostPair((helement_STI *) 0 ,-1) {
    return;
  }

  template < class A, class X, class MX > typename Hbnd4PllInternal < A, X, MX >::HbndPll::bnd_t Hbnd4PllInternal < A, X, MX >::HbndPll:: bndtype () const {
    return Gitter::hbndseg_STI::closure;
  }

  template < class A, class X, class MX > ElementPllXIF_t & Hbnd4PllInternal < A, X, MX >::HbndPll::accessPllX () {
    return _ext;
  }

  template < class A, class X, class MX > const ElementPllXIF_t & Hbnd4PllInternal < A, X, MX >::HbndPll::accessPllX () const {
    return _ext;
  }

  template < class A, class X, class MX > void Hbnd4PllInternal < A, X, MX >::HbndPll::detachPllXFromMacro () {
    abort ();
    return;
  }

  template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX >::HbndPll::bndNotifyBalance (balrule_t r, int w)
  {
    if( r == balrule_t::iso4 )
    {
      _ext.notifyBalance( r, w );
      return true;
    }
    else
    {
      std::cerr << "WARNING (ignored): Ignoring balancing request of type " << r << "." << std::endl;
      return false;
    }
  }

  template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX >::HbndPll::lockedAgainstCoarsening () const {
    return _ext.lockedAgainstCoarsening ();
  }

  template < class A, class X, class MX > inline int Hbnd4PllInternal < A, X, MX >::HbndPll:: ghostLevel () const {
    return _ext.ghostLevel ();
  }

  template < class A, class X, class MX >
  inline Gitter::ghostpair_STI &
  Hbnd4PllInternal < A, X, MX >::HbndPll::getGhost () const
  {
    // assert is not needed here when we dont use ghost cells
    return _ghostPair;
  }

  template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX >::
  HbndPllMacro::HbndPllMacro (myhface4_t * f, int t,
                const bnd_t bt,
                BuilderIF & mgb,
                MacroGhostInfoHexa* ghInfo )
  : Hbnd4Top < micro_t > (0,f,t,bt)
  , _mxt (0)
  , _mgb(mgb)
  , _gm(  new MacroGhostHexa( _mgb , ghInfo, f ) )
  {
    alugrid_assert ( _gm );
    this->setGhost ( _gm->getGhost() );
    _mxt = new MX (*this, _gm->getGhostInfo() );
    alugrid_assert ( _mxt );

    this->restoreFollowFace ();
    return;
  }

  template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX >::
  HbndPllMacro::HbndPllMacro (myhface4_t * f, int t,
                                const bnd_t bt,
                                BuilderIF & mgb)
  : Hbnd4Top < micro_t > (0,f,t,bt)
  , _mxt (new MX (*this))
  , _mgb(mgb)
  , _gm(0)
  {
    alugrid_assert ( _mxt );
    this->restoreFollowFace ();
    return;
  }

  template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX >::HbndPllMacro::~HbndPllMacro () {
    delete _gm;
    _gm = 0;
    delete _mxt;
    _mxt = 0;
    return;
  }

  template < class A, class X, class MX > ElementPllXIF_t & Hbnd4PllInternal < A, X, MX >::HbndPllMacro::accessPllX () {
    alugrid_assert (_mxt);
    return * _mxt;
  }

  template < class A, class X, class MX > const ElementPllXIF_t & Hbnd4PllInternal < A, X, MX >::HbndPllMacro::accessPllX () const {
    alugrid_assert (_mxt);
    return * _mxt;
  }

  template < class A, class X, class MX > void Hbnd4PllInternal < A, X, MX >::HbndPllMacro::detachPllXFromMacro () {
    delete _mxt;
    _mxt = 0;
    return;
  }

  template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX >::HbndPllMacro::bndNotifyBalance (balrule_t r, int w)
  {
    if( r == balrule_t::iso4 )
    {
      _mxt->notifyBalance (r,w);
      return true;
    }
    else
    {
      std::cerr << "WARNING (ignored): Ignoring balancing request of type " << r << "." << std::endl;
      return false;
    }
  }

  template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX >::
  HbndPllMacro::lockedAgainstCoarsening () const {
    return _mxt->lockedAgainstCoarsening ();
  }

  template < class A, class X, class MX > inline int Hbnd4PllInternal < A, X, MX >::HbndPllMacro::ghostLevel () const {
    return this->level ();
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_HEXA_TOP_PLL_H_INCLUDED
