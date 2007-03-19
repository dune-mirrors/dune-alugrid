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
        HbndPllMacro (myhface3_t *,int, ProjectVertex *, const bnd_t bt , 
                      IndexManagerType & im, Gitter * , 
                      BuilderIF& , MacroGhostInfoTetra* ) ;
        HbndPllMacro (myhface3_t *,int, ProjectVertex *, const bnd_t bt , 
                      IndexManagerType & im, Gitter * , BuilderIF& ) ;
       ~HbndPllMacro () ;
        ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
        const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
        void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
        inline int ghostLevel () const ; 

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
inline void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  splitGhost
(GhostChildrenInfo_t & info ) 
{
  if(_ghostPair.first)
  {
    GhostTetra_t & ghost = static_cast<GhostTetra_t &> (*_ghostPair.first); 
    if(!ghost.down())
    {
      ghost.tagForGlobalRefinement();
      ghost.refine();
    }

    // get the childs 
    typedef typename Gitter :: Geometric :: tetra_GEO tetra_GEO;
    typedef typename Gitter :: Geometric :: hface3_GEO hface3_GEO;

    // ghostpair.second is the internal face number of the face 
    // connected to the interior of the process 
    hface3_GEO * face = ghost.myhface3( _ghostPair.second ); 
    int count = 0;
    for(face = face->down(); face; face = face->next())
    {
      assert(face);

      // NOTE: we need here tetra_GEO because we cannot cast from hasFace
      // to tetra_IMPL (GhostTetra_t)
      tetra_GEO * ghch = 0;

      typedef pair < Gitter :: Geometric :: hasFace3 *, int > neigh_t;
      neigh_t neighbour = face->nb.front();
      if( ! neighbour.first->isboundary ())
      {
        ghch = dynamic_cast<tetra_GEO *> (neighbour.first);
        assert(ghch);
        assert( ghch->up() == &ghost );
      }
      else 
      {
        neighbour = face->nb.rear();
        assert( ! neighbour.first->isboundary () );
        ghch = dynamic_cast<tetra_GEO *> (neighbour.first);
      }
      
      assert(ghch);
      assert(ghch->up() == &ghost );
     
      // set element pointer and local face number 
      info.setGhostPair( ghostpair_STI( ghch, neighbour.second ) , count );

      ++count ;
    }
  }
}

template < class A, class X, class MX > 
inline void Hbnd3PllInternal < A, X, MX > :: HbndPll :: 
removeDescendents( helement_STI & elem ) 
{
  elem.resetRefinementRequest(); 
  // check all children first 
  for( helement_STI* child = elem.down(); child; child = child->next() )
  {
    // if child is not leaf coarse childs first 
    if( ! child->leaf() )
      removeDescendents( *child );
    
    assert( child->leaf () );
    // mark child for coarsening 
    child->tagForGlobalCoarsening(); 
  }

#ifndef NDEBUG
  for( helement_STI* child = elem.down(); child; child = child->next() )
  {
    assert( child->isGhost ());
    child->tagForGlobalCoarsening();
    GhostTetra_t * tet = dynamic_cast<GhostTetra_t *> (child);
    assert( tet->requestrule() == Gitter :: Geometric :: TetraRule :: crs ); 
  }
#endif

  // if element is not already leaf call coarse 
  if( ! elem.leaf () )
  {
    elem.coarse();
  }
}

template < class A, class X, class MX > 
inline void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  coarseGhost () 
{
  if(_ghostPair.first)
  {
    helement_STI& ghost = (*_ghostPair.first); 
    if( ghost.leaf() ) return ;

    // remove all descendents if possible 
    removeDescendents( ghost );
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
    const bnd_t bt, IndexManagerType & im , Gitter * grd, 
    BuilderIF& mgb ,
    MacroGhostInfoTetra* ghInfo) 
 : Hbnd3Top < micro_t > (0,f,t,ppv,0,bt,im,grd) 
 , _mxt(0)
 , _mgb(mgb)
 , _gm( new MacroGhostTetra( _mgb , ghInfo, f ) ) 
{
  assert( _gm );
  this->setGhost ( _gm->getGhost() );   
  _mxt = new MX (*this, _gm->getGhostInfo() );
  
  this->restoreFollowFace () ;
  return ;
}

template < class A, class X, class MX > 
Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: 
HbndPllMacro (myhface3_t * f, int t, ProjectVertex *ppv , 
    const bnd_t bt, IndexManagerType & im , Gitter * grd, 
    BuilderIF& mgb ) 
 : Hbnd3Top < micro_t > (0,f,t,ppv,0,bt,im,grd) 
 , _mxt ( new MX (*this) )
 , _mgb(mgb)
 , _gm( 0 ) 
{
  assert( _mxt );
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

template < class A, class X, class MX > 
inline const MacroGhostInfo_STI* Hbnd3PllInternal < A, X, MX > :: 
HbndPllMacro :: buildGhostCell(ObjectStream& os, int fce)
{
  assert( _gm == 0 ); 
  int code = MacroGridMoverIF :: ENDMARKER ;
  os.readObject (code); 
  assert( code == MacroGridMoverIF :: HBND3INT );

  {
    int bfake, v [3] ;
    os.readObject (bfake) ;
#ifndef NDEBUG 
    Gitter :: hbndseg :: bnd_t b = (Gitter :: hbndseg :: bnd_t) bfake;
    assert( b == Gitter :: hbndseg :: closure );
#endif

    os.readObject (v[0]) ;
    os.readObject (v[1]) ;
    os.readObject (v[2]) ;

    int readPoint = 0;
    os.readObject( readPoint );

    MacroGhostInfoTetra * ghInfo = 0;
    if( readPoint == MacroGridMoverIF :: POINTTRANSMITTED )
    {
      // create ghInfo and read data from stream 
      ghInfo = new MacroGhostInfoTetra(os);
    }

    {
      assert( ghInfo );
      myhface3_t * f = this->myhface3(0);
      assert( f );

      _gm = new MacroGhostTetra( _mgb , ghInfo,  f );
      this->setGhost ( _gm->getGhost() );   
    }
  }

  assert( _gm );
  return _gm->getGhostInfo();
}


template < class A, class X, class MX > inline int Hbnd3PllInternal < A, X, MX > :: HbndPllMacro :: ghostLevel () const {
  return this->level () ;
}

#endif  // GITTER_TETRA_TOP_PLL_H_INCLUDED
