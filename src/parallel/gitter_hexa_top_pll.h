// (c) bernhard schupp 1997 - 1998
// modification for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_HEXA_TOP_PLL_H_INCLUDED
#define GITTER_HEXA_TOP_PLL_H_INCLUDED

#include "../serial/parallel.h"
#include "gitter_pll_impl.h"
#include "gitter_hexa_top.h"

template < class A, class MX > class Hbnd4PllExternal : public Hbnd4Top < A > {
  public :
    typedef MX mypllx_t ;
  protected :
    typedef typename A :: myhface4_t myhface4_t ;
    typedef typename A :: bnd_t     bnd_t ;
  public :
    inline Hbnd4PllExternal (myhface4_t *, int,ProjectVertex *, 
                  const bnd_t bt, IndexManagerType & , Gitter * ) ;
    inline ~Hbnd4PllExternal () ;
    ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
    const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
    void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
  private :
    mypllx_t * _mxt ;
} ;

template < class A, class X, class MX > class Hbnd4PllInternal {
  public :

    //***************************************************************
    //  HbndPll
    //***************************************************************
    // for example: A = GitterBasis :: Objects :: Hbnd4Default
    class HbndPll : public A {
      public :
        typedef X mypllx_t ;
      protected :

        typedef typename GitterBasisImpl::Objects::tetra_IMPL GhostElement_t;
        typedef typename A :: myhface4_t myhface4_t ;
        typedef typename A :: balrule_t  balrule_t ;
        typedef typename A :: bnd_t     bnd_t ;

        inline HbndPll (myhface4_t *, int, ProjectVertex *, Gitter *) ;
       ~HbndPll () {}
        virtual bool bndNotifyBalance (balrule_t,int) ;
        virtual bool lockedAgainstCoarsening () const ;

      public :
        bnd_t bndtype () const ;
        ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
        const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
        void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
      private :
        mypllx_t _ext ;

      protected :
        // ghost element behind pllx bnd, can be pointer to null 
        GhostElement_t * _ghost;
        int _ghostFace; 

        // refine ghost if face is refined and ghost is not zero 
        void splitGhost ();

        // set ghost pointer, use this method otherwise all constructors
        // have to be changed 
        void setGhost (Gitter::helement_STI *gh, int);
      public:
        // return ghost pointer 
        Gitter::helement_STI * getGhost ();
        int getGhostFaceNumber () const;

      public:
        inline const double (& barycenter () const)[3] ;

        // for dune 
        inline int ghostLevel () const ;
    } ;
    typedef class HbndPll micro_t ;

    // NOTE: ghost element support is missing. 
    // the necessary changes are similar to the changes in 
    // gitter_tetra_top* 
  public :
    class HbndPllMacro : public Hbnd4Top < micro_t > {
      public :
        typedef MX mypllx_t ;
      protected :
        typedef typename A :: myhface4_t myhface4_t ;
        typedef typename A :: balrule_t  balrule_t ;
        typedef typename A :: bnd_t     bnd_t ;

        virtual bool bndNotifyBalance (balrule_t,int) ;
        virtual bool lockedAgainstCoarsening () const ;
      public :
        HbndPllMacro (myhface4_t *,int, ProjectVertex *,
              const bnd_t bt , IndexManagerType & im, Gitter * , MacroGhost * gh) ;
       ~HbndPllMacro () ;
        ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
        const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
        void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;

        // for dune 
        inline int ghostLevel () const ;
      private :
        mypllx_t * _mxt ;
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

template < class A, class MX > inline Hbnd4PllExternal < A, MX > :: 
Hbnd4PllExternal (myhface4_t * f, int t, ProjectVertex *ppv, const bnd_t bt , 
    IndexManagerType & im, Gitter * grd ) 
  : Hbnd4Top < A > (0,f,t,ppv,bt,im,grd), _mxt (new MX (*this)) 
{
  this->restoreFollowFace () ;
  return ;
}

template < class A, class MX > inline Hbnd4PllExternal < A, MX > :: ~Hbnd4PllExternal () {
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class MX > ElementPllXIF_t & Hbnd4PllExternal < A, MX > :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class MX > const ElementPllXIF_t & Hbnd4PllExternal < A, MX > :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class MX > void Hbnd4PllExternal < A, MX > :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class X, class MX > inline Hbnd4PllInternal < A, X, MX > :: HbndPll :: 
HbndPll (myhface4_t * f, int t, ProjectVertex *ppv, Gitter * grd) : A (f,t,ppv,grd), _ext (*this) , _ghost(0) {
  return ;
}

template < class A, class X, class MX > typename Hbnd4PllInternal < A, X, MX > :: HbndPll :: bnd_t Hbnd4PllInternal < A, X, MX > :: HbndPll ::  bndtype () const {
  return Gitter :: hbndseg_STI :: closure ;
}

template < class A, class X, class MX > ElementPllXIF_t & Hbnd4PllInternal < A, X, MX > :: HbndPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _ext ;
}

template < class A, class X, class MX > const ElementPllXIF_t & Hbnd4PllInternal < A, X, MX > :: HbndPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _ext ;
}

template < class A, class X, class MX > void Hbnd4PllInternal < A, X, MX > :: HbndPll :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  abort () ;
  return ;
}

template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX > :: HbndPll :: bndNotifyBalance (balrule_t r, int w) {
  if (r == balrule_t :: iso4) {
    _ext.notifyBalance (r,w) ;
    return true ;
  } else {
    cerr << "**WARNUNG Balancierungsanforderung vom Typ " << r << " ignoriert,\n" ;
    cerr << "  weil nicht vorgesehen. In " << __FILE__ << " " << __LINE__ << endl ;
    return false ;
  }
}

template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX > :: HbndPll :: lockedAgainstCoarsening () const {
  return _ext.lockedAgainstCoarsening () ;
}

// Schwerpunkt des anliegenden Elements beschaffen:
// Vorsicht: Basisklasse des Extenders X bzw. MX muss die Methode barycenter  () bereitstellen !
// Standard: Datei gitter_pll_impl.h, Klasse BndsegPllBaseXClosure < A >
template < class A, class X, class MX > inline const double (& Hbnd4PllInternal < A, X, MX > :: HbndPll ::  barycenter () const)[3] {
  return _ext.barycenter () ;
}

template < class A, class X, class MX > inline int Hbnd4PllInternal < A, X, MX > :: HbndPll ::  ghostLevel () const {
  return _ext.ghostLevel () ;
}

template < class A, class X, class MX >
inline Gitter :: helement_STI * Hbnd4PllInternal < A, X, MX > :: HbndPll :: getGhost ()
{
  // assert is not needed here when we dont use ghost cells 
  //assert(_ghost);
  return _ghost;
}

template < class A, class X, class MX >
inline int Hbnd4PllInternal < A, X, MX > :: HbndPll :: getGhostFaceNumber () const
{
  return _ghostFace;
}

template < class A, class X, class MX >
inline void Hbnd4PllInternal < A, X, MX > :: HbndPll ::
setGhost ( Gitter :: helement_STI * gh , int gFace)
{
  if(gh)
  {
    _ghost = static_cast<GhostElement_t *> (gh);
    _ghostFace = gFace;
  }
  else 
  {
    _ghost = 0;
    _ghostFace = -1;
  }
}

template < class A, class X, class MX >
inline void Hbnd4PllInternal < A, X, MX > :: HbndPll ::  splitGhost ()
{
  if(_ghost)
  {
    (*_ghost).request( Gitter::Geometric::HexaRule::iso8 );
    (*_ghost).refine();
  }
}
  
template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: 
HbndPllMacro (myhface4_t * f, int t, ProjectVertex *ppv,
    const bnd_t bt, IndexManagerType & im , Gitter * grd , MacroGhost * gh) 
: Hbnd4Top < micro_t > (0,f,t,ppv,bt,im,grd), _mxt (0) , _gm(gh) {

  if(_gm)
  {
    this->setGhost (_gm->getGhost(), _gm->ghostFaceNumber());   
    _mxt = new MX (*this, _gm->getGhostPoints() );
  }
  else
  { 
    _mxt = new MX (*this);
  }

  this->restoreFollowFace () ;
  return ;
}

template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: ~HbndPllMacro () {
  delete _gm;
  _gm = 0;
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class X, class MX > ElementPllXIF_t & Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class X, class MX > const ElementPllXIF_t & Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class X, class MX > void Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: bndNotifyBalance (balrule_t r, int w) {
  if (r == balrule_t :: iso4) {
    _mxt->notifyBalance (r,w) ;
    return true ;
  } else {
    cerr << "**WARNUNG Balancierungsanforderung vom Typ " << r << " ignoriert,\n" ;
    cerr << "  weil nicht vorgesehen. In " << __FILE__ << " " << __LINE__ << endl ;
    return false ;
  }
}

template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: lockedAgainstCoarsening () const {
  return _mxt->lockedAgainstCoarsening () ;
}

template < class A, class X, class MX > inline int Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: ghostLevel () const {
  return this->level () ;
}

#endif  // GITTER_HEXA_TOP_PLL_H_INCLUDED
