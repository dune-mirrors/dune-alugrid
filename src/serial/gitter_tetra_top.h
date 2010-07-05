// (c) mario ohlberger, 1998
// modifications for dune interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_TETRATOP_H_INCLUDED
#define GITTER_TETRATOP_H_INCLUDED

#include "gitter_sti.h"
#include "gitter_hexa_top.h"

template < class A > class Hface3Top : public A 
{
  public :
    typedef Hface3Top < A >             innerface_t ;
    typedef typename A :: inneredge_t   inneredge_t;
    typedef typename A :: innervertex_t innervertex_t;
    typedef typename A :: myhedge1_t    myhedge1_t ;
    typedef typename A :: myvertex_t    myvertex_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef InnerStorage< InnerEdgeStorage< innerface_t , false > > inner_t ;

  private :
    innerface_t * _bbb ;
    inner_t  * _inner ;

    unsigned char _lvl ;
    const signed char _nChild;
    myrule_t _rule ;

  protected:  
    // we need this because TetraTop needs access to this indexManager 
    inline IndexManagerType & indexManager () { 
      return  this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Faces ); }    

  private:
    inline myhedge1_t * subedge1 (int,int) ;
    inline const myhedge1_t * subedge1 (int,int) const ;
    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_iso4 () ;
  public :
    // constructor for macro elements 
    inline Hface3Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int ) ;
    // constructor for refined elements 
    inline Hface3Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int, int nChild ) ;
    virtual inline ~Hface3Top () ;
    innervertex_t * subvertex (int) ;
    const innervertex_t * subvertex (int) const ;
    inneredge_t * subedge1 (int) ;
    const inneredge_t * subedge1 (int) const ;
    innerface_t * subface3 (int) ;
    const innerface_t * subface3 (int) const ;
    inline int level () const ;
    inline int nChild () const ; 
    innervertex_t * innerVertex () ;
    const innervertex_t * innerVertex () const ;
    inneredge_t * innerHedge () ;
    const inneredge_t * innerHedge () const ;
    innerface_t * down () ;
    const innerface_t * down () const ;
    innerface_t * next () ;
    const innerface_t * next () const ;
    void append (innerface_t * f) ;
  public:
    virtual myrule_t getrule () const ;
    virtual bool refine (myrule_t,int) ;
    virtual void refineImmediate (myrule_t) ;
    virtual bool coarse () ;
  public :
    virtual void backup (ostream &) const ;
    virtual void restore (istream &) ;
    
    virtual void backup (ObjectStream &) const ;
    virtual void restore (ObjectStream &) ;

  protected:
    // non-virtual methods of down and innerVertex 
    innerface_t* dwnPtr() ;
    const innerface_t* dwnPtr() const ;
    inneredge_t* inEd() ;
    const inneredge_t* inEd() const ;

    template <class OutStream_t> 
    void doBackup(OutStream_t &) const;
    
    template <class InStream_t> 
    void doRestore(InStream_t &);
} ;


template < class A > class Hbnd3Top : public A {
  protected :
    typedef Hbnd3Top < A >    innerbndseg_t ;
    typedef typename A :: myhface3_t  myhface3_t ;
    typedef typename A :: balrule_t balrule_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef typename A :: bnd_t bnd_t;
    bool refineLikeElement (balrule_t) ;
    inline void append (innerbndseg_t *) ;
  private :
    innerbndseg_t * _bbb, * _dwn , * _up ;
    IndexManagerType & _indexManager;

    int _lvl ;
    const bnd_t _bt; // type of boundary 
    int _segmentIndex; // segment index of macro face 

    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_iso4 () ;
    inline bool coarse () ;
   
  public:
    // constructor for serial macro boundary elements  
    inline Hbnd3Top (int,myhface3_t *,int,
                     const bnd_t b ,
                     IndexManagerType& );
    
    // constructor for children 
    inline Hbnd3Top (int, myhface3_t *,int, 
                     innerbndseg_t * up, const bnd_t b, 
                     IndexManagerType& ,
                     typename Gitter::helement_STI * gh, int gFace ) ;

    inline virtual ~Hbnd3Top () ;
    bool refineBalance (balrule_t,int) ;
    bool bndNotifyCoarsen () ;
    void restoreFollowFace () ;
    inline int level () const ;
    inline int segmentIndex () const ;
    inline innerbndseg_t * next () ;
    inline innerbndseg_t * down () ;
    inline const innerbndseg_t * next () const ;
    inline const innerbndseg_t * down () const ;

    // for dune 
    inline innerbndseg_t * up () ;
    inline const innerbndseg_t * up () const ;
    
    inline bnd_t bndtype () const { return _bt; }
    
  protected:
    // set boundary id for all item connected to this hbnd
    void setBoundaryId( const int id ); 
} ;

template < class A > class TetraTop : public A {
  public :
    typedef TetraTop < A >    innertetra_t  ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: inneredge_t   inneredge_t ;
    typedef typename A :: innerface_t   innerface_t ;
    typedef typename A :: myhedge1_t  myhedge1_t ;
    typedef typename A :: myhface3_t  myhface3_t ;
    typedef typename A :: myrule_t  myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    typedef InnerStorage< InnerFaceStorage< innertetra_t , false > > inner_t ;

  protected:  
    void refineImmediate (myrule_t) ;
    inline void append (innertetra_t * h) ;
  private :
    innertetra_t * _bbb, * _up ; 
    inner_t * _inner ;
    const double _volume;

    unsigned char _lvl ;
    const signed char _nChild;
    myrule_t _req, _rule ;
    
  private :
    inline IndexManagerType & indexManager() { 
      return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Elements ); }
    double calculateChildVolume(const double) const;
    
    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_e23 () ;
    void split_e30 () ;
    void split_e31 () ;
    void splitISO8 () ;
  protected :
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    myhface3_t * subface3 (int,int) ;
    const myhface3_t * subface3 (int i, int j) const ;
  public:
    // constructor for refined elements 
    inline TetraTop (int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,
                     myhface3_t *,int,innertetra_t *up, int nChild, double vol) ;
    // constructor for macro elements 
    inline TetraTop (int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,
                     myhface3_t *, int ) ;
    virtual inline ~TetraTop () ;
    inline innertetra_t * up () ;
    inline const innertetra_t * up () const;
    inline innertetra_t * down () ;
    inline const innertetra_t * down () const ;
    inline innertetra_t * next () ;
    inline const innertetra_t * next () const ;
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
    
    void backup (ObjectStream &) const ;
    void restore (ObjectStream &) ;
    
    // backup and restore index 
    void backupIndex (ostream &) const ;
    void restoreIndex (istream &, vector<bool>(&)[4] ) ;
  protected:  
    // non-virtual methods of down and innerVertex 
    innertetra_t* dwnPtr() ;
    const innertetra_t* dwnPtr() const ;
    inneredge_t* inEd() ;
    const inneredge_t* inEd() const ;
    innerface_t* inFce() ;
    const innerface_t* inFce() const ;

    template <class OutStream_t> 
    void doBackup(OutStream_t &) const;
    
    template <class InStream_t> 
    void doRestore(InStream_t &);
};

template < class A > class Periodic3Top : public A {
  protected :
    typedef Periodic3Top < A >    innerperiodic3_t  ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: inneredge_t   inneredge_t ;
    typedef typename A :: innerface_t   innerface_t ;
    typedef typename A :: myhedge1_t  myhedge1_t ;
    typedef typename A :: myhface3_t  myhface3_t ;
    typedef typename A :: myrule_t  myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    void refineImmediate (myrule_t) ;
    inline void append (innerperiodic3_t * h) ;
    
    //us
    typedef typename A :: GhostElement_t GhostElement_t;
    typedef Gitter :: ghostpair_STI ghostpair_STI;
    
  private :
    innerperiodic3_t * _dwn, * _bbb, * _up ; 
    unsigned char _lvl ;
    const signed char _nChild; 
    myrule_t _rule ;
  private :
    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_iso4 () ;
  protected :
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    myhface3_t * subface3 (int,int) ;
    const myhface3_t * subface3 (int i, int j) const ;
  public:
    // constructor for macro elements 
    inline Periodic3Top (int,myhface3_t *,int,myhface3_t *,int) ;
    // construtor for refined elements 
    inline Periodic3Top (int,myhface3_t *,int,myhface3_t *,int,innerperiodic3_t * up, int nChild ) ;
    virtual inline ~Periodic3Top () ;
    inline innerperiodic3_t * up () ;
    inline const innerperiodic3_t * up () const;
    inline innerperiodic3_t * down () ;
    inline const innerperiodic3_t * down () const ;
    inline innerperiodic3_t * next () ;
    inline const innerperiodic3_t * next () const ;
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
  public:
    void backupCMode (ostream &) const ;
    void backup (ostream &) const ;
    void restore (istream &) ;
    
    void backup (ObjectStream &) const ;
    void restore (ObjectStream &) ;
  protected:  
    template <class OutStream_t> 
    void doBackup(OutStream_t &) const;
    
    template <class InStream_t> 
    void doRestore(InStream_t &);
    
  public:   
    // get ghost pair 
    inline const ghostpair_STI & getGhost (int) const ;
    // set ghost pair, nr should be 0 or 1, I guess 
    inline void setGhost ( const pair< Gitter :: helement * , int > & pair, int nr);

  private:
    mutable ghostpair_STI _ghostPair [2];
    //_ghostPair[0] liegt an myhface3[0] und ist das affine Bild vom Tetra an myhface3(1)
    
    // refine ghost if face is refined and ghost is not zero
    void splitGhosts () {} ; 
    // coarse ghost if face is coarsened
    void coarseGhosts () {};

};
  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //


// #     #                                  #####  #######
// #     #  ######    ##     ####   ###### #     #    #      ####   #####
// #     #  #        #  #   #    #  #            #    #     #    #  #    #
// #######  #####   #    #  #       #####   #####     #     #    #  #    #
// #     #  #       ######  #       #            #    #     #    #  #####
// #     #  #       #    #  #    #  #      #     #    #     #    #  #
// #     #  #       #    #   ####   ######  #####     #      ####   #


template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: dwnPtr () {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: dwnPtr () const {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: inEd () {
  return (_inner) ? _inner->ed() : 0 ;
}

template < class A > inline const typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: inEd () const {
  return (_inner) ? _inner->ed() : 0 ;
}
template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: down () {
  return dwnPtr() ;
}

template < class A > inline const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: down () const {
  return dwnPtr() ;
}

template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: next () {
  return _bbb ;
}

template < class A > const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: next () const {
  return _bbb ;
}

template < class A > inline int Hface3Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline int Hface3Top < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 4 );
  return _nChild ;
}

template < class A > inline typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: innerVertex () {
  return 0 ;
}

template < class A > inline const typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: innerVertex () const {
  return 0 ;
}

template < class A > inline typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: innerHedge () {
  return inEd();
}

template < class A > inline const typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: innerHedge () const {
  return inEd();
}

template < class A > inline typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: subvertex (int) {
  assert (getrule() == myrule_t :: iso4) ;
  return 0 ;
}

template < class A > inline const typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: subvertex (int) const {
  assert (getrule() == myrule_t :: iso4) ;
  return 0 ;
}

template < class A > inline typename Hface3Top < A > :: myhedge1_t * Hface3Top < A > :: subedge1 (int i,int j) {
  assert(j == 0 || j == 1) ;
  return this->myhedge1 (i)->subedge1 (j ? 1 - this->twist(i) : this->twist(i)) ;
}

template < class A > inline const typename Hface3Top < A > :: myhedge1_t * Hface3Top < A > :: subedge1 (int i,int j) const {
  assert(j == 0 || j == 1) ;
  return this->myhedge1 (i)->subedge1 (j ? 1 - this->twist(i) : this->twist(i)) ;
}

template < class A > inline typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: subedge1 (int n) {
  inneredge_t * e = inEd() ;
  for (int i = 0 ; i < n ; i ++ ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > inline const typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: subedge1 (int n) const {
  const inneredge_t * e = inEd();
  for (int i = 0 ; i < n ; i ++ ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: subface3 (int n) {
  innerface_t * f = dwnPtr() ;
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > inline const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: subface3 (int n) const {
  const innerface_t * f = dwnPtr();
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > inline void Hface3Top < A > :: append (innerface_t * f) {
  assert (!_bbb) ;
  _bbb = f ;
  return ;
}

template < class A > inline typename Hface3Top < A > :: myrule_t Hface3Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

// constructor called during refinement 
template < class A > inline Hface3Top < A > :: 
Hface3Top (int l, myhedge1_t * e0, 
  int t0, myhedge1_t * e1, int t1, myhedge1_t * e2, int t2,
  int nChild ) : 
  A (e0, t0, e1, t1, e2, t2), 
  _bbb (0), _inner(0) ,
  _lvl (l),
  _nChild (nChild),
  _rule (myrule_t :: nosplit)
{
  this->setIndex( indexManager().getIndex() );
  return ;
}

// constructor called while creating macro face 
template < class A > inline Hface3Top < A > :: 
Hface3Top (int l, myhedge1_t * e0, 
  int t0, myhedge1_t * e1, int t1, myhedge1_t * e2, int t2) : 
  A (e0, t0, e1, t1, e2, t2), 
  _bbb (0), _inner (0), 
  _lvl (l),
  _nChild (0),
  _rule (myrule_t :: nosplit)
{
  this->setIndex( indexManager().getIndex() );
  return ;
}

template < class A > inline Hface3Top < A > :: ~Hface3Top () 
{
  this->freeIndex( indexManager() );
  if (_bbb) delete _bbb ;
  if (_inner) delete _inner ;
  return ;
}

// #     #                          #####  #######
// #     #  #####   #    #  #####  #     #    #      ####   #####
// #     #  #    #  ##   #  #    #       #    #     #    #  #    #
// #######  #####   # #  #  #    #  #####     #     #    #  #    #
// #     #  #    #  #  # #  #    #       #    #     #    #  #####
// #     #  #    #  #   ##  #    # #     #    #     #    #  #
// #     #  #####   #    #  #####   #####     #      ####   #

// serial macro bnd constructor 
template < class A > inline Hbnd3Top < A > :: 
Hbnd3Top (int l, myhface3_t * f, int i, 
          const bnd_t bt, 
          IndexManagerType & im ) :
  A (f, i ), _bbb (0), _dwn (0), _up (0) , 
  _indexManager(im) ,
  _lvl (l), _bt (bt) 
{
  this->setIndex( _indexManager.getIndex() );
  // for macro bnd faces store current index as segment index 
  _segmentIndex = this->getIndex();
  setBoundaryId( _bt ); 
  return ;
}

template < class A > inline Hbnd3Top < A > :: 
Hbnd3Top (int l, myhface3_t * f, 
          int i,  
          innerbndseg_t * up, bnd_t bt, 
          IndexManagerType & im, 
          Gitter::helement_STI * gh, int gFace ) : 
  A (f, i ), _bbb (0), _dwn (0), _up (up) , 
  _indexManager(im) ,
  _lvl (l), 
  _bt (bt) 
{
  typedef Gitter :: ghostpair_STI ghostpair_STI;
  this->setGhost ( ghostpair_STI (gh , gFace) );
  this->setIndex( _indexManager.getIndex() );

  // get segment index from father if existent 
  _segmentIndex = (_up) ? _up->_segmentIndex : this->getIndex() ;
 
  setBoundaryId( _bt ); 
  return ;
}

template < class A > inline Hbnd3Top < A > :: ~Hbnd3Top () 
{
  this->freeIndex( this->_indexManager );
  if (this->isLeafEntity()) this->detachleafs();
  if (_bbb) delete _bbb ; 
  if (_dwn) delete _dwn ;
  return ; 
}

template < class A > inline int Hbnd3Top < A > :: segmentIndex () const {
  return _segmentIndex;
}

template < class A > inline int Hbnd3Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: next () { 
  return _bbb ;
}

template < class A > inline const typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: next () const { 
  return _bbb ;
}

template < class A > inline typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: down () { 
  return _dwn ;
}

template < class A > inline const typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: down () const { 
  return _dwn ;
}

template < class A > inline typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: up () { 
  return _up ;
}

template < class A > inline const typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: up () const { 
  return _up ;
}

template < class A > inline void Hbnd3Top < A > :: append (innerbndseg_t * b) {
  assert (_bbb == 0) ;
  _bbb = b ;
  return ;
}

// #######                                 #######
//    #     ######   #####  #####     ##      #      ####   #####
//    #     #          #    #    #   #  #     #     #    #  #    #
//    #     #####      #    #    #  #    #    #     #    #  #    #
//    #     #          #    #####   ######    #     #    #  #####
//    #     #          #    #   #   #    #    #     #    #  #
//    #     ######     #    #    #  #    #    #      ####   #

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: dwnPtr() {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A > :: dwnPtr() const {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline typename TetraTop < A > :: inneredge_t * TetraTop < A > :: inEd() {
  return (_inner) ? _inner->ed() : 0 ;
}

template < class A > inline const typename TetraTop < A > :: inneredge_t * TetraTop < A > :: inEd() const {
  return (_inner) ? _inner->ed() : 0 ;
}

template < class A > inline typename TetraTop < A > :: innerface_t * TetraTop < A > :: inFce() {
  return (_inner) ? _inner->fce() : 0 ;
}

template < class A > inline const typename TetraTop < A > :: innerface_t * TetraTop < A > :: inFce() const {
  return (_inner) ? _inner->fce() : 0 ;
}

template < class A > inline double TetraTop < A > :: calculateChildVolume (const double childVolume) const 
{
  // if vertex projection is available on a neighbor 
  // volume has to be recalculated 
  return ( this->myGrid()->vertexProjection() ) ? -1.0 : childVolume; 
}

template < class A > inline int TetraTop < A > :: level () const {
  return _lvl ;
}

template < class A > inline double TetraTop < A > :: volume () const {
  return _volume ;
}

template < class A > inline int TetraTop < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 8 );
  return _nChild ;
}

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: up () {
  return _up ;
}
template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A> :: up () const {
  return _up ;
} 

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: down () {
  return dwnPtr() ;
}

template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A > :: down () const {
  return dwnPtr() ;
}

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: next () {
  return _bbb ;
}

template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A > :: next () const {
  return _bbb ;
}

template < class A > inline typename TetraTop < A > :: innervertex_t * TetraTop < A > :: innerVertex () {
  return 0 ;
}

template < class A > inline const typename TetraTop < A > :: innervertex_t * TetraTop < A > :: innerVertex () const {
  return 0 ;
}

template < class A > inline typename TetraTop < A > :: inneredge_t * TetraTop < A > :: innerHedge () {
  return inEd();
}

template < class A > inline const typename TetraTop < A > :: inneredge_t * TetraTop < A > :: innerHedge () const {
  return inEd();
}

template < class A > inline typename TetraTop < A > :: innerface_t * TetraTop < A > :: innerHface () {
  return inFce();
}

template < class A > inline const typename TetraTop < A > :: innerface_t * TetraTop < A > :: innerHface () const {
  return inFce();
}

template < class A > inline void TetraTop < A > :: append (TetraTop < A > * h) {
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > inline typename TetraTop < A > :: myrule_t TetraTop < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > inline typename TetraTop < A > :: myrule_t TetraTop < A > :: requestrule () const {
  return myrule_t (_req) ;
}

template < class A > inline void TetraTop < A > :: request (myrule_t r) 
{
  assert (r.isValid ()) ;
  _req = r ;
  return ;
}

template < class A > inline bool TetraTop < A > :: bndNotifyCoarsen () {
  return true ;
}

// ######                                                           #####  #######
// #     #  ######  #####      #     ####   #####      #     ####  #     #   #      ####   #####
// #     #  #       #    #     #    #    #  #    #     #    #    #       #   #     #    #  #    #
// ######   #####   #    #     #    #    #  #    #     #    #       #####    #     #    #  #    #
// #        #       #####      #    #    #  #    #     #    #            #   #     #    #  #####
// #        #       #   #      #    #    #  #    #     #    #    # #     #   #     #    #  #
// #        ######  #    #     #     ####   #####      #     ####   #####    #      ####   #
   
template < class A > inline Periodic3Top < A > :: Periodic3Top (int l, myhface3_t * f0, int t0,
  myhface3_t * f1, int t1) 
 : A (f0, t0, f1, t1)
 , _dwn (0), _bbb (0), _up(0)
 , _lvl (l) 
 , _nChild(0)
 , _rule (myrule_t :: nosplit)
{ 
 return ;
}

template < class A > inline Periodic3Top < A > :: Periodic3Top (int l, myhface3_t * f0, int t0,
  myhface3_t * f1, int t1, innerperiodic3_t * up, int nChild ) 
  : A (f0, t0, f1, t1)
  , _dwn (0), _bbb (0), _up(up)
  , _lvl (l) 
  , _nChild (nChild) 
  , _rule (myrule_t :: nosplit)
{
  return ;
}

template < class A > inline Periodic3Top < A > :: ~Periodic3Top () {
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
  return ;
}

template < class A > inline int Periodic3Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline int Periodic3Top < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 4 );
  return _nChild ;
}

template < class A > inline typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: up () {
  return _up ;
}
template < class A > inline const typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A> :: up () const {
  return _up ;
}

template < class A > inline typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: down () {
  return _dwn ;
}

template < class A > inline const typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: down () const {
  return _dwn ;
}

template < class A > inline typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: next () {
  return _bbb ;
}

template < class A > inline const typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: next () const {
  return _bbb ;
}

template < class A > inline typename Periodic3Top < A > :: innervertex_t * Periodic3Top < A > :: innerVertex () {
  return 0 ;
}

template < class A > inline const typename Periodic3Top < A > :: innervertex_t * Periodic3Top < A > :: innerVertex () const {
  return 0 ;
}

template < class A > inline typename Periodic3Top < A > :: inneredge_t * Periodic3Top < A > :: innerHedge () {
  return 0 ;
}

template < class A > inline const typename Periodic3Top < A > :: inneredge_t * Periodic3Top < A > :: innerHedge () const {
  return 0 ;
}

template < class A > inline typename Periodic3Top < A > :: innerface_t * Periodic3Top < A > :: innerHface () {
  return 0 ;
}

template < class A > inline const typename Periodic3Top < A > :: innerface_t * Periodic3Top < A > :: innerHface () const {
  return 0 ;
}

template < class A > inline void Periodic3Top < A > :: append (Periodic3Top < A > * h) {
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > inline typename Periodic3Top < A > :: myrule_t Periodic3Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > inline void Periodic3Top < A > :: request (myrule_t) {

  // Einen Request zur Verfeinerung zu setzen, ist vorl"aufig inhaltlich nicht
  // vorgesehen und wird deshalb ignoriert (leise).

  return ;
}

template < class A > 
inline void  Periodic3Top < A > :: setGhost ( const pair< Gitter :: helement * , int > & pair, int nr)
{
  assert( (nr >= 0) && (nr < 2));  
  _ghostPair[nr] = pair;
}

template < class A > 
inline const Gitter :: ghostpair_STI & 
Periodic3Top < A > :: getGhost ( int nr ) const 
{
  assert( (nr >= 0) && (nr < 2));  
  return _ghostPair[nr];
}

#endif  //  GITTER_TetraTop_H_INCLUDED
