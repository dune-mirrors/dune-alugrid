	// (c) bernhard schupp 1997 - 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$
	// $Date$
	
/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:39:53  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.5  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.4  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef GITTER_TETRA_TOP_PLL_H_INCLUDED
#define GITTER_TETRA_TOP_PLL_H_INCLUDED

#include "parallel.h"
#include "gitter_tetra_top.h"

static volatile char RCSId_gitter_tetra_top_pll_h [] = "$Id$" ;

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
    inline Hbnd3PllExternal (myhface3_t *, int, ProjectVertex *, const bnd_t bt ) ;
    inline ~Hbnd3PllExternal () ;
    ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
    const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
    void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
  private :
    mypllx_t * _mxt ;
} ;

template < class A, class X, class MX > class Hbnd3PllInternal {
  public :
    class HbndPll : public A {
      public :
        typedef X mypllx_t ;
      protected :
        typedef typename A :: myhface3_t myhface3_t ;
	typedef typename A :: balrule_t balrule_t ;
	typedef typename A :: bnd_t     bnd_t ;
        inline HbndPll (myhface3_t *, int, ProjectVertex *) ;
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
// Schwerpunkt des anliegenden Elements beschaffen:
      public:
        inline const double (& barycenter () const)[3] ;
// Ende
    } ;
    typedef class HbndPll micro_t ;
  public :
    class HbndPllMacro : public Hbnd3Top < micro_t > {
      public :
        typedef MX mypllx_t ;
      protected :
        typedef typename A :: myhface3_t myhface3_t ;
        typedef typename A :: balrule_t  balrule_t ; 
	      typedef typename A :: bnd_t     bnd_t ;
	virtual bool bndNotifyBalance (balrule_t,int) ;
	virtual bool lockedAgainstCoarsening () const ;
      public :
        HbndPllMacro (myhface3_t *,int, ProjectVertex *, const bnd_t bt ) ;
       ~HbndPllMacro () ;
        ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
        const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
        void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
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


template < class A, class MX > inline Hbnd3PllExternal < A, MX > :: Hbnd3PllExternal (myhface3_t * f, int t, ProjectVertex *ppv, const bnd_t bt ) 
    : Hbnd3Top < A > (0,f,t,ppv,bt), _mxt (new MX (*this)) {
  restoreFollowFace () ;
  return ;
}

template < class A, class MX > inline Hbnd3PllExternal < A, MX > :: ~Hbnd3PllExternal () {
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class MX > ElementPllXIF_t & Hbnd3PllExternal < A, MX > :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class MX > const ElementPllXIF_t & Hbnd3PllExternal < A, MX > :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class MX > void Hbnd3PllExternal < A, MX > :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class X, class MX > inline Hbnd3PllInternal < A, X, MX > :: HbndPll :: HbndPll (myhface3_t * f, int t, ProjectVertex *ppv) : A (f,t,ppv), _ext (*this) {
  return ;
}

template < class A, class X, class MX > Hbnd3PllInternal < A, X, MX > :: HbndPll :: bnd_t Hbnd3PllInternal < A, X, MX > :: HbndPll ::  bndtype () const {
  return Gitter :: hbndseg_STI :: closure ;
}

template < class A, class X, class MX > ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPll :: accessPllX () throw (Parallel :: AccessPllException) {
  return _ext ;
}

template < class A, class X, class MX > const ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPll :: accessPllX () const throw (Parallel :: AccessPllException) {
  return _ext ;
}

template < class A, class X, class MX > void Hbnd3PllInternal < A, X, MX > :: HbndPll :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  abort () ;
  return ;
}

template < class A, class X, class MX > bool Hbnd3PllInternal < A, X, MX > :: HbndPll :: bndNotifyBalance (balrule_t r, int w) {
  if (r == balrule_t :: iso4) {
    _ext.notifyBalance (r,w) ;
    return true ;
  } else {
    cerr << "**WARNUNG Balancierungsanforderung vom Typ " << r << " ignoriert,\n" ;
    cerr << "  weil nicht vorgesehen. In " << __FILE__ << " " << __LINE__ << endl ;
    return false ;
  }
}

template < class A, class X, class MX > bool Hbnd3PllInternal < A, X, MX > :: HbndPll :: lockedAgainstCoarsening () const {
  return _ext.lockedAgainstCoarsening () ;
}

// Schwerpunkt des anliegenden Elements beschaffen:
// Vorsicht: Basisklasse des Extenders X bzw. MX muss die Methode barycenter  () bereitstellen !
// Standard: Datei gitter_pll_impl.h, Klasse BndsegPllBaseXClosure < A >
template < class A, class X, class MX > inline const double (& Hbnd3PllInternal < A, X, MX > :: HbndPll ::  barycenter () const)[3] {
  return _ext.barycenter () ;
}
// Ende

template < class A, class X, class MX > Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: HbndPllMacro (myhface3_t * f, int t, ProjectVertex *ppv , const bnd_t bt ) 
    : Hbnd3Top < micro_t > (0,f,t,ppv,bt), _mxt (new MX (*this)) {
  restoreFollowFace () ;
  return ;
}

template < class A, class X, class MX > Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: ~HbndPllMacro () {
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class X, class MX > ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: accessPllX () throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class X, class MX > const ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: accessPllX () const throw (Parallel :: AccessPllException) {
  assert (_mxt) ;
  return * _mxt ;
}

template < class A, class X, class MX > void Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: detachPllXFromMacro () throw (Parallel :: AccessPllException) {
  delete _mxt ;
  _mxt = 0 ;
  return ;
}

template < class A, class X, class MX > bool Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: bndNotifyBalance (balrule_t r, int w) {
 if (r == balrule_t :: iso4) {
    assert (_mxt) ;
    _mxt->notifyBalance (r,w) ;
    return true ;
  } else {
    cerr << "**WARNUNG Balancierungsanforderung vom Typ " << r << " ignoriert,\n" ;
    cerr << "  weil nicht vorgesehen. In " << __FILE__ << " " << __LINE__ << endl ;
    return false ;
  }
}

template < class A, class X, class MX > bool Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: lockedAgainstCoarsening () const {
  assert (_mxt) ;
  return _mxt->lockedAgainstCoarsening () ;
}

#endif	// GITTER_TETRA_TOP_PLL_H_INCLUDED
