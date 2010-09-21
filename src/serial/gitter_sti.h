// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2007 
#ifndef GITTER_STI_H_INCLUDED
#define GITTER_STI_H_INCLUDED

// typdef these stream because this code uses a lot strstream 
typedef stringbuf     strstreambuf_t ; 
typedef stringstream  strstream_t;

#include "myalloc.h"
#include "parallel.h"
#include "xdrclass.h"

// interface class for projecting vertices for boundary adjustment 
typedef VertexProjection<3, alucoord_t > ProjectVertex;
// see ../projectvertex.h 

// pair of projection and bnd segment index 
typedef pair<const ProjectVertex* , const int > ProjectVertexPair;

// forward declaration, see ghost_info.h 
class MacroGhostInfoHexa;
class MacroGhostInfoTetra;

// forward declaration 
class Gitter;

class IndexManagerStorage
{
public:  
  // number of different index manager that exists 
  enum { numOfIndexManager = 6 };

  enum { IM_Elements = 0, // 0 == elements 
         IM_Faces = 1,    // 1 == faces 
         IM_Edges = 2,    // 2 == edges 
         IM_Vertices = 3, // 3 == vertices
         IM_Bnd = 4,      // 4 == boundary elements 
         IM_Internal = 5  // 5 == internal bnds, parallel only 
  };

  IndexManagerStorage() : _myGrid( 0 ) 
  {}
  void setGrid( Gitter * grid ) { _myGrid = grid ; }

  explicit IndexManagerStorage(Gitter * gitter) : _myGrid( gitter ) 
  {}

  Gitter* myGrid() 
  { 
    assert( _myGrid );
    return _myGrid; 
  }
  const Gitter* myGrid() const 
  { 
    assert( _myGrid );
    return _myGrid; 
  }
  IndexManagerType& get(const int codim) 
  {
    assert( codim >= 0 );
    assert( codim < numOfIndexManager );
    return _indexmanager[ codim ];
  }
  int getIndex( const int codim ) { return get( codim ).getIndex(); }

  void compress() 
  {
    for(int i=0; i<numOfIndexManager; ++i)
    {
      _indexmanager[i].compress();
    }
  }
private:
  IndexManagerStorage( const IndexManagerStorage& );

protected:
  Gitter* _myGrid;

  // this variable is located here, because all the elements in
  // this lists use this objects to get  thier numbers 
  // index provider, for every codim one , 4 is for boundary
  IndexManagerType _indexmanager[ numOfIndexManager ];
};

typedef IndexManagerStorage IndexManagerStorageType;

// Einfacher Referenzenz"ahler mit cast-around-const
// feature, der zum Z"ahlen der Referenzen auf Fl"achen
// Kanten und Knoten verwendet wird. Vorteil: Objekte,
// die einen Z"ahler dieser Klasse enthalten, werden
// durch Inkrementierung bzw. Dekrementierung des Z"ahlers
// nicht ver"andert (k"onnen also auch 'const' sein).

class Refcount {
  
#ifndef NDEBUG
#ifdef DEBUG_ALUGRID
  // Der Globale Z"ahler soll helfen, nicht gel"oschte
  // Gitterobjekte oder Iteratorobjekte zu erkennen.
  // (Wird aber nur in den DEBUG-Versionen angelegt.) 
  //
  // Refcounting only turned on, if NDEBUG is not defined and
  // DEBUG_ALUGRID is defined 

  class Globalcount {
    int _c ;
  public :
    inline Globalcount() ;
    ~Globalcount() ;
    inline void operator ++ (int) const ;
    inline void operator -- (int) const ;
  } ;
  static Globalcount _g ;
#endif 
  // end DEBUG_ALUGRID
#endif 
  // end NDEBUG

  mutable unsigned char _c;
public :
  inline void reset () { _c = 0 ; }
  inline bool positive () const { return _c > 0 ; }
  inline Refcount () ;
  inline ~Refcount () ;
  inline int operator ++ (int) const ;
  inline int operator ++ () const ;
  inline int operator -- (int) const ;
  inline int operator -- () const ;
  inline bool operator ! () const ;
  inline operator int () const ;
} ;

////////////////////////////////////////////////////////////////////
//
// Schnittstelle des Iterationsobjekts vgl. Gamma, Helm, Johnson &
// Vlissides: Design Patterns; Addison Wesley 
// Die Schnittstellenbeschreibung wird sowohl polymorph als auch
// in den verschiedenen Schablonen f"ur einfache Iterationsobjekte
// s.a. Datei 'walk.h' verwendet.
//
////////////////////////////////////////////////////////////////////
template < class A > class IteratorSTI 
{
protected:  
  IteratorSTI () {}
public :
  typedef A val_t ;
  virtual ~IteratorSTI () {}
  virtual void first () = 0 ;
  virtual void next () = 0 ;
  virtual int done () const = 0 ;
  virtual int size () = 0 ;
  virtual val_t & item () const = 0 ;
  virtual IteratorSTI < A > * clone () const = 0;
} ;


// EmptyIterator is an iterator of an empty set  
// for some default values 
template < class A > class EmptyIterator : public IteratorSTI < A > {
  EmptyIterator (const EmptyIterator < A > &) {}
public :
  typedef A val_t ;
  EmptyIterator () {}
  virtual ~EmptyIterator () {}
  virtual void first () ;
  virtual void next () ;
  virtual int done () const ;
  virtual int size () ;
  virtual val_t & item () const ;
  virtual IteratorSTI < A > * clone () const ;
} ;

// AccessIterator < . > ist eine Schnittstellenschablone, die
// ausgepr"agt mit vertex, hedge, hface, helement und hbndseg
// die Schnittstellen des Grobgittercontainers zur Erstellung
// und Kopie von Iterationsobjekten erzeugt.

template < class A > class any_has_level;

template < class A > class AccessIterator {
public :
  Refcount ref ; 

  // creates leaf iterators 
  virtual IteratorSTI < A > * iterator (const A *) const = 0 ;

  // creates level iterators 
  virtual IteratorSTI < A > * levelIterator (const A * a, const any_has_level < A  > & ) const {  return iterator(a); } 

  // this methods are needed because for the PureElementAccessIterator we
  // want to call overloaded method that only insert lists with elements 
  // but for edges,faces,vertices this method is the same, therefor default
  // implementation 
  virtual IteratorSTI < A > * pureElementIterator (const A * a) const { return iterator(a); }

public :
  
  // Handle ist ein einfaches Iteratorproxy, das ein abstraktes
  // Iterationsobjekt verwendet, um nach aussen die entsprechenden
  // Schnittstellenmethoden zu implementieren. Dabei ist dem Proxy
  // nur bekannt, wie ein Iterator von der entsprechenden Schnittstelle
  // zu bekommen ist, bzw. wie ein bestehender mit einer Schnittstellen-
  // methode kopiert werden kann.
  
  class Handle : public IteratorSTI < A > {
  protected:
    AccessIterator < A > * _fac ;
    A * _a ;
    IteratorSTI < A > * _w ;
    // typedef handle 
    typedef typename AccessIterator < A > :: Handle ThisType;
  public :
    inline Handle (AccessIterator < A > &) ;
    inline Handle (const ThisType &) ;
    inline Handle () ;
    inline ~Handle () ;
    inline const Handle & operator = (const ThisType &) ;
    inline bool operator == (const ThisType &) const ;
    inline bool operator < (const ThisType &) const ;
    inline void first () ;
    inline void next () ;
    inline int done () const ;
    inline int size () ;
    inline A & item () const ;
    inline virtual IteratorSTI< A > * clone () const;
  protected: 
    void removeObj(); 
    void assign (const ThisType &);
  } ;
protected :
  AccessIterator () {}
  virtual ~AccessIterator () 
  { 
#ifndef NDEBUG 
    if(ref) 
      cerr << "WARNING: (IGNORED) There still exist iterators while corresponding grid is deleted! in: " << __FILE__ << " line: " << __LINE__ << endl; 
#endif
  }
} ;

// the Leaf Iterators 
template < class A > class LeafIterator ;

// the Level Iterators 
template < class A > class LevelIterator ;


// --Gitter 
class Gitter {
public :

  class helement ;
  class hbndseg ;
///////////////////////////////////////////////////
//  parallel element interfaces 
///////////////////////////////////////////////////
#include "elementif.h"

///////////////////////////////////////////////////

  Refcount ref ;
  static inline bool debugOption (int = 0) ;

  typedef Parallel stiExtender_t ;  // parallel.h
   
  // Nachfolgend sind die Iterationsschnittstellen der Knoten,
  // Kanten, Fl"achen, Elemente und Randelemente definiert.
  class DuneIndexProvider 
  {
  public:
    enum { interior = 0 , border = 111 , ghost = 222 };

    enum Flag
    {
      // set if index is copy from outside and should not be freed
      flagCopy = 0,
      // set if object is locked against coarsening
      flagLock = 1
    };

  protected:
    // internal index of item 
    int _idx;

    // boundary id, zero for internal items, 
    // otherwise > 0 (but always positive )
    // negative id are for internal usage only 
    typedef unsigned char bndid_t ;
    bndid_t _bndid;
    
    // reference counter of leaf elements holding pointer to this item 
    typedef unsigned char leafref_t;
    leafref_t _leafref;

    unsigned char _flags;

    // store refcount here to fill up the 8 byte of mem 
    Refcount ref ;

    // constructor 
    DuneIndexProvider () : 
      _idx(-1), 
      _bndid(interior),
      _leafref(0),
      _flags( 0 )
    {}

  public:
    void set ( const Flag &flag )
    {
      _flags |= (1 << flag);
    }

    void unset ( const Flag &flag )
    {
      _flags &= ~(1 << flag);
    }

    bool isSet ( const Flag &flag ) const
    {
      return (_flags & (1 << flag));
    }

    // backupIndexErr message 
    void backupIndexErr () const {
      cerr << "DuneIndexProvider :: backupIndex : Implemenation should be in inherited class " << __FILE__  << " " << __LINE__ << "\n";
      abort();
    }
    // restoreIndexErr message 
    void restoreIndexErr () const {
      cerr << "DuneIndexProvider :: restoreIndex : Implemenation should be in inherited class " << __FILE__  << __LINE__ << "\n";
      abort();
    }

    // return index of item 
    inline int getIndex () const 
    { 
      assert( _idx >= 0);
      return _idx; 
    }
    // set index of item 
    inline void setIndex ( const int index ) 
    { 
      assert( index >= 0 );
      _idx = index; 
    }
    
    inline void freeIndex ( IndexManagerType & im ) 
    {
      if( !isSet( flagCopy ) )
      {
        assert( _idx >= 0 );
        im.freeIndex(_idx); 
      }
    }

    //for the ghost helements, set index from outside 
    inline void setIndex ( IndexManagerType & im, const int index )
    {
      // free old index 
      freeIndex(im);
      // set given index 
      setIndex(index); 
      // now it's a copy 
      set( flagCopy );
    }

    //for the ghost helements, set index from outside 
    inline void resetGhostIndex( IndexManagerType & im )
    {
      // if already copy then do nothing
      if( !isSet( flagCopy ) && isGhost() )
      {
        // only call this method on ghosts 
        // set new index 
        setIndex( im.getIndex() );
      }
    }

    //for defining leaf entities in dune notation:] 
    inline void addleaf() 
    {
      // check upper limit of unsigned char 
      ++_leafref;
    }
    
    // decrease reference counter by one  
    inline void removeleaf() 
    {
      --_leafref;
    }

    // returns true, if item is leaf item 
    inline bool isLeafEntity() const {
      return ( _leafref > 0 );
    }
    // return actual leaf ref counter 
    inline leafref_t leafRefCount() const {
      return _leafref;
    }

    // return bnd id 
    inline bndid_t bndId() const { return _bndid; }

    // set bnd id, id is only set if id is larger then actual id
    inline void setBndId (const bndid_t id) 
    { 
      if( id > _bndid ) _bndid = id; 
    }
    
    // set bnd id, id is overwritten in any case 
    inline void setGhostBndId (const bndid_t id) 
    { 
      _bndid = id; 
    }

    // returns trus, if item is interior item (not ghost or border)
    bool isInterior () const 
    { 
      // interior is also external boundary which is not 0
      return ((bndId() != ghost) && (bndId() != border)); 
    }
   
    // returns true if item is ghost item 
    bool isGhost () const 
    {
      return (bndId() == ghost);
    }
    
    // returns trus, if item is border item 
    bool isBorder () const 
    {
      return (bndId() == border);
    }
  };
    
public :
  class vertex : public stiExtender_t :: VertexIF  
               , public DuneIndexProvider 
  {
  protected :
    vertex () {}
    virtual ~vertex () {}
  public :
    virtual int ident () const = 0 ;
    virtual int level () const = 0 ;

    // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
    virtual void project(const ProjectVertexPair &pv) = 0; 
  
    // Extrainteger, damit die Element zu Vertex Zuordnug klappt,
    // wenn die Daten zur Visualisierung mit GRAPE rausgeschrieben
    // werden sollen:
  
    // backup and restore index of vertices, should be overloaded in
    // derived classes, because some need to go down the hierarchiy
    virtual void backupIndex  (ostream & os ) const { backupIndexErr(); }
    virtual void restoreIndex (istream & is , vector<bool>(&)[4] ) { restoreIndexErr(); }

  } ;
   
  class hedge : public EdgePllXDefault , 
                public DuneIndexProvider 
  {
  protected :
    hedge () {}
    virtual ~hedge () {}
  public :
    virtual hedge * down () = 0 ;
    virtual const hedge * down () const = 0 ;
    virtual hedge  * next () = 0 ;
    virtual const hedge  * next () const = 0 ;
    virtual vertex * innerVertex () = 0 ;
    virtual const vertex * innerVertex () const = 0 ;
    virtual int level () const = 0 ;
    virtual int nChild () const = 0; 
    inline  int leaf () const ;
  public :
    virtual bool coarse () = 0 ;
    virtual void backup (ostream &) const = 0 ;
    virtual void restore (istream &) = 0 ;

    virtual void backup (ObjectStream &) const = 0 ;
    virtual void restore (ObjectStream &) = 0 ;

    // new xdr methods 
    // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
    virtual void projectInnerVertex(const ProjectVertexPair &pv) = 0; 

    // backup and restore index of vertices, should be overloaded in
    // derived classes, because some need to go down the hierarchiy
    virtual void backupIndex  (ostream & os ) const { backupIndexErr(); }
    virtual void restoreIndex (istream & is , vector<bool>(&)[4] ) { restoreIndexErr(); }
  } ;
    
  class hface : public FacePllXDefault, 
                public DuneIndexProvider 
  {
  protected :
    hface () {}
    virtual ~hface () {}
  public :
    virtual hface * down () = 0 ;
    virtual const hface * down () const = 0 ;
    virtual hface * next () = 0 ;
    virtual const hface * next () const = 0 ;
    virtual vertex * innerVertex () = 0 ;
    virtual const vertex * innerVertex () const = 0 ;
    virtual hedge  * innerHedge () = 0 ;
    virtual const hedge  * innerHedge () const = 0 ;
    virtual int level () const = 0 ;
    virtual int nChild () const = 0; 
    inline int leaf () const ;
  public :
    virtual bool coarse () = 0 ;
    virtual void backup (ostream &) const = 0 ;
    virtual void restore (istream &) = 0 ;
        
    virtual void backup (ObjectStream &) const = 0 ;
    virtual void restore (ObjectStream &) = 0 ;
        
    // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
    virtual void projectVertex(const ProjectVertexPair &pv) = 0;

    // returns true if element conected to face is leaf 
    virtual bool isInteriorLeaf() const = 0;

    // backup and restore index of vertices, should be overloaded in
    // derived classes, because some need to go down the hierarchiy
    virtual void backupIndex  (ostream & os ) const { backupIndexErr(); }
    virtual void restoreIndex (istream & is , vector<bool>(&)[4] ) { restoreIndexErr(); }

  };

  // class with all extensions for helement 
  class Dune_helement : public DuneIndexProvider 
  {
  protected: 
    // abuse ref to refined tag 
    using DuneIndexProvider :: ref ;
    Dune_helement () 
    {
      // mark as new element by increasing reference counter 
      ++ ref ;
    }
  public:
    // reset the _refinedTag to false 
    void resetRefinedTag(); 
    // true if element was refined this adaptation step 
    bool hasBeenRefined () const;
  };


  struct AdaptRestrictProlong
  {
    virtual ~AdaptRestrictProlong () {}
    virtual int preCoarsening (helement & elem )   = 0;
    virtual int postRefinement (helement  & elem ) = 0;
    virtual int preCoarsening (hbndseg & bnd )     = 0;
    virtual int postRefinement (hbndseg & bnd )    = 0;
  };
  typedef AdaptRestrictProlong AdaptRestrictProlongType;

  // --helement 
  class helement : public ElementPllXIF
                 , public Dune_helement 
  {
  protected :
    helement () {}
    virtual ~helement () {}
  public :
    //testweise us
    virtual helement * up () = 0;
    virtual const helement * up () const = 0;
    virtual void os2VertexData(ObjectStream &, GatherScatterType &, int) { assert(false); abort();}
    virtual void os2EdgeData  (ObjectStream &, GatherScatterType &, int) { assert(false); abort();} 
    virtual void os2FaceData  (ObjectStream &, GatherScatterType &, int) { assert(false); abort();} 

    virtual void VertexData2os(ObjectStream &, GatherScatterType &, int) { assert(false); abort();}
    virtual void EdgeData2os(ObjectStream &, GatherScatterType &, int) { assert(false); abort(); }
    virtual void FaceData2os(ObjectStream &, GatherScatterType &, int) { assert(false); abort(); }
    //us
    virtual helement * down () = 0 ;
    virtual const helement * down () const = 0 ;
    virtual helement * next () = 0 ;
    virtual const helement * next () const = 0 ;
    virtual vertex * innerVertex () = 0 ;
    virtual const vertex * innerVertex () const = 0 ;
    virtual hedge * innerHedge () = 0 ;
    virtual const hedge * innerHedge () const = 0 ;
    virtual hface * innerHface () = 0 ;
    virtual const hface * innerHface () const = 0 ;
    // return level of element, implemented in Top classes 
    virtual int level () const = 0; 
    // return number of child 
    virtual int nChild () const = 0 ;

    virtual int nFaces() const = 0;
    virtual int nEdges() const = 0;
    
    // return index of boundary segment 
    virtual int segmentIndex (const int) const { return -1; }
        
    // mark element for using iso8 rule 
    virtual int tagForGlobalRefinement () = 0 ;
    // mark element for coarsening 
    virtual int tagForGlobalCoarsening () = 0 ;
    // set marker of element to nosplit 
    virtual int resetRefinementRequest () = 0 ;
    virtual int tagForBallRefinement (const alucoord_t (&)[3],double,int) = 0 ;
    virtual int test () const = 0 ;
    inline  int leaf () const ;

    virtual double volume () const { assert(false); abort(); return 0.0; } //= 0;
    virtual void setIndicesAndBndId (const hface & , int ) { assert(false); abort(); }
    virtual void resetGhostIndices() = 0;
  public :
    virtual bool refine () = 0 ;
    virtual bool coarse () = 0 ;
    virtual void backupCMode (ostream &) const = 0 ;
    virtual void backup (ostream &) const = 0 ;
    virtual void restore (istream &) = 0 ;
    
    virtual void backup (ObjectStream &) const = 0 ;
    virtual void restore (ObjectStream &) = 0 ;

    // backup and restore index of vertices, should be overloaded in
    // derived classes, because some need to go down the hierarchiy
    virtual void backupIndex  (ostream & os ) const { backupIndexErr(); }
    virtual void restoreIndex (istream & is , vector<bool>(&)[4] ) { restoreIndexErr(); }

  public: 
    virtual grid_t type() const = 0;
  } ;

  // this little helper class stored information for the splitGhost 
  // method but is only needed for the parallel case 
  class GhostChildrenInfo 
  {
    helement *(_ghchl)[4]; 
    int _gFace[4]; 
    public: 
      GhostChildrenInfo()  
      {
        _ghchl[0] = _ghchl[1] = _ghchl[2] = _ghchl[3] = 0; 
        _gFace[0] = _gFace[1] = _gFace[2] = _gFace[3] = -1;
      }

      helement * child(int i) const
      {
        assert( i >= 0 && i < 4 );
        return _ghchl[i];
      }

      int face(int i) const
      {
        assert( i >= 0 && i < 4 );
        return _gFace[i];
      }

      void setGhostPair(const pair < helement * , int > & g, int i)
      {
        assert( i >= 0 && i < 4 );
        assert( g.first );
        _ghchl[i] = g.first;
        assert( g.second >=0 );
        _gFace[i] = g.second;
      }
  };

  // --hbndseg 
  class hbndseg  : public DuneIndexProvider
  {
  protected :
    hbndseg () {}
    virtual ~hbndseg () {}
  public :
    typedef enum { 
      none = DuneIndexProvider :: interior, // also the value of interior items 
      inflow = 1, 
      outflow = 2, 
      noslip = 3, 
      slip = 4, 
      sym_xy = 5,
      sym_xz = 6, 
      sym_yz = 7, 
      reflect = 8, 
      fluxtube3d = 9, 
      closure = DuneIndexProvider :: border,  // also the value of border items 
      ghost_closure = DuneIndexProvider :: ghost , // also the value of ghost items 
      periodic = 254, // periodic boundaries 
      undefined = 255 } bnd_t ;

    // returns true if bnd id is in range 
    static bool bndRangeCheck (const int pbt) 
    {
      const size_t bt = std::abs( pbt );
      return ( bt >= 0 && bt < 254 );
    }

    // returns ranges of boundary type as string 
    static std::string validRanges() 
    {
      return "+-[0,...,253]";
    }
    
    virtual bnd_t bndtype () const = 0 ;

    // return index of boundary segment 
    virtual int   segmentIndex () const = 0 ;
        
    // for dune 
    virtual hbndseg * up () = 0 ;
    virtual const hbndseg * up () const = 0 ;
  
    virtual hbndseg * down () = 0 ;
    virtual const hbndseg * down () const = 0 ;
    virtual hbndseg * next () = 0 ;
    virtual const hbndseg * next () const = 0 ;
    virtual int level () const = 0 ;
    virtual int nChild () const = 0 ;
    inline  int leaf () const ;

    // for dune 
    virtual int ghostLevel () const = 0 ;
    virtual bool ghostLeaf () const = 0 ;
    
    // getGhost returns pointer to ghost, which might be 0 in case that
    // bndseg is external bnd seg, 
    // the int is -1 by default, or the internal ghostFace number (
    // getGhostFaceNumber) when ghost is non-zero 
    virtual const pair < helement * ,int> & getGhost () const = 0 ;

  protected:
    // if ghost element exists, then ghost is splitted, when bnd is splitted 
    // info will be filled with the new ghost cells and local face to the
    // internal boundary, default just does nothing 
    // implementation see gitter_{tetra,hexa}_top_pll.h 
    virtual inline void splitGhost ( GhostChildrenInfo & info ) {}

    // if ghost element exists, then ghost is coarsened, when bnd is coarsened  
    virtual inline void coarseGhost () {}
    virtual inline void setGhost ( const pair< helement * , int > & ) {}
  public :
    virtual void restoreFollowFace () = 0 ;
    virtual void attachleafs() { abort(); }
    virtual void detachleafs() { abort(); }
  } ;
public :
  typedef hbndseg hbndseg_STI ;
  typedef helement  helement_STI ;
  typedef hface hface_STI ;
  typedef hedge hedge_STI ;
  typedef vertex  vertex_STI ;
  typedef pair < helement_STI * , int > ghostpair_STI;

  // Die Klassen Internal-*- sind nur daf"ur da, aus einem Element, einer
  // Fl"ache oder einer Kante die inneren geometrischen Objekte, die von
  // dort verwaltet werden 'ans Licht zu f"ordern'. Sie werden ben"otigt,
  // um z.B. von einem Iterator mit Element-Item auf einen Teilbaumiterator
  // der inneren Fl"achen "uberzugehen.

  class InternalVertex {
  public :
    typedef vertex_STI val_t ;
    val_t & operator () (helement_STI & e) const { return * e.innerVertex () ; }
    val_t & operator () (hface_STI & f) const { return * f.innerVertex () ; }
    val_t & operator () (hedge_STI & d) const { return * d.innerVertex () ; }
    val_t & operator () (vertex_STI & v) const { return v ; }
  } ;
  class InternalEdge {
  public :
    typedef hedge_STI val_t ;
    val_t & operator () (helement_STI & e) const { return * e.innerHedge () ; }
    val_t & operator () (hface_STI & f) const { return * f.innerHedge () ; }
    val_t & operator () (hedge_STI & e) const { return e ; }
  } ;
  class InternalFace {
  public :
    typedef hface_STI val_t ;
    val_t & operator () (helement_STI & e) const { return * e.innerHface () ; }
    val_t & operator () (hface_STI & f) const { return f ; }
  } ;
  class InternalBndseg {
  public :
    typedef hbndseg_STI val_t ;
    val_t & operator () (hbndseg_STI & b) const { return b ; }
  } ;
  class InternalElement {
  public :
    typedef helement_STI val_t ;
    val_t & operator () (helement_STI & e) const { return e ; }
  } ;

public :
  
  // Die allgemeinste Schnittstelle zum Grobgittercontainer enth"alt Methoden zum
  // Anfordern und Kopieren von Iterationsobjekten, die aber nur im Sinne ihres
  // abstrakten Interfaces bekannt sind.
  // Das Makrogitterinterface beerbt erst mal die verschiedenen Auspr"agungen f"ur
  // Schnittstellen zu Anforderung von Iterationsobjekten. Alles ganz abstrakt bis hier.
  
  class Makrogitter : public AccessIterator < vertex_STI >, public AccessIterator < hedge_STI >,
                      public AccessIterator < hface_STI >, public AccessIterator < hbndseg_STI >, 
                      public AccessIterator < helement_STI > {
  protected :
    Makrogitter () {}
    virtual ~Makrogitter () ;
  public :
    virtual int iterators_attached () const ;
    virtual void backup (ostream &) const = 0 ;
    virtual void backup (const char*,const char *) const = 0 ;
    virtual void backupCMode (ostream &) const = 0 ;
    virtual void backupCMode (const char *,const char *) const = 0 ;
  
    // return size of used memory of macro gitter 
    // (size of lists storing the pointers )
    virtual size_t memUsage () = 0;
  } ;
public :
  class Geometric {
    
    // Innerhalb des Namensraums Geometric sind zuerst die Klassen mit den
    // Verfeinerungsregeln und dann die Geometriesockelklassen VertexGeo,
    // Hedge1, Hface3, Hface4, Tetra, Hexa, Hbndseg3/4
    // sowie die Polygonverbinder (Schnittstellen) hasFace3/4 definiert.
    
  public :
    class VertexGeo ;
    class hedge1 ;
    class hface4 ;
    class hface3 ;
    class hasFace3 ;
    class hasFace4 ;
    class Tetra ;
    class Hexa ;
    class hbndseg3 ;
    class hbndseg4 ;
  public :
      
    // Die Verfeinerungsregeln sind nur enumerierte Typen, mit Zuweisung
    // Vergleich und Typkonversion, im Falle der Regeln f"ur die Dreiecks-
    // bzw Vierecksfl"ache sind aber auch Methoden n"otig, die eine Regel
    // mit dem Twist der Fl"ache mitdrehen, damit der "Ubergang der
    // Verfeinerung stimmt.
      
    struct Hedge1Rule
    {
      enum rule_enum { nosplit=1, iso2 };
      typedef signed char rule_t;

      explicit Hedge1Rule ( const rule_t & );
      Hedge1Rule ( const rule_enum & = nosplit );
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &);
      inline Hedge1Rule rotate (int) const ;
      
    private:
      rule_t _r ;
    } ;
      
    struct Hface3Rule
    {
      enum rule_enum { nosplit=1, e01, e12, e20, iso4, undefined=-2 };
      typedef signed char rule_t;

      explicit Hface3Rule ( const rule_t & );
      Hface3Rule ( const rule_enum & = nosplit );
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;
      inline Hface3Rule rotate (int) const ;

    private :
      rule_t _r ;
    } ;
      
    struct Hface4Rule
    {
      enum rule_enum { nosplit=1, iso4,  undefined=-2
                       // ni02, ni13
                     };
      typedef signed char rule_t;

      explicit Hface4Rule ( const rule_t & );
      Hface4Rule ( const rule_enum & = nosplit) ;
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;
      inline Hface4Rule rotate (int) const ;

    private :
      rule_t _r ;
    } ;
      
    struct TetraRule
    {
      enum rule_enum { crs=-1, nosplit=1, e01, e12, e20, e23, e30, e31, iso8 };
      typedef signed char rule_t;

      explicit TetraRule ( const rule_t & );
      TetraRule ( const int );
      TetraRule ( const rule_enum & = nosplit) ;
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;

    private :
      rule_t _r ;
    } ;
      
    struct HexaRule
    {
      enum rule_enum { crs = -1, nosplit = 1, iso8 };
      typedef signed char rule_t;

      explicit HexaRule ( const rule_t & );
      HexaRule ( const int );
      HexaRule ( const rule_enum & = nosplit);
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;

    private :
      rule_t _r ;
    } ;
     
  public :

    // Die Geometriesockelklassen sind die Grundlage zur Implementierung
    // numerischer Verfahren auf den bestimmten Elementtypen, und erlauben
    // alle Man"over, die "uber die geometrische Information verf"ugen
    // m"ussen, wie z.B. Navigation zur Fl"ache, zu den Kanten und Knoten,
    // aber auch Anforderungen an den Nachbarn.

    class hasFace3 : public virtual stiExtender_t :: hasFacePllXIF 
    {
    public :
      typedef Hface3Rule balrule_t ;
      virtual bool refineBalance (balrule_t,int) = 0 ;
      virtual bool bndNotifyCoarsen () = 0 ;

      // returns true, if underlying object is real 
      virtual bool isRealObject () const { return true; }
    protected :
      hasFace3 () {}
      virtual ~hasFace3 () {}
      inline bool bndNotifyBalance (balrule_t,int) ;

    public:
      virtual int calcSortnr (int,int) {return (abort(),0);}   
      virtual bool isboundary() const = 0;    
      virtual int nbLevel() const = 0;
      virtual int nbLeaf() const = 0;

      // returns true if a vertex projection is set 
      virtual bool hasVertexProjection () const = 0;
    } ;

    class hasFace4 : public virtual stiExtender_t :: hasFacePllXIF {
    public :
      typedef Hface4Rule balrule_t ;
      virtual bool refineBalance (balrule_t,int) = 0 ;
      virtual bool bndNotifyCoarsen () = 0 ;

      // returns true, if underlying object is real 
      virtual bool isRealObject () const { return true; }
    protected :
      hasFace4 () {}
      virtual ~hasFace4 () {}
      inline bool bndNotifyBalance (balrule_t,int) ;

    public :
      virtual bool isboundary() const = 0;
      virtual int nbLevel() const  = 0;
      virtual int nbLeaf() const = 0; 

      // returns true if a vertex projection is set 
      virtual bool hasVertexProjection () const = 0;
    } ;

    // hasFace_t is hasFace3 and hasFace4 
    // this class is used as default value for the face neighbour
    // the pointer is set in gitter_geo.cc where the null neigbours are
    // initialized. This means that having no neighbour will not result in 
    // a segementation falut, but just return some default values 
    template <class hasFace_t>
    class hasFaceEmpty : public hasFace_t 
    {
    public :
      typedef typename hasFace_t::balrule_t balrule_t ;
      typedef hasFaceEmpty<hasFace_t> ThisType ;
      // returning true, means that face is also refined 
      bool refineBalance (balrule_t,int) { return true; }
      // true means coarsening allowed 
      bool bndNotifyCoarsen () { return true; }
      // return reference to the one instance we need 
      static ThisType & instance () 
      { 
        static ThisType singleton;
        return singleton;
      }

      // as we have not a real element or boundary here, return false 
      bool isRealObject () const { return false; }

      // return false for vertex projection  
      inline bool hasVertexProjection() const { return false; }
    private:
      hasFaceEmpty () {}
      hasFaceEmpty (const hasFaceEmpty & );

    public:
      int calcSortnr (int,int) {return (abort(),0);}   
      // this is counted as boundary to seperate from elements 
      bool isboundary() const { return true; }    
      int nbLevel() const { return (assert(false),abort(),-1); }
      int nbLeaf() const { return (assert(false),abort(),-1);}
    } ;

    typedef class VertexGeo : public vertex_STI, public MyAlloc 
    {
    public:  
      // VertexGeo is provided for the vertices on lower levels 
      inline VertexGeo (int,double,double,double, VertexGeo & ) ;
      inline VertexGeo (int,double,double,double, IndexManagerStorageType & im ) ;
      inline virtual ~VertexGeo () ;

      // return coordinates of vertex  
      inline const alucoord_t (& Point () const) [3] ;
      // return level of vertex 
      inline int level () const ;
      // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
      virtual inline void project(const ProjectVertexPair &pv) ; 
            
      // overload backupIndex and restoreIndex here
      inline void backupIndex  (ostream & os ) const;
      inline void restoreIndex (istream & is , vector<bool>(&)[4] );

      // backup does nothing 
      inline void backup (ostream & os ) const {}
      inline void restore (istream & is ) {}

      int nChild () const { return 0 ; }

      // return pointer to grid 
      Gitter* myGrid() { return _indexManagerStorage.myGrid(); }
      const Gitter* myGrid() const { return _indexManagerStorage.myGrid(); }
      IndexManagerStorageType& indexManagerStorage () { return _indexManagerStorage; }
    private :
      // the coordinates of this vertex 
      alucoord_t _c [3] ;
    protected:
      // index manager
      IndexManagerStorageType & _indexManagerStorage;

      IndexManagerType& indexManager() { 
        return _indexManagerStorage.get( IndexManagerStorageType :: IM_Vertices ); }

    private:   
      // the level of creation 
      unsigned char _lvl ;
    public :
      // reference counter 
      using DuneIndexProvider :: ref ;
    } vertex_GEO ;
  
    typedef class hedge1 : public hedge_STI , public MyAlloc 
    {
    protected :
      typedef VertexGeo myvertex_t ;
      inline hedge1 (myvertex_t *,myvertex_t *) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
      inline bool lockedAgainstCoarsening () const ;
    public :
      typedef Hedge1Rule myrule_t ;
      inline virtual ~hedge1 () ;
      inline myvertex_t * myvertex (int) ;
      inline const myvertex_t * myvertex (int) const ;
      virtual myvertex_t * subvertex (int) = 0 ;
      virtual const myvertex_t * subvertex (int) const = 0 ;
      virtual hedge1 * subedge1 (int) = 0 ;
      virtual const hedge1 * subedge1 (int) const = 0 ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual void refineImmediate (myrule_t) = 0 ;
    private :
      myvertex_t * v0, * v1 ; // 16 bytes + 16 (24 with comm buffer) 32 (40) 
    public:  
      // reference counter 
      using DuneIndexProvider :: ref ;
    } hedge1_GEO ;
  
    typedef class hface3 : public hface_STI, public MyAlloc {
    public :
      typedef hasFace3  myconnect_t ;
      typedef Hface3Rule myrule_t ;
      enum { polygonlength = 3 } ;
      class face3Neighbour 
      {
        myconnect_t *_faceFront;
        myconnect_t *_faceRear;
        signed char _numFront;
        signed char _numRear;
        signed char s [polygonlength] ;  // 12 bytes 

      public:  
        myrule_t _parRule;  // 1 bytes 
      public :
        static const pair < myconnect_t *, int > null ;
        inline face3Neighbour () ;
        void setFront ( const pair< myconnect_t *, int > &p );
        void setRear ( const pair< myconnect_t *, int > &p );
        inline void operator = (const face3Neighbour &) ;
        inline int complete (const face3Neighbour &) ;
        inline pair < myconnect_t *, int > front () ;
        inline pair < const myconnect_t *, int > front () const ;
        inline pair < myconnect_t *, int > rear () ;
        inline pair < const myconnect_t *, int > rear () const ;
        friend class hface3 ;
      } nb ; // 24 bytes 
    protected :
      typedef VertexGeo   myvertex_t ;
      typedef hedge1_GEO  myhedge1_t ;
    protected :
      inline hface3 (myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
    public :
      inline virtual ~hface3 () ;
      inline void attachElement (const pair < hasFace3 *, int > &,int) ;
      inline void detachElement (int) ;
    public :
      inline int twist (int) const ;
      inline myvertex_t * myvertex (int) ;
      inline const myvertex_t * myvertex (int) const ;
      inline myhedge1_t * myhedge1 (int) ;
      inline const myhedge1_t * myhedge1 (int) const ;
      virtual hface3 * down () = 0 ;
      virtual const hface3 * down () const = 0 ;
      virtual hface3 * next () = 0 ;
      virtual const hface3 * next () const = 0 ;
      virtual myvertex_t * subvertex (int) = 0 ;
      virtual const myvertex_t * subvertex (int) const = 0 ;
      virtual myhedge1_t * subedge1 (int) = 0 ;
      virtual const myhedge1_t * subedge1 (int) const = 0 ;
      virtual hface3 * subface3 (int) = 0 ;
      virtual const hface3 * subface3 (int) const = 0 ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual bool refine (myrule_t,int) = 0 ;
      virtual void refineImmediate (myrule_t) = 0 ;
    public :
      myrule_t parentRule() const;

      // returns true, if element conected to face is leaf 
      virtual bool isInteriorLeaf() const ;

    protected :
      myhedge1_t * e [polygonlength] ; // 24 bytes 
    public:  
      // reference counter 
      using DuneIndexProvider :: ref ;
    } hface3_GEO ;

    typedef class hface4 : public hface_STI, public MyAlloc {
    public :
      typedef hasFace4  myconnect_t ;
      typedef Hface4Rule myrule_t ;
      enum { polygonlength = 4 } ;
      class face4Neighbour {
        myconnect_t *_faceFront;
        myconnect_t *_faceRear;
        signed char _numFront;
        signed char _numRear;
        // put here to save memory because of padding 
        signed char s [polygonlength] ;
      public:  
        // put here to save memory because of padding 
        myrule_t _parRule;

      public :
        static const pair < myconnect_t *, int > null ;
        void setFront ( const pair< myconnect_t *, int > &p );
        void setRear ( const pair< myconnect_t *, int > &p );
        inline face4Neighbour () ;
        inline void operator = (const face4Neighbour &) ;
        inline int complete (const face4Neighbour &) ;
        inline pair < myconnect_t *, int > front () ;
        inline pair < const myconnect_t *, int > front () const ;
        inline pair < myconnect_t *, int > rear () ;
        inline pair < const myconnect_t *, int > rear () const ;
        friend class hface4 ;
      } nb ; // 24 byte 
    protected :
      typedef VertexGeo  myvertex_t ;
      typedef hedge1_GEO  myhedge1_t ;
    public :
    protected :
      inline hface4 (myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
    public :
      inline virtual ~hface4 () ;
      inline void attachElement (const pair < hasFace4 *, int > &,int) ;
      inline void detachElement (int) ;
    public :
      inline int twist (int) const ;
      inline myvertex_t * myvertex (int) ;
      inline const myvertex_t * myvertex (int) const ;
      inline myhedge1_t * myhedge1 (int) ;
      inline const myhedge1_t * myhedge1 (int) const ;
      virtual hface4 * down () = 0 ;
      virtual const hface4 * down () const = 0 ;
      virtual hface4 * next () = 0 ;
      virtual const hface4 * next () const = 0 ;
      virtual myvertex_t * subvertex (int) = 0 ;
      virtual const myvertex_t * subvertex (int) const = 0 ;
      virtual myhedge1_t * subedge1 (int) = 0 ;
      virtual const myhedge1_t * subedge1 (int) const = 0 ;
      virtual hface4 * subface4 (int) = 0 ;
      virtual const hface4 * subface4 (int) const = 0 ;

      // returns true, if element connected to face is leaf 
      virtual bool isInteriorLeaf() const ;

    public :
      virtual myrule_t getrule () const = 0 ;
      virtual bool refine (myrule_t,int) = 0 ;
      virtual void refineImmediate (myrule_t) = 0 ;
    public :
      myrule_t parentRule() const;
    private :
      myhedge1_t * e [polygonlength] ; // polygonlength * 8 = 32 

    public:  
      // reference counter 
      using DuneIndexProvider :: ref ;
    } hface4_GEO ; // 56 + 16 = 72 
  
    // Geometriesockelklasse des Tetraeders: Vorsicht der Prototyp der dem
    // Tetraeder zugrunde liegt, hat eine nach links (gegen Uhrzeigersinn)
    // umlaufende Numerierung der Knoten auf den Randfl"achen, wenn aus
    // dem Element herausgeblickt wird. Die Konvention f"ur den Hexaeder
    // ist leider genau umgekehrt. Dies sollte beim Aufbau von Pyramiden
    // und Prismen sorgf"altig bedacht werden.
    // Der Prototyp steht in 'gitter_geo.cc'.
  
    typedef class Tetra : public helement_STI, public hasFace3, public MyAlloc {
    protected :

      typedef VertexGeo  myvertex_t ;
      typedef hedge1_GEO myhedge1_t ;
      typedef hface3_GEO myhface3_t ;
      typedef TetraRule  myrule_t ;
      inline Tetra (myhface3_t *, int, myhface3_t *, int, 
                    myhface3_t *, int, myhface3_t *, int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
    public :
      using hasFace3     :: accessPllX ;
      static const int prototype [4][3] ;
      static const int edgeMap [6][2] ;

      static const int edgeTwist [6][3] ;
      static const int vertexTwist [6][3] ;

      // returns 3 which is the lenght of the edges not on face number
      static const vector<int> & verticesNotOnFace( const int face ) ; 
      static const vector<int> & edgesNotOnFace( const int face ) ; 
      static const vector<int> & facesNotOnFace( const int face ) ; 
      
      inline virtual ~Tetra () ;
      inline hface3_GEO * myhface3 (int) ;
      inline const hface3_GEO * myhface3 (int) const ;
      inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
      inline hedge1_GEO * myhedge1(int);
      inline const  hedge1_GEO * myhedge1(int) const;
      inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace3 *, int > myneighbour (int) ;
      inline pair < const hasFace3 *, int > myneighbour (int) const ;

      // Dune extension 
      // return pair, first = pointer to face, second = twist of face
      inline pair < hface3_GEO *, int > myintersection (int) ;
      inline pair < const hface3_GEO *, int > myintersection (int) const;
      
      virtual int nFaces() const { return 4; }
      virtual int nEdges() const { return 6; }
      inline int twist (int) const ;
      int test () const ;
      // returns level of this object 
      virtual int nbLevel() const {return level();}
      // returns leaf 
      virtual int nbLeaf() const {return leaf();}

      // returns false because only bnd segments have projections 
      virtual bool hasVertexProjection () const { return false; }
    public :
      virtual myrule_t getrule () const = 0 ;
      
      // return rule which was set by request 
      virtual myrule_t requestrule () const = 0 ; 

      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
      int tagForGlobalCoarsening () ;
      int resetRefinementRequest () ;
      int tagForBallRefinement (const alucoord_t (&)[3],double,int) ;

      virtual bool isboundary() const { return false; }
      virtual grid_t type() const { return tetra; }
      virtual void attachleafs() { abort(); }
      virtual void detachleafs() { abort(); }
    private :
      inline int evalVertexTwist(int, int) const;
      inline int evalEdgeTwist(int, int) const;
      
      inline int originalVertexTwist(int, int) const;
      inline int originalEdgeTwist(int, int) const;
    private:
      myhface3_t * f [4] ;
      signed char s [4] ;
    } tetra_GEO ;
  
    // Geometriesockelklasse des periodischen Randelements mit zwei
    // 3-Punkt-Fl"achen.
  
    typedef class Periodic3 : public helement_STI, public hasFace3, public MyAlloc {
    protected :
      typedef VertexGeo  myvertex_t ;
      typedef hedge1_GEO myhedge1_t ;
      typedef hface3_GEO myhface3_t ;
      typedef Hface3Rule myrule_t ;
      inline Periodic3 (myhface3_t *, int, myhface3_t *, int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
    public :
      using hasFace3     :: accessPllX ;
      static const int prototype [2][3] ;
      inline virtual ~Periodic3 () ;
      inline hface3_GEO * myhface3 (int) ;
      inline const hface3_GEO * myhface3 (int) const ;
      inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
      inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace3 *, int > myneighbour (int) ;
      inline pair < const hasFace3 *, int > myneighbour (int) const ;
      virtual int nFaces() const { return 2; }
      virtual int nEdges() const { 
        cerr << "Periodic3 :: nEdges not implemented! \n"; abort(); return 6; 
      }
    
      inline int twist (int) const ;
      int test () const ;
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
      int tagForGlobalCoarsening () ;
      int resetRefinementRequest () ;
      int tagForBallRefinement (const alucoord_t (&)[3],double,int) ;
      virtual bool isboundary() const { return true; }
      virtual grid_t type() const { return tetra_periodic; }

      // just returns level 
      virtual int nbLevel() const {return level();}
      // just returns leaf 
      virtual int nbLeaf() const {return leaf();}

      // return false for vertex projection  
      virtual bool hasVertexProjection() const { return false; }
    private :
      myhface3_t * f [2] ;
      signed char s [2] ;
    } periodic3_GEO ;

    // Anfang - Neu am 23.5.02 (BS)

    // Geometriesockelklasse des periodischen Randelements mit zwei
    // 4-Punkt-Fl"achen.
  
    typedef class Periodic4 : public helement_STI, public hasFace4, public MyAlloc {
    protected :
      typedef VertexGeo  myvertex_t ;
      typedef hedge1_GEO myhedge1_t ;
      typedef hface4_GEO myhface4_t ;
      typedef Hface4Rule myrule_t ;
      inline Periodic4 (myhface4_t *, int, myhface4_t *, int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
    public :
      using hasFace4     :: accessPllX ;
      static const int prototype [2][4] ;
      inline virtual ~Periodic4 () ;
      inline hface4_GEO * myhface4 (int) ;
      inline const hface4_GEO * myhface4 (int) const ;
      inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
      inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace4 *, int > myneighbour (int) ;
      inline pair < const hasFace4 *, int > myneighbour (int) const ;

      virtual int nFaces() const { return 2; }
      virtual int nEdges() const { 
        cerr << "Periodic4 :: nEdges not implemented! \n"; abort(); return 8; 
      }
      inline int twist (int) const ;
      int test () const ;

      virtual bool isboundary() const { return true; }
      virtual grid_t type() const { return hexa_periodic; }

    public :
      virtual myrule_t getrule () const = 0 ;
      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
      int tagForGlobalCoarsening () ;
      int resetRefinementRequest () ;
      int tagForBallRefinement (const alucoord_t (&)[3],double,int) ;
      // just returns level 
      virtual int nbLevel() const {return level();}
      // just returns leaf 
      virtual int nbLeaf() const {return leaf();}

      // returns false because only bnd segments have projections 
      virtual bool hasVertexProjection () const { return false; }
    private :
      myhface4_t * f [2] ;
      signed char s [2] ;
    } periodic4_GEO ;

    // Ende - Neu am 23.5.02 (BS)

    // Der Prototyp f"ur das Hexaederelement bedingt eine im Uhrzeigersinn
    // umlaufende Numerierung der lokalen Knoten einer Aussenfl"ache, falls
    // aus dem Element herausgeschaut wird. Gegensatz zum Tetraeder.
    // Der Prototyp steht in 'gitter_geo.cc'
  
    typedef class Hexa : public helement_STI, public hasFace4, public MyAlloc {
    protected :
      typedef VertexGeo myvertex_t ;
      typedef hedge1_GEO myhedge1_t ;
      typedef hface4_GEO myhface4_t ;
      typedef HexaRule  myrule_t ;
      inline Hexa (myhface4_t *, int, myhface4_t *, int,
                   myhface4_t *, int, myhface4_t *, int, 
                   myhface4_t *, int, myhface4_t *, int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;

    public :
      using hasFace4     :: accessPllX ;
      static const int prototype [6][4] ;
      static const int oppositeFace [6] ;
      static const int edgeMap [12][2];

      // cached possible twists 
      static const int edgeTwist [8][4] ;
      static const int vertexTwist [8][4] ;
      
      static const int vertex2Face[8][2];
      

      static const vector<int> & verticesNotOnFace( const int face ); 
      static const vector<int> & edgesNotOnFace( const int face ); 
      static const vector<int> & facesNotOnFace( const int face ); 

      inline virtual ~Hexa () ;
      inline hface4_GEO * myhface4 (int) ;
      inline const hface4_GEO * myhface4 (int) const ;
      inline VertexGeo * myvertex (int) ;
      inline const VertexGeo * myvertex (int) const ;
      inline hedge1_GEO * myhedge1(int);
      inline const  hedge1_GEO * myhedge1(int) const;
      inline VertexGeo * myvertex (int,int) ;
      inline const VertexGeo * myvertex (int,int) const ;
      inline pair < hasFace4 *, int > myneighbour (int) ;
      inline pair < const hasFace4 *, int > myneighbour (int) const ;

      // Dune extension
      // return pair, first = pointer to face, second = twist of face
      inline pair < hface4_GEO *, int > myintersection (int) ;
      inline pair < const hface4_GEO *, int > myintersection (int) const;
      virtual int nFaces() const { return 6; }
      virtual int nEdges() const { return 12; }

      inline int twist (int) const ;
      int test () const ;
      // just returns level 
      virtual int nbLevel() const {return level();}
      // just returns leaf 
      virtual int nbLeaf() const {return leaf();}

      // returns false because only bnd segments have projections 
      virtual bool hasVertexProjection () const { return false; }
    public :
      virtual myrule_t getrule () const = 0 ;
      virtual myrule_t requestrule () const = 0;
      virtual void request (myrule_t) = 0 ;
      int tagForGlobalRefinement () ;
      int tagForGlobalCoarsening () ;
      int resetRefinementRequest () ;
      int tagForBallRefinement (const alucoord_t (&)[3],double,int) ;

      virtual bool isboundary() const { return false; }
      virtual grid_t type() const { return hexa; }

      virtual void attachleafs() { abort(); }
      virtual void detachleafs() { abort(); }
    private :
      // original formulas of twist evaluation 
      inline int originalVertexTwist(int, int) const;
      inline int originalEdgeTwist(int, int) const;

      // cached twist 
      inline int evalVertexTwist(int, int) const;
      inline int evalEdgeTwist(int, int) const;
    private:
      myhface4_t * f [6] ;
      signed char s [6] ;
    } hexa_GEO ;
  
    // Auch hier ist Vorsicht geboten: Der Protoyp des Dreiecksrandelement
    // numeriert seine Knoten gegen den Uhrzeigersinn, wenn aus dem Randelement
    // auf die Randfl"ache geschaut wird. Das Vierecksrandelement hat die
    // entgegengesetzte Konvention.
  
    typedef class hbndseg3 : public hbndseg_STI, public hasFace3, public MyAlloc {
    public :
      typedef VertexGeo   myvertex_t ;
      typedef hedge1_GEO  myhedge1_t ;
      typedef hface3_GEO  myhface3_t ;
      typedef hface3_GEO  myhface_t ;
      typedef Hface3Rule  myrule_t ;
      
      typedef hbndseg_STI :: bnd_t bnd_t;
    protected :
      inline hbndseg3 (myhface3_t *,int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
      inline bool lockedAgainstCoarsening () const { return false ; }
      inline bool hasVertexProjection() const { return (projection() != 0); }
    public :
      inline virtual ~hbndseg3 () ;
      inline myrule_t getrule () const ;
      virtual bool refineLikeElement (balrule_t) = 0 ;
      inline myvertex_t * myvertex (int,int) const ;
      inline myhface3_t * myhface3 (int) const ;
      inline int twist (int) const ;
      inline hface3_GEO * subface3 (int,int) const ;
      
      virtual bool isboundary() const { return true; }
      virtual int nChild () const;
      // just returns level 
      virtual int nbLevel() const { return level(); }
      // just returns leaf 
      virtual int nbLeaf() const { return leaf(); }
      
      // mark edges and vertices as leaf 
      virtual void attachleafs() ;

      // unmark edges and vertices as leaf 
      virtual void detachleafs() ;

    protected :
      // no projection for ghost faces 
      const ProjectVertex* projection() const { return ( this->isGhost() ) ? 0 : _face->myvertex(0)->myGrid()->vertexProjection(); }
      ProjectVertex* projection() { return ( this->isGhost() ) ? 0 : _face->myvertex(0)->myGrid()->vertexProjection(); }

    private:
      myhface3_t * _face ;
      int _twist ;
    public:  
    } hbndseg3_GEO ;
  

    typedef class hbndseg4 : public hbndseg_STI, public hasFace4, public MyAlloc {
    public :
      typedef VertexGeo myvertex_t ;
      typedef hedge1_GEO  myhedge1_t ;
      typedef hface4_GEO  myhface4_t ;
      typedef hface4_GEO  myhface_t ;
      typedef Hface4Rule  myrule_t ;

      typedef hbndseg_STI :: bnd_t bnd_t;
    protected :
      inline hbndseg4 (myhface4_t *,int) ;
      inline int postRefinement () ;
      inline int preCoarsening () ;
      inline bool lockedAgainstCoarsening () const { return false ; }
      inline bool hasVertexProjection() const { return (projection() != 0); }
    public :
      inline virtual ~hbndseg4 () ;
      inline myrule_t getrule () const ;
      virtual bool refineLikeElement (balrule_t) = 0 ;
      inline myvertex_t * myvertex (int,int) const ;
      inline myhface4_t * myhface4 (int) const ;
      inline int twist (int) const ;
      inline hface4_GEO * subface4 (int,int) const ;
      
      virtual bool isboundary() const { return true; }
      virtual int nChild () const;
      virtual int nbLevel() const {return level();}
      virtual int nbLeaf() const {return leaf();}
      virtual void attachleafs() ;
      
      virtual void detachleafs() ;

    protected :
      // no projection for ghost faces 
      const ProjectVertex* projection() const { return ( this->isGhost() ) ? 0 : _face->myvertex(0)->myGrid()->vertexProjection(); }
      ProjectVertex* projection() { return ( this->isGhost() ) ? 0 : _face->myvertex(0)->myGrid()->vertexProjection(); }

    private:
      myhface4_t * _face ;
      int _twist ;

    public:   
    } hbndseg4_GEO ;
  
    class InternalHasFace3 {
    public :
      typedef hasFace3 val_t ;
      val_t * operator () (hasFace3 * x) const { return x ; }
      val_t & operator () (hasFace3 & x) const { return x ; }
    } ;
  
    class InternalHasFace4 {
    public :
      typedef hasFace4 val_t ;
      val_t * operator () (hasFace4 * x) const { return x ; }
      val_t & operator () (hasFace4 & x) const { return x ; }
    } ;
  public :
    class BuilderIF : public Makrogitter {
  
      // BuilderIF ist die Stelle des Makrogitters an der der Builder angreift, wenn das
      // Gitter erbaut werden soll. Der Builder geht direkt mit den Listen um und
      // wendet sich an die Factorymethoden insert_--*-- (), um neue Objekte zu erhalten.
    public:
      typedef list < VertexGeo * >     vertexlist_t;
      typedef list < hedge1_GEO * >    hedge1list_t ;
      typedef list < hface4_GEO * >    hface4list_t ;
      typedef list < hface3_GEO * >    hface3list_t ;
      typedef list < tetra_GEO * >     tetralist_t ;
      typedef list < periodic3_GEO * > periodic3list_t ;

      typedef list < periodic4_GEO * > periodic4list_t ;
      typedef list < hexa_GEO * >      hexalist_t ;

      typedef list < hbndseg3_GEO * >  hbndseg3list_t ;
      typedef list < hbndseg4_GEO * >  hbndseg4list_t ;

    private:   
      // macro object lists 
      vertexlist_t    _vertexList ;
      hedge1list_t    _hedge1List ;
      hface4list_t    _hface4List ;
      hface3list_t    _hface3List ;
      tetralist_t     _tetraList ;
      periodic3list_t _periodic3List ;

      periodic4list_t _periodic4List ;
      hexalist_t      _hexaList ;

      hbndseg3list_t  _hbndseg3List ;
      hbndseg4list_t  _hbndseg4List ;

      bool _modified ; // true if macro grid was modified 

    protected :
      BuilderIF () : _modified (true) {}

      virtual ~BuilderIF () ;

      // return size of used memory in bytes 
      virtual size_t memUsage ();

      // generates macro image from macro file 
      void generateRawHexaImage (istream &, ostream &) ;
      
      virtual void macrogridBuilder (istream &) ;
      virtual VertexGeo     * insert_vertex (double, double, double, int) = 0 ;
      virtual VertexGeo     * insert_ghostvx(double, double, double, int) = 0 ;
      virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) = 0 ;
      virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) = 0 ;
      virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) = 0 ;
      virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) = 0 ;
      
      virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) = 0 ;
      virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) = 0 ;
      
      virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) = 0 ;
      
      virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, hbndseg_STI :: bnd_t) = 0 ;

      // insert ghost element 
      virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, hbndseg_STI:: bnd_t, 
                                            MacroGhostInfoTetra* ) = 0 ;
      
      virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, hbndseg_STI :: bnd_t) = 0 ;

      // method to insert internal boundary with ghost 
      virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, hbndseg_STI :: bnd_t, 
                                            MacroGhostInfoHexa* ) = 0 ;

      IteratorSTI < vertex_STI > * iterator (const vertex_STI *) const ;
      IteratorSTI < vertex_STI > * iterator (const IteratorSTI < vertex_STI > *) const ;
      IteratorSTI < hedge_STI >  * iterator (const hedge_STI *) const ;
      IteratorSTI < hedge_STI >  * iterator (const IteratorSTI < hedge_STI > *) const ;
      IteratorSTI < hface_STI >  * iterator (const hface_STI *) const ;
      IteratorSTI < hface_STI >  * iterator (const IteratorSTI < hface_STI > *) const ;
      IteratorSTI < helement_STI > * iterator (const helement_STI *) const ;
      IteratorSTI < helement_STI > * iterator (const IteratorSTI < helement_STI > *) const ;
      IteratorSTI < hbndseg_STI > * iterator (const hbndseg_STI *) const ;
      IteratorSTI < hbndseg_STI > * iterator (const IteratorSTI < hbndseg_STI > *) const ;
    public:  
      enum { numOfIndexManager = IndexManagerStorageType :: numOfIndexManager };

      // number of different index manager that exists 
      enum { IM_Elements = IndexManagerStorageType :: IM_Elements, // 0 == elements 
             IM_Faces    = IndexManagerStorageType :: IM_Faces,    // 1 == faces 
             IM_Edges    = IndexManagerStorageType :: IM_Edges,    // 2 == edges 
             IM_Vertices = IndexManagerStorageType :: IM_Vertices, // 3 == vertices
             IM_Bnd      = IndexManagerStorageType :: IM_Bnd,      // 4 == boundary elements 
             IM_Internal = IndexManagerStorageType :: IM_Internal  // 5 == internal bnds, parallel only 
      };

    protected:
      // this variable is located here, because all the elements in
      // this lists use this objects to get  thier numbers 
      // index provider, for every codim one , 4 is for boundary
      IndexManagerStorageType _indexManagerStorage;

      // default implementations just use the iterator method  
      IteratorSTI < vertex_STI > * pureElementIterator (const vertex_STI * a) const { return iterator(a); }
      IteratorSTI < vertex_STI > * pureElementIterator (const IteratorSTI < vertex_STI > * a) const { return iterator(a); }
      IteratorSTI < hedge_STI > * pureElementIterator (const hedge_STI * a) const { return iterator(a); }
      IteratorSTI < hedge_STI > * pureElementIterator (const IteratorSTI < hedge_STI > * a) const { return iterator(a); }
      IteratorSTI < hface_STI > * pureElementIterator (const hface_STI * a) const { return iterator(a); }
      IteratorSTI < hface_STI > *   pureElementIterator (const IteratorSTI < hface_STI > * a) const { return iterator(a); }
      IteratorSTI < hbndseg_STI > * pureElementIterator (const hbndseg_STI * a) const { return iterator(a); }
      IteratorSTI < hbndseg_STI > *   pureElementIterator (const IteratorSTI < hbndseg_STI > * a) const { return iterator(a); }

      // different implementation for elements 
      IteratorSTI < helement_STI > * pureElementIterator (const helement_STI *) const ;
      IteratorSTI < helement_STI > * pureElementIterator (const IteratorSTI < helement_STI > *) const ;
    public :
      // return reference to indexManager 
      virtual IndexManagerType& indexManager(int codim);
      // return reference to indexManagerStorage
      virtual IndexManagerStorageType& indexManagerStorage();

      // return number of macro boundary segments 
      virtual size_t numMacroBndSegments() const;

      // compress all index manager 
      virtual void compressIndexManagers();
      
      virtual void backup (ostream &) const ;
      virtual void backup (const char*,const char *) const ;
      virtual void backupCMode (ostream &) const ;
      virtual void backupCMode (const char*,const char *) const ;
      friend class MacroGridBuilder ;
      friend class MacroGhostBuilder;
      friend class DuneParallelGridMover;
    } ;
  } ;
private :
  IteratorSTI < vertex_STI >   * iterator (const vertex_STI *) ;
  IteratorSTI < hedge_STI >    * iterator (const hedge_STI *) ;
  IteratorSTI < hface_STI >    * iterator (const hface_STI *) ;
  IteratorSTI < hbndseg_STI >  * iterator (const hbndseg_STI *) ;
  IteratorSTI < helement_STI > * iterator (const helement_STI *) ;

  IteratorSTI < vertex_STI >   * levelIterator (const vertex_STI *, const any_has_level<vertex_STI> &) ;
  IteratorSTI < hedge_STI >    * levelIterator (const hedge_STI *, const any_has_level<hedge_STI> & ) ;
  IteratorSTI < hface_STI >    * levelIterator (const hface_STI * , const any_has_level<hface_STI> &) ;
  IteratorSTI < hbndseg_STI >  * levelIterator (const hbndseg_STI *, const any_has_level<hbndseg_STI> &) ;
  IteratorSTI < helement_STI > * levelIterator (const helement_STI *, const any_has_level<helement_STI> &) ;

public:
  template <class StopRule_t >
  IteratorSTI < hedge_STI >    * createIterator(const hedge_STI * , const StopRule_t rule);
  
  template <class StopRule_t >
  IteratorSTI < hface_STI >    * createIterator(const hface_STI * , const StopRule_t rule);
  
  template <class StopRule_t >
  IteratorSTI < helement_STI > * createIterator(const helement_STI * ,const StopRule_t rule);
  
  template <class StopRule_t >
  IteratorSTI < hbndseg_STI >  * createIterator(const hbndseg_STI * , const StopRule_t rule);
  
protected :
  virtual bool refine () ;
  virtual void coarse () ;
  virtual Makrogitter & container () = 0 ;
  virtual const Makrogitter & container () const = 0 ;
  virtual inline int iterators_attached () const ;
  virtual void notifyGridChanges () ;
  virtual void notifyMacroGridChanges () ;
protected :
  Gitter () {} 
  virtual ~Gitter () ;

public :
  // callback for Dune 
  virtual int preCoarsening ( helement_STI & ) { return 0; }
  virtual int postRefinement( helement_STI & ) { return 0; }
  // callback for Dune 
  virtual int preCoarsening ( hbndseg_STI & ) { return 0; }
  virtual int postRefinement( hbndseg_STI & ) { return 0; }

  // return pointer to vertex projection 
  virtual ProjectVertex* vertexProjection() const = 0; 

  virtual void fullIntegrityCheck () ;
  virtual void printsize () ;
  virtual bool adapt () ;
  // this method just calls adapt 
  virtual bool adaptWithoutLoadBalancing() ;
  // adaptation with callback functionality 
  virtual bool duneAdapt ( AdaptRestrictProlongType & arp ) ;
  virtual void refineGlobal () ;
  virtual void refineBall (const alucoord_t (&)[3],double,int) ;
  virtual void refineRandom (double) ;
  virtual void backupCMode (ostream &) ;
  virtual void backupCMode (const char*,const char *) ;
  virtual void backup (ostream &) ;

  virtual void backup (const char*,const char *) ;
  virtual void restore (istream &) ;
  virtual void restore (const char*,const char *) ;

  // print memory consumption of grid 
  virtual void printMemUsage () = 0 ;

// only when xdrclass header was included  
#ifdef __XDRCLASS_INCLUDED__
  // new xdr backup and restore method
  virtual void backup (XDRstream_out &) ;
  virtual void restore (XDRstream_in &) ;
#endif

  virtual size_t numMacroBndSegments() const = 0;

  // return index manager of macro grid 
  virtual IndexManagerType & indexManager (int codim) = 0;

  // return reference to indexManagerStorage
  virtual IndexManagerStorageType& indexManagerStorage() = 0;

protected:
  // these classes are friend because the must call the method iterator on grid 
  friend class LeafIterator < helement_STI > ;
  friend class LeafIterator < vertex_STI > ;
  friend class LeafIterator < hbndseg_STI > ;
  friend class LeafIterator < hedge_STI > ;
  friend class LeafIterator < hface_STI > ;
  
  friend class LevelIterator < helement_STI > ;
  friend class LevelIterator < vertex_STI > ;
  friend class LevelIterator < hbndseg_STI > ;
  friend class LevelIterator < hedge_STI > ;
  friend class LevelIterator < hface_STI > ;
} ; 
// --endGitter

// --parallel types 
typedef Gitter :: Parallel Parallel ;
typedef Gitter :: VertexPllXIF  VertexPllXIF_t ;
typedef Gitter :: EdgePllXIF    EdgePllXIF_t ;
typedef Gitter :: FacePllXIF    FacePllXIF_t ;
typedef Gitter :: ElementPllXIF ElementPllXIF_t ;

// "Ausseres Iteratorproxy oder auch einfach ein Smartpointer
// um von aussen vom Gitter Iterationsobjekte zu bekommen und
// zu verwalten.

template < class A > class LeafIterator : public MyAlloc {
  Gitter * _grd ;
  IteratorSTI < A > * _w ;
  const A * _a ;
  void * operator new (size_t) { return 0 ; }
  void operator delete (void *) { }
  inline LeafIterator () ;
public :
  typedef A val_t;
  inline LeafIterator (Gitter &) ;
  inline LeafIterator (const LeafIterator < A > & ) ;
  inline ~LeafIterator () ;
  inline IteratorSTI < A > * operator -> () const ;
  inline IteratorSTI < A > & operator * () const ;
  inline LeafIterator < A > & operator = (const LeafIterator < A > &) ;
private: 
  inline void removeObj();
  inline void assign(const LeafIterator < A > & );
} ;

template < class A, class StopRule_t > class GridIterator : public MyAlloc {
  Gitter * _grd ;
  IteratorSTI < A > * _w ;
  const A * _a ;
  void * operator new (size_t) { return 0 ; }
  void operator delete (void *) { }
  inline GridIterator () ;
public :
  typedef A val_t;
  inline GridIterator (Gitter &, const StopRule_t ) ;
  inline GridIterator (const GridIterator < A , StopRule_t > & ) ;
  inline ~GridIterator () ;
  inline IteratorSTI < A > * operator -> () const ;
  inline IteratorSTI < A > & operator * () const ;
  inline GridIterator < A , StopRule_t > & operator = (const GridIterator < A , StopRule_t > &) ;
private: 
  inline void removeObj();
  inline void assign(const GridIterator < A , StopRule_t > & );
} ;

// LevelIterator is the same construct as LeafIterator, but the iterator
// rule differs, here we use any_has_level, see walk.h 
template < class A > class LevelIterator : public MyAlloc {
  Gitter * _grd ;
  const any_has_level < A > _ahl;
  IteratorSTI   < A > * _w ;
  const A * _a ;
  void * operator new (size_t) { return 0 ; }
  void operator delete (void *) { }
  inline LevelIterator () ;
public :
  typedef A val_t;
  // constructor with no level given creates macro iterator
  inline LevelIterator (Gitter &, int l = 0) ;
  inline LevelIterator (const LevelIterator < A > & ) ;
  inline ~LevelIterator () ;
  inline IteratorSTI < A > * operator -> () const ;
  inline IteratorSTI < A > & operator * () const ;
  inline LevelIterator < A > & operator = (const LevelIterator < A > &) ;
private: 
  inline void removeObj();
  inline void assign(const LevelIterator < A > & );
} ;

//
//    #    #    #  #          #    #    #  ######
//    #    ##   #  #          #    ##   #  #
//    #    # #  #  #          #    # #  #  #####
//    #    #  # #  #          #    #  # #  #
//    #    #   ##  #          #    #   ##  #
//    #    #    #  ######     #    #    #  ######
//

inline pair < int, int > operator += (pair < int, int> & a, const pair < int, int > & b) {
  return pair < int, int > (a.first += b.first, a.second += b.second) ;
}

#ifndef NDEBUG
#ifdef DEBUG_ALUGRID 
inline Refcount :: Globalcount :: Globalcount () : _c (0) {
  return ;
}

inline void Refcount :: Globalcount :: operator ++ (int) const {
  // ! cast around const
  ++ (int &) _c ;
  return ;
}

inline void Refcount :: Globalcount :: operator -- (int) const {
  -- (int &) _c ;
  return ;
}

inline Refcount :: Refcount () : _c (0) {
  _g ++ ;
  return ;
}

inline Refcount :: ~Refcount () {
  _g -- ;
  return ;
}
#else // else DEBUG_ALUGRID 
inline Refcount :: Refcount () : _c (0) { return ; }
inline Refcount :: ~Refcount () {  return ;}
#endif 
#else  
inline Refcount :: Refcount () : _c (0) { return ; }
inline Refcount :: ~Refcount () {  return ;}
#endif

inline int Refcount :: operator ++ (int) const {
  return _c++;
}

inline int Refcount ::operator ++ () const {
  return ++_c;
}
        
inline int Refcount :: operator -- (int) const {
  return _c--;
}
        
inline int Refcount :: operator -- () const {
  return --_c;
}

inline bool Refcount :: operator ! () const {
  return !_c;
}
        
inline Refcount :: operator int () const {
  return _c ;
}

//////////////////////////////////////////////////////////////////
//
// Empty Iterator  
//
//////////////////////////////////////////////////////////////////

template < class A > inline void EmptyIterator < A > :: first () {
  return ;
}

template < class A > inline void EmptyIterator < A > :: next () {
  return ;
}

template < class A > inline int EmptyIterator < A > :: done () const {
  return 1 ;
}

template < class A > inline int EmptyIterator < A > :: size () {
  return 0 ;
}

template < class A > inline A & EmptyIterator < A > :: item () const 
{
  // don't dereference an empty iterator 
  assert( ! done () );
  abort() ;
  A * p = 0;
  return *p;
}

template < class A > inline IteratorSTI < A > * EmptyIterator < A > :: clone () const 
{
  return new EmptyIterator < A > (*this);
}

template < class A > inline AccessIterator < A > :: Handle :: Handle (AccessIterator < A > & f) 
  : _fac (&f), _a (0), _w (0) 
{
  _fac->ref ++ ; 
  _w = _fac->iterator (_a) ;
  return ;
}

template < class A > inline AccessIterator < A > :: Handle :: Handle (const ThisType & p) 
  : _fac (0), _a (0) , _w(0) 
{ 
  assign(p);
  return ;
}

template < class A > inline IteratorSTI< A > * AccessIterator < A > :: Handle :: 
clone () const
{
  return new ThisType (*this);
}

template < class A > inline AccessIterator < A > :: Handle :: Handle () 
  : _fac (0)
  , _a (0)
  , _w ( new EmptyIterator < A > () ) 
{
  return ;
}

template < class A > inline AccessIterator < A > :: Handle :: ~Handle () {
  removeObj();
  return ;
}

template < class A > inline void AccessIterator < A > :: Handle :: 
removeObj () 
{
  if(_fac) _fac->ref-- ;
  _fac = 0; 
  if(_w) delete _w ;
  _w = 0;
  return ;
}

template < class A > inline const typename AccessIterator < A > :: Handle & AccessIterator < A > :: Handle :: operator = (const ThisType & x) 
{
  removeObj();
  assign(x);
  return x ;
}

template < class A > inline void 
AccessIterator < A > :: Handle :: assign (const ThisType & x) 
{
  assert( _fac == 0 );
  assert( _w == 0 );
  
  _fac = x._fac; 
  if( _fac ) _fac->ref ++ ;
  _w = x._w->clone();
}

template < class A > inline bool AccessIterator < A > :: Handle :: operator == (const ThisType & x) const {
  return (x._fac == _fac) ? ((& x._w->item ()) == (& _w->item ()) ? 1 : 0) : 0 ;
}

template < class A > inline bool AccessIterator < A > :: Handle :: operator < (const ThisType & x) const {
  return (abort (), false ) ;
}

template < class A > inline void AccessIterator < A > :: Handle :: first () {
  _w->first () ;
  return ;
}

template < class A > inline void AccessIterator < A > :: Handle :: next () {
  _w->next () ;
  return ;
}

template < class A > inline int AccessIterator < A > :: Handle :: done () const {
  return _w->done () ;
}

template < class A > inline int AccessIterator < A > :: Handle :: size () {
  return _w->size () ;
}

template < class A > inline A & AccessIterator < A > :: Handle :: item () const {
  return _w->item () ;
}

#include "walk.h"


template <class StopRule_t> 
inline IteratorSTI< Gitter ::hedge_STI > * Gitter :: createIterator(const hedge_STI * , const StopRule_t rule)
{
  typedef Insert < AccessIterator < hedge_STI > :: Handle,
                   TreeIterator < hedge_STI, StopRule_t > > level_edge__macro_edge__iterator ;

  vector < IteratorSTI < hedge_STI > * > _iterators ;

  _iterators.push_back ( new level_edge__macro_edge__iterator (container (), rule )) ;
  Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > nf (container ()) ;
  Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > > ne (container ()) ;
  Wrapper < Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > ef (nf) ;
  Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > ee (ne) ;

  _iterators.push_back ( new  Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, StopRule_t > > (ef , rule )) ;
  _iterators.push_back ( new Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, StopRule_t > > (ee, rule )) ;
  Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > > nef (container ()) ;
  Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fnef (nef) ;
  Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fnef) ;
  Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > efie (fie) ;
  _iterators.push_back (new Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, StopRule_t > > (efie, rule )) ;
  return new VectorAlign < hedge_STI > (_iterators) ;
}

template <class StopRule_t> 
inline IteratorSTI< Gitter :: hface_STI > * Gitter :: createIterator(const hface_STI * , const StopRule_t rule)
{
  typedef Insert < AccessIterator < hface_STI > :: Handle,
                   TreeIterator < hface_STI, StopRule_t > >  macro_face__iterator ;

  macro_face__iterator w1 (container (), rule ) ;
  Insert < AccessIterator < helement_STI > :: Handle,
           TreeIterator < helement_STI, has_int_face < helement_STI > > > nw (container ()) ;

  Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
            TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > ww (nw) ;

  Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
              TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
              TreeIterator < hface_STI, StopRule_t > > www (nw, rule ) ;

  return new AlignIterator < macro_face__iterator,
  Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, StopRule_t > >, hface_STI > (w1, www) ;
}

template <class StopRule_t> 
inline IteratorSTI< Gitter :: helement_STI > * Gitter :: createIterator(const helement_STI * , const StopRule_t rule)
{
  typedef Insert < AccessIterator < Gitter :: helement_STI > :: Handle,
                   TreeIterator < Gitter :: helement_STI, StopRule_t > >
             tree_element__macro_element__iterator ;

  return new tree_element__macro_element__iterator (container (), rule ) ;
}

template <class StopRule_t> 
inline IteratorSTI< Gitter :: hbndseg_STI > * Gitter :: createIterator(const hbndseg_STI * ,const StopRule_t rule)
{
  typedef Insert < AccessIterator < hbndseg_STI > :: Handle,
    TreeIterator < hbndseg_STI, StopRule_t > >
          tree_bnd__macro_bnd__iterator ;
  return new tree_bnd__macro_bnd__iterator (container (), rule ) ;
}

inline bool Gitter :: debugOption (int level) {
  return (getenv ("VERBOSE") ? ( atoi (getenv ("VERBOSE")) > level ? true : (level == 0)) : false) ;
}

inline int Gitter :: iterators_attached () const {
  return ref ;
}

inline int Gitter :: hedge :: leaf () const {
  return ! down () ;
}

inline int Gitter :: hface :: leaf () const {
  return ! down () ;
}

inline int Gitter :: helement :: leaf () const {
  return ! down () ;
}

// Dune extensions 
inline void Gitter :: Dune_helement :: resetRefinedTag () {
  ref.reset ();
}

inline bool Gitter :: Dune_helement :: hasBeenRefined () const {
  return ref.positive();
}

inline int Gitter :: hbndseg :: leaf () const {
  return ! down () ;
}

inline bool Gitter :: Geometric :: hasFace3 :: bndNotifyBalance (balrule_t,int) {
  return true ;
}

inline bool Gitter :: Geometric :: hasFace4 :: bndNotifyBalance (balrule_t,int) {
  return true ;
}




// #     #                                          #####
// #     #  ######  #####    #####  ######  #    # #     #  ######   ####
// #     #  #       #    #     #    #        #  #  #        #       #    #
// #     #  #####   #    #     #    #####     ##   #  ####  #####   #    #
//  #   #   #       #####      #    #         ##   #     #  #       #    #
//   # #    #       #   #      #    #        #  #  #     #  #       #    #
//    #     ######  #    #     #    ######  #    #  #####   ######   ####

inline Gitter :: Geometric :: VertexGeo :: VertexGeo (int l, double x, double y, double z, IndexManagerStorageType & ims) 
  : _indexManagerStorage (ims) 
  , _lvl (l) 
{
  _c [0] = x ; _c [1] = y ; _c [2] = z ;
  this->setIndex( indexManager().getIndex() );
  return ;
}

inline Gitter :: Geometric :: VertexGeo :: VertexGeo (int l, double x, double y, double z, VertexGeo & vx) 
  : _indexManagerStorage ( vx._indexManagerStorage ) 
  , _lvl (l)  
{
  _c [0] = x ; _c [1] = y ; _c [2] = z ;
  this->setIndex( indexManager().getIndex() );
  return ;
}

inline Gitter :: Geometric :: VertexGeo :: ~VertexGeo () 
{
  this->freeIndex( indexManager() );
  assert (ref ? (cerr << "**WARNING VertexGeo::refcount was " << ref << endl, 1) : 1) ;
  return ;
}

inline const alucoord_t (& Gitter :: Geometric :: VertexGeo :: Point () const) [3] {
  return _c ;
}

inline int Gitter :: Geometric :: VertexGeo :: level () const {
  return _lvl ;
}

inline void Gitter :: Geometric :: VertexGeo :: project(const ProjectVertexPair &pv) 
{
  // copy current coordinates  
  const alucoord_t p[3] = {_c[0],_c[1],_c[2]};
  // call projection operator 
  assert( pv.first );
  const int ok = (*pv.first)( p, pv.second, _c );

  if ( ! ok ) 
  {
    cerr << "ERROR in Gitter :: Geometric :: VertexGeo :: project(const ProjectVertexPair &pv) " 
         << "no boundary projection possible!" << endl;
    _c[0] = p[0]; _c[1] = p[1]; _c[2] = p[2];
  }
}

inline void Gitter :: Geometric :: VertexGeo :: backupIndex ( ostream & os ) const {
  os.write( ((const char *) &_idx ), sizeof(int) ) ;
}

inline void Gitter :: Geometric :: VertexGeo :: restoreIndex ( istream & is, vector<bool>(&isHole)[4] ) 
{
  is.read ( ((char *) &_idx), sizeof(int) ); 

  // mark vertex entry as not a hole 
  typedef Gitter :: Geometric :: BuilderIF BuilderIF;
  // make sure sizes match 
  assert( _idx < (int) isHole[BuilderIF :: IM_Vertices].size() );
  isHole[BuilderIF :: IM_Vertices][_idx] = false;
}

// #     #                                    #    ######
// #     #  ######  #####    ####   ######   ##    #     #  #    #  #       ######
// #     #  #       #    #  #    #  #       # #    #     #  #    #  #       #
// #######  #####   #    #  #       #####     #    ######   #    #  #       #####
// #     #  #       #    #  #  ###  #         #    #   #    #    #  #       #
// #     #  #       #    #  #    #  #         #    #    #   #    #  #       #
// #     #  ######  #####    ####   ######  #####  #     #   ####   ######  ######


inline Gitter :: Geometric :: Hedge1Rule :: Hedge1Rule ( const rule_t &r )
: _r ( r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: Hedge1Rule :: Hedge1Rule ( const rule_enum &r )
: _r( r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: Hedge1Rule :: operator rule_t () const
{
  return int( _r );
}

inline bool Gitter :: Geometric :: Hedge1Rule :: isValid (const rule_t &r)
{
  return r == nosplit || r == iso2 ;
}

inline bool Gitter :: Geometric :: Hedge1Rule :: isValid () const {
  return isValid( _r );
}

inline Gitter :: Geometric :: Hedge1Rule Gitter :: Geometric :: Hedge1Rule :: rotate (int i) const 
{
  assert (i == 0 || i == 1) ;
  switch (_r) 
  {
  case nosplit :
    return Hedge1Rule (nosplit) ;
  case iso2 :
    return Hedge1Rule (iso2) ;
  default :
    cerr << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
    return Hedge1Rule (nosplit) ;
  }
}


#if 0
inline ostream &operator<< ( ostream &out, const Gitter :: Geometric :: Hedge1Rule &rule )
{
  switch( rule )
  {
    case Gitter :: Geometric :: Hedge1Rule :: nosplit:
      return out << "nosplit";
    case Gitter :: Geometric :: Hedge1Rule :: iso2:
      return out << "iso2";
    default:
      return out << "!!! unknown !!!";
  }
}
#endif


// #     #                                    #
// #     #  ######  #####    ####   ######   ##
// #     #  #       #    #  #    #  #       # #
// #######  #####   #    #  #       #####     #
// #     #  #       #    #  #  ###  #         #
// #     #  #       #    #  #    #  #         #
// #     #  ######  #####    ####   ######  #####

inline Gitter :: Geometric :: hedge1 :: hedge1 (myvertex_t * a, myvertex_t * b) : v0 (a), v1 (b) { 
  v0->ref ++ ; 
  v1->ref ++ ;
  return ;
}

inline Gitter :: Geometric :: hedge1 :: ~hedge1 () {
  assert (ref ? (cerr << "**WARNING hedge1::refcount was " << ref << endl, 1) : 1) ;
  v0->ref -- ; 
  v1->ref -- ;
  return ;
}

inline int Gitter :: Geometric :: hedge1 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: hedge1 :: preCoarsening () {
  return 0 ;
}

inline bool Gitter :: Geometric :: hedge1 :: lockedAgainstCoarsening () const {
  return false ;
}

inline Gitter :: Geometric :: VertexGeo * Gitter :: Geometric :: hedge1 :: myvertex (int i) {
  assert (i == 0 || i == 1) ;
  return i == 1 ? v1 : v0 ;
}

inline const Gitter :: Geometric :: hedge1 :: myvertex_t * Gitter :: Geometric :: hedge1 :: myvertex (int i) const {
  assert (i == 0 || i == 1) ;
  return i == 1 ? v1 : v0 ;
}

// #     #                                  #####  ######
// #     #  ######    ##     ####   ###### #     # #     #  #    #  #       ######
// #     #  #        #  #   #    #  #            # #     #  #    #  #       #
// #######  #####   #    #  #       #####   #####  ######   #    #  #       #####
// #     #  #       ######  #       #            # #   #    #    #  #       #
// #     #  #       #    #  #    #  #      #     # #    #   #    #  #       #
// #     #  #       #    #   ####   ######  #####  #     #   ####   ######  ######

inline Gitter :: Geometric :: Hface3Rule :: Hface3Rule ( const rule_t &r )
: _r( r )
{
  assert( _r == undefined || isValid() );
}

inline Gitter :: Geometric :: Hface3Rule :: Hface3Rule ( const rule_enum &r )
: _r( r )
{
  assert( _r == undefined || isValid() );
}

inline Gitter :: Geometric :: Hface3Rule :: operator rule_t () const
{
  return int( _r );
}

inline bool Gitter :: Geometric :: Hface3Rule :: isValid (const rule_t& r) {
  return r == nosplit || r == iso4 || r == e01 || r == e12 || r == e20 ;
}

inline bool Gitter :: Geometric :: Hface3Rule :: isValid () const {
  return isValid( _r );
}

inline Gitter :: Geometric :: Hface3Rule Gitter :: Geometric :: Hface3Rule :: rotate (int t) const {
  assert ((-4 < t) && (t < 3)) ;
  switch (_r) {
  case nosplit :
    return Hface3Rule (nosplit) ;
  case e01 :
  case e12 :
  case e20 :
    if (t == 0 || t == -3) {    // twist 0 bzw. -2 : e01 bleibt und e12 <-> e20 event. swappen
      return Hface3Rule (_r == e01 ? e01 : (_r == e12 ? (t == 0 ? e12 : e20) : (t == 0 ? e20 : e12))) ;
    } else if (t == 1 || t == -1) { // twist 1 bzw. -1 : e20 -> e01 (beidesmal)
      return Hface3Rule (_r == e20 ? e01 : (_r == e12 ? (t == 1 ? e20 : e12) : (t == 1 ? e12 : e20))) ;
    } else if (t == 2 || t == -2) { // twist 2 bzw. -3 : e12 -> e01 (beidesmal)
      return Hface3Rule (_r == e12 ? e01 : (_r == e01 ? (t == 2 ? e20 : e12) : (t == 2 ? e12 : e20))) ;
    } else {
      abort () ;
    }
  case iso4 :
    return Hface3Rule (iso4) ;
  default :
    cerr << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
    return Hface3Rule (nosplit) ;
  }
}

#if 0
inline ostream &operator<< ( ostream &out, const Gitter :: Geometric :: Hface3Rule &rule )
{
  switch( rule )
  {
    case Gitter :: Geometric :: Hface3Rule :: nosplit:
      return out << "nosplit";
    case Gitter :: Geometric :: Hface3Rule :: e01:
      return out << "e01";
    case Gitter :: Geometric :: Hface3Rule :: e12:
      return out << "e12";
    case Gitter :: Geometric :: Hface3Rule :: e20:
      return out << "e20";
    case Gitter :: Geometric :: Hface3Rule :: iso4:
      return out << "iso4";
    case Gitter :: Geometric :: Hface3Rule :: undefined:
      return out << "undefined";
    default:
      return out << "!!! unknown !!!";
  }
}
#endif

// #     #                                 #       ######
// #     #  ######    ##     ####   ###### #    #  #     #  #    #  #       ######
// #     #  #        #  #   #    #  #      #    #  #     #  #    #  #       #
// #######  #####   #    #  #       #####  #    #  ######   #    #  #       #####
// #     #  #       ######  #       #      ####### #   #    #    #  #       #
// #     #  #       #    #  #    #  #           #  #    #   #    #  #       #
// #     #  #       #    #   ####   ######      #  #     #   ####   ######  ######

inline Gitter :: Geometric :: Hface4Rule :: Hface4Rule ( const rule_t &r )
: _r( r )
{
  assert( _r == undefined || isValid() );
}

inline Gitter :: Geometric :: Hface4Rule :: Hface4Rule ( const rule_enum &r )
: _r( r )
{
  assert( _r == undefined || isValid() );
}

inline Gitter :: Geometric :: Hface4Rule :: operator rule_t () const
{
  return int( _r );
}

inline bool Gitter :: Geometric :: Hface4Rule :: isValid (const rule_t& r) {
  return r == nosplit || r == iso4 /* || _r == ni02 || _r == ni13 */ ;
}

inline bool Gitter :: Geometric :: Hface4Rule :: isValid () const {
  return isValid( _r );
}

inline Gitter :: Geometric :: Hface4Rule Gitter :: Geometric :: Hface4Rule :: rotate (int t) const {
  switch (_r) {
  case nosplit :
    return Hface4Rule (nosplit) ;
  case iso4 :
    return Hface4Rule (iso4) ;
  default :
    cerr << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
    return Hface4Rule (nosplit) ;
  }
}

#if 0
inline ostream &operator<< ( ostream &out, const Gitter :: Geometric :: Hface4Rule &rule )
{
  switch( rule )
  {
    case Gitter :: Geometric :: Hface4Rule :: nosplit:
      return out << "nosplit";
    case Gitter :: Geometric :: Hface4Rule :: iso4:
      return out << "iso4";
    case Gitter :: Geometric :: Hface4Rule :: ni02:
      return out << "ni02";
    case Gitter :: Geometric :: Hface4Rule :: ni13:
      return out << "ni13";
    case Gitter :: Geometric :: Hface4Rule :: undefined:
      return out << "undefined";
    default:
      return out << "!!! unknown !!!";
  }
}
#endif
 
//                                         #####
// #    #  ######    ##     ####   ###### #     #
// #    #  #        #  #   #    #  #            #
// ######  #####   #    #  #       #####   #####
// #    #  #       ######  #       #            #
// #    #  #       #    #  #    #  #      #     #
// #    #  #       #    #   ####   ######  #####
//
  
inline Gitter :: Geometric :: hface3 :: face3Neighbour :: face3Neighbour ()
{
  setFront( null );
  setRear( null );
  return ;
}

inline void
Gitter :: Geometric :: hface3 :: face3Neighbour :: setFront ( const pair < myconnect_t *, int > &p )
{
  _faceFront = p.first;
  _numFront = p.second;
}

inline void
Gitter :: Geometric :: hface3 :: face3Neighbour :: setRear ( const pair < myconnect_t *, int > &p )
{
  _faceRear = p.first;
  _numRear = p.second;
}

inline void Gitter :: Geometric :: hface3 :: face3Neighbour :: operator = (const face3Neighbour & n)
{
  _faceFront = n._faceFront;
  _faceRear = n._faceRear;
  _numFront = n._numFront;
  _numRear = n._numRear;
  return ;
}

inline int Gitter :: Geometric :: hface3 :: face3Neighbour :: complete (const face3Neighbour & n)
{
  int ret = 0;

  if( front() == null )
  {
    setFront( pair< hasFace3 *, int >( n._faceFront, n._numFront ) );
    ++ret;
  }

  if( rear() == null )
  {
    setRear( pair< hasFace3 *, int >( n._faceRear, n._numRear ) );
    ++ret;
  }

  return ret;
}

inline pair < Gitter :: Geometric :: hface3 :: myconnect_t *, int >
Gitter :: Geometric :: hface3 :: face3Neighbour :: front ()
{
  return pair< myconnect_t *, int >( _faceFront, _numFront );
}

inline pair < const Gitter :: Geometric :: hface3 :: myconnect_t *, int >
Gitter :: Geometric :: hface3 :: face3Neighbour :: front () const
{
  return pair< const myconnect_t *, int >( _faceFront, _numFront );
}

inline pair < Gitter :: Geometric :: hface3 :: myconnect_t *, int >
Gitter :: Geometric :: hface3 :: face3Neighbour :: rear ()
{
  return pair< myconnect_t *, int >( _faceRear, _numRear );
}

inline pair < const Gitter :: Geometric :: hface3 :: myconnect_t *, int >
Gitter :: Geometric :: hface3 :: face3Neighbour :: rear () const
{
  return pair< const myconnect_t *, int >( _faceRear, _numRear );
}

inline Gitter :: Geometric :: hface3 :: 
hface3 (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2) 
{
  nb._parRule = (Hface3Rule::undefined);
  assert(e0 && e1 && e2) ;
  (e [0] = e0)->ref ++ ; nb.s [0] = s0 ;
  (e [1] = e1)->ref ++ ; nb.s [1] = s1 ;
  (e [2] = e2)->ref ++ ; nb.s [2] = s2 ;
  return ;
}

inline Gitter :: Geometric :: hface3 :: ~hface3 () {
  assert (ref ? (cerr << "**WARNING hface3::refcount was " << ref << endl, 1) : 1) ;
  e [0] -> ref -- ;
  e [1] -> ref -- ;
  e [2] -> ref -- ;
  return ;
}

inline void Gitter :: Geometric :: hface3 :: attachElement (const pair < myconnect_t *, int > & p, int t)
{
  if( t < 0 )
    nb.setRear( p );
  else
    nb.setFront( p );
  ref ++ ;
  return ;
}

inline void Gitter :: Geometric :: hface3 :: detachElement (int t)
{
  if( t < 0 )
    nb.setRear( nb.null );
  else
    nb.setFront( nb.null );
  ref -- ;
  return ;
}

inline int Gitter :: Geometric :: hface3 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface3 :: preCoarsening () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface3 :: twist (int i) const {
  assert (i < 3) ;
  return nb.s [i] ;
}

inline Gitter :: Geometric :: hface3 :: myhedge1_t * Gitter :: Geometric :: hface3 :: myhedge1 (int i) {
  assert (i < 3) ;
  return e [i] ;
}

inline const Gitter :: Geometric :: hface3 :: myhedge1_t * Gitter :: Geometric :: hface3 :: myhedge1 (int i) const {
  assert (i < 3) ;
  return e [i] ;
}

inline Gitter :: Geometric :: hface3 :: myvertex_t * Gitter :: Geometric :: hface3 :: myvertex (int i) {
  assert(0<=i && i < 3) ;
  return myhedge1 (i)->myvertex (nb.s[i]) ;
}

inline const Gitter :: Geometric :: hface3 :: myvertex_t * Gitter :: Geometric :: hface3 :: myvertex (int i) const {
  assert(0<=i && i < 3) ;
  return myhedge1 (i)->myvertex (nb.s[i]) ;
}

inline Gitter::Geometric::hface3::myrule_t 
Gitter::Geometric::hface3::parentRule() const {
  return (myrule_t) nb._parRule;
}

inline bool Gitter :: Geometric :: hface3 :: 
isInteriorLeaf() const 
{
  const myconnect_t & nbRear  = *(nb.rear().first);
  const myconnect_t & nbFront = *(nb.front().first);

  if( nbFront.isboundary() )
  {
    return (nbRear.nbLeaf() && nbRear.nbLevel() == this->level());
  }
  else if (nbRear.isboundary())
  {
    return (nbFront.nbLeaf() && nbFront.nbLevel() == this->level());
  }
  else
  {
    return (nbRear.nbLeaf() || (nbFront.nbLeaf()));
  }
}

//                                        #
// #    #  ######    ##     ####   ###### #    #
// #    #  #        #  #   #    #  #      #    #
// ######  #####   #    #  #       #####  #    #
// #    #  #       ######  #       #      #######
// #    #  #       #    #  #    #  #           #
// #    #  #       #    #   ####   ######      #

inline Gitter :: Geometric :: hface4 :: face4Neighbour :: face4Neighbour ()
{
  setFront( null );
  setRear( null );
  return ;
}

inline void
Gitter :: Geometric :: hface4 :: face4Neighbour :: setFront ( const pair < myconnect_t *, int > &p )
{
  _faceFront = p.first;
  _numFront = p.second;
}

inline void
Gitter :: Geometric :: hface4 :: face4Neighbour :: setRear ( const pair < myconnect_t *, int > &p )
{
  _faceRear = p.first;
  _numRear = p.second;
}

inline void Gitter :: Geometric :: hface4 :: face4Neighbour :: operator = (const face4Neighbour & n)
{
  _faceFront = n._faceFront;
  _faceRear = n._faceRear;
  _numFront = n._numFront;
  _numRear = n._numRear;
  return ;
}

inline int Gitter :: Geometric :: hface4 :: face4Neighbour :: complete (const face4Neighbour & n)
{
  int ret = 0;

  if( front() == null )
  {
    setFront( pair< myconnect_t *, int >( n._faceFront, n._numFront ) );
    ++ret;
  }

  if( rear() == null )
  {
    setRear( pair< myconnect_t *, int >( n._faceRear, n._numRear ) );
    ++ret;
  }

  return ret;
  // return (_v == null ? (_v = n._v, 1) : 0 ) + (_h == null ? (_h = n._h, 1) : 0 ) ;
}

inline pair < Gitter :: Geometric :: hface4 :: myconnect_t *, int >
Gitter :: Geometric :: hface4 :: face4Neighbour :: front ()
{
  return pair< myconnect_t *, int >( _faceFront, _numFront );
  //assert (!(_v == null)) ;
  //return _v ;
}

inline pair < const Gitter :: Geometric :: hface4 :: myconnect_t *, int >
Gitter :: Geometric :: hface4 :: face4Neighbour :: front () const
{
  return pair< const hasFace4 *, int >( _faceFront, _numFront );
  //assert (!(_v == null)) ;
  //return pair < const hasFace4 *, int > (_v.first,_v.second) ;
}

inline pair < Gitter :: Geometric :: hface4 :: myconnect_t *, int >
Gitter :: Geometric :: hface4 :: face4Neighbour :: rear ()
{
  return pair< myconnect_t *, int >( _faceRear, _numRear );
  //assert (!(_h == null)) ;
  //return _h ;
}

inline pair < const Gitter :: Geometric :: hface4 :: myconnect_t *, int >
Gitter :: Geometric :: hface4 :: face4Neighbour :: rear () const
{
  return pair< const myconnect_t *, int >( _faceRear, _numRear );
  //assert (!(_h == null)) ;
  //return pair < const hasFace4 *, int > (_h.first,_h.second) ; ;
}

inline Gitter :: Geometric :: hface4 :: 
hface4 (myhedge1_t * e0, int s0, myhedge1_t * e1, int s1, myhedge1_t * e2, int s2, myhedge1_t * e3, int s3) 
   //:  _parRule(Hface4Rule::undefined)
{
  assert(e0 && e1 && e2 && e3) ;
  (e [0] = e0)->ref ++ ; nb.s [0] = s0 ;
  (e [1] = e1)->ref ++ ; nb.s [1] = s1 ;
  (e [2] = e2)->ref ++ ; nb.s [2] = s2 ;
  (e [3] = e3)->ref ++ ; nb.s [3] = s3 ;
  nb._parRule = (Hface4Rule::undefined);
  return ;
}

inline Gitter :: Geometric :: hface4 :: ~hface4 () {
  assert (ref ? (cerr << "**WARNING hface4::refcount was " << ref << endl, 1) : 1) ;
  e [0] -> ref -- ;
  e [1] -> ref -- ;
  e [2] -> ref -- ;
  e [3] -> ref -- ;
  return ;
}

inline void Gitter :: Geometric :: hface4 :: attachElement (const pair < myconnect_t *, int > & p, int t) 
{
  if( t < 0 )
    nb.setRear( p );
  else
    nb.setFront( p );
  ref ++ ;
  return ;
}

inline void Gitter :: Geometric :: hface4 :: detachElement (int t)
{
  if( t < 0 )
    nb.setRear( nb.null );
  else
    nb.setFront( nb.null );
  ref -- ;
  return ;
}

inline int Gitter :: Geometric :: hface4 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface4 :: preCoarsening () {
  return 0 ;
}

inline int Gitter :: Geometric :: hface4 :: twist (int i) const {
  assert (i < 4) ;
  return nb.s [i] ;
}

inline Gitter :: Geometric :: hface4 :: myhedge1_t * Gitter :: Geometric :: hface4 :: myhedge1 (int i) {
  assert (i < 4) ;
  return e [i] ;
}

inline const Gitter :: Geometric :: hface4 :: myhedge1_t * Gitter :: Geometric :: hface4 :: myhedge1 (int i) const {
  assert (i < 4) ;
  return e [i] ;
}

inline Gitter :: Geometric :: hface4 :: myvertex_t * Gitter :: Geometric :: hface4 :: myvertex (int i) {
  assert(0<=i && i < 4) ;
  return myhedge1 (i)->myvertex (nb.s[i]) ;
}

inline const Gitter :: Geometric :: hface4 :: myvertex_t * Gitter :: Geometric :: hface4 :: myvertex (int i) const {
  assert(0<=i && i < 4) ;
  return myhedge1 (i)->myvertex (nb.s[i]) ;
}

inline Gitter::Geometric::hface4::myrule_t
Gitter::Geometric::hface4::parentRule() const {
  return nb._parRule;
}

inline bool 
Gitter::Geometric::hface4::isInteriorLeaf() const 
{
  const myconnect_t & nbRear  = *(nb.rear().first);
  const myconnect_t & nbFront = *(nb.front().first);
  
  if (nbFront.isboundary())
  {
    return ( nbRear.nbLeaf() && 
             nbRear.nbLevel() == this->level());
  }
  else if (nbRear.isboundary())
  {
    return ( nbFront.nbLeaf() && 
             nbFront.nbLevel() == this->level());
  }
  else
  {
    return (nbRear.nbLeaf() || nbFront.nbLeaf());
  }
}


// #######                                 ######
//    #     ######   #####  #####     ##   #     #  #    #  #       ######
//    #     #          #    #    #   #  #  #     #  #    #  #       #
//    #     #####      #    #    #  #    # ######   #    #  #       #####
//    #     #          #    #####   ###### #   #    #    #  #       #
//    #     #          #    #   #   #    # #    #   #    #  #       #
//    #     ######     #    #    #  #    # #     #   ####   ######  ######

inline Gitter :: Geometric :: TetraRule :: TetraRule ( const rule_t &r )
: _r( r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: TetraRule :: TetraRule ( const int r )
: _r( (rule_t) r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: TetraRule :: TetraRule ( const rule_enum &r )
: _r( r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: TetraRule :: operator rule_t () const
{
  return int( _r );
}

inline bool Gitter :: Geometric :: TetraRule :: isValid (const rule_t& r) {
  return r == crs || r == nosplit || r == iso8 || r == e01 
    || r == e12 || r == e20 || r == e23 || r == e30 || r == e31;
}

inline bool Gitter :: Geometric :: TetraRule :: isValid () const {
  return isValid( _r );
}

#if 0
inline ostream &operator<< ( ostream &out, const Gitter :: Geometric :: TetraRule &rule )
{
  switch( rule )
  {
    case Gitter :: Geometric :: TetraRule :: crs:
      return out << "coarsen";
    case Gitter :: Geometric :: TetraRule :: nosplit:
      return out << "nosplit";
    case Gitter :: Geometric :: TetraRule :: e01:
      return out << "e01";
    case Gitter :: Geometric :: TetraRule :: e12:
      return out << "e12";
    case Gitter :: Geometric :: TetraRule :: e20:
      return out << "e20";
    case Gitter :: Geometric :: TetraRule :: e23:
      return out << "e23";
    case Gitter :: Geometric :: TetraRule :: e30:
      return out << "e30";
    case Gitter :: Geometric :: TetraRule :: e31:
      return out << "e31";
    case Gitter :: Geometric :: TetraRule :: iso8:
      return out << "iso8";
    default:
      return out << "!!! unknown !!!";
  }
}
#endif


// #######
//    #     ######   #####  #####     ##
//    #     #          #    #    #   #  #
//    #     #####      #    #    #  #    #
//    #     #          #    #####   ######
//    #     #          #    #   #   #    #
//    #     ######     #    #    #  #    #

inline Gitter :: Geometric :: Tetra :: 
Tetra (myhface3_t * f0, int t0, myhface3_t * f1, int t1, 
       myhface3_t * f2, int t2, myhface3_t * f3, int t3) 
{
  (f [0] = f0)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 1),(s [1] = t1)) ;
  (f [2] = f2)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 2),(s [2] = t2)) ;
  (f [3] = f3)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 3),(s [3] = t3)) ;
  return ;
}

inline Gitter :: Geometric :: Tetra :: ~Tetra () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  f [2] ->detachElement (s [2]) ;
  f [3] ->detachElement (s [3]) ;
  return ;
}

inline int Gitter :: Geometric :: Tetra :: twist (int i) const {
  assert (i < 4) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Tetra :: myhface3_t * Gitter :: Geometric :: Tetra :: myhface3 (int i) {
  assert (i < 4) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Tetra :: myhface3_t * Gitter :: Geometric :: Tetra :: myhface3 (int i) const {
  assert (i < 4) ;
  return f [i] ;
}

inline int Gitter::Geometric::Tetra::originalVertexTwist(int face, int vertex) const {
  return (twist(face) < 0 ? 
          (7 - vertex + twist(face)) % 3 : 
          (vertex + twist(face)) % 3);
}

inline int Gitter::Geometric::Tetra::evalVertexTwist(int face, int vertex) const {
  // make sure vertex and face are in range is 
  assert( (twist(face) + 3 >= 0) && (twist(face)+3 < 6) );
  assert( vertex >= 0 && vertex < 3 );
  // make sure that we get the same result 
  assert( originalVertexTwist(face,vertex) == vertexTwist[twist(face)+3][vertex] );
  return vertexTwist[twist(face)+3][vertex];  
}

inline int Gitter::Geometric::Tetra::originalEdgeTwist(int face, int vertex) const 
{
  return (twist(face) < 0 ? 
          (6 - vertex + twist(face)) % 3 : 
          (vertex + twist(face)) % 3);
}

inline int Gitter::Geometric::Tetra::evalEdgeTwist(int face, int vertex) const 
{
  // make sure vertex and face are in range is 
  assert( (twist(face) + 3 >= 0) && (twist(face)+3 < 6) );
  assert( vertex >= 0 && vertex < 3 );
  // make sure that we get the same result 
  assert( originalEdgeTwist(face,vertex) == edgeTwist[twist(face)+3][vertex]);
  return edgeTwist[twist(face)+3][vertex];
}

inline Gitter :: Geometric :: Tetra :: myhedge1_t * Gitter :: Geometric :: Tetra :: myhedge1 (int edge) 
{
  assert(edge >= 0 && edge < 6);

  typedef Gitter::Geometric::Tetra ThisType;

  return myhface3(ThisType::edgeMap[edge][0])->
    myhedge1(evalEdgeTwist(ThisType::edgeMap[edge][0],ThisType::edgeMap[edge][1]));
}

inline const Gitter :: Geometric :: Tetra :: myhedge1_t * Gitter :: Geometric :: Tetra :: myhedge1 (int edge) const
{
  assert(edge >= 0 && edge < 6);

  typedef Gitter::Geometric::Tetra ThisType;

  return myhface3(ThisType::edgeMap[edge][0])->
    myhedge1(evalEdgeTwist(ThisType::edgeMap[edge][0],ThisType::edgeMap[edge][1]));

}

inline Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i, int j) {
  return myhface3(i)->myvertex(evalVertexTwist(i, j));
}

inline const Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i, int j) const {
  return myhface3(i)->myvertex(evalVertexTwist(i, j));
}

inline Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i) {
  assert (0 <= i && i < 4) ;
  return (i < 3) ? myvertex (3,i) : myvertex (2,1) ;
}

inline const Gitter :: Geometric :: Tetra :: myvertex_t * Gitter :: Geometric :: Tetra :: myvertex (int i) const {
  assert (0 <= i && i < 4) ;
  return (i < 3) ? myvertex (3,i) : myvertex (2,1) ;
}

inline pair < Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Tetra :: myneighbour (int i) 
{
  return twist (i) < 0 ? myhface3 (i)->nb.front () : myhface3 (i)->nb.rear ();
}

inline pair < const Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Tetra :: myneighbour (int i) const {
  return twist (i) < 0 ? pair < const hasFace3 *, int > (myhface3 (i)->nb.front ().first, myhface3 (i)->nb.front ().second)
    : pair < const hasFace3 *, int > (myhface3 (i)->nb.rear ().first, myhface3 (i)->nb.rear ().second) ;
}

inline pair < Gitter :: Geometric :: hface3_GEO *, int > Gitter :: Geometric :: Tetra :: myintersection (int i) 
{
  // return pair, first = pointer to face, second = twist of face
  return pair< Gitter::Geometric::hface3_GEO *,int> (myhface3 (i) ,twist (i));
}

inline pair < const Gitter :: Geometric :: hface3_GEO *, int > Gitter :: Geometric :: Tetra :: myintersection (int i) const
{
  // return pair, first = pointer to face, second = twist of face
  return  pair< const Gitter::Geometric::hface3_GEO * , int > (myhface3 (i) , twist (i) );
}

inline int Gitter :: Geometric :: Tetra :: postRefinement () {
  return 0;
}

inline int Gitter :: Geometric :: Tetra :: preCoarsening () {
  return 0; 
}

// ######                                                           #####
// #     #  ######  #####      #     ####   #####      #     ####  #     #
// #     #  #       #    #     #    #    #  #    #     #    #    #       #
// ######   #####   #    #     #    #    #  #    #     #    #       #####
// #        #       #####      #    #    #  #    #     #    #            #
// #        #       #   #      #    #    #  #    #     #    #    # #     #
// #        ######  #    #     #     ####   #####      #     ####   #####

inline Gitter :: Geometric :: Periodic3 :: 
Periodic3 (myhface3_t * f0, int t0, myhface3_t * f1, int t1) 
{
  (f [0] = f0)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this), 1),(s [1] = t1)) ;
  return ;
}

inline Gitter :: Geometric :: Periodic3 :: ~Periodic3 () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  return ;
}

inline int Gitter :: Geometric :: Periodic3 :: twist (int i) const {
  assert (0 <= i && i < 2) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Periodic3 :: myhface3_t * Gitter :: Geometric :: Periodic3 :: myhface3 (int i) {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Periodic3 :: myhface3_t * Gitter :: Geometric :: Periodic3 :: myhface3 (int i) const {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i, int j) {
  assert (0 <= i && i < 2) ;
  return (twist(i) < 0) ? myhface3(i)->myvertex((7 - j + twist(i)) % 3) : myhface3(i)->myvertex((j + twist(i)) % 3) ;
}

inline const Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i, int j) const {
  return (twist(i) < 0) ? myhface3(i)->myvertex((7 - j + twist(i)) % 3) : myhface3(i)->myvertex((j + twist(i)) % 3) ;
}

inline Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i) {
  assert (0 <= i && i < 6) ;
  
  // Der Ausdruck liefert 0-> (0,0)
  //      1-> (0,1)
  //      2-> (0,2)
  //      3-> (1,0)
  //      4-> (1,2)
  //      5-> (1,1)
  return (i < 3) ? myvertex (0,i) : myvertex (1,(6-i)%3) ;
}

inline const Gitter :: Geometric :: Periodic3 :: myvertex_t * Gitter :: Geometric :: Periodic3 :: myvertex (int i) const {
  assert (0 <= i && i < 6) ;
  return (i < 3) ? myvertex (0,i) : myvertex (1,(6-i)%3) ;
}

inline pair < Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Periodic3 :: myneighbour (int i) {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? myhface3 (i)->nb.front () : myhface3 (i)->nb.rear () ;
}

inline pair < const Gitter :: Geometric :: hasFace3 *, int > Gitter :: Geometric :: Periodic3 :: myneighbour (int i) const {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? pair < const hasFace3 *, int > (myhface3 (i)->nb.front ().first, myhface3 (i)->nb.front ().second)
    : pair < const hasFace3 *, int > (myhface3 (i)->nb.rear ().first, myhface3 (i)->nb.rear ().second) ;
}

inline int Gitter :: Geometric :: Periodic3 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: Periodic3 :: preCoarsening () {
  return 0 ;
}

// Anfang - Neu am 23.5.02 (BS)

// ######                                                          #
// #     #  ######  #####      #     ####   #####      #     ####  #    #
// #     #  #       #    #     #    #    #  #    #     #    #    # #    #
// ######   #####   #    #     #    #    #  #    #     #    #      #    #
// #        #       #####      #    #    #  #    #     #    #      #######
// #        #       #   #      #    #    #  #    #     #    #    #      #
// #        ######  #    #     #     ####   #####      #     ####       #

inline Gitter :: Geometric :: Periodic4 :: 
Periodic4 (myhface4_t * f0, int t0, myhface4_t * f1, int t1) 
{
  (f [0] = f0)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 1),(s [1] = t1)) ;
  return ;
}

inline Gitter :: Geometric :: Periodic4 :: ~Periodic4 () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  return ;
}

inline int Gitter :: Geometric :: Periodic4 :: twist (int i) const {
  assert (0 <= i && i < 2) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Periodic4 :: myhface4_t * Gitter :: Geometric :: Periodic4 :: myhface4 (int i) {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Periodic4 :: myhface4_t * Gitter :: Geometric :: Periodic4 :: myhface4 (int i) const {
  assert (0 <= i && i < 2) ;
  return f [i] ;
}

inline Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i, int j) {
  assert (0 <= i && i < 2) ;
  return (twist(i) < 0) ? myhface4(i)->myvertex((9 - j + twist(i)) % 4) : myhface4(i)->myvertex((j + twist(i)) % 4) ;
}

inline const Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i, int j) const {
  assert (0 <= i && i < 2) ;
  return (twist(i) < 0) ? myhface4(i)->myvertex((9 - j + twist(i)) % 4) : myhface4(i)->myvertex((j + twist(i)) % 4) ;
}

inline Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i) { // ok
  assert (0 <= i && i < 8) ;
  return (i < 4) ? myvertex (0, (4 - i) % 4) : myvertex (1, i - 4) ;
}

inline const Gitter :: Geometric :: Periodic4 :: myvertex_t * Gitter :: Geometric :: Periodic4 :: myvertex (int i) const { // ok
  assert (0 <= i && i < 8) ;
  return (i < 4) ? myvertex (0,i) : myvertex (1,(8-i)%4) ;
}

inline pair < Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Periodic4 :: myneighbour (int i) {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? myhface4 (i)->nb.front () : myhface4 (i)->nb.rear () ;
}

inline pair < const Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Periodic4 :: myneighbour (int i) const {
  assert (0 <= i && i < 2) ;
  return twist (i) < 0 ? pair < const hasFace4 *, int > (myhface4 (i)->nb.front ().first, myhface4 (i)->nb.front ().second)
    : pair < const hasFace4 *, int > (myhface4 (i)->nb.rear ().first, myhface4 (i)->nb.rear ().second) ;
}

inline int Gitter :: Geometric :: Periodic4 :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: Periodic4 :: preCoarsening () {
  return 0 ;
}

// Ende - Neu am 23.5.02 (BS)

// #     #                         ######
// #     #  ######  #    #    ##   #     #  #    #  #       ######
// #     #  #        #  #    #  #  #     #  #    #  #       #
// #######  #####     ##    #    # ######   #    #  #       #####
// #     #  #         ##    ###### #   #    #    #  #       #
// #     #  #        #  #   #    # #    #   #    #  #       #
// #     #  ######  #    #  #    # #     #   ####   ######  ######

inline Gitter :: Geometric :: HexaRule :: HexaRule ( const rule_t &r )
: _r( r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: HexaRule :: HexaRule ( const int r )
: _r( (rule_t) r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: HexaRule :: HexaRule ( const rule_enum &r )
: _r( r )
{
  assert( isValid() );
}

inline Gitter :: Geometric :: HexaRule :: operator rule_t () const
{
  return int( _r );
}

inline bool Gitter :: Geometric :: HexaRule :: isValid (const rule_t& r) {
  return r == crs || r == nosplit || r == iso8 ;
}

inline bool Gitter :: Geometric :: HexaRule :: isValid () const {
  return isValid( _r );
}

#if 0
inline ostream &operator<< ( ostream &out, const Gitter :: Geometric :: HexaRule &rule )
{
  switch( rule )
  {
    case Gitter :: Geometric :: HexaRule :: crs:
      return out << "coarsen";
    case Gitter :: Geometric :: HexaRule :: nosplit:
      return out << "nosplit";
    case Gitter :: Geometric :: HexaRule :: iso8:
      return out << "iso8";
    default:
      return out << "!!! unknown !!!";
  }
}
#endif

// #     #
// #     #  ######  #    #    ##
// #     #  #        #  #    #  #
// #######  #####     ##    #    #
// #     #  #         ##    ######
// #     #  #        #  #   #    #
// #     #  ######  #    #  #    #

inline Gitter :: Geometric :: Hexa :: 
Hexa (myhface4_t * f0, int t0, myhface4_t * f1, int t1,
      myhface4_t * f2, int t2, myhface4_t * f3, int t3, 
      myhface4_t * f4, int t4, myhface4_t * f5, int t5) 
{
  (f [0] = f0)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 0),(s [0] = t0)) ;
  (f [1] = f1)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 1),(s [1] = t1)) ;
  (f [2] = f2)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 2),(s [2] = t2)) ;
  (f [3] = f3)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 3),(s [3] = t3)) ;
  (f [4] = f4)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 4),(s [4] = t4)) ;
  (f [5] = f5)->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this), 5),(s [5] = t5)) ;
  return ;
}

inline Gitter :: Geometric :: Hexa :: ~Hexa () {
  f [0] ->detachElement (s [0]) ;
  f [1] ->detachElement (s [1]) ;
  f [2] ->detachElement (s [2]) ;
  f [3] ->detachElement (s [3]) ;
  f [4] ->detachElement (s [4]) ;
  f [5] ->detachElement (s [5]) ;
  return ;
}

inline int Gitter :: Geometric :: Hexa :: twist (int i) const {
  assert (i < 6) ;
  return s [i] ; 
}

inline Gitter :: Geometric :: Hexa :: myhface4_t * Gitter :: Geometric :: Hexa :: myhface4 (int i) {
  assert (i < 6) ;
  return f [i] ;
}

inline const Gitter :: Geometric :: Hexa :: myhface4_t * Gitter :: Geometric :: Hexa :: myhface4 (int i) const {
  assert (i < 6) ;
  return f [i] ;
}

inline Gitter :: Geometric :: Hexa :: myvertex_t * Gitter :: Geometric :: Hexa :: myvertex (int i, int j) 
{
  return myhface4(i)->myvertex(evalVertexTwist(i, j));
}

inline const Gitter :: Geometric :: Hexa :: myvertex_t * Gitter :: Geometric :: Hexa :: myvertex (int i, int j) const 
{
  return myhface4(i)->myvertex(evalVertexTwist(i, j));
}

inline Gitter :: Geometric :: Hexa :: myvertex_t * 
Gitter :: Geometric :: Hexa :: myvertex (int i) 
{
  assert (0 <= i && i < 8) ;
  return myvertex( vertex2Face[i][0] , vertex2Face[i][1] );
}

inline const Gitter :: Geometric :: Hexa :: myvertex_t * 
Gitter :: Geometric :: Hexa :: myvertex (int i) const 
{
  assert (0 <= i && i < 8) ;
  return myvertex( vertex2Face[i][0] , vertex2Face[i][1] );
}

inline Gitter :: Geometric :: Hexa :: myhedge1_t * Gitter :: Geometric :: Hexa :: myhedge1(int i) {
  assert (0 <= i && i < 12);

  typedef Gitter::Geometric::Hexa MyType;
  return myhface4(MyType::edgeMap[i][0])->
    myhedge1(evalEdgeTwist(MyType::edgeMap[i][0], MyType::edgeMap[i][1]));
}

inline const Gitter :: Geometric :: Hexa :: myhedge1_t * Gitter :: Geometric :: Hexa :: myhedge1(int i) const {
  assert (0 <= i && i < 12);

  typedef Gitter::Geometric::Hexa MyType;
  return myhface4(MyType::edgeMap[i][0])->
    myhedge1(evalEdgeTwist(MyType::edgeMap[i][0], MyType::edgeMap[i][1]));
}

inline pair < Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Hexa :: myneighbour (int i) {
  return twist (i) < 0 ? myhface4 (i)->nb.front () : myhface4 (i)->nb.rear () ;
}

inline pair < const Gitter :: Geometric :: hasFace4 *, int > Gitter :: Geometric :: Hexa :: myneighbour (int i) const {
  return twist (i) < 0 ? pair < const hasFace4 *, int > (myhface4 (i)->nb.front ().first, myhface4 (i)->nb.front ().second)
    : pair < const hasFace4 *, int > (myhface4 (i)->nb.rear ().first, myhface4 (i)->nb.rear ().second) ;
}

inline pair<Gitter::Geometric::hface4_GEO*, int> 
Gitter::Geometric::Hexa::myintersection(int i) {
  return make_pair(myhface4(i), twist(i));
}

inline pair<const Gitter::Geometric::hface4_GEO*, int> 
Gitter::Geometric::Hexa::myintersection(int i) const {
  return make_pair(myhface4(i), twist(i));
}

inline int Gitter :: Geometric :: Hexa :: postRefinement () {
  return 0 ;
}

inline int Gitter :: Geometric :: Hexa :: preCoarsening () {
  return 0 ;
}

inline int Gitter :: Geometric :: Hexa :: evalVertexTwist (int face, int vertex) const 
{
  assert( (twist(face) + 4 >= 0) && (twist(face)+4 < 8) );
  assert( vertex >= 0 && vertex < 4 );
  // make sure that we get the same result 
  assert( originalVertexTwist(face,vertex) == vertexTwist[twist(face)+4][vertex] );
  return vertexTwist[twist(face)+4][vertex];
}

inline int Gitter :: Geometric :: Hexa :: evalEdgeTwist (int face, int edge) const 
{
  assert( (twist(face) + 4 >= 0) && (twist(face)+4 < 8) );
  assert( edge >= 0 && edge < 4 );
  // make sure that we get the same result 
  assert( originalEdgeTwist(face,edge) == edgeTwist[twist(face)+4][edge] );
  return edgeTwist[twist(face)+4][edge];
}


inline int Gitter :: Geometric :: Hexa :: originalVertexTwist (int face, int vertex) const {
  return (twist(face) < 0 ? 
          (9 - vertex + twist(face)) % 4 :
          (vertex + twist(face)) % 4);
}

inline int Gitter :: Geometric :: Hexa :: originalEdgeTwist (int face, int edge) const {
  return (twist(face) < 0 ? 
          (8 - edge + twist(face)) % 4 :
          (edge + twist(face)) % 4);
}


// #     #                                                  #####
// #     #  #####   #    #  #####    ####   ######   ####  #     #
// #     #  #    #  ##   #  #    #  #       #       #    #       #
// #######  #####   # #  #  #    #   ####   #####   #       #####
// #     #  #    #  #  # #  #    #       #  #       #  ###       #
// #     #  #    #  #   ##  #    #  #    #  #       #    # #     #
// #     #  #####   #    #  #####    ####   ######   ####   #####

inline Gitter :: Geometric :: hbndseg3 :: 
hbndseg3 (myhface3_t * a, int b) 
  : _face( a ), 
    _twist (b)
{
  _face->attachElement (pair < hasFace3 *, int > (InternalHasFace3 ()(this),0), _twist) ;
  return ;
}

inline Gitter :: Geometric :: hbndseg3 :: ~hbndseg3 () {
  _face->detachElement (_twist) ;
  return ;
}

inline void Gitter :: Geometric :: hbndseg3 :: attachleafs () 
{
  this->addleaf();
  
  myhface3_t & face = *(myhface3(0));
  face.addleaf();
  for (int i=0; i<3; ++i) 
  {
    face.myhedge1(i)->addleaf();
    face.myvertex(i)->addleaf();
  }
}

inline void Gitter :: Geometric :: hbndseg3 :: detachleafs () 
{
  this->removeleaf();

  myhface3_t & face = *(myhface3(0));
  face.removeleaf();
  for (int i=0; i<3; ++i) 
  {
    face.myhedge1(i)->removeleaf();
    face.myvertex(i)->removeleaf();
  }
}

inline int Gitter :: Geometric :: hbndseg3 :: postRefinement () 
{
  ProjectVertexPair pv( projection(), segmentIndex() );
  if ( pv.first ) 
  {
    myhface3(0)->projectVertex( pv );
  }
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg3 :: preCoarsening () 
{
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg3 :: twist (int i) const {
  assert (i == 0) ;
  return _twist ;
}

inline Gitter :: Geometric :: hbndseg3 :: myhface3_t * Gitter :: Geometric :: hbndseg3 :: myhface3 (int i) const {
  assert (i == 0) ;
  return _face ;
}

inline Gitter :: Geometric :: hbndseg3 :: myvertex_t * Gitter :: Geometric :: hbndseg3 :: myvertex (int,int j) const {
  return (twist (0) < 0) ? myhface3 (0)->myvertex ((7 - j + twist (0)) % 3) : myhface3 (0)->myvertex ((j + twist (0)) % 3) ;
}

inline Gitter :: Geometric :: hbndseg3 :: myhface3_t * Gitter :: Geometric :: hbndseg3 :: subface3 (int,int i) const {
  return myhface3 (0)->subface3 (i) ;
}

inline Gitter :: Geometric :: hbndseg3 :: myrule_t Gitter :: Geometric :: hbndseg3 :: getrule () const {
  return myhface3 (0)->getrule () ;
}

inline int Gitter :: Geometric :: hbndseg3 :: nChild () const {
  assert(_face);
  return _face->nChild () ;
}


// #     #                                                 #
// #     #  #####   #    #  #####    ####   ######   ####  #    #
// #     #  #    #  ##   #  #    #  #       #       #    # #    #
// #######  #####   # #  #  #    #   ####   #####   #      #    #
// #     #  #    #  #  # #  #    #       #  #       #  ### #######
// #     #  #    #  #   ##  #    #  #    #  #       #    #      #
// #     #  #####   #    #  #####    ####   ######   ####       #

inline Gitter :: Geometric :: hbndseg4 :: hbndseg4 (myhface4_t * a, int b) 
  : _face( a ),
    _twist (b) 
{
  _face->attachElement (pair < hasFace4 *, int > (InternalHasFace4 ()(this),0), _twist) ;
  return ;
}

inline Gitter :: Geometric :: hbndseg4 :: ~hbndseg4 () {
  _face->detachElement (_twist) ;
  return ;
}

inline void Gitter :: Geometric :: hbndseg4 :: attachleafs () 
{
  assert(this->leafRefCount()==0);
  this->addleaf();
  
  hface4_GEO & face = *(myhface4(0));
  face.addleaf();
  for (int i=0; i<4; ++i) 
  {
    face.myhedge1(i)->addleaf();
    face.myvertex(i)->addleaf();
  }
}

inline void Gitter :: Geometric :: hbndseg4 :: detachleafs () 
{
  assert(this->leafRefCount()==1);
  this->removeleaf();

  hface4_GEO & face = *(myhface4(0));
  face.removeleaf();
  for (int i=0; i<4; ++i) 
  {
    face.myhedge1(i)->removeleaf();
    face.myvertex(i)->removeleaf();
  }
}

inline int Gitter :: Geometric :: hbndseg4 :: postRefinement () 
{
  ProjectVertexPair pv( projection(), segmentIndex() );
  if( pv.first )
  {
    myhface4(0)->projectVertex( pv );
  }
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg4 :: preCoarsening () 
{
  return 0 ;
}

inline int Gitter :: Geometric :: hbndseg4 :: twist (int i) const 
{
  assert (i == 0) ;
  return _twist ;
}

inline Gitter :: Geometric :: hbndseg4 :: myhface4_t * Gitter :: Geometric :: hbndseg4 :: myhface4 (int i) const {
  assert (i == 0) ;
  return _face ;
}

inline Gitter :: Geometric :: hbndseg4 :: myvertex_t * Gitter :: Geometric :: hbndseg4 :: myvertex (int,int j) const {
  return (twist (0) < 0) ? myhface4 (0)->myvertex ((9 - j + twist (0)) % 4) : myhface4 (0)->myvertex ((j + twist (0)) % 4) ;
}

inline Gitter :: Geometric :: hbndseg4 :: myhface4_t * Gitter :: Geometric :: hbndseg4 :: subface4 (int,int i) const {
  return myhface4 (0)->subface4 (i) ;
}

inline Gitter :: Geometric :: hbndseg4 :: myrule_t Gitter :: Geometric :: hbndseg4 :: getrule () const {
  return myhface4 (0)->getrule () ;
}

inline int Gitter :: Geometric :: hbndseg4 :: nChild () const {
  assert(_face);
  return _face->nChild () ;
}


// #                                 ###
// #        ######    ##    ######    #      #####  ######  #####     ##     #####   ####   #####
// #        #        #  #   #         #        #    #       #    #   #  #      #    #    #  #    #
// #        #####   #    #  #####     #        #    #####   #    #  #    #     #    #    #  #    #
// #        #       ######  #         #        #    #       #####   ######     #    #    #  #####
// #        #       #    #  #         #        #    #       #   #   #    #     #    #    #  #   #
// #######  ######  #    #  #        ###       #    ######  #    #  #    #     #     ####   #    #

template < class A > inline LeafIterator < A > :: LeafIterator () : _grd (0), _w (0) {
  return ;
}

template < class A > inline LeafIterator < A > :: LeafIterator (Gitter & g) 
  : _grd (&g), _w (0) , _a(0) {
  _grd->ref ++ ;
  _w = _grd->iterator (_a) ;
  return ;
}

template < class A > inline LeafIterator < A > :: LeafIterator (const LeafIterator < A > & x) : _grd(0), _w(0), _a(0) 
{
  assign(x);
  return ;
}

template < class A > inline LeafIterator < A > & 
LeafIterator < A > :: operator = (const LeafIterator < A > & x) 
{
  removeObj();
  assign(x);
  return *this;
}

template < class A > inline LeafIterator < A > :: ~LeafIterator () {
  removeObj();
  return ;
}

template < class A > inline void LeafIterator < A > :: removeObj () 
{
  if (_grd) _grd->ref -- ;
  _grd = 0;
  if(_w) delete _w ;
  _w = 0;
}

template < class A > inline void LeafIterator < A > :: assign (const LeafIterator < A > & x)  
{
  assert( _grd == 0 );
  assert( _w   == 0 );
  _grd = x._grd; 
  _grd->ref ++ ;
  assert( x._w );
  _w = x._w->clone();
}

template < class A > inline IteratorSTI < A > * LeafIterator < A > :: operator -> () const {
  return _w ;
}

template < class A > inline IteratorSTI < A > & LeafIterator < A > :: operator * () const {
  return * _w ;
}

////////////////////////////////////////////////////////////////////////////////////
//
//  --GridIterator
//
///////////////////////////////////////////////////////////////////////////////////

template < class A , class StopRule_t > 
inline GridIterator < A , StopRule_t > :: GridIterator () : _grd (0), _w (0) {
  return ;
}

template < class A , class StopRule_t > 
inline GridIterator < A , StopRule_t > :: 
GridIterator (Gitter & g , const StopRule_t rule ) 
  : _grd (&g), _w (0) , _a(0) 
{
  _grd->ref ++ ;
  _w = _grd->createIterator(_a,rule); 
  return ;
}

template < class A , class StopRule_t > 
inline GridIterator < A , StopRule_t > :: GridIterator (const GridIterator < A , StopRule_t > & x) 
  : _grd(0), _w(0), _a(0) 
{
  assign(x);
  return ;
}

template < class A , class StopRule_t > 
inline GridIterator < A , StopRule_t > & 
GridIterator < A , StopRule_t > :: operator = (const GridIterator < A , StopRule_t > & x) 
{
  removeObj();
  assign(x);
  return *this;
}

template < class A , class StopRule_t > 
inline GridIterator < A , StopRule_t > :: ~GridIterator () {
  removeObj();
  return ;
}

template < class A , class StopRule_t > 
inline void GridIterator < A , StopRule_t > :: removeObj () 
{
  if (_grd) 
  {
    _grd->ref -- ;
    _grd = 0;
  }
  if(_w) 
  { 
    delete _w ;
    _w = 0;
  }
}

template < class A , class StopRule_t > 
inline void GridIterator < A , StopRule_t > :: 
assign (const GridIterator < A , StopRule_t > & x)  
{
  assert( _grd == 0 );
  assert( _w   == 0 );
  _grd = x._grd; 
  _grd->ref ++ ;
  assert( x._w );
  _w = x._w->clone();
}

template < class A , class StopRule_t > 
inline IteratorSTI < A > * 
GridIterator < A , StopRule_t > :: operator -> () const {
  return _w ;
}

template < class A , class StopRule_t > 
inline IteratorSTI < A > & 
GridIterator < A , StopRule_t > :: operator * () const {
  return * _w ;
}

//////////////////////////////////////////////////////////////////////////////
//
//  --LevelIterator 
//
//////////////////////////////////////////////////////////////////////////////

template < class A > inline LevelIterator < A > :: LevelIterator () : _grd (0), _w (0) {
  return ;
}

template < class A > inline LevelIterator < A > :: LevelIterator (Gitter & g , int l ) : _grd (&g), _ahl (l) , _w (0) , _a(0)
{
  _grd->ref ++ ;
  _w = _grd->levelIterator (_a,_ahl) ;
  return ;
}

template < class A > inline LevelIterator < A > :: LevelIterator (const LevelIterator < A > & x) 
: _grd (0), _ahl(x._ahl), _w (0) , _a(0) 
{
  assign(x);
  return ;
}

template < class A > inline LevelIterator < A > & 
LevelIterator < A > :: operator = (const LevelIterator < A > & x) 
{
  removeObj();
  assign(x);
  return *this;
}


template < class A > inline LevelIterator < A > :: ~LevelIterator () {
  removeObj();
  return ;
}

template < class A > inline IteratorSTI < A > * LevelIterator < A > :: operator -> () const {
  return _w ;
}

template < class A > inline IteratorSTI < A > & LevelIterator < A > :: operator * () const {
  return * _w ;
}

template < class A > inline void LevelIterator < A > :: removeObj () 
{
  if (_grd) _grd->ref -- ;
  _grd = 0;
  if(_w) delete _w ;
  _w = 0;
}

template < class A > inline void LevelIterator < A > :: assign (const LevelIterator < A > & x)  
{
  assert( _grd == 0 );
  assert( _w == 0 );
  _grd = x._grd; 
  _grd->ref ++ ;
  assert( x._w );
  _w = x._w->clone();
}

#endif  // GITTER_STI_H_INCLUDED
