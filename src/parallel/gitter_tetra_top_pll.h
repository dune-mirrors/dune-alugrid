// (c) bernhard schupp 1997 - 1998
// modification for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_TETRA_TOP_PLL_H_INCLUDED
#define GITTER_TETRA_TOP_PLL_H_INCLUDED

#include "../serial/parallel.h"
#include "gitter_pll_impl.h"
#include "gitter_tetra_top.h"

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
    inline Hbnd3PllExternal (myhface3_t *, int, ProjectVertex *, const bnd_t bt, IndexManagerType & , Gitter * ) ;
    inline ~Hbnd3PllExternal () ;
    ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
    const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
    void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
  private :
    mypllx_t * _mxt ;
} ;

template < class A, class X, class MX > class Hbnd3PllInternal {
  public :

    //***************************************************************
    //  HbndPll
    //***************************************************************
    // for example: A = GitterBasis :: Objects :: Hbnd3Default
    class HbndPll : public A {
      public :
        typedef X mypllx_t ;
      protected :
        
        typedef Gitter :: ghostpair_STI  ghostpair_STI;
        typedef typename GitterBasisImpl::Objects::tetra_IMPL GhostElement_t;
        typedef typename A :: myhface3_t myhface3_t ;
        typedef typename A :: balrule_t balrule_t ;
        typedef typename A :: bnd_t     bnd_t ;
        
        inline HbndPll (myhface3_t *, int, ProjectVertex * , Gitter * );
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
        mutable ghostpair_STI _ghostPair;

        // refine ghost if face is refined and ghost is not zero 
        void splitGhost (); 
        
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
        virtual bool bndNotifyBalance (balrule_t,int) ;
        virtual bool lockedAgainstCoarsening () const ;
      public :
        HbndPllMacro (myhface3_t *,int, ProjectVertex *, const bnd_t bt , 
                      IndexManagerType & im, Gitter * , MacroGhost * ) ;
       ~HbndPllMacro () ;
        ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
        const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
        void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
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
template < class A, class MX > inline Hbnd3PllExternal < A, MX > :: 
Hbnd3PllExternal (myhface3_t * f, int t, ProjectVertex *ppv, const bnd_t bt , IndexManagerType & im, Gitter * grd ) 
    : Hbnd3Top < A > (0,f,t,ppv,this,bt,im,grd), _mxt (new MX (*this)) {
  this->restoreFollowFace () ;
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

template < class A, class X, class MX > inline Hbnd3PllInternal < A, X, MX > :: HbndPll :: 
HbndPll (myhface3_t * f, int t, ProjectVertex *ppv , Gitter * grd )
  : A (f,t,ppv,grd), _ext (*this), _ghostPair(0,-1) {
  return ;
}

template < class A, class X, class MX > typename Hbnd3PllInternal < A, X, MX > :: HbndPll :: bnd_t Hbnd3PllInternal < A, X, MX > :: HbndPll ::  bndtype () const {
  return Gitter :: hbndseg_STI :: closure ;
}

template < class A, class X, class MX > ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPll :: 
accessPllX () throw (Parallel :: AccessPllException) {
  return _ext ;
}

template < class A, class X, class MX > const ElementPllXIF_t & Hbnd3PllInternal < A, X, MX > :: HbndPll :: 
accessPllX () const throw (Parallel :: AccessPllException) {
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

template < class A, class X, class MX > 
bool Hbnd3PllInternal < A, X, MX > :: HbndPll :: lockedAgainstCoarsening () const {
  return _ext.lockedAgainstCoarsening () ;
}

template < class A, class X, class MX > 
inline int Hbnd3PllInternal < A, X, MX > :: HbndPll ::  ghostLevel () const {
  return _ext.ghostLevel () ;
}

template < class A, class X, class MX > 
inline void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  splitGhost () 
{
  if(_ghostPair.first)
  {
    GhostElement_t & ghost = static_cast<GhostElement_t &> (*_ghostPair.first); 
    ghost.request( Gitter::Geometric::TetraRule::iso8 );
    assert( Gitter::Geometric::TetraRule::iso8 == ghost.requestrule ());  
    ghost.refine();
  }
}

template < class A, class X, class MX > 
inline void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  coarseGhost () 
{
  if(_ghostPair.first)
  {
    GhostElement_t & ghost = static_cast<GhostElement_t &> (*_ghostPair.first); 
    ghost.request( Gitter::Geometric::TetraRule::crs );
    assert( Gitter::Geometric::TetraRule::crs == ghost.requestrule ());  
    ghost.coarse();
    assert( ghost.leaf() );
  }
}

template < class A, class X, class MX > 
inline const Gitter :: ghostpair_STI &
Hbnd3PllInternal < A, X, MX > :: HbndPll :: getGhost () const
{
  return _ghostPair; 
}

template < class A, class X, class MX > 
inline void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  
setGhost ( const ghostpair_STI & gpair ) 
{
  if(gpair.first)
  {
    _ghostPair = gpair; 
    assert( _ghostPair.first );
    
    // copy indices from internal boundry to myhface3(.) of ghost
    _ghostPair.first->setIndicesAndBndId ( *(this->myhface3(0)) , _ghostPair.second );
  }
  else 
  {
    _ghostPair.first  =  0 ;
    _ghostPair.second = -1 ;
  }
}

//***************************************************************************************
//  --HbndPllMacro
//***************************************************************************************
template < class A, class X, class MX > 
Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: 
HbndPllMacro (myhface3_t * f, int t, ProjectVertex *ppv , 
    const bnd_t bt, IndexManagerType & im , Gitter * grd, MacroGhost * gh) 
 : Hbnd3Top < micro_t > (0,f,t,ppv,0,bt,im,grd) 
 , _mxt(0)
 , _gm(gh) 
{
  if(_gm) 
  {
    this->setGhost ( _gm->getGhost() );   
    _mxt = new MX (*this, _gm->getGhostPoints() );
  }
  else 
  {
    _mxt = new MX (*this);
  }
  
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

template < class A, class X, class MX > inline int Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: ghostLevel () const {
  return this->level () ;
}

#endif  // GITTER_TETRA_TOP_PLL_H_INCLUDED
