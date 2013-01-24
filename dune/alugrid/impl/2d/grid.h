#ifndef __HEADER__GRID
#define __HEADER__GRID

#include <cassert>
#include <cmath>
#include <iostream>
#include <stack>
#include <vector>

//#define PERIODIC_VERTICES

/***************************************************
// #begin(header)
// #filename:
//   grid.h
// #description:
//   Basisklassen f"ur Punkte, Gitterelemente und 
//   Hierarchien. 
// #classes:
//   template < class A > class Listagent
//   class Restrict_basic
//   class Basic
//   template < int N > class Vertex : public Listagent < Vertex < N > >, public Basic
//   template < int N > class Fullvertex : public Vertex < N >
//   template < int N > class Thinelement : public Basic
//   class Refco
//   class Refco_el : private Refco
//   template < int N > class Element : public Thinelement < N >, public Refco_el
//   template < class A > class Hier : public A
//   template < int N > class Bndel : public Thinelement < N >, protected Refco
// #end(header)
***************************************************/

#include "vtx_btree.h"

namespace ALU2DGrid
{

// #define NCOORD 2
// #define NVTX 4

//****************************************************************

struct IndexProvider;
template <int,int>
class Hmesh_basic;
template <int,int>
class Hmesh;

class Basic {

  private :
    Basic(const Basic & );

    Basic & operator=(const Basic &);

  protected :

    Basic() : _idx(-1), refcount(0), _level(0), childNr_(0), numchild(0) {}
    virtual ~Basic() { assert(!refcount); }

    int _idx;
    unsigned char refcount; // 16 byte + 8 vtable
    // The macro level is -1, because new vertices always get the level of the
    // edge they split.
    signed char _level; 
    unsigned char childNr_;
    unsigned char numchild;

    int &setIndex () { return _idx; }

  public:
    virtual void sethdl(IndexProvider *phdl) = 0; 

    inline int getIndex () const { return _idx; }

    enum { nparts = 4, max_points = 4 };

  protected:  
    void attach() { refcount ++; }

    void detach() { refcount --; }

    int isfree() const { return refcount == 0; }

  public:  
    virtual void write( std::ostream & ) const
    { 
      std::cerr << "ERROR: method Basic::write( ostream& ) not overloaded! " << std::endl;
      abort(); 
    }

    virtual void read ( std::istream & ) 
    { 
      std::cerr << "ERROR: method Basic::read( istream& ) not overloaded! " << std::endl;
      abort(); 
    }

    template <int,int>
    friend class Hmesh;
};

template < int N > class Vertex;
template < int N, int NV > class Multivertexadapter;
template < class A > class Listagency;
template <class A> class Hier;

/***************************************************
// #begin(class)
// #description:
// #definition: 
****************************************************/
template < class A > class Listagent : public Basic {

  A * next_list;
  
  A * prev_list;

  int number_list;  // 24 byte 
  
  Listagent(const Listagent &) { }

  Listagent & operator=(const Listagent &) { }

  protected :
  
    Listagent() : next_list(0) 
                , prev_list(0)
                { }
     
  public :
  
    virtual ~Listagent() { }
  
    A * next() const { return next_list; }
    
    A * prev() const { return prev_list; }

    int number() const { return number_list; }
          
  friend class Listagency < A >;
  
};
// #end(class)
// ***************************************************


template < int N, int NV > class Element;
template < int N, int NV > class Bndel;

template < int N, int NV > struct AdaptRestrictProlong2d
{
  typedef Hier < Element < N,NV > > helement_t;

  virtual ~AdaptRestrictProlong2d () {}
  virtual int preCoarsening ( helement_t &elem ) = 0;
  virtual int postRefinement ( helement_t  &elem ) = 0;
};

template < int N, int NV > struct Restrict_basic
{
  typedef Hier < Element < N, NV > > helement_t;
  typedef Hier < Bndel < N, NV > > hbndel_t;

  virtual ~Restrict_basic () {}
  virtual void operator ()(helement_t *) = 0;
  virtual void operator ()(hbndel_t *) = 0;
};

template < int N, int NV > struct Prolong_basic
{
  typedef Hier < Element < N, NV > > helement_t;
  typedef Hier < Bndel < N, NV > > hbndel_t;

  virtual ~Prolong_basic () {}
  virtual void operator ()(helement_t *) = 0;
  virtual void operator ()(hbndel_t *) = 0;
};

template < int N, int NV > struct nconf_vtx
{
  typedef Vertex < N > vertex_t;
  typedef Hier < Element < N, NV > > helement_t;

  vertex_t *vtx;
  helement_t *el[2];
  nconf_vtx(vertex_t *v, helement_t *e1, helement_t *e2) {
    vtx=v;el[0]=e1;el[1]=e2;
  }
};

// ***************************************************


// ***************************************************
// #begin(class)
// #description:
//   Interfaceklasse f"ur Punkte
// #definition:
template < int N > class Vertex : public Listagent < Vertex < N > > // , public Basic 
{
  typedef Listagent < Vertex < N > >  BaseType;
  using Listagent < Vertex < N > > ::_idx;
  using Listagent < Vertex < N > > ::_level;
  public:
    enum { ncoord = N };

    using BaseType :: attach;
    using BaseType :: detach; 
    using BaseType :: isfree;

  private :

    Vertex & operator=(const Vertex &);
    
    Vertex(const Vertex &);

 protected :
    IndexProvider* hdl;

#ifdef PERIODIC_VERTICES 
    Vertex *pernb[3];
    int nr_of_pernbs;
#endif

  protected :
    explicit Vertex( int level = -1 ) 
    {
      _level = level;
#ifdef PERIODIC_VERTICES 
      nr_of_pernbs = 0;
      pernb[0] = (Vertex *)0;
      pernb[1] = (Vertex *)0;
      pernb[2] = (Vertex *)0;
#endif
    }

  public :
    virtual void sethdl(IndexProvider *phdl);
    IndexProvider* gethdl() { assert( hdl ); return hdl; }

    virtual ~Vertex(); 

   
    virtual const double (& coord() const )[ncoord] = 0;


    virtual void write ( std::ostream & ) const = 0;

#ifdef PERIODIC_VERTICES
    int get_nr_of_per_nbs() { return nr_of_pernbs; }
    int no_pernbs() { return nr_of_pernbs; }

    // see grid_imp.cc 
    void set_pernb(Vertex *pv);
    Vertex *get_pernb(int pidx);
#endif

    virtual void read ( std::istream & ) = 0;
 
    int level() const {  return _level;  }

    bool isMacro () const { return level() == -1; }
};
// #end(class)
// ***************************************************


// ***************************************************
// #begin(class)
// #description:
//  Punkte
// #definition:
template < int N >
class Fullvertex : public Vertex < N > {

  public:
    enum { ncoord = Vertex< N >::ncoord };

  private:
    double vcoord[ncoord];
  
    Fullvertex(const Fullvertex &);

    Fullvertex & operator = (const Fullvertex &);

  public :


    Fullvertex() {}

    Fullvertex(double (&)[ncoord],int );

   ~Fullvertex() { }
  
    const double (& coordTest() const )[ncoord] { return vcoord; }
    const double (& coord() const )[ncoord] { return vcoord; }

    void write ( std::ostream & ) const;

    void read ( std::istream & );

    friend std::ostream &operator<< ( std::ostream& os, const Fullvertex &fv )
    {
      os << "(" << fv.vcoord[0];
      for (int i=1;i<ncoord;++i) 
        os << "," << fv.vcoord[i];
      os << ")";
      return os;
    }
};

// #end(class)
// ***************************************************
class Edge
: public Basic
{
  virtual void sethdl(IndexProvider *phdl);
public:
  using Basic :: attach;
  using Basic :: detach; 
  using Basic :: isfree;

  explicit Edge(IndexProvider *phdl) { sethdl(phdl); }
  void freeIndex(IndexProvider* phdl);
  void write ( std::ostream & ) const;
  void read ( std::istream & );
};

template < int N, int NV > class Triang;
template < int N, int NV > class Bndel_triang;


// ***************************************************
// #begin(class)
// #description:
//   Basisklasse f"ur Verfeinerungsinformation
// #definition:
class Refco
: public Basic
{
  public :
    
  typedef enum { crs = -1 , ref = 1 , 
                 none = 0, crs_1 = -2, crs_2 = -3, crs_3 = -4, crs_4 = -5,
                 ref_1 = 2, ref_2 = 3, ref_3 = 4, ref_4 = 5, notref = 10,
                 quart=9, notcrs = -10  } tag_t;
      
    virtual ~Refco () {} 
   
    void clear(tag_t) {}
      
    int is(tag_t) const { return 0; }  // ist nie irgendein tag

    void mark(Refco::tag_t t) {}

    virtual void clearWas() {}    
    
  protected:
    virtual void writeToWas() {}
};
// #end(class)
// ***************************************************

// ***************************************************
// #begin(class)
// #description:
//   Verfeinerungsinformation
// #definition:
class Refco_el : public Refco
{
    Refco::tag_t tag, tag_last;
    
  public :

    Refco_el() : tag(none), tag_last(none) {}

    void clear(Refco::tag_t t = none) { tag = (t == tag ? none : tag); }
      
    void mark(Refco::tag_t t) { tag = t; }
      
    int is(Refco::tag_t t) const { return tag == t ? 1 : 0; }

    int wasRefined() const { return tag_last == ref ? 1 : 0; }    

    void clearWas() { tag_last = none; }
    
  protected:
    void writeToWas() { tag_last = ref; }
};
// #end(class)
// ***************************************************

// ***************************************************
// #begin(class)
// #description:
//   Interfaceklasse f"ur Elemente
// #definition:
template < int N, int NV > class Thinelement : public Refco_el {

  public :
    typedef enum { unsplit = 0, triang_bnd = -2, compatibility = 1,
                   triang_conf2 = 2, triang_quarter = 4 } splitrule_t;


    typedef enum { bndel_like, element_like } thintype_t;

    typedef Vertex < N > vertex_t;

    typedef Multivertexadapter < N, NV > multivertexadapter_t;
    typedef nconf_vtx < N, NV > nconf_vtx_t;

    typedef Prolong_basic < N, NV > prolong_basic_t;
    typedef Restrict_basic < N, NV > restrict_basic_t;

    typedef Triang < N, NV > triang_t;
    typedef Bndel_triang < N, NV > bndel_triang_t;

    enum { ncoord = vertex_t::ncoord };

  private :
  
    Thinelement(const Thinelement & );

    Thinelement & operator=(const Thinelement &);

  protected :
    splitrule_t mysplit;
    Thinelement() : mysplit( unsplit ) { }
    
    virtual ~Thinelement() { }

  public :
    using Refco_el :: read;
    
    virtual int thinis(thintype_t ) const = 0;

    splitrule_t splitrule() const { return mysplit; }
     
    int numfacevertices(int ) const { return 2; }
    
    virtual int facevertex(int , int ) const = 0;

    virtual vertex_t *vertex(int ) const = 0;
    
    inline vertex_t *vertex(int fce, int j) const { return vertex(facevertex(fce, j)); }

    virtual Thinelement * neighbour(int ) const = 0;

    virtual int opposite(int fce) const = 0;

    virtual Edge *edge(int fce) const = 0;

    virtual void nbconnect(int , Thinelement *, int ) = 0;

    virtual int segmentIndex() const = 0;

    virtual void write ( std::ostream & ) const = 0;

    virtual void read ( std::istream &, vertex_t **, const int ) = 0;

    virtual int get_splitpoint(int, double, double (&) [ncoord]) const = 0;

    virtual int split(void * (&) [nparts], Listagency < vertex_t > *,
                      multivertexadapter_t &,nconf_vtx_t *,splitrule_t,
          int,Refco::tag_t, prolong_basic_t *pro_el) = 0;

    virtual int docoarsen(nconf_vtx_t*,int, restrict_basic_t *rest_el) { return 1; }

    triang_t * nbel(const int l) const
    {
      Thinelement *el = neighbour(l);
      return (el->thinis(element_like)) ? ((triang_t *)el) : 0;
    }   

    bndel_triang_t * nbbnd(int l) const
    {
      Thinelement *el=neighbour(l);
      assert(el != NULL);
      return (el->thinis(bndel_like)) ? ((bndel_triang_t *)el) : 0;
    }   

};
// ***************************************************


// ***************************************************
// #begin(class)
// #description:
//   Elementinterface mit verfeinerungsinfo.
// #definition:

template<class T>
class Hier;

template < int N, int NV > class Element : public Thinelement < N, NV > {

  private :
             
    Element(const Element &);

    Element & operator = (const Element &);

  public :
    typedef Vertex < N > vertex_t;
    typedef Fullvertex < N > fullvertex_t;
    typedef Vtx_btree < N, NV > vtx_btree_t;

    typedef Thinelement < N, NV > thinelement_t;
    typedef typename thinelement_t::thintype_t thintype_t;

    typedef Triang < N, NV > triang_t;

    enum { ncoord = vertex_t::ncoord };

    virtual void sethdl(IndexProvider *phdl);
    IndexProvider* gethdl() const { return vertex(0)->gethdl(); }

    int thinis(thintype_t t) const { return t == thinelement_t::element_like; }

  protected:

    struct c {  //  die vertex und (thin-)element verbindungen

      enum {pv=2};

      vertex_t * vtx [NV];

      thinelement_t * nb [NV];

      Edge * edge [NV];

      mutable vtx_btree_t *hvtx[NV];

      signed char bck [NV];
      
      signed char normdir [NV];

      c();

     ~c();

      void set(vertex_t * v, int i) { (vtx[i] = v)->attach(); }

      void unset(int i) { vtx[i]->detach(); vtx[i] = 0; }

      void write ( std::ostream & ) const;

      int read ( std::istream &, vertex_t **, const int );

      int check();
    } connect;

    // use refcount for storage of nvertices (saves 8 bytes) 
    unsigned char& nvertices () { return refcount; }
    unsigned char nvertices () const { return refcount; }

    int nv() const { return (NV==3) ? 3 : nvertices(); }
    int mod(const int i) const { return i % nv(); }

    using thinelement_t::_idx;
    using thinelement_t::refcount; // used as nvertices 

    double _area;
    double _outernormal[NV][ncoord]; // NV * ncoord * 8 = ( z.B. 48 )

  public :
    typedef c connect_t;

    int numfaces() const { return nv(); }

    int numvertices() const { return nv(); }

    using thinelement_t::getIndex;
    using thinelement_t::splitrule;
 
    Element();
    virtual ~Element();

    int facevertex(int , int ) const;
      
    void edge_vtx(int e, vertex_t * (& ) [2] ) const;

    // this is not a boundary segment and thus return negtive value 
    int segmentIndex() const { return -1; }

    vertex_t * vertex(int ) const;

    fullvertex_t * getVertex(int ) const;

    vertex_t * vertex(int fce, int j) const { return vertex(facevertex(fce, j)); }
    
    thinelement_t * neighbour(int ) const;

    int opposite(int ) const;

    int edge_idx(int ) const;

    Edge *edge(int ) const;

    int normaldir(int ) const;

    void nbconnect(int , thinelement_t * , int );

    void edgeconnect(int, Edge *);

    void setnormdir(int , int );

    void setrefine(int );
  
    void init();

    int setrefine();

    template <class O>
    void setorientation ( std::vector< O > &str );
    void setorientation();
    void switchorientation(int a,int b);

    double sidelength(int pfce) const 
    {
      double normal[ ncoord ];
      outernormal( pfce , normal );
      double length  = 0;
      for (int k=0; k<ncoord; ++k)
        length += normal[k] * normal[k];
      return std::sqrt( length );
    }

    double area() const { return _area; }

    void outernormal(int ,double (& )[ncoord]) const;

    void dirnormal(int ,double (& )[ncoord]) const;

    int get_splitpoint(int, double, double (&) [ncoord]) const;
    int get_splitpoint(const double (&)[2], double (&) [ncoord]) const;

    void addhvtx(vertex_t *inv, thinelement_t *lnb, thinelement_t *rnb,int fce);

    bool hashvtx(const int fce) const { return connect.hvtx[fce]; }

    // return whether hanging node for given face exsits 
    bool hasHangingNode(const int fce) const 
    {
      return (connect.hvtx[fce] && connect.hvtx[fce]->head);
    }
    
    thinelement_t *getLeftIntersection(const int fce) {
      return connect.hvtx[fce]->head->leftElement();
    }
    
    thinelement_t *getRightIntersection(const int fce) {
      return connect.hvtx[fce]->head->rightElement();
    }
        
  private:
    void getAllNb ( typename vtx_btree_t::Node *node, std::stack< thinelement_t * > vec );
   
  public:
    void removehvtx ( int fce, vertex_t *vtx );

    int check();

    friend std::ostream &operator<< ( std::ostream &out, const Element &elem )
    {
      return out;
    }

    friend std::istream &operator>> ( std::istream &in, Element &elem )
    {
      return in;
    }
};
// #author:
// #end(class)
// ***************************************************

template <class T>
class SubtreeIterator;

// ***************************************************
// #begin(class)
// #description:
//   Hierachieklasse
// #definition:
template < class A > class Hier : public A {

  public :

  typedef typename A::vertex_t vertex_t;

  typedef typename A::multivertexadapter_t multivertexadapter_t;
  typedef typename A::nconf_vtx_t nconf_vtx_t;

  typedef typename A::prolong_basic_t prolong_basic_t;
  typedef typename A::restrict_basic_t restrict_basic_t;

  private :

  Hier * dwn;

  Hier * nxt;
  
  Hier * up;

  using A :: _level;
  using A :: childNr_; 
  using A :: numchild;

  protected :

  Hier() : dwn(0), nxt(0), up(0) 
  {
  }

  void deletesubtree() { delete dwn; dwn = 0; 
    //  this->check();
  }  // Wird f"ur konf. Dreiecke gebraucht

  public :

    virtual ~Hier();

    Hier * down() const { return dwn; }

    Hier * father() const { return up; }

    Hier * next() const { return nxt; }

    int leaf() const { return ! dwn; }

    // use level for Basic 
    int level() const { return _level; }
    // for assigment of level 
    signed char& lvl() { return _level; }

    int childNr() const { return childNr_; }

    int nchild() const { return numchild; }

    int count() const { return (nxt ? nxt->count() : 0) + (dwn ? dwn->count() : 1); }

    int count(int i) const { 

      return (nxt ? nxt->count(i) : 0) + 

             (level() == i ? 1 : (dwn ? dwn->count(i) : 0) );

    }

    SubtreeIterator<A> stIterator() {
      return SubtreeIterator<A>(this);
    }

    int deepestLevel();

    int coarse(nconf_vtx_t *ncv,int nconfDeg, restrict_basic_t *rest_el);

    int refine_leaf(Listagency < vertex_t > * a, 
        multivertexadapter_t * b ,nconf_vtx_t *ncv,
        int nconfDeg,Refco::tag_t default_ref,
        prolong_basic_t *pro_el);

    void clearAllWas() 
    {
      this->clearWas();
      if (nxt)
        nxt->clearAllWas();
      if (dwn)
        dwn->clearAllWas();
    }

    int refine(Listagency < vertex_t > * a, multivertexadapter_t * b,
         nconf_vtx_t *ncv,
         int nconfDeg,Refco::tag_t default_ref,prolong_basic_t *pro_el);

    using A :: write;
    using A :: read ;

    void write ( std::ostream & ) const {}
    void read ( std::istream & ) {}

};
// #end(class)
// ***************************************************

// ***************************************************
// #begin(class)
// #description:
//   Interfaceklasse f"ur Randelemente
// #definition:
template < int N, int NV > class Bndel : public Thinelement < N,NV > {

  public:
    typedef Vertex < N > vertex_t;
    typedef Thinelement < N, NV > thinelement_t;
    typedef typename thinelement_t::thintype_t thintype_t;

    typedef Element < N,NV > element_t;

    enum { ncoord = vertex_t::ncoord };

  protected :

  struct c {

    enum {nf=1,nv=2};

    vertex_t * vtx[Basic::max_points];  // ????
    thinelement_t * nb;
    Edge *edge;

    signed char bck;

    c();

   ~c();

    void set(vertex_t * a, int i) { (vtx[i] = a)->attach(); }

    void unset(int i) { vtx[i]->detach(); vtx[i] = 0; }

    void write ( std::ostream & ) const;

    void read ( std::istream &, vertex_t **, const int );

  } connect;

  public :
  
  enum {periodic = 1111,general_periodic = 2222};
    typedef int bnd_t;

    // typedef int bnd_part_t[max_bndnr+offset_bndnr+1];

    virtual void sethdl(IndexProvider *phdl);
    IndexProvider* gethdl() const { return vertex(0)->gethdl(); }

    using thinelement_t::nbel;

  private :
      
    Bndel(const Bndel &);
    
    Bndel & operator = (const Bndel &);
 
  protected :

    using thinelement_t::none;

    Bndel(bnd_t t = none) : typ(t) , _segmentIndex( -1 ) { }

    using thinelement_t::_idx;

    bnd_t typ;

#ifdef ALU2D_OLD_BND_PROJECTION
    double (*lf)(double);
    double (*lDf)(double);
#else 
    int _segmentIndex;
#endif
 
  public :

    void check() {}
    
    virtual ~Bndel();

    bnd_t type() const { return typ; }

    int thinis(thintype_t t) const { return t == thinelement_t::bndel_like; }

    int facevertex(int , int ) const;

    // int numfacevertices(int ) const { return connect.nv; }
 
    void edge_vtx(int e, vertex_t * (& ) [c::nv] ) const;


    vertex_t * vertex(int ) const;
 
    vertex_t * vertex(int , int j) const { return vertex(j); }

    int segmentIndex() const 
    { 
      assert( _segmentIndex >= 0 );
      return _segmentIndex; 
    }

    thinelement_t * neighbour(int ) const { return connect.nb; }

    int neighbours(int ) const { return 1; }

    int opposite(int ) const { return connect.bck; }

    int edge_idx(int ) const { return connect.edge->getIndex(); }

    Edge *edge(int ) const { return connect.edge; }

    void nbconnect(int, thinelement_t *, int );
    void edgeconnect(int , Edge *);

#ifdef ALU2D_OLD_BND_PROJECTION
    void set_bndfunctions(double (*pf)(double), double (*pDf)(double))
    {
      lf  = pf;
      lDf = pDf;
    }
#else 
    void copySegmentIndex(const int segmentIndex)
    {
      _segmentIndex = segmentIndex;
    }
#endif

    int get_splitpoint(int, double, double (&) [ncoord]) const;

    double area() const;

    virtual Bndel *create(vertex_t * , vertex_t *,bnd_t) const = 0;

    int setorientation();
};
// #end(class)
// ***************************************************

} // namespace ALU2DGrid

#include "grid_imp.cc"

#endif
