// (c) bernhard schupp 1997 - 1998
#ifndef GITTER_HEXA_TOP_PLL_H_INCLUDED
#define GITTER_HEXA_TOP_PLL_H_INCLUDED

#include "parallel.h"
#include "gitter_hexa_top.h"

template < class A, class MX > class Hbnd4PllExternal : public Hbnd4Top < A > {
  public :
    typedef MX mypllx_t ;
  protected :
    typedef typename A :: myhface4_t myhface4_t ;
  public :
    inline Hbnd4PllExternal (myhface4_t *, int,ProjectVertex *) ;
    inline ~Hbnd4PllExternal () ;
    ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
    const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
    void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
  private :
    mypllx_t * _mxt ;
} ;

template < class A, class X, class MX > class Hbnd4PllInternal {
  public :
    class HbndPll : public A {
      public :
        typedef X mypllx_t ;
      protected :
        typedef typename A :: myhface4_t myhface4_t ;
	typedef typename A :: balrule_t  balrule_t ;
	typedef typename A :: bnd_t     bnd_t ;
        inline HbndPll (myhface4_t *, int, ProjectVertex *) ;
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
// Anfang - Neu am 23.5.02 (BS)
// Schwerpunkt des anliegenden Elements beschaffen:
      public:
        inline const double (& barycenter () const)[3] ;

        // for dune 
        inline int ghostLevel () const ;
// Ende
// Ende - Neu am 23.5.02 (BS)
    } ;
    typedef class HbndPll micro_t ;
  public :
    class HbndPllMacro : public Hbnd4Top < micro_t > {
      public :
        typedef MX mypllx_t ;
      protected :
        typedef typename A :: myhface4_t myhface4_t ;
        typedef typename A :: balrule_t  balrule_t ;
	virtual bool bndNotifyBalance (balrule_t,int) ;
	virtual bool lockedAgainstCoarsening () const ;
      public :
        HbndPllMacro (myhface4_t *,int, ProjectVertex *) ;
       ~HbndPllMacro () ;
        ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
        const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
        void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;

        // for dune 
        inline int ghostLevel () const ;
      private :
        mypllx_t * _mxt ;
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

template < class A, class MX > inline Hbnd4PllExternal < A, MX > :: Hbnd4PllExternal (myhface4_t * f, int t, ProjectVertex *ppv) : Hbnd4Top < A > (0,f,t,ppv,this), _mxt (new MX (*this)) {
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

template < class A, class X, class MX > inline Hbnd4PllInternal < A, X, MX > :: HbndPll :: HbndPll (myhface4_t * f, int t, ProjectVertex *ppv) : A (f,t,ppv), _ext (*this) {
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

// Anfang - Neu am 23.5.02 (BS)
// Schwerpunkt des anliegenden Elements beschaffen:
// Vorsicht: Basisklasse des Extenders X bzw. MX muss die Methode barycenter  () bereitstellen !
// Standard: Datei gitter_pll_impl.h, Klasse BndsegPllBaseXClosure < A >
template < class A, class X, class MX > inline const double (& Hbnd4PllInternal < A, X, MX > :: HbndPll ::  barycenter () const)[3] {
  return _ext.barycenter () ;
}
// Ende
// Ende - Neu am 23.5.02 (BS)
template < class A, class X, class MX > inline int Hbnd4PllInternal < A, X, MX > :: HbndPll ::  ghostLevel () const {
  return _ext.ghostLevel () ;
}

template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: HbndPllMacro (myhface4_t * f, int t, ProjectVertex *ppv) : Hbnd4Top < micro_t > (0,f,t,ppv,0), _mxt (new MX (*this)) {
  this->restoreFollowFace () ;
  return ;
}

template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: ~HbndPllMacro () {
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

#endif	// GITTER_HEXA_TOP_PLL_H_INCLUDED
