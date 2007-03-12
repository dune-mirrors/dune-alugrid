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

        typedef Gitter :: ghostpair_STI ghostpair_STI;
        typedef Gitter :: helement_STI helement_STI;
        typedef Gitter :: GhostChildrenInfo GhostChildrenInfo_t ;
        typedef typename GitterBasisImpl::Objects::hexa_IMPL GhostElement_t;
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
        mutable ghostpair_STI _ghostPair;

        // refine ghost if face is refined and ghost is not zero 
        void splitGhost ( GhostChildrenInfo_t & );

        void markDescendents ( helement_STI & );
        // coarse ghost if face is coarsened  
        void coarseGhost ();

        // set ghost pointer, use this method otherwise all constructors
        // have to be changed 
        void setGhost (const ghostpair_STI & gpair);
      public:
        // return ghost pointer 
        const ghostpair_STI & getGhost () const;

      public:
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
        typedef typename Gitter :: Geometric :: BuilderIF BuilderIF;

        virtual bool bndNotifyBalance (balrule_t,int) ;
        virtual bool lockedAgainstCoarsening () const ;
      public :
        HbndPllMacro (myhface4_t *,int, ProjectVertex *,
                      const bnd_t bt , IndexManagerType & im, 
                      Gitter * , BuilderIF & ,
                      MacroGhostInfoHexa* ) ;
        HbndPllMacro (myhface4_t *,int, ProjectVertex *,
                      const bnd_t bt , IndexManagerType & im, 
                      Gitter * , BuilderIF & );

       ~HbndPllMacro () ;
        ElementPllXIF_t & accessPllX () throw (Parallel :: AccessPllException) ;
        const ElementPllXIF_t & accessPllX () const throw (Parallel :: AccessPllException) ;
        void detachPllXFromMacro () throw (Parallel :: AccessPllException) ;
  
        // builds ghost cell if not exists 
        virtual const MacroGhostInfo_STI* buildGhostCell(ObjectStream& os, int fce);
        // for dune 
        inline int ghostLevel () const ;
      private :
        mypllx_t * _mxt ;
        BuilderIF & _mgb;
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
HbndPll (myhface4_t * f, int t, ProjectVertex *ppv, Gitter * grd) : A (f,t,ppv,grd), _ext (*this) , _ghostPair(0,-1) {
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

template < class A, class X, class MX > inline int Hbnd4PllInternal < A, X, MX > :: HbndPll ::  ghostLevel () const {
  return _ext.ghostLevel () ;
}

template < class A, class X, class MX >
inline const Gitter :: ghostpair_STI & 
Hbnd4PllInternal < A, X, MX > :: HbndPll :: getGhost () const
{
  // assert is not needed here when we dont use ghost cells 
  return _ghostPair;
}

template < class A, class X, class MX >
inline void Hbnd4PllInternal < A, X, MX > :: HbndPll ::
setGhost ( const ghostpair_STI & gpair )
{
  if(gpair.first)
  {
    _ghostPair = gpair; 
    assert( _ghostPair.first );
    // copy indices from internal boundry to myhface3(.) of ghost
    _ghostPair.first->setIndicesAndBndId( *this->myhface4(0), _ghostPair.second );
  }
  else 
  {
    _ghostPair.first  =  0 ;
    _ghostPair.second = -1 ;
  }
}

template < class A, class X, class MX >
inline void Hbnd4PllInternal < A, X, MX > :: HbndPll ::  
splitGhost ( GhostChildrenInfo_t & info )
{
  if(_ghostPair.first)
  {
    typedef typename Gitter :: Geometric :: hexa_GEO  hexa_GEO;
    typedef typename Gitter :: Geometric :: hface4_GEO hface4_GEO;

    hexa_GEO & ghost = static_cast<hexa_GEO &> (*(_ghostPair.first)); 
    if(!ghost.down())
    {
      ghost.tagForGlobalRefinement();
      ghost.refine();
    } 

    int gFaceNum = _ghostPair.second;
    assert( gFaceNum >= 0 );
    assert( gFaceNum < 6 );

    hface4_GEO * face = ghost.myhface4( gFaceNum );
    assert( face );

    int count = 0; 
    for(face = face->down(); face; face = face->next() )
    {
      assert(face);
      hexa_GEO * ghch = 0; 

      typedef pair < Gitter :: Geometric :: hasFace4 *, int > neigh_t;
      neigh_t neighbour = face->nb.front();

      if( ! neighbour.first->isboundary ())
      {
        ghch = dynamic_cast<hexa_GEO *> (neighbour.first);
        assert(ghch);
        assert( ghch->up() == &ghost );
      }
      else
      {
        neighbour = face->nb.rear();
        assert( ! neighbour.first->isboundary () );
        ghch = dynamic_cast<hexa_GEO *> (neighbour.first);
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
inline void Hbnd4PllInternal < A, X, MX > :: HbndPll ::
markDescendents( helement_STI & elem )
{
  for( helement_STI * child = elem.down(); child; child = child->next() )
  {
    if( child->leaf())
    {
      child->tagForGlobalCoarsening();
    }
    else
    {
      child->resetRefinementRequest();
    }
    this->markDescendents( *child );
  }
}
 
template < class A, class X, class MX >
inline void Hbnd4PllInternal < A, X, MX > :: HbndPll ::  coarseGhost ()
{
  /*
  if(_ghostPair.first)
  {
    GhostElement_t & ghost = static_cast<GhostElement_t &> (*_ghostPair.first);
    if( ghost.leaf() ) return ;

    while ( ! ghost.leaf() )
    {
      this->markDescendents( ghost );

      // set me status to nosplit 
      ghost.resetRefinementRequest();

      assert( ghost.requestrule () == Gitter :: Geometric::HexaRule::nosplit );
      // coarse element 
      ghost.coarse();
    }
  }
  */
}
  
template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX > :: 
HbndPllMacro :: HbndPllMacro (myhface4_t * f, int t, ProjectVertex *ppv,
              const bnd_t bt, IndexManagerType & im , 
              Gitter * grd , BuilderIF & mgb,
              MacroGhostInfoHexa* ghInfo ) 
: Hbnd4Top < micro_t > (0,f,t,ppv,bt,im,grd)
, _mxt (0) 
, _mgb(mgb) 
, _gm(  new MacroGhostHexa( _mgb , ghInfo, f ) )  
{
  assert( _gm );
  this->setGhost ( _gm->getGhost() );   
  _mxt = new MX (*this, _gm->getGhostInfo() );
  assert( _mxt );

  this->restoreFollowFace () ;
  return ;
}

template < class A, class X, class MX > Hbnd4PllInternal < A, X, MX > :: 
HbndPllMacro :: HbndPllMacro (myhface4_t * f, int t, ProjectVertex *ppv,
              const bnd_t bt, IndexManagerType & im , 
              Gitter * grd , BuilderIF & mgb)
: Hbnd4Top < micro_t > (0,f,t,ppv,bt,im,grd)
, _mxt (new MX (*this))
, _mgb(mgb) 
, _gm(0)  
{
  assert( _mxt );
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

template < class A, class X, class MX > bool Hbnd4PllInternal < A, X, MX > :: 
HbndPllMacro :: lockedAgainstCoarsening () const {
  return _mxt->lockedAgainstCoarsening () ;
}

template < class A, class X, class MX >
inline const MacroGhostInfo_STI* Hbnd4PllInternal < A, X, MX > ::
HbndPllMacro :: buildGhostCell(ObjectStream& os, int fce)
{
  assert( _gm == 0 );
  int code = MacroGridMoverIF :: ENDMARKER ;
  os.readObject (code);
  assert( code == MacroGridMoverIF :: HBND4INT );

  {
    int bfake, v [4] = {-1,-1,-1,-1};

    os.readObject (bfake) ;
#ifndef NDEBUG 
    Gitter :: hbndseg :: bnd_t b = (Gitter :: hbndseg :: bnd_t) bfake;
    assert( b == Gitter :: hbndseg :: closure );
#endif

    os.readObject (v[0]) ;
    os.readObject (v[1]) ;
    os.readObject (v[2]) ;
    os.readObject (v[3]) ;

    int readPoint = 0; 
    os.readObject( readPoint ); 
    
    // read ghost information 
    MacroGhostInfoHexa* ghInfo = 0;
    if( readPoint == MacroGridMoverIF :: POINTTRANSMITTED ) 
    {
      // read data from stream 
      ghInfo = new MacroGhostInfoHexa( os );
    }

    // create macro ghost cell     
    {
      assert( ghInfo );

      myhface4_t * f = this->myhface4(0);
      assert( f );

      _gm = new MacroGhostHexa( _mgb , ghInfo, f );
      this->setGhost ( _gm->getGhost() );
    }
  }

  assert( _gm );
  return _gm->getGhostInfo();
}

template < class A, class X, class MX > inline int Hbnd4PllInternal < A, X, MX > :: HbndPllMacro :: ghostLevel () const {
  return this->level () ;
}

#endif  // GITTER_HEXA_TOP_PLL_H_INCLUDED
