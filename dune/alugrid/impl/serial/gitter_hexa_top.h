// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface
// (c) Robert Kloefkorn 2004 - 2005
#ifndef GITTER_HEXA_TOP_H_INCLUDED
#define GITTER_HEXA_TOP_H_INCLUDED

#include <cmath>
#include "gitter_sti.h"

namespace ALUGrid
{
  template< class Impl , bool hasVertex >
  class InnerVertexStorage
  : public MyAlloc
  {
    InnerVertexStorage ( const InnerVertexStorage & );

  protected:
    typedef Impl                           down_t;
    typedef typename Impl::innervertex_t innervertex_t;
    typedef void* inneredge_t;
    typedef void* innerface_t;
    innervertex_t* _cv;
  public:
    InnerVertexStorage( int level,
                        double x, double y,
                        double z, innervertex_t& vx )
      : _cv( new innervertex_t(level, x, y, z, vx) )
    {}

    InnerVertexStorage()
      : _cv( 0 )
    {}

    // destructor
    ~InnerVertexStorage() { delete _cv; _cv = 0; }

    // vertex methods
    innervertex_t* cv()             { return _cv; }
    const innervertex_t* cv() const { return _cv; }

    // do nothing here
    void store()  {}
  };

  template < class Impl >
  class InnerVertexStorage< Impl, false > : public MyAlloc
  {
    InnerVertexStorage( const InnerVertexStorage& );
  protected:
    typedef Impl                           down_t;
    typedef typename Impl::innervertex_t innervertex_t;
    typedef void* inneredge_t;
    typedef void* innerface_t;
  public:
    InnerVertexStorage( int level,
                        double x, double y,
                        double z, innervertex_t& vx )
    {
        std::cerr << "This method should not be called" << std::endl;
    }

    InnerVertexStorage( )
    {    }

    // do nothing here
    void store()  {}

    // vertex methods
    innervertex_t* cv()             { return 0; }
    const innervertex_t* cv() const { return 0; }
  };

  template < class Impl, bool hasVertex >
  class InnerEdgeStorage : public InnerVertexStorage< Impl, hasVertex >
  {
  protected:
    typedef InnerVertexStorage< Impl, hasVertex >    base_t;
    typedef typename base_t::innervertex_t innervertex_t;
    typedef typename Impl::inneredge_t  inneredge_t;
    typedef void* innerface_t;
    inneredge_t * _ed;
  public:
    using base_t::store;

    InnerEdgeStorage( int level,
                      double x, double y,
                      double z, innervertex_t& vx )
      : base_t( level, x, y, z, vx ) , _ed ( 0 )
    {  }

    InnerEdgeStorage ( inneredge_t* ed )
      : _ed ( ed ) {}

    // store edge pointer
    void store( inneredge_t* ed )  {  _ed = ed;  }

    // destructor
    ~InnerEdgeStorage() { delete _ed; _ed = 0; }
    // edge methods
    inneredge_t* ed() { return _ed;}
    const inneredge_t* ed() const { return _ed;}
  };

  template < class Impl, bool hasVertex  >
  class InnerFaceStorage : public InnerEdgeStorage< Impl, hasVertex >
  {
  protected:
    typedef InnerEdgeStorage< Impl , hasVertex >      base_t;
    typedef typename base_t::innervertex_t innervertex_t;
    typedef typename base_t::inneredge_t   inneredge_t;
    typedef typename Impl  ::innerface_t   innerface_t;
    innerface_t * _fce;
  public:
    using base_t::store;

    InnerFaceStorage( int level,
                      double x, double y,
                      double z, innervertex_t& vx )
      : base_t( level, x, y, z, vx ) , _fce ( 0 )
    {  }

    InnerFaceStorage( ) : _fce( 0 ) {}

    // store face pointer
    void store( innerface_t* fce )  {  _fce = fce;  }

    InnerFaceStorage ( innerface_t* fce,
                       inneredge_t* ed = 0 )
      : base_t( ed ), _fce ( fce ) {}

    // destructor
    ~InnerFaceStorage() { delete _fce; _fce = 0; }
    // face methods
    innerface_t* fce() { return _fce;}
    const innerface_t* fce() const { return _fce;}
  };

  template < class ImplStorage >
  class InnerStorage : public ImplStorage
  {
    InnerStorage( const InnerStorage& );
  protected:
    typedef ImplStorage  base_t;
    typedef typename base_t::innervertex_t  innervertex_t;
    typedef typename base_t::inneredge_t    inneredge_t;
    typedef typename base_t::innerface_t    innerface_t;
    typedef typename base_t::down_t  down_t;
    down_t      *_dwn;
  public:
    using base_t::store;

    InnerStorage( int level,
                    double x, double y,
                    double z, innervertex_t& vx )
      : base_t( level, x, y, z, vx ) , _dwn( 0 )
    {  }

    InnerStorage() : _dwn ( 0 ) { }
    // constructor taking down pointer
    InnerStorage( down_t * dwn ) : _dwn ( dwn ) {}

    // constructor taking down and edge
    InnerStorage ( down_t * dwn,
                   inneredge_t* ed )
      : base_t( ed ), _dwn( dwn ) {}

    // constructor taking down, face and edge
    InnerStorage ( down_t * dwn,
                   innerface_t* fce,
                   inneredge_t* ed = 0 )
      : base_t( fce, ed ), _dwn( dwn ) {}

    // store down pointer
    void store( down_t* dwn )  {  _dwn = dwn;  }

    // destructor
    ~InnerStorage() { delete _dwn; _dwn = 0;  }

    // down methods
    down_t* dwn() { return _dwn;}
    const down_t* dwn() const { return _dwn;}
  };



  template< class A >
  class Hedge1Top
  : public A
  {
    public:
      using A::myvertex;

      typedef Hedge1Top < A >             inneredge_t;
      typedef typename A::innervertex_t innervertex_t;
      typedef typename A::myvertex_t    myvertex_t;
      typedef typename A::myrule_t      myrule_t;
      typedef InnerStorage < InnerVertexStorage< inneredge_t, true > > inner_t;
    protected :
      inneredge_t * _bbb;  // 8
      inner_t * _inner;    // 8

      myrule_t _rule;
      const unsigned char _lvl;
      const unsigned char _child;  // 8 = 24

    public :
      // need for refinement
      IndexManagerType &indexManager ()
      {
        return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType::IM_Edges );
      }

      inline Hedge1Top (int,myvertex_t *,myvertex_t *);
      inline Hedge1Top (int,myvertex_t *,myvertex_t *, int nChild );
      virtual ~Hedge1Top ();
      inneredge_t * subedge (int);
      const inneredge_t * subedge (int) const;
      inneredge_t * down ();
      const inneredge_t * down () const;
      inneredge_t * next ();
      const inneredge_t * next () const;
      inline int level () const;
      inline int nChild () const;
      inline void append (inneredge_t *);
      innervertex_t *innerVertex ();
      const innervertex_t * innerVertex () const;
      innervertex_t *subvertex ( int );
      const innervertex_t * subvertex (int) const;
    public :
      virtual myrule_t getrule () const;
      virtual void refineImmediate (myrule_t);
      virtual bool coarse ();

    public :
      virtual void backup (std::ostream &) const;
      virtual void restore (std::istream &);

      virtual void backup (ObjectStream&) const;
      virtual void restore (ObjectStream&);
    protected:
      bool isRealLine() const;
      // non-virtual methods of down and innerVertex
      inneredge_t* dwnPtr();
      const inneredge_t* dwnPtr() const;
      innervertex_t* inVx();
      const innervertex_t* inVx() const;

      template <class OutStream_t>
      void doBackup(OutStream_t &) const;
      template <class InStream_t>
      void doRestore(InStream_t &);
  };

  template < class A > class Hface4Top : public A
  {
    public :
      using A::twist;
      using A::myvertex;
      using A::myhedge;

      typedef Hface4Top < A >                innerface_t;
      typedef typename A::inneredge_t        inneredge_t;
      typedef typename A::innervertex_t      innervertex_t;
      typedef typename A::myhedge_t          myhedge_t;
      typedef typename A::myvertex_t         myvertex_t;
      typedef typename A::myrule_t           myrule_t;

      //for iso2 we do not get an inner Vertex
      typedef InnerStorage < InnerEdgeStorage< innerface_t , true > > inner_t;

    private :
      innerface_t * _bbb; // 8
      inner_t *_inner;    // 8

      myrule_t _rule;    // 8  = 24 byte
      const unsigned char _lvl;
      const signed char _nChild;

    private:
      inline myhedge_t * subedge (int,int);
      inline const myhedge_t * subedge (int,int) const;
      void splitISO4 ();
      void splitISO2 ();
    public:
      // for index get/free, when refinement is done
      IndexManagerType & indexManager() {
        return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType::IM_Faces ); }

      // constructor for macro faces
      inline Hface4Top (int,myhedge_t *,int,myhedge_t *,int,myhedge_t *,int,myhedge_t *,int);
      // constructor for refined faces
      inline Hface4Top (int,myhedge_t *,int,myhedge_t *,int,myhedge_t *,int,myhedge_t *,int, int nChild );
      virtual ~Hface4Top ();
      innervertex_t * subvertex (int);
      const innervertex_t * subvertex (int) const;
      inneredge_t * subedge (int);
      const inneredge_t * subedge (int) const;
      innerface_t * subface (int);
      const innerface_t * subface (int) const;

      inline int level () const;
      inline int nChild () const;
      inline innervertex_t * innerVertex ();
      inline const innervertex_t * innerVertex () const;
      inline inneredge_t * innerHedge ();
      inline const inneredge_t * innerHedge () const;
      inline innerface_t * down ();
      inline const innerface_t * down () const;
      inline innerface_t * next ();
      inline const innerface_t * next () const;
      inline void append (innerface_t * f);

    public :
      virtual myrule_t getrule () const;
      virtual bool refine (myrule_t,int);
      virtual void refineImmediate (myrule_t);
      virtual bool coarse ();
    public :
      virtual void backup (std::ostream &) const;
      virtual void restore (std::istream &);

      virtual void backup (ObjectStream&) const;
      virtual void restore (ObjectStream&);
    protected:
      // non-virtual methods of down and innerVertex
      innerface_t* dwnPtr();
      const innerface_t* dwnPtr() const;
      innervertex_t* inVx();
      const innervertex_t* inVx() const;
      inneredge_t* inEd();
      const inneredge_t* inEd() const;

      template <class OutStream_t>
      void doBackup(OutStream_t &) const;
      template <class InStream_t>
      void doRestore(InStream_t &);
  };

  template < class A > class Hbnd4Top : public A
  {
    public:
      using A::twist;
      using A::myhface4;
      using A::subface;

    protected :
      typedef Hbnd4Top < A >              innerbndseg_t;
      typedef typename A::myhface4_t    myhface4_t;
      typedef typename A::myrule_t      myrule_t;
      typedef typename A::balrule_t     balrule_t;
      typedef typename A::bnd_t         bnd_t;

      void splitISO4 ();
      void splitISO2 ();
      bool refineLikeElement (balrule_t);

      using A::bndNotifyBalance;

      // need for indices
      IndexManagerType& indexManager() {
        return myhface4(0)->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType::IM_Bnd );
      }
    private :
      innerbndseg_t * _bbb, * _dwn, * _up;     // 24 bytes
      const bnd_t _bt; // type of boundary     //  4 bytes
      int _segmentId; // index of macro boundary segment (4 bytes)

      using A :: _lvl; // declared in hbndseg4_GEO for memory padding

      inline bool coarse ();
      inline void append (innerbndseg_t *);
    public :
      // constructor for refinement
      inline Hbnd4Top (int,myhface4_t *,int, innerbndseg_t *, Gitter::helement_STI *, int);

      // constructor for macro element in the serial case
      inline Hbnd4Top (int,myhface4_t *,int, const bnd_t bt );

      virtual ~Hbnd4Top ();
      using A::refineBalance;
      bool refineBalance (balrule_t,int);
      bool bndNotifyCoarsen ();
      void restoreFollowFace ();
      int level () const;
      int segmentId () const;
      innerbndseg_t * next ();
      innerbndseg_t * down ();
      const innerbndseg_t * next () const;
      const innerbndseg_t * down () const;

      // for dune
      innerbndseg_t * up ();
      const innerbndseg_t * up () const;
      inline bnd_t bndtype () const { return _bt; }
    protected:
      void setBoundaryId (const int id);

  };

  template < class A > class HexaTop : public A {
    public :
      using A::twist;
      using A::myvertex;
      using A::myhedge;
      using A::myhface4;

      typedef HexaTop < A >             innerhexa_t;
      typedef typename A::innerface_t   innerface_t;
      typedef typename A::inneredge_t   inneredge_t;
      typedef typename A::innervertex_t innervertex_t;
      typedef typename A::myhface4_t    myhface4_t;
      typedef typename A::myhedge_t     myhedge_t;
      typedef typename A::myvertex_t    myvertex_t;
      typedef typename A::myrule_t      myrule_t;
      typedef typename A::balrule_t     balrule_t;
      typedef InnerStorage < InnerFaceStorage< innerhexa_t , true > > inner_t;

    protected:
      inline void refineImmediate (myrule_t);
      inline void append (innerhexa_t * h);
    private :
      innerhexa_t * _bbb, * _up;
      inner_t * _inner;
      double _volume;
      const unsigned char _lvl;
      const signed char _nChild;
      myrule_t _rule, _req;

      void splitISO8 ();
      void splitISO4 ();

    protected:
      // for HexaTop, when refinement is done
      IndexManagerType & indexManager() {
        return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType::IM_Elements ); }

      myhedge_t * subedge (int,int);
      const myhedge_t * subedge (int,int) const;
      myhface4_t * subface (int,int);
      const myhface4_t * subface (int,int) const;

    public:
      // Constructor for macro elements
      HexaTop (int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
               myhface4_t *,int,myhface4_t *,int,myhface4_t *,int);

      // constructor for refinement
      HexaTop (int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
               myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,
               innerhexa_t * up, int nChild , double vol );

      virtual ~HexaTop ();
      inline innerhexa_t * up ();
      inline const innerhexa_t * up () const;
      inline innerhexa_t * down ();
      inline const innerhexa_t * down () const;
      inline innerhexa_t * next ();
      inline const innerhexa_t * next () const;
      inline innervertex_t * innerVertex ();
      inline const innervertex_t * innerVertex () const;
      inline inneredge_t * innerHedge ();
      inline const inneredge_t * innerHedge () const;
      inline innerface_t * innerHface ();
      inline const innerface_t * innerHface () const;
      inline int level () const;
      inline int nChild () const;
      inline double volume () const;
    public :
      myrule_t getrule () const;
      myrule_t requestrule () const;
      bool refine ();
      void request (myrule_t);
      using A::refineBalance;
      bool refineBalance (balrule_t,int);
      bool coarse ();
      bool bndNotifyCoarsen ();
      int  backup  (std::ostream &) const;
      void restore (std::istream &);

      // backup and restore index
      void backupIndex (std::ostream &) const;
      // set entry of element to false when index is read
      void restoreIndex (std::istream &, RestoreInfo& );

      // backup and restore index
      void backupIndex (ObjectStream &) const;
      // set entry of element to false when index is read
      void restoreIndex (ObjectStream &, RestoreInfo& );

      int  backup  (ObjectStream&) const;
      void restore (ObjectStream&);

      int  vertexTwist ( const int twst, const int vx ) const  ;
      int calculateFace2Twist( const int vxIndex, const myhface4_t* subFace ) const ;
      int calculateFace3Twist( const int (&vx)[4], const myhface4_t* subFace, const int thirdVx ) const ;
      bool checkHexa( const innerhexa_t* hexa, const int  ) const;

      // change coordinates of this element (for ghost elements only)
      void changeVertexCoordinates( const int face, const std::array< std::array<alucoord_t,3>, 8 >& newCoords, const double volume )
      {
        // this should only be called for ghost elements
        alugrid_assert( this->isGhost() );

        for( int i=0; i < 4; ++i )
        {
          myvertex_t* vx = static_cast< myvertex_t* > (this->myvertex(face, i));
          vx->setCoordinates( newCoords[ i ] );
        }

        const int oppFace = Gitter::Geometric::Hexa::oppositeFace[face];
        for( int i=0; i < 4; ++i )
        {
          myvertex_t* vx = static_cast< myvertex_t* > (this->myvertex(oppFace, i));
          vx->setCoordinates( newCoords[ i+4 ] );
        }

        _volume = volume;
      }

    protected:
      // non-virtual methods of down and innerVertex
      innerhexa_t* dwnPtr();
      const innerhexa_t* dwnPtr() const;
      innervertex_t* inVx();
      const innervertex_t* inVx() const;
      inneredge_t* inEd();
      const inneredge_t* inEd() const;
      innerface_t* inFce();
      const innerface_t* inFce() const;

      template <class OutStream_t>
      int  doBackup(OutStream_t &) const;
      template <class InStream_t>
      void doRestore(InStream_t &);
      template< class istream_t >
      void restoreIndexImpl( istream_t &, RestoreInfo& );
  };

  template < class A > class Periodic4Top : public A {
    public:
      using A::twist;
      using A::myhface4;

    protected :
      typedef Periodic4Top < A >    innerperiodic4_t ;
      typedef typename A::innervertex_t innervertex_t;
      typedef typename A::inneredge_t   inneredge_t;
      typedef typename A::innerface_t   innerface_t;
      typedef typename A::myhedge_t    myhedge_t;
      typedef typename A::myhface4_t    myhface4_t;
      typedef typename A::myrule_t      myrule_t;
      typedef typename A::balrule_t     balrule_t;
      typedef typename A::bnd_t         bnd_t;

      void refineImmediate (myrule_t);
      inline void append (innerperiodic4_t * h);

    private :
      innerperiodic4_t * _dwn, * _bbb, * _up;
      // we need two indices since this pointer
      // is available on the two periodic sides
      int _segmentId[ 2 ];
      bnd_t _bt[ 2 ];
      unsigned char _lvl;
      const signed char _nChild;
      myrule_t _rule;
    private :
      void splitISO4 ();
      void splitISO2 ();
    protected :
      myhedge_t * subedge (int,int);
      const myhedge_t * subedge (int,int) const;
      myhface4_t * subface (int,int);
      const myhface4_t * subface (int i, int j) const;

      // we need this for the boundary segment index
      inline IndexManagerType & indexManager () {
        return  this->myhface4(0)->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType::IM_Bnd );
      }

    public:

      Periodic4Top (int,myhface4_t *,int,myhface4_t *,int, const bnd_t (&bt)[ 2 ] );
      Periodic4Top (int,myhface4_t *,int,myhface4_t *,int,
                    innerperiodic4_t * up, int nChild );
      virtual ~Periodic4Top ();

      inline innerperiodic4_t * up ();
      inline const innerperiodic4_t * up () const;

      inline innerperiodic4_t * down ();
      inline const innerperiodic4_t * down () const;
      inline innerperiodic4_t * next ();
      inline const innerperiodic4_t * next () const;
      inline innervertex_t * innerVertex ();
      inline const innervertex_t * innerVertex () const;
      inline inneredge_t * innerHedge ();
      inline const inneredge_t * innerHedge () const;
      inline innerface_t * innerHface ();
      inline const innerface_t * innerHface () const;
      inline int level () const;
      inline int nChild () const;
      inline int segmentId (const int) const;
      inline bnd_t bndtype (const int i) const
      {
        alugrid_assert ( i==0 || i == 1 );
        return _bt[ i ];
      }
    public :
      myrule_t getrule () const;
      bool refine ();
      void request (myrule_t);
      using A::refineBalance;
      bool refineBalance (balrule_t,int);
      bool coarse ();
      bool bndNotifyCoarsen ();

    public:
      int  backup (std::ostream &) const;
      void restore (std::istream &);

      int  backup (ObjectStream&) const;
      void restore (ObjectStream&);
    protected:
      template <class OutStream_t>
      int  doBackup(OutStream_t &) const;
      template <class InStream_t>
      void doRestore(InStream_t &);
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


  template < class A > inline Hedge1Top < A >::
    Hedge1Top (int l, myvertex_t * a, myvertex_t * b )
    : A (a,b),
    _bbb ( 0 ),
    _inner (0),
    _rule (myrule_t::nosplit),
    _lvl (l),
    _child( 0 )
  {
    //std::cout << a << b << std::endl;
    this->setIndex( indexManager().getIndex() );
    if( ( a->is2d() ) != ( b->is2d() ) ) this->set2dFlag();
    //dont assert length > 0 for fake edges
    else alugrid_assert ( isRealLine() );
    return;
  }

  template < class A > inline Hedge1Top < A >::Hedge1Top (int l, myvertex_t * a, myvertex_t * b, int nChild )
    : A (a,b),
    _bbb (0),
    _inner (0),
    _rule (myrule_t::nosplit),
    _lvl (l),
    _child( nChild )
  {
    alugrid_assert ( _child == 0 || _child == 1 );
    this->setIndex( indexManager().getIndex() );
    if( ( a->is2d() ) != ( b->is2d() ) ) this->set2dFlag();
    //dont assert length > 0 for fake edges
    else alugrid_assert ( isRealLine() );
    return;
  }

  template < class A > Hedge1Top < A >::~Hedge1Top ()
  {
    this->freeIndex( indexManager() );
    if(_bbb) delete _bbb;
    if(_inner) delete _inner;
    return;
  }

  template < class A > inline bool Hedge1Top < A > ::isRealLine () const
  {
    double sum = 0;
    const alucoord_t (&p0)[ 3 ] = myvertex(0)->Point();
    const alucoord_t (&p1)[ 3 ] = myvertex(1)->Point();

    for(int j=0; j<3; ++j)
    {
      double diff = p0[j] - p1[j];
      sum += (diff * diff );
    }
    sum = std::sqrt( sum );
    //std::cout << sum << " sum " << std::endl;
    return  sum > 1e-8;
  }

  template < class A > inline Hedge1Top < A > * Hedge1Top < A >::dwnPtr () {
    return _inner ? _inner->dwn() : 0;
  }

  template < class A > inline const Hedge1Top < A > * Hedge1Top < A >::dwnPtr () const {
    return _inner ? _inner->dwn() : 0;
  }

  template < class A > inline typename Hedge1Top < A >::innervertex_t *
  Hedge1Top < A >::inVx () {
    return _inner ? _inner->cv() : 0;
  }

  template < class A > inline const typename Hedge1Top < A >::innervertex_t *
  Hedge1Top < A >::inVx () const {
    return _inner ? _inner->cv() : 0;
  }

  template < class A > inline int Hedge1Top < A >::level () const {
    return _lvl;
  }

  template < class A > inline int Hedge1Top < A >::nChild () const {
    alugrid_assert ( _child == 0 || _child == 1 );
    return _child;
  }

  template < class A > Hedge1Top < A > * Hedge1Top < A >::down () {
    return dwnPtr();
  }

  template < class A > const Hedge1Top < A > * Hedge1Top < A >::down () const {
    return dwnPtr();
  }

  template < class A > Hedge1Top < A > * Hedge1Top < A >::next () {
    return _bbb;
  }

  template < class A > const Hedge1Top < A > * Hedge1Top < A >::next () const {
    return _bbb;
  }

  template < class A > inline void Hedge1Top < A >::backup (std::ostream & os) const
  {
    doBackup( os );
  }

  template < class A > inline void Hedge1Top < A >::backup (ObjectStream& os) const
  {
    doBackup( os );
  }

  template < class A > template <class OutStream_t>
  inline void Hedge1Top < A >::doBackup (OutStream_t& os) const
  {
    os.put ((char) getrule ());
    {
      for (const inneredge_t * d = dwnPtr(); d; d = d->next ()) d->backup (os);
    }
    return;
  }

  template < class A > inline void Hedge1Top < A >::restore (std::istream & is)
  {
    doRestore( is );
  }
  template < class A > inline void Hedge1Top < A >::restore (ObjectStream& is)
  {
    doRestore( is );
  }

  template < class A > template <class InStream_t>
  inline void Hedge1Top < A >::doRestore (InStream_t & is)
  {
    char r = (char) is.get ();
    refineImmediate (myrule_t (r));
    {
      for (inneredge_t * d = dwnPtr(); d; d = d->next ()) d->restore (is);
    }
    return;
  }

  template < class A >  inline void Hedge1Top < A >::append (inneredge_t * e) {
    alugrid_assert (!_bbb && e);
    _bbb = e;
    return;
  }

  template < class A > typename Hedge1Top < A >::myrule_t
  Hedge1Top < A >::getrule () const {
    return myrule_t (_rule);
  }

  template < class A > Hedge1Top < A > * Hedge1Top < A >::subedge (int n) {
    alugrid_assert (n == 0 || n == 1);
    alugrid_assert (n ? this->dwnPtr()->next () : this->dwnPtr());
    return n ? this->dwnPtr()->next () : this->dwnPtr();
  }

  template < class A > const Hedge1Top < A > * Hedge1Top < A >::subedge (int n) const {
    alugrid_assert (n == 0 || n == 1);
    alugrid_assert (n ? this->dwnPtr()->next () : this->dwnPtr());
    return n ? this->dwnPtr()->next () : this->dwnPtr();
  }

  template < class A > inline typename Hedge1Top < A >::innervertex_t *
  Hedge1Top < A >::innerVertex () {
    return inVx();
  }

  template < class A > inline const typename Hedge1Top < A >::innervertex_t * Hedge1Top < A >::innerVertex () const {
    return inVx();
  }

  template < class A > inline typename Hedge1Top < A >::innervertex_t * Hedge1Top < A >::subvertex (int) {
    return inVx();
  }

  template < class A > inline const typename Hedge1Top < A >::innervertex_t * Hedge1Top < A >::subvertex (int) const {
    return inVx();
  }

  // #     #                                 #       #######
  // #     #  ######    ##     ####   ###### #    #     #      ####   #####
  // #     #  #        #  #   #    #  #      #    #     #     #    #  #    #
  // #######  #####   #    #  #       #####  #    #     #     #    #  #    #
  // #     #  #       ######  #       #      #######    #     #    #  #####
  // #     #  #       #    #  #    #  #           #     #     #    #  #
  // #     #  #       #    #   ####   ######      #     #      ####   #


  template < class A >
  inline typename Hface4Top < A >::innerface_t * Hface4Top < A >::down () {
    return dwnPtr();
  }

  template < class A >
  inline const typename Hface4Top < A >::innerface_t * Hface4Top < A >::down () const {
    return dwnPtr();
  }

  template < class A >
  inline typename Hface4Top < A >::innerface_t * Hface4Top < A >::next () {
    return _bbb;
  }

  template < class A >
  inline const typename Hface4Top < A >::innerface_t * Hface4Top < A >::next () const {
    return _bbb;
  }

  template < class A > inline int Hface4Top < A >::level () const {
    return _lvl;
  }

  template < class A > inline int Hface4Top < A >::nChild () const {
    alugrid_assert ( _nChild >= 0 && _nChild < 4 );
    return _nChild;
  }

  template < class A > typename Hface4Top < A >::myhedge_t *
  Hface4Top < A >::subedge (int i,int j) {
    alugrid_assert (j == 0 || j == 1);
    return this->myhedge (i)->subedge (j ? 1 - this->twist(i) : this->twist(i));
  }

  template < class A > const typename Hface4Top < A >::myhedge_t *
  Hface4Top < A >::subedge (int i,int j) const {
    alugrid_assert (j == 0 || j == 1);
    return this->myhedge (i)->subedge (j ? 1 - this->twist(i) : this->twist(i));
  }

  template < class A > inline typename Hface4Top < A >::innervertex_t *
  Hface4Top < A >::subvertex (int) {
    alugrid_assert (getrule() == myrule_t::iso4);
    return inVx();
  }

  template < class A > inline const typename Hface4Top < A >::innervertex_t *
  Hface4Top < A >::subvertex (int) const {
    alugrid_assert (getrule() == myrule_t::iso4);
    return inVx();
  }

  template < class A > typename Hface4Top < A >::inneredge_t *
  Hface4Top < A >::subedge (int n) {
    inneredge_t * e = inEd();
    for (int i = 0; i < n; i ++ ) e = e ? e->next () : 0;
    alugrid_assert (e);
    return e;
  }

  template < class A > const typename Hface4Top < A >::inneredge_t *
  Hface4Top < A >::subedge (int n) const {
    const inneredge_t * e = inEd();
    for (int i = 0; i < n; i ++ ) e = e ? e->next () : 0;
    alugrid_assert (e);
    return e;
  }

  template < class A > inline typename Hface4Top < A >::innerface_t *
  Hface4Top < A >::subface (int n) {
    innerface_t * f = dwnPtr();
    for (int i = 0; i < n; i++ ) f = f ? f->next () : 0;
    alugrid_assert (f);
    return f;
  }

  template < class A > inline const typename Hface4Top < A >::innerface_t *
  Hface4Top < A >::subface (int n) const {
    const innerface_t * f = dwnPtr();
    for (int i = 0; i < n; i++ ) f = f ? f->next () : 0;
    alugrid_assert (f);
    return f;
  }

  template < class A > inline Hface4Top < A >::
  Hface4Top (int l, myhedge_t * e0, int t0, myhedge_t * e1, int t1,
    myhedge_t * e2, int t2, myhedge_t * e3, int t3 )
    : A (e0, t0, e1, t1, e2, t2, e3, t3),
    _bbb (0), _inner (0),
    _rule (myrule_t::nosplit),
    _lvl (l),
    _nChild(0)
  {
    this->setIndexAnd2dFlag( indexManager() );
    if( !( e0->is2d() ) ) this->reset2dFlag();
  }

  template < class A > inline Hface4Top < A >::Hface4Top (int l, myhedge_t * e0, int t0, myhedge_t * e1, int t1,
    myhedge_t * e2, int t2, myhedge_t * e3, int t3,int nChild )
    : A (e0, t0, e1, t1, e2, t2, e3, t3),
    _bbb (0), _inner (0),
    _rule (myrule_t::nosplit),
    _lvl (l),
    _nChild(nChild)
  {
    this->setIndexAnd2dFlag( indexManager() );
    if( !( e0->is2d() ) ) this->reset2dFlag();
  }

  template < class A > Hface4Top < A >::~Hface4Top ()
  {
    this->freeIndex( indexManager() );
    if (_bbb) delete _bbb;
    if (_inner) delete _inner;
    return;
  }

  template < class A >
  inline typename Hface4Top < A >::innerface_t * Hface4Top < A >::dwnPtr () {
    return (_inner) ? _inner->dwn() : 0;
  }

  template < class A >
  inline const typename Hface4Top < A >::innerface_t * Hface4Top < A >::dwnPtr () const {
    return (_inner) ? _inner->dwn() : 0;
  }

  template < class A > inline typename Hface4Top < A >::innervertex_t *
  Hface4Top < A >::inVx () {
    return (_inner) ? _inner->cv() : 0;
  }

  template < class A > inline const typename Hface4Top < A >::innervertex_t *
  Hface4Top < A >::inVx () const {
    return (_inner) ? _inner->cv() : 0;
  }

  template < class A > inline typename Hface4Top < A >::inneredge_t *
  Hface4Top < A >::inEd () {
    return (_inner) ? _inner->ed() : 0;
  }

  template < class A > inline const typename Hface4Top < A >::inneredge_t *
  Hface4Top < A >::inEd () const {
    return (_inner) ? _inner->ed() : 0;
  }

  template < class A > inline typename Hface4Top < A >::innervertex_t *
  Hface4Top < A >::innerVertex () {
    return inVx();
  }

  template < class A > inline const typename Hface4Top < A >::innervertex_t *
  Hface4Top < A >::innerVertex () const {
    return inVx();
  }

  template < class A > inline typename Hface4Top < A >::inneredge_t *
  Hface4Top < A >::innerHedge () {
    return inEd();
  }

  template < class A > inline const typename Hface4Top < A >::inneredge_t *
  Hface4Top < A >::innerHedge () const {
    return inEd();
  }

  template < class A > inline void Hface4Top < A >::append (innerface_t * f) {
    alugrid_assert (_bbb == 0);
    _bbb = f;
    return;
  }

  template < class A > inline typename Hface4Top < A >::myrule_t
  Hface4Top < A >::getrule () const {
    return myrule_t (_rule);
  }

  template < class A > inline void Hface4Top < A >::backup (std::ostream & os) const
  {
    doBackup(os);
  }
  template < class A > inline void Hface4Top < A >::backup (ObjectStream& os) const
  {
    doBackup(os);
  }

  template < class A > template <class OutStream_t>
  inline void Hface4Top < A >::doBackup (OutStream_t& os) const
  {
    os.put ((char) getrule ());
    {for (const inneredge_t * e = inEd(); e; e = e->next ()) e->backup (os); }
    {for (const innerface_t * c = dwnPtr(); c; c = c->next ()) c->backup (os); }
    return;
  }

  template < class A > inline void Hface4Top < A >::restore (std::istream & is)
  {
    doRestore( is );
  }
  template < class A > inline void Hface4Top < A >::restore (ObjectStream& is)
  {
    doRestore( is );
  }

  template < class A > template <class InStream_t>
  inline void Hface4Top < A >::doRestore (InStream_t & is)
  {
    refineImmediate (myrule_t ((char) is.get ()));
    {for (inneredge_t * e = inEd(); e; e = e->next ()) e->restore (is); }
    {for (innerface_t * c = dwnPtr(); c; c = c->next ()) c->restore (is); }
    return;
  }

  // #     #                         #       #######
  // #     #  #####   #    #  #####  #    #     #      ####   #####
  // #     #  #    #  ##   #  #    # #    #     #     #    #  #    #
  // #######  #####   # #  #  #    # #    #     #     #    #  #    #
  // #     #  #    #  #  # #  #    # #######    #     #    #  #####
  // #     #  #    #  #   ##  #    #      #     #     #    #  #
  // #     #  #####   #    #  #####       #     #      ####   #


  template < class A > inline Hbnd4Top < A >::
  Hbnd4Top (int l, myhface4_t * f, int i,
            innerbndseg_t * up, Gitter::helement_STI * gh, int gFace ) :
    A (f, i), _bbb (0), _dwn (0), _up(up) ,
    _bt(_up->_bt)
  {
    // set level (declared in hbndseg4_GEO for memory reasons)
    _lvl = l ;

    // store ghost element
    typedef Gitter::ghostpair_STI ghostpair_STI;
    ghostpair_STI p ( gh, gFace );
    this->setGhost ( p );

    // get index from manager
    this->setIndex( indexManager().getIndex() );

    if( _up )
    {
      // store segment index
      _segmentId = _up->_segmentId; // get segment index from father
      // set boundary projection from father
      this->_pvPtr = _up->_pvPtr;
    }
    else
    {
      _segmentId = this->getIndex();
    }

    // store boundary id
    setBoundaryId( _bt );
    return;
  }

  template < class A > inline Hbnd4Top < A >::
  Hbnd4Top (int l, myhface4_t * f, int i, const bnd_t bt )
    : A (f, i),
      _bbb (0), _dwn (0), _up(0) ,
      _bt(bt)
  {
    // set level (declared in hbndseg4_GEO for memory reasons)
    _lvl = l ;

    // get index from manager
    this->setIndex( indexManager().getIndex() );

    // store segment by using index
    _segmentId = this->getIndex();

    // store boundary id
    setBoundaryId( _bt );
    return;
  }

  template < class A > Hbnd4Top < A >::~Hbnd4Top ()
  {
    // free index
    indexManager().freeIndex( this->getIndex() );
    // detach leaf entities
    if (this->isLeafEntity()) this->detachleafs();
    // delete down and next
    if (_bbb) delete _bbb;
    if (_dwn) delete _dwn;
    return;
  }

  template< class A > inline void Hbnd4Top < A >::
  setBoundaryId( const int id )
  {
    // set my id to the same as bnd
    this->setBndId( id );
    myhface4_t & face = *(myhface4(0));
    face.setBndId( id );
    // 4 fertices and edges
    for(int i=0; i<4; ++i)
    {
      face.myvertex(i)->setBndId( id );
      face.myhedge(i)->setBndId( id );
    }
  }

  template < class A > inline int Hbnd4Top < A >::segmentId () const {
    return _segmentId;
  }

  template < class A > inline int Hbnd4Top < A >::level () const {
    return _lvl;
  }

  template < class A > inline typename Hbnd4Top < A >::innerbndseg_t * Hbnd4Top < A >::next () {
    return _bbb;
  }

  template < class A > inline const typename Hbnd4Top < A >::innerbndseg_t * Hbnd4Top < A >::next () const {
    return _bbb;
  }

  template < class A > inline typename Hbnd4Top < A >::innerbndseg_t * Hbnd4Top < A >::down () {
    return _dwn;
  }

  template < class A > inline const typename Hbnd4Top < A >::innerbndseg_t * Hbnd4Top < A >::down () const {
    return _dwn;
  }

  template < class A > inline typename Hbnd4Top < A >::innerbndseg_t * Hbnd4Top < A >::up () {
    return _up;
  }

  template < class A > inline const typename Hbnd4Top < A >::innerbndseg_t * Hbnd4Top < A > ::up () const {
    return _up;
  }

  template < class A > inline void Hbnd4Top < A >::append (innerbndseg_t * b) {
    alugrid_assert (_bbb == 0);
    _bbb = b;
    return;
  }

  // #     #                         #######
  // #     #  ######  #    #    ##      #      ####   #####
  // #     #  #        #  #    #  #     #     #    #  #    #
  // #######  #####     ##    #    #    #     #    #  #    #
  // #     #  #         ##    ######    #     #    #  #####
  // #     #  #        #  #   #    #    #     #    #  #
  // #     #  ######  #    #  #    #    #      ####   #

  template < class A > inline typename HexaTop < A >::innerhexa_t * HexaTop < A >::dwnPtr() {
    return (_inner) ? _inner->dwn() : 0;
  }

  template < class A > inline const typename HexaTop < A >::innerhexa_t * HexaTop < A >::dwnPtr() const {
    return (_inner) ? _inner->dwn() : 0;
  }

  template < class A > inline typename HexaTop < A >::innervertex_t * HexaTop < A >::inVx () {
    return (_inner) ? _inner->cv() : 0;
  }

  template < class A > inline const typename HexaTop < A >::innervertex_t * HexaTop < A >::inVx () const {
    return (_inner) ? _inner->cv() : 0;
  }

  template < class A > inline typename HexaTop < A >::inneredge_t * HexaTop < A >::inEd() {
    return (_inner) ? _inner->ed() : 0;
  }

  template < class A > inline const typename HexaTop < A >::inneredge_t * HexaTop < A >::inEd() const {
    return (_inner) ? _inner->ed() : 0;
  }

  template < class A > inline typename HexaTop < A >::innerface_t * HexaTop < A >::inFce() {
    return (_inner) ? _inner->fce() : 0;
  }

  template < class A > inline const typename HexaTop < A >::innerface_t * HexaTop < A >::inFce() const {
    return (_inner) ? _inner->fce() : 0;
  }

  template < class A > inline typename HexaTop < A >::innerhexa_t * HexaTop < A >::up () {
    return _up;
  }

  template < class A > inline const typename HexaTop < A >::innerhexa_t * HexaTop < A >::up () const {
    return _up;
  }

  template < class A > inline typename HexaTop < A >::innerhexa_t * HexaTop < A >::down () {
    return dwnPtr();
  }

  template < class A > inline const typename HexaTop < A >::innerhexa_t * HexaTop < A >::down () const {
    return dwnPtr();
  }

  template < class A > inline typename HexaTop < A >::innerhexa_t * HexaTop < A >::next () {
    return _bbb;
  }

  template < class A > inline const typename HexaTop < A >::innerhexa_t * HexaTop < A >::next () const {
    return _bbb;
  }

  template < class A > inline typename HexaTop < A >::innervertex_t * HexaTop < A >::innerVertex () {
    return inVx();
  }

  template < class A > inline const typename HexaTop < A >::innervertex_t * HexaTop < A >::innerVertex () const {
    return inVx();
  }

  template < class A > inline typename HexaTop < A >::inneredge_t * HexaTop < A >::innerHedge () {
    return inEd();
  }

  template < class A > inline const typename HexaTop < A >::inneredge_t * HexaTop < A >::innerHedge () const {
    return inEd();
  }

  template < class A > inline typename HexaTop < A >::innerface_t * HexaTop < A >::innerHface () {
    return inFce();
  }

  template < class A > inline const typename HexaTop < A >::innerface_t * HexaTop < A >::innerHface () const {
    return inFce();
  }

  template < class A > inline void HexaTop < A >::append (HexaTop < A > * h) {
    alugrid_assert (_bbb == 0);
    _bbb = h;
    return;
  }

  template < class A > inline int HexaTop < A >::level () const {
    return _lvl;
  }

  template < class A > inline double HexaTop < A >::volume () const {
    return _volume;
  }

  template < class A > inline int HexaTop < A >::nChild () const {
    alugrid_assert ( _nChild >= 0 && _nChild < 8 );
    return _nChild;
  }

  template < class A > typename HexaTop < A >::myrule_t HexaTop < A >::getrule () const {
    return myrule_t (_rule);
  }

  template < class A > typename HexaTop < A >::myrule_t HexaTop < A >::requestrule () const {
    return myrule_t (_req);
  }

  template < class A > void HexaTop < A >::request (myrule_t r) {
    alugrid_assert (r.isValid ());
    _req = r;
    return;
  }

  // Anfang - Neu am 23.5.02 (BS)

  // ######                                                          #       #######
  // #     #  ######  #####      #     ####   #####      #     ####  #    #     #
  // #     #  #       #    #     #    #    #  #    #     #    #    # #    #     #
  // ######   #####   #    #     #    #    #  #    #     #    #      #    #     #
  // #        #       #####      #    #    #  #    #     #    #      #######    #
  // #        #       #   #      #    #    #  #    #     #    #    #      #     #
  // #        ######  #    #     #     ####   #####      #     ####       #     #

  template < class A > inline int Periodic4Top < A >::level () const {
    return _lvl;
  }

  template < class A > inline int Periodic4Top < A >::nChild () const {
    alugrid_assert ( _nChild >= 0 && _nChild < 4 );
    return _nChild;
  }

  template < class A > inline typename Periodic4Top < A >::innerperiodic4_t * Periodic4Top < A >::up () {
    return _up;
  }

  template < class A > inline const typename Periodic4Top < A >::innerperiodic4_t * Periodic4Top < A >::up () const {
     return _up;
  }

  template < class A > inline typename Periodic4Top < A >::innerperiodic4_t * Periodic4Top < A >::down () {
    return _dwn;
  }

  template < class A > inline const typename Periodic4Top < A >::innerperiodic4_t * Periodic4Top < A >::down () const {
    return _dwn;
  }

  template < class A > inline typename Periodic4Top < A >::innerperiodic4_t * Periodic4Top < A >::next () {
    return _bbb;
  }

  template < class A > inline const typename Periodic4Top < A >::innerperiodic4_t * Periodic4Top < A >::next () const {
    return _bbb;
  }

  template < class A > inline typename Periodic4Top < A >::innervertex_t * Periodic4Top < A >::innerVertex () {
    return 0;
  }

  template < class A > inline const typename Periodic4Top < A >::innervertex_t * Periodic4Top < A >::innerVertex () const {
    return 0;
  }

  template < class A > inline typename Periodic4Top < A >::inneredge_t * Periodic4Top < A >::innerHedge () {
    return 0;
  }

  template < class A > inline const typename Periodic4Top < A >::inneredge_t * Periodic4Top < A >::innerHedge () const {
    return 0;
  }

  template < class A > inline typename Periodic4Top < A >::innerface_t * Periodic4Top < A >::innerHface () {
    return 0;
  }

  template < class A > inline const typename Periodic4Top < A >::innerface_t * Periodic4Top < A >::innerHface () const {
    return 0;
  }

  template < class A > inline void Periodic4Top < A >::append (Periodic4Top < A > * h) {
    alugrid_assert (_bbb == 0);
    _bbb = h;
    return;
  }

  template < class A > typename Periodic4Top < A >::myrule_t Periodic4Top < A >::getrule () const {
    return myrule_t (_rule);
  }

  template < class A > void Periodic4Top < A >::request (myrule_t) {

    // Einen Request zur Verfeinerung zu setzen, ist vorl"aufig inhaltlich nicht
    // vorgesehen und wird deshalb ignoriert (leise).

    return;
  }

  template < class A > bool Periodic4Top < A >::refine () {

    // Das refine () reagiert nicht auf die Elementaktivierung zur Verfeinerung
    // in der globalen Schleife, weil das perioodische Randelement sich nur auf
    // Anforderung zur Balancierung aus einem anliegenden Element direkt verfeinert.

    return true;
  }

  template< class A >
  inline int Periodic4Top < A >::segmentId ( const int fce ) const
  {
    alugrid_assert ( fce == 0 || fce == 1 );
    return _segmentId[ fce ];
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_HEXA_TOP_H_INCLUDED
