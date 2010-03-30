// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 

#ifndef GITTER_HEXA_TOP_H_INCLUDED
#define GITTER_HEXA_TOP_H_INCLUDED

#include "mapp_cube_3d.h"

template < class A > class Hedge1Top : public A {
  protected :
    typedef Hedge1Top < A >       inneredge_t ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: myvertex_t  myvertex_t ;
    typedef typename A :: myrule_t  myrule_t ;
  private :
    inneredge_t * _dwn, * _bbb ;
    innervertex_t * _cv ;
    IndexManagerType & _indexManager;

    myrule_t _rule ;  
    int _lvl ;       
    const signed char _nChild;  
    
  public :
    // need for refinement 
    IndexManagerType & getIndexManager() { return _indexManager; }
    
    inline Hedge1Top (int,myvertex_t *,myvertex_t *, IndexManagerType & im) ;
    inline Hedge1Top (int,myvertex_t *,myvertex_t *, IndexManagerType & im, int nChild ) ;
    virtual ~Hedge1Top () ;
    inneredge_t * subedge1 (int) ;
    const inneredge_t * subedge1 (int) const ;
    inneredge_t * down () ;
    const inneredge_t * down () const ;
    inneredge_t * next () ;
    const inneredge_t * next () const ;
    inline int level () const ;
    inline int nChild () const ;
    inline void append (inneredge_t *) ;
    innervertex_t * innerVertex () ;
    const innervertex_t * innerVertex () const ;
    innervertex_t * subvertex (int) ;
    const innervertex_t * subvertex (int) const ;
  public :
    virtual myrule_t getrule () const ;
    virtual void refineImmediate (myrule_t) ;
    virtual bool coarse () ;

  public :
    virtual void backup (ostream &) const ;
    virtual void restore (istream &) ;
    
    virtual void backup (ObjectStream&) const ;
    virtual void restore (ObjectStream&) ;
  protected:
    template <class OutStream_t> 
    void doBackup(OutStream_t &) const ;  
    template <class InStream_t> 
    void doRestore(InStream_t &) ;  
} ;

template < class A > class Hface4Top : public A {
  protected :
    typedef Hface4Top < A >      innerface_t ;
    typedef typename A :: inneredge_t      inneredge_t ;
    typedef typename A :: innervertex_t    innervertex_t ;
    typedef typename A :: myhedge1_t       myhedge1_t ;
    typedef typename A :: myvertex_t       myvertex_t ;
    typedef typename A :: myrule_t         myrule_t ;
  private :
    innerface_t * _dwn, * _bbb ;
    innervertex_t * _cv ;
    inneredge_t   * _ed ;
    IndexManagerType & _indexManager;

    int _lvl ;
    myrule_t _rule ;
    const signed char _nChild;
    
  private:
    inline myhedge1_t * subedge1 (int,int) ;
    inline const myhedge1_t * subedge1 (int,int) const ;
    void splitISO4 () ;
    IndexManagerType & getEdgeIndexManager () ;
  public:
    // for HexaTop, when refinement is done 
    IndexManagerType & getIndexManager() { return _indexManager; }
    
    // constructor for macro faces 
    inline Hface4Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int, IndexManagerType & im) ;
    // constructor for refined faces 
    inline Hface4Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int, IndexManagerType & im, int nChild ) ;
    virtual ~Hface4Top () ;
    innervertex_t * subvertex (int) ;
    const innervertex_t * subvertex (int) const ;
    inneredge_t * subedge1 (int) ;
    const inneredge_t * subedge1 (int) const ;
    innerface_t * subface4 (int) ;
    const innerface_t * subface4 (int) const ;
    
    inline int level () const ;
    inline int nChild () const ;
    inline innervertex_t * innerVertex () ;
    inline const innervertex_t * innerVertex () const ;
    inline inneredge_t * innerHedge () ;
    inline const inneredge_t * innerHedge () const ;
    inline innerface_t * down () ;
    inline const innerface_t * down () const ;
    inline innerface_t * next () ;
    inline const innerface_t * next () const ;
    inline void append (innerface_t * f) ;

  public :
    virtual myrule_t getrule () const ;
    virtual bool refine (myrule_t,int) ;
    virtual void refineImmediate (myrule_t) ;
    virtual bool coarse () ;
  public :
    virtual void backup (ostream &) const ;
    virtual void restore (istream &) ;   

    virtual void backup (ObjectStream&) const ;
    virtual void restore (ObjectStream&) ;
  protected:
    template <class OutStream_t> 
    void doBackup(OutStream_t &) const ;  
    template <class InStream_t> 
    void doRestore(InStream_t &) ;  
} ;

template < class A > class Hbnd4Top : public A {
  protected :
    typedef Hbnd4Top < A >              innerbndseg_t ;
    typedef typename A :: myhface4_t    myhface4_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    typedef typename A :: bnd_t         bnd_t;
    
    void splitISO4 () ;
    bool refineLikeElement (balrule_t) ;
  private :
    innerbndseg_t * _bbb, * _dwn, * _up ;
    IndexManagerType & _indexManager;
    int _lvl ;
    const bnd_t _bt; // type of boundary 
    int _segmentIndex; // index of macro boundary segment 
    
    inline bool coarse () ;
    inline void append (innerbndseg_t *) ;
  public :
    // constructor for refinement 
    inline Hbnd4Top (int,myhface4_t *,int,ProjectVertex *, innerbndseg_t *, Gitter::helement_STI *, int) ;

    // constructor for macro element in the serial case 
    inline Hbnd4Top (int,myhface4_t *,int,ProjectVertex *, const bnd_t bt , IndexManagerType & ) ;

    virtual ~Hbnd4Top () ;
    bool refineBalance (balrule_t,int) ;
    bool bndNotifyCoarsen () ;
    void restoreFollowFace () ;
    int level () const ;
    int segmentIndex () const ;
    innerbndseg_t * next () ;
    innerbndseg_t * down () ;
    const innerbndseg_t * next () const ;
    const innerbndseg_t * down () const ;
    
    // for dune 
    innerbndseg_t * up () ;
    const innerbndseg_t * up () const ;
    inline bnd_t bndtype () const { return _bt; }
  protected: 
    void setBoundaryId (const int id);
    
} ;

template < class A > class HexaTop : public A {
  protected :
    typedef HexaTop < A >           innerhexa_t ;
    typedef typename A :: innerface_t innerface_t ;
    typedef typename A :: inneredge_t inneredge_t ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: myhface4_t    myhface4_t ;
    typedef typename A :: myhedge1_t  myhedge1_t ;
    typedef typename A :: myvertex_t  myvertex_t ;
    typedef typename A :: myrule_t  myrule_t ;
    typedef typename A :: balrule_t   balrule_t ;
    inline void refineImmediate (myrule_t) ;
    inline void append (innerhexa_t * h) ;
  private :
    innerhexa_t * _bbb, * _dwn, * _up ;
    innerface_t * _fc ;
    inneredge_t * _ed ;
    innervertex_t * _cv ;
    IndexManagerType & _indexManager; 
    double _volume; 
    int _lvl ;
    myrule_t _rule, _req ;
    const signed char _nChild; 
    bool _affine;

private:    
    IndexManagerType & getEdgeIndexManager () ;
    IndexManagerType & getFaceIndexManager () ;

    void splitISO8 () ;
protected:
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    myhface4_t * subface4 (int,int) ;
    const myhface4_t * subface4 (int,int) const ;
    
  public:
    // Constructor for macro elements 
    HexaTop (int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
             myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
             IndexManagerType & im, Gitter* mygrid) ;
    
    // constructor for refinement 
    HexaTop (int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
             myhface4_t *,int,myhface4_t *,int,myhface4_t *,int, 
             innerhexa_t * up, int nChild , double vol ) ;
    
    virtual ~HexaTop () ;
    inline innerhexa_t * up () ;
    inline const innerhexa_t * up () const;
    inline innerhexa_t * down () ;
    inline const innerhexa_t * down () const ;
    inline innerhexa_t * next () ;
    inline const innerhexa_t * next () const ;
    inline innervertex_t * innerVertex () ;
    inline const innervertex_t * innerVertex () const ;
    inline inneredge_t * innerHedge () ;
    inline const inneredge_t * innerHedge () const ;
    inline innerface_t * innerHface () ;
    inline const innerface_t * innerHface () const ;
    inline int level () const ;
    inline int nChild () const ;
    inline double volume () const ;
  public :
    myrule_t getrule () const ;
    myrule_t requestrule () const ;
    bool refine () ;
    void request (myrule_t) ;
    bool refineBalance (balrule_t,int) ;
    bool coarse () ;
    bool bndNotifyCoarsen () ;
    void backupCMode (ostream &) const ;
    void backup (ostream &) const ;
    void restore (istream &) ;

    // backup and restore index 
    void backupIndex (ostream &) const ;
    // set entry of element to false when index is read 
    void restoreIndex (istream &, vector<bool>(&) [4]) ;

    void backup (ObjectStream&) const ;
    void restore (ObjectStream&) ;
  protected:
    template <class OutStream_t> 
    void doBackup(OutStream_t &) const ;  
    template <class InStream_t> 
    void doRestore(InStream_t &) ;  
} ;

template < class A > class Periodic4Top : public A {
  protected :
    typedef Periodic4Top < A >    innerperiodic4_t  ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: inneredge_t   inneredge_t ;
    typedef typename A :: innerface_t   innerface_t ;
    typedef typename A :: myhedge1_t  myhedge1_t ;
    typedef typename A :: myhface4_t  myhface4_t ;
    typedef typename A :: myrule_t  myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    void refineImmediate (myrule_t) ;
    inline void append (innerperiodic4_t * h) ;
    
    //us
    typedef typename A :: GhostElement_t GhostElement_t;
    typedef Gitter :: ghostpair_STI ghostpair_STI;
    
  private :
    innerperiodic4_t * _dwn, * _bbb, * _up ; 
    int _lvl ;
    myrule_t _rule ;
    const signed char _nChild; 
  private :
    void splitISO4 () ;
  protected :
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    myhface4_t * subface4 (int,int) ;
    const myhface4_t * subface4 (int i, int j) const ;
  public:
    Periodic4Top (int,myhface4_t *,int,myhface4_t *,int) ;
    Periodic4Top (int,myhface4_t *,int,myhface4_t *,int,
                  innerperiodic4_t * up, int nChild ) ;
    virtual ~Periodic4Top () ;

    inline innerperiodic4_t * up () ;
    inline const innerperiodic4_t * up () const;
    
    inline innerperiodic4_t * down () ;
    inline const innerperiodic4_t * down () const ;
    inline innerperiodic4_t * next () ;
    inline const innerperiodic4_t * next () const ;
    inline innervertex_t * innerVertex () ;
    inline const innervertex_t * innerVertex () const ;
    inline inneredge_t * innerHedge () ;
    inline const inneredge_t * innerHedge () const ;
    inline innerface_t * innerHface () ;
    inline const innerface_t * innerHface () const ;
    inline int level () const ;
    inline int nChild () const ;
  public :
    myrule_t getrule () const ;
    bool refine () ;
    void request (myrule_t) ;
    bool refineBalance (balrule_t,int) ;
    bool coarse () ;
    bool bndNotifyCoarsen () ;
    //Per4-Geister: (us)
    const ghostpair_STI & getGhost (int) const ;
    virtual inline void setGhost ( const pair< Gitter :: helement * , int > & pair, int nr);
  private:
    mutable ghostpair_STI _ghostPair [2];
    //_ghostPair[0] liegt an myhface4[0] und ist das affine Bild vom Hexa an myhface4(1)
 
    // refine ghost if face is refined and ghost is not zero
    void splitGhosts () ;
    // coarse ghost if face is coarsened
    void coarseGhosts () ;
  public:  
    void backupCMode (ostream &) const ;
    void backup (ostream &) const ;
    void restore (istream &) ;

    void backup (ObjectStream&) const ;
    void restore (ObjectStream&) ;
  protected:
    template <class OutStream_t> 
    void doBackup(OutStream_t &) const ;  
    template <class InStream_t> 
    void doRestore(InStream_t &) ;  
};

//
//    #    #    #  #          #    #    #  ######
//    #    ##   #  #          #    ##   #  #
//    #    # #  #  #          #    # #  #  #####
//    #    #  # #  #          #    #  # #  #
//    #    #   ##  #          #    #   ##  #
//    #    #    #  ######     #    #    #  ######
//

// #     #                                    #    #######
// #     #  ######  #####    ####   ######   ##       #      ####   #####
// #     #  #       #    #  #    #  #       # #       #     #    #  #    #
// #######  #####   #    #  #       #####     #       #     #    #  #    #
// #     #  #       #    #  #  ###  #         #       #     #    #  #####
// #     #  #       #    #  #    #  #         #       #     #    #  #
// #     #  ######  #####    ####   ######  #####     #      ####   #


template < class A > inline Hedge1Top < A > :: Hedge1Top (int l, myvertex_t * a, myvertex_t * b, IndexManagerType & im ) 
  : A (a,b), 
  _dwn (0), _bbb (0), _cv (0), 
  _indexManager (im) , 
  _rule (myrule_t :: nosplit) , 
  _lvl (l), 
  _nChild(0) {
  this->setIndex( _indexManager.getIndex() );  
  return ;
}

template < class A > inline Hedge1Top < A > :: Hedge1Top (int l, myvertex_t * a, myvertex_t * b, IndexManagerType & im, int nChild ) 
  : A (a,b), 
  _dwn (0), _bbb (0), _cv (0), 
  _indexManager (im) ,
  _rule (myrule_t :: nosplit) , 
  _lvl (l), 
  _nChild(nChild) {
  this->setIndex( _indexManager.getIndex() );  
  return ;
}

template < class A > Hedge1Top < A > :: ~Hedge1Top () {
  this->freeIndex( this->_indexManager );
  if(_bbb) delete _bbb;
  if(_dwn) delete _dwn;
  if(_cv)  delete _cv;
  return ;
}

template < class A > inline int Hedge1Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline int Hedge1Top < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 2 );
  return _nChild ;
}

template < class A > Hedge1Top < A > * Hedge1Top < A > :: down () {
  return _dwn ;
}

template < class A > const Hedge1Top < A > * Hedge1Top < A > :: down () const {
  return _dwn ;
}

template < class A > Hedge1Top < A > * Hedge1Top < A > :: next () {
  return _bbb ;
}

template < class A > const Hedge1Top < A > * Hedge1Top < A > :: next () const {
  return _bbb ;
}

template < class A > inline void Hedge1Top < A > :: backup (ostream & os) const 
{
  doBackup( os );
}

template < class A > inline void Hedge1Top < A > :: backup (ObjectStream& os) const 
{
  doBackup( os );
}

template < class A > template <class OutStream_t>
inline void Hedge1Top < A > :: doBackup (OutStream_t& os) const 
{
  os.put ((char) getrule ()) ;
  {for (const inneredge_t * d = down () ; d ; d = d->next ()) d->backup (os) ; }
  return ;
}

template < class A > inline void Hedge1Top < A > :: restore (istream & is) 
{
  doRestore( is );
}
template < class A > inline void Hedge1Top < A > :: restore (ObjectStream& is) 
{
  doRestore( is );
}

template < class A > template <class InStream_t>
inline void Hedge1Top < A > :: doRestore (InStream_t & is) 
{
  char r = (char) is.get () ;
  refineImmediate (myrule_t (r)) ;
  {for (inneredge_t * d = down () ; d ; d = d->next ()) d->restore (is) ; }
  return ;
}

template < class A >  inline void Hedge1Top < A > :: append (inneredge_t * e) {
  assert (!_bbb && e) ; 
  _bbb = e ;
  return ;
}

template < class A > typename Hedge1Top < A > :: myrule_t 
Hedge1Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > Hedge1Top < A > * Hedge1Top < A > :: subedge1 (int n) {
  assert (n == 0 || n == 1) ;
  assert (n ? this->down ()->next () : this->down ()) ;
  return n ? this->down ()->next () : this->down () ;
}

template < class A > const Hedge1Top < A > * Hedge1Top < A > :: subedge1 (int n) const {
  assert (n == 0 || n == 1) ;
  assert (n ? this->down ()->next () : this->down ()) ;
  return n ? this->down ()->next () : this->down () ;
}

template < class A > inline typename Hedge1Top < A > :: innervertex_t * 
Hedge1Top < A > :: innerVertex () {
  return _cv ;
}

template < class A > inline const typename Hedge1Top < A > :: innervertex_t * Hedge1Top < A > :: innerVertex () const {
  return _cv ;
}

template < class A > inline typename Hedge1Top < A > :: innervertex_t * Hedge1Top < A > :: subvertex (int) {
  return _cv ;
}

template < class A > inline const typename Hedge1Top < A > :: innervertex_t * Hedge1Top < A > :: subvertex (int) const {
  return _cv ;
}

// #     #                                 #       #######
// #     #  ######    ##     ####   ###### #    #     #      ####   #####
// #     #  #        #  #   #    #  #      #    #     #     #    #  #    #
// #######  #####   #    #  #       #####  #    #     #     #    #  #    #
// #     #  #       ######  #       #      #######    #     #    #  #####
// #     #  #       #    #  #    #  #           #     #     #    #  #
// #     #  #       #    #   ####   ######      #     #      ####   #


template < class A > 
inline typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: down () {
  return _dwn ;
}

template < class A > 
inline const typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: down () const {
  return _dwn ;
}

template < class A > 
inline typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: next () {
  return _bbb ;
}

template < class A > 
inline const typename Hface4Top < A > :: innerface_t * Hface4Top < A > :: next () const {
  return _bbb ;
}

template < class A > inline int Hface4Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline int Hface4Top < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 4 );
  return _nChild ;
}

template < class A > typename Hface4Top < A > :: myhedge1_t * 
Hface4Top < A > :: subedge1 (int i,int j) {
  assert(j == 0 || j == 1) ;
  return this->myhedge1 (i)->subedge1 (j ? 1 - this->twist(i) : this->twist(i)) ;
}

template < class A > const typename Hface4Top < A > :: myhedge1_t * 
Hface4Top < A > :: subedge1 (int i,int j) const {
  assert(j == 0 || j == 1) ;
  return this->myhedge1 (i)->subedge1 (j ? 1 - this->twist(i) : this->twist(i)) ;
}

template < class A > inline typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: subvertex (int) {
  assert (getrule() == myrule_t :: iso4) ;
  return _cv ;
}

template < class A > inline const typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: subvertex (int) const {
  assert (getrule() == myrule_t :: iso4) ;
  return _cv ;
}

template < class A > typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: subedge1 (int n) {
  inneredge_t * e = _ed ;
  for (int i = 0 ; i < n ; i ++ ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > const typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: subedge1 (int n) const {
  const inneredge_t * e = _ed ;
  for (int i = 0 ; i < n ; i ++ ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > inline typename Hface4Top < A > :: innerface_t * 
Hface4Top < A > :: subface4 (int n) {
  innerface_t * f = this->down () ;
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > inline const typename Hface4Top < A > :: innerface_t * 
Hface4Top < A > :: subface4 (int n) const {
  const innerface_t * f = this->down () ;
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > inline Hface4Top < A > :: Hface4Top (int l, myhedge1_t * e0, int t0, myhedge1_t * e1, int t1, 
  myhedge1_t * e2, int t2, myhedge1_t * e3, int t3, IndexManagerType & im) 
  : A (e0, t0, e1, t1, e2, t2, e3, t3), 
  _dwn (0), _bbb (0), _cv (0), _ed (0), 
  _indexManager(im) ,
  _lvl (l), 
  _rule (myrule_t :: nosplit)  
  , _nChild(0) {
  this->setIndex( _indexManager.getIndex() );  
  return ;
}

template < class A > inline Hface4Top < A > :: Hface4Top (int l, myhedge1_t * e0, int t0, myhedge1_t * e1, int t1, 
  myhedge1_t * e2, int t2, myhedge1_t * e3, int t3, IndexManagerType & im,
  int nChild ) 
  : A (e0, t0, e1, t1, e2, t2, e3, t3), 
  _dwn (0), _bbb (0), _cv (0), _ed (0), 
  _indexManager(im) ,
  _lvl (l), 
  _rule (myrule_t :: nosplit) ,
  _nChild(nChild)
{
  this->setIndex( _indexManager.getIndex() );  
  return ;
}

template < class A > Hface4Top < A > :: ~Hface4Top () {
  this->freeIndex( this->_indexManager );
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  if (_ed) delete _ed ;
  if (_cv) delete _cv ;
  return ;
}

template < class A > inline typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: innerVertex () {
  return _cv ;
}

template < class A > inline const typename Hface4Top < A > :: innervertex_t * 
Hface4Top < A > :: innerVertex () const {
  return _cv ;
}

template < class A > inline typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: innerHedge () {
  return _ed ;
}

template < class A > inline const typename Hface4Top < A > :: inneredge_t * 
Hface4Top < A > :: innerHedge () const {
  return _ed ;
}

template < class A > inline void Hface4Top < A > :: append (innerface_t * f) {
  assert (_bbb == 0) ;
  _bbb = f ;
  return ;
}

template < class A > inline typename Hface4Top < A > :: myrule_t 
Hface4Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > inline IndexManagerType & Hface4Top < A > :: getEdgeIndexManager () {
  return static_cast<inneredge_t &> (*(this->myhedge1(0))).getIndexManager();
}

template < class A > inline void Hface4Top < A > :: backup (ostream & os) const 
{
  doBackup(os);
}
template < class A > inline void Hface4Top < A > :: backup (ObjectStream& os) const 
{
  doBackup(os);
}

template < class A > template <class OutStream_t>
inline void Hface4Top < A > :: doBackup (OutStream_t& os) const 
{
  os.put ((char) getrule ()) ;
  {for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ; }
  {for (const innerface_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > inline void Hface4Top < A > :: restore (istream & is) 
{
  doRestore( is );
}
template < class A > inline void Hface4Top < A > :: restore (ObjectStream& is) 
{
  doRestore( is );
}
  
template < class A > template <class InStream_t>
inline void Hface4Top < A > :: doRestore (InStream_t & is) 
{
  refineImmediate (myrule_t ((char) is.get ())) ;
  {for (inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->restore (is) ; }
  {for (innerface_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
  return ;
}

// #     #                         #       #######
// #     #  #####   #    #  #####  #    #     #      ####   #####
// #     #  #    #  ##   #  #    # #    #     #     #    #  #    #
// #######  #####   # #  #  #    # #    #     #     #    #  #    #
// #     #  #    #  #  # #  #    # #######    #     #    #  #####
// #     #  #    #  #   ##  #    #      #     #     #    #  #
// #     #  #####   #    #  #####       #     #      ####   #


template < class A > inline Hbnd4Top < A > :: 
Hbnd4Top (int l, myhface4_t * f, int i, ProjectVertex *ppv, 
          innerbndseg_t * up, Gitter::helement_STI * gh, int gFace ) : 
  A (f, i, ppv), _bbb (0), _dwn (0), _up(up) , 
  _indexManager(_up->_indexManager) ,
  _lvl (l), 
  _bt(_up->_bt),
  _segmentIndex( _up->_segmentIndex ) // get segment index from father 
{
  typedef Gitter :: ghostpair_STI ghostpair_STI;
  ghostpair_STI p ( gh, gFace );
  this->setGhost ( p );
  this->setIndex( _indexManager.getIndex() );  

  setBoundaryId( _bt );
  return ;
}

template < class A > inline Hbnd4Top < A > :: 
Hbnd4Top (int l, myhface4_t * f, int i, ProjectVertex *ppv, const bnd_t bt , IndexManagerType & im)
  : A (f, i, ppv), _bbb (0), _dwn (0), _up(0) , 
  _indexManager(im) ,
  _lvl (l) , _bt(bt)  
{
  this->setIndex( _indexManager.getIndex() );  

  // store segment by using index 
  _segmentIndex = this->getIndex() ;

  setBoundaryId( _bt );
  return ;
}

template < class A > Hbnd4Top < A > :: ~Hbnd4Top () {
  this->freeIndex( this->_indexManager );
  if (this->isLeafEntity()) this->detachleafs();
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  return ;
}

template < class A > inline int Hbnd4Top < A > :: segmentIndex () const {
  return _segmentIndex ;
}

template < class A > inline int Hbnd4Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: next () { 
  return _bbb ;
}

template < class A > inline const typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: next () const { 
  return _bbb ;
}

template < class A > inline typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: down () { 
  return _dwn ;
}

template < class A > inline const typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: down () const { 
  return _dwn ;
}

template < class A > inline typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > :: up () { 
  return _up ;
}

template < class A > inline const typename Hbnd4Top < A > :: innerbndseg_t * Hbnd4Top < A > ::up () const { 
  return _up ;
}

template < class A > inline void Hbnd4Top < A > :: append (innerbndseg_t * b) {
  assert (_bbb == 0) ;
  _bbb = b ;
  return ;
}

// #     #                         #######
// #     #  ######  #    #    ##      #      ####   #####
// #     #  #        #  #    #  #     #     #    #  #    #
// #######  #####     ##    #    #    #     #    #  #    #
// #     #  #         ##    ######    #     #    #  #####
// #     #  #        #  #   #    #    #     #    #  #
// #     #  ######  #    #  #    #    #      ####   #

// constructor for macro elements  
template < class A > inline HexaTop < A > 
:: HexaTop (int l, myhface4_t * f0, int t0, myhface4_t * f1, int t1, 
            myhface4_t * f2, int t2, myhface4_t * f3, int t3, myhface4_t * f4, 
            int t4, myhface4_t * f5, int t5, IndexManagerType & im, Gitter* mygrid ) 
  : A (f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5, mygrid)
  , _bbb (0), _dwn (0), _up(0), _fc (0), _ed (0), _cv (0)
  , _indexManager(im) 
  , _volume (0.0) 
  , _lvl (l)
  , _rule (myrule_t :: nosplit), _req (myrule_t :: nosplit) 
  , _nChild(0) 
  , _affine( false )
{ 
  TrilinearMapping trMap (this->myvertex(0)->Point(), this->myvertex(1)->Point(),
                          this->myvertex(2)->Point(), this->myvertex(3)->Point(),
                          this->myvertex(4)->Point(), this->myvertex(5)->Point(),
                          this->myvertex(6)->Point(), this->myvertex(7)->Point());
  // calculate volume 
  _volume = QuadraturCube3D < VolumeCalc > (trMap).integrate2 (0.0);
  // check whether mapping is affine 
  _affine = trMap.affine(); 

  assert( this->level() == l );
  
  this->setIndex( _indexManager.getIndex() );   
  return ;
}

// constructor for refinement 
template < class A > inline HexaTop < A > 
:: HexaTop (int l, myhface4_t * f0, int t0, myhface4_t * f1, int t1, 
            myhface4_t * f2, int t2, myhface4_t * f3, int t3, myhface4_t * f4, 
            int t4, myhface4_t * f5, int t5, innerhexa_t * up , int nChild , double vol ) 
  : A (f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5, up->_myGrid)
  , _bbb (0), _dwn (0), _up(up), _fc (0), _ed (0), _cv (0)
  , _indexManager(_up->_indexManager)
  , _volume ( vol )
  , _lvl (l)
  , _rule (myrule_t :: nosplit), _req (myrule_t :: nosplit)
  , _nChild(nChild) 
  , _affine(_up->_affine)
{ 
  assert( this->level() == l );

  this->setIndex( _indexManager.getIndex() );   

  // set bndid to fathers bndid now 
  this->_bndid = _up->bndId();

  // if mapping is not affine recalculate volume 
  if( ! _affine )
  {
    TrilinearMapping triMap (this->myvertex(0)->Point(),
                             this->myvertex(1)->Point(),
                             this->myvertex(2)->Point(),
                             this->myvertex(3)->Point(),
                             this->myvertex(4)->Point(),
                             this->myvertex(5)->Point(),
                             this->myvertex(6)->Point(),
                             this->myvertex(7)->Point() );

#ifndef NDEBUG 
    // make sure determinant is ok 
    double point[3] = { 0.0, 0.0, 0.0 };
    assert( triMap.det( point ) > 0 );
#endif

    // calculate volume 
    _volume = QuadraturCube3D < VolumeCalc > (triMap).integrate2 (0.0);
  }

  // make sure that given volume is the same as calulated 
  assert( fabs (
      QuadraturCube3D < VolumeCalc >
       (TrilinearMapping (this->myvertex(0)->Point(), this->myvertex(1)->Point(),
                          this->myvertex(2)->Point(), this->myvertex(3)->Point(),
                          this->myvertex(4)->Point(), this->myvertex(5)->Point(),
                          this->myvertex(6)->Point(), this->myvertex(7)->Point())).integrate2 (0.0) - _volume) < 1e-10 );
  return ;
}

template < class A > inline HexaTop < A > :: ~HexaTop () 
{
  this->freeIndex( this->_indexManager );
    
  if (!_dwn) this->detachleafs();
  else assert(!this->isLeafEntity());
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  if (_fc) delete _fc ;
  if (_ed) delete _ed ;
  if (_cv) delete _cv ;
  return ;
}

template < class A > inline typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: up () {
  return _up ;
} 

template < class A > inline const typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: up () const {
  return _up ; 
}

template < class A > inline typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: down () {
  return _dwn ;
}

template < class A > inline const typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: down () const {
  return _dwn ;
}

template < class A > inline typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: next () {
  return _bbb ;
}

template < class A > inline const typename HexaTop < A > :: innerhexa_t * HexaTop < A > :: next () const {
  return _bbb ;
}

template < class A > inline typename HexaTop < A > :: innervertex_t * HexaTop < A > :: innerVertex () {
  return _cv ;
}

template < class A > inline const typename HexaTop < A > :: innervertex_t * HexaTop < A > :: innerVertex () const {
  return _cv ;
}

template < class A > inline typename HexaTop < A > :: inneredge_t * HexaTop < A > :: innerHedge () {
  return _ed ;
}

template < class A > inline const typename HexaTop < A > :: inneredge_t * HexaTop < A > :: innerHedge () const {
  return _ed ;
}

template < class A > inline typename HexaTop < A > :: innerface_t * HexaTop < A > :: innerHface () {
  return _fc ;
}

template < class A > inline const typename HexaTop < A > :: innerface_t * HexaTop < A > :: innerHface () const {
  return _fc ;
}

template < class A > inline void HexaTop < A > :: append (HexaTop < A > * h) {
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > inline int HexaTop < A > :: level () const {
  return _lvl ;
}

template < class A > inline double HexaTop < A > :: volume () const {
  return _volume;
}

template < class A > inline int HexaTop < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 8 );
  return _nChild ;
}

template < class A > inline IndexManagerType & HexaTop < A > :: getEdgeIndexManager () {
  return static_cast<inneredge_t &> (*(this->subedge1(0,0))).getIndexManager();
}

template < class A > inline IndexManagerType & HexaTop < A > :: getFaceIndexManager () {
  return static_cast<innerface_t &> (*(this->subface4(0,0))).getIndexManager();
}

template < class A > typename HexaTop < A > :: myrule_t HexaTop < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > typename HexaTop < A > :: myrule_t HexaTop < A > :: requestrule () const {
  return myrule_t (_req) ;
}

template < class A > void HexaTop < A > :: request (myrule_t r) {
  assert (r.isValid ()) ;
  _req = r ;
  return ;
}

template < class A > inline void HexaTop < A > :: backupCMode (ostream & os) const {

  // Das backup im alten Stil, d.h. levelweise die Verfeinerungsregeln
  // vom Gitter runterschreiben. Diese Technik wird nur f"ur das backup
  // noch unterst"utzt, um die Daten mit "alteren Konstruktionen visual.
  // zu k"onnen.

  os << getrule () << " " ;
  return ;
}

template < class A > inline void HexaTop < A > :: backupIndex (ostream & os) const {
#ifndef _DUNE_NOT_USES_ALU3DGRID_
  os.write(((const char *) & this->_idx ), sizeof(int));
  for (const innerhexa_t* c = down(); c; c = c->next()) {
    c->backupIndex(os);
  }
#endif
  return;
}

template < class A > inline void HexaTop < A > :: backup (ostream & os) const 
{
  doBackup( os );
}
template < class A > inline void HexaTop < A > :: backup (ObjectStream& os) const 
{
  doBackup( os );
}

template < class A > template <class OutStream_t>
inline void HexaTop < A > :: doBackup (OutStream_t& os) const 
{
  os.put ((char) getrule ()) ;
  {for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ; }
  {for (const innerface_t * f = innerHface () ; f ; f = f->next ()) f->backup (os) ; }
  {for (const innerhexa_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > inline void HexaTop < A > :: 
restoreIndex (istream & is, vector<bool> (&isHole) [4]) 
{
#ifndef _DUNE_NOT_USES_ALU3DGRID_
  // free index from constructor
  // indexManager is cleared from outside 
  is.read ( ((char *) &(this->_idx) ), sizeof(int) );

  // mark this element a non hole 
  typedef typename Gitter :: Geometric :: BuilderIF BuilderIF;

  // make sure sizes match 
  assert( this->getIndex() < (int) isHole[BuilderIF::IM_Elements].size() );
  // set entry to false, because this is not a hole 
  isHole[BuilderIF :: IM_Elements][this->getIndex()] = false;
  
  {for (innerhexa_t * c = down () ; c ; c = c->next ()) c->restoreIndex (is, isHole ) ; }
#endif
  return;
}

template < class A > inline void HexaTop < A > :: restore (istream & is) 
{
  doRestore( is );
}
template < class A > inline void HexaTop < A > :: restore (ObjectStream& is) 
{
  doRestore( is );
}

template < class A > template <class InStream_t>
inline void HexaTop < A > :: doRestore (InStream_t & is) 
{
  // restore () stellt den Elmentbaum aus der Verfeinerungs
  // geschichte wieder her. Es ruft refine () auf und testet
  // auf den korrekten Vollzug der Verfeinerung. Danach werden
  // die inneren Gitterteile restore'd.

  myrule_t r ((char) is.get ()) ;
  assert(getrule() == myrule_t :: nosplit) ;
  if (r == myrule_t :: nosplit) {
  
  // Vorsicht: beim restore m"ussen sich sowohl Element als auch
  // Randelement um die Korrektheit der Nachbarschaft k"ummern,
  // und zwar dann wenn sie "on the top" sind (= die gelesene
  // Verfeinerungsregel ist nosplit). (s.a. beim Randelement)
  
    for (int i = 0 ; i < 6 ; i ++) {
      myhface4_t & f (*(this->myhface4 (i))) ;
      if (!f.leaf ()) {
        for (int j = 0 ; j < 4 ; j ++) f.subface4 (j)->nb.complete (f.nb) ;
      }
    }
  } else {
    request (r) ;
    refine () ;
    assert (getrule() == r) ;
    {for (inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->restore (is) ; }
    {for (innerface_t * f = innerHface () ; f ; f = f->next ()) f->restore (is) ; }
    {for (innerhexa_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
  }
  return ;
}

// Anfang - Neu am 23.5.02 (BS)

// ######                                                          #       #######
// #     #  ######  #####      #     ####   #####      #     ####  #    #     #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #     #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #     #
// #        #       #####      #    #    #  #    #     #    #      #######    #
// #        #       #   #      #    #    #  #    #     #    #    #      #     #
// #        ######  #    #     #     ####   #####      #     ####       #     #

template < class A > inline int Periodic4Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline int Periodic4Top < A > :: nChild () const { 
  assert( _nChild >= 0 && _nChild < 4 );
  return _nChild ;
}

template < class A > inline typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: up () {
  return _up ;
}

template < class A > inline const typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: up () const {
   return _up ; 
}

template < class A > inline typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: down () {
  return _dwn ;
}

template < class A > inline const typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: down () const { 
  return _dwn ;
}

template < class A > inline typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: next () { 
  return _bbb ;
}

template < class A > inline const typename Periodic4Top < A > :: innerperiodic4_t * Periodic4Top < A > :: next () const { 
  return _bbb ;
}

template < class A > inline typename Periodic4Top < A > :: innervertex_t * Periodic4Top < A > :: innerVertex () { 
  return 0 ;
}

template < class A > inline const typename Periodic4Top < A > :: innervertex_t * Periodic4Top < A > :: innerVertex () const { 
  return 0 ;
}

template < class A > inline typename Periodic4Top < A > :: inneredge_t * Periodic4Top < A > :: innerHedge () { 
  return 0 ;
}

template < class A > inline const typename Periodic4Top < A > :: inneredge_t * Periodic4Top < A > :: innerHedge () const { 
  return 0 ;
}

template < class A > inline typename Periodic4Top < A > :: innerface_t * Periodic4Top < A > :: innerHface () { 
  return 0 ;
}

template < class A > inline const typename Periodic4Top < A > :: innerface_t * Periodic4Top < A > :: innerHface () const { 
  return 0 ;
}

template < class A > const typename Gitter :: ghostpair_STI & Periodic4Top < A > :: getGhost (int g) const {
  assert (g == 0 || g == 1);
  return _ghostPair[g];
}

template < class A > inline void Periodic4Top < A > :: setGhost ( const pair< Gitter:: helement * , int > & pair, int nr) {
  _ghostPair[nr] = pair;
}

template < class A > inline void Periodic4Top < A > :: append (Periodic4Top < A > * h) { 
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > typename Periodic4Top < A > :: myrule_t Periodic4Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > void Periodic4Top < A > :: request (myrule_t) {

  // Einen Request zur Verfeinerung zu setzen, ist vorl"aufig inhaltlich nicht
  // vorgesehen und wird deshalb ignoriert (leise).

  return ;
}

template < class A > bool Periodic4Top < A > :: refine () { 

  // Das refine () reagiert nicht auf die Elementaktivierung zur Verfeinerung
  // in der globalen Schleife, weil das perioodische Randelement sich nur auf
  // Anforderung zur Balancierung aus einem anliegenden Element direkt verfeinert.

  return true ;
}

template < class A > void Periodic4Top < A > :: backupCMode (ostream & os) const {

  // Das backup im alten Stil, d.h. levelweise die Verfeinerungsregeln
  // vom Gitter runterschreiben. Diese Technik wird nur f"ur das backup
  // noch unterst"utzt, um die Daten mit "alteren Konstruktionen visual.
  // zu k"onnen.
  
  os << getrule () << " " ;
  return ;
}

template < class A > inline void Periodic4Top < A > :: backup (ostream & os) const 
{
  doBackup( os );
}
template < class A > inline void Periodic4Top < A > :: backup (ObjectStream& os) const 
{
  doBackup( os );
}
template < class A > template <class OutStream_t>
inline void Periodic4Top < A > :: doBackup (OutStream_t& os) const 
{
  os.put ((char) getrule ()) ;
  {for (const innerperiodic4_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
  return ;
}

template < class A > inline void Periodic4Top < A > :: restore (istream & is) 
{
  doRestore( is );
}
template < class A > inline void Periodic4Top < A > :: restore (ObjectStream& is) 
{
  doRestore( is );
}

template < class A > template <class InStream_t>
inline void Periodic4Top < A > :: doRestore (InStream_t& is) 
{
  myrule_t r ((char) is.get ()) ;
  assert(getrule () == myrule_t :: nosplit) ; // Testen auf unverfeinerten Zustand
  if (r == myrule_t :: nosplit) {
    for (int i = 0 ; i < 2 ; i ++) {
      myhface4_t & f (*(this->myhface4 (i))) ;
      if (!f.leaf ()) {
        switch (f.getrule ()) {
    case balrule_t :: iso4 :
            {for (int j = 0 ; j < 4 ; j ++) f.subface4 (j)->nb.complete (f.nb) ;}
      break ;
    default :
      cerr << "**FEHLER (FATAL) beim restore mit unbekannter Balancierungsregel: "
                 << "[" << r << "]. In " << __FILE__ << __LINE__ << endl ;
      abort () ;
      break ;
  }
      }
    }
  } 
  else 
  {
    refineImmediate (r) ;
    assert (getrule() == r) ;
    {for (innerperiodic4_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
  }
  return ;
}

// Ende - Neu am 23.5.02 (BS)

#endif  //  GITTER_HEXA_TOP_H_INCLUDED
