// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 

#ifndef GITTER_HEXA_TOP_H_INCLUDED
#define GITTER_HEXA_TOP_H_INCLUDED


template < class A > class Hedge1Top : public A 
{
  protected :
    typedef Hedge1Top < A >       inneredge_t ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: myvertex_t  myvertex_t ;
    typedef typename A :: myrule_t  myrule_t ;
  protected :
    inneredge_t * _dwn, * _bbb ;
    innervertex_t * _cv ;

    unsigned char _lvl ;       
    const signed char _nChild;  
    myrule_t _rule ;  
    
  public :
    // need for refinement 
    IndexManagerType & indexManager() 
    { return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Edges ); }
    
    inline Hedge1Top (int,myvertex_t *,myvertex_t *) ;
    inline Hedge1Top (int,myvertex_t *,myvertex_t *, int nChild ) ;
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

    unsigned char _lvl ;
    const signed char _nChild;
    myrule_t _rule ;
    
  private:
    inline myhedge1_t * subedge1 (int,int) ;
    inline const myhedge1_t * subedge1 (int,int) const ;
    void splitISO4 () ;
  public:
    // for index get/free, when refinement is done 
    IndexManagerType & indexManager() { 
      return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Faces ); }
    
    // constructor for macro faces 
    inline Hface4Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
    // constructor for refined faces 
    inline Hface4Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int, int nChild ) ;
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
    inline Hbnd4Top (int,myhface4_t *,int, innerbndseg_t *, Gitter::helement_STI *, int) ;

    // constructor for macro element in the serial case 
    inline Hbnd4Top (int,myhface4_t *,int, const bnd_t bt , IndexManagerType & ) ;

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
    double _volume; 
    unsigned char _lvl ;
    const signed char _nChild; 
    myrule_t _rule, _req ;
    bool _affine;

    void splitISO8 () ;
  protected:
    // for HexaTop, when refinement is done 
    IndexManagerType & indexManager() { 
      return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Elements ); }
    
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    myhface4_t * subface4 (int,int) ;
    const myhface4_t * subface4 (int,int) const ;
    
  public:
    // Constructor for macro elements 
    HexaTop (int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
             myhface4_t *,int,myhface4_t *,int,myhface4_t *,int);
    
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
    unsigned char _lvl ;
    const signed char _nChild; 
    myrule_t _rule ;
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


template < class A > inline Hedge1Top < A > :: 
  Hedge1Top (int l, myvertex_t * a, myvertex_t * b ) 
  : A (a,b), 
  _dwn (0), _bbb (0), _cv (0), 
  _lvl (l), 
  _nChild(0),
  _rule (myrule_t :: nosplit)
{
  this->setIndex( indexManager().getIndex() );  
  return ;
}

template < class A > inline Hedge1Top < A > :: Hedge1Top (int l, myvertex_t * a, myvertex_t * b, int nChild ) 
  : A (a,b), 
  _dwn (0), _bbb (0), _cv (0), 
  _lvl (l), 
  _nChild(nChild),
  _rule (myrule_t :: nosplit)
{
  this->setIndex( indexManager().getIndex() );  
  return ;
}

template < class A > Hedge1Top < A > :: ~Hedge1Top () 
{
  this->freeIndex( indexManager() );
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

template < class A > inline Hface4Top < A > :: 
Hface4Top (int l, myhedge1_t * e0, int t0, myhedge1_t * e1, int t1, 
  myhedge1_t * e2, int t2, myhedge1_t * e3, int t3 ) 
  : A (e0, t0, e1, t1, e2, t2, e3, t3), 
  _dwn (0), _bbb (0), _cv (0), _ed (0), 
  _lvl (l), 
  _nChild(0),
  _rule (myrule_t :: nosplit)  
{
  this->setIndex( indexManager().getIndex() );  
  return ;
}

template < class A > inline Hface4Top < A > :: Hface4Top (int l, myhedge1_t * e0, int t0, myhedge1_t * e1, int t1, 
  myhedge1_t * e2, int t2, myhedge1_t * e3, int t3,int nChild ) 
  : A (e0, t0, e1, t1, e2, t2, e3, t3), 
  _dwn (0), _bbb (0), _cv (0), _ed (0), 
  _lvl (l), 
  _nChild(nChild),
  _rule (myrule_t :: nosplit)
{
  this->setIndex( indexManager().getIndex() );  
  return ;
}

template < class A > Hface4Top < A > :: ~Hface4Top () 
{
  this->freeIndex( indexManager() );
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
Hbnd4Top (int l, myhface4_t * f, int i, 
          innerbndseg_t * up, Gitter::helement_STI * gh, int gFace ) : 
  A (f, i), _bbb (0), _dwn (0), _up(up) , 
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
Hbnd4Top (int l, myhface4_t * f, int i, const bnd_t bt , IndexManagerType & im)
  : A (f, i), _bbb (0), _dwn (0), _up(0) , 
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

#endif  //  GITTER_HEXA_TOP_H_INCLUDED
