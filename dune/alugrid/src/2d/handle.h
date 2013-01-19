#ifndef __HEADER__HANDLE
#define __HEADER__HANDLE

#include "../indexstack.h"
#include "../projectvertex.h"
#include "vtxprojection.h"
#include "grid.h"                               
#include "listwalk.h"

// is defined in indexstack.h 
typedef ALUGrid::IndexManagerType IndexManager2dType;

// is defined in indexstack.h 
typedef ALUGrid::RestoreInfo RestoreInfo;

// number of different index manager that exists
enum { numOfIndexManager2d = 4 };
// see specific codes in class Hmesh below 


// +----------------------------------------------------------------------------+
// | Vorsicht ! Die Reihenfolge der Deklarationen ist wesentlich: Zuerst werden |
// |          die Rand- Elemente abgebaut, dann erst die Vertices,  damit ist |
// |          sicher, dass der Refcount runtergez"ahlt war. Sonst knirsch ... |
// +----------------------------------------------------------------------------+

struct IndexProvider
{
   enum { IM_Elements = 0, // index manager id for elements 
          IM_Edges = 1,    // index manager id for edges  
          IM_Vertices = 2, // index manager id for vertices  
          IM_Bnd = 3       // index manager id for bnd 
   };

   int getIndex(int indextype) 
   {
     assert( indextype >= 0 && indextype < numOfIndexManager2d );
     return indexmanager[indextype].getIndex();
   }

   void freeIndex(int indextype, int index) 
   {
     assert( indextype >= 0 && indextype < numOfIndexManager2d );
     indexmanager[indextype].freeIndex(index);
   }

   // return current size of used indices 
   int indexManagerSize (int cd) const 
   {
     // only for elements, edges, and vertices 
     assert(cd<3 && cd>=0);
     return indexmanager[cd].getMaxIndex();
   }
   
   virtual ~IndexProvider() {}
  protected:
   IndexManager2dType indexmanager[numOfIndexManager2d];
};

template < int N, int NV > class Bndel_periodic;

template < int N, int NV >
class Hmesh_basic : public IndexProvider {

  protected :

    enum { ncoord = N, nvtx = NV };

  public :
    typedef Thinelement < ncoord,nvtx > thinelement_t;

    typedef Element < ncoord,nvtx > element_t;
    typedef Bndel < ncoord,nvtx > bndel_t;

    typedef Hier < element_t > helement_t;
    typedef Hier < bndel_t > hbndel_t;

    typedef Macro < element_t > macroelement_t;
    typedef Macro < bndel_t > macrobndel_t;

    typedef Triang < ncoord, nvtx > triang_t;
    typedef Bndel_triang < ncoord, nvtx > bndel_triang_t;
    typedef Bndel_periodic < ncoord, nvtx > bndel_periodic_t;

    typedef Vertex < ncoord > vertex_t;
    typedef Fullvertex < ncoord > fullvertex_t;

    typedef nconf_vtx < ncoord, nvtx > nconf_vtx_t;

    typedef VtxProjection < ncoord, nvtx > ProjectVertex_t;
    typedef ALUGrid::VertexProjection< N , double > CompatibilityProjectVertex_t;

    struct OrientStr
    {
      element_t *el;
      int nextNb;
      double n[3];
    };

  protected :

    struct CompatibilityVertexProjection
    : public ProjectVertex_t
    {
      using ProjectVertex_t :: operator ();

      typedef typename ProjectVertex_t::hbndel_t hbndel_t;
      typedef typename ProjectVertex_t::helement_t helement_t;

      enum { ncoord = ProjectVertex_t::ncoord };

      CompatibilityVertexProjection ()
      : _projectVertex( 0 )
      {}

      virtual int operator() ( const hbndel_t *bndel, const double, double (&p) [ ncoord ] ) const
      {
        if( !_projectVertex )
          return 1;

        double pp [ ncoord ];
        for( int i = 0; i < ncoord; ++i )
          pp[ i ] = p[ i ];
        return (*_projectVertex)( pp, bndel->segmentIndex(), p );
      }

      void setVertexProjection ( const CompatibilityProjectVertex_t *ppv )
      {
        _projectVertex = ppv;
      }

    private:
      const CompatibilityProjectVertex_t *_projectVertex;
    };

    using IndexProvider::indexmanager;

    Listagency < vertex_t > vl;

    Listagency < macroelement_t > mel;

    Listagency < macrobndel_t >  mbl;

    const ProjectVertex_t  *_projectVertex;
    CompatibilityVertexProjection _compatibilityVertexProjection;
    
    Listwalk < helement_t > * walk( helement_t *) { return new Leafwalk < element_t > (mel); }

    // von mir dazugeschrieben...
    Listwalk < helement_t > * walk( helement_t *, int level) { return new Levelwalk < element_t > (mel, level); }
    
    Listwalk < vertex_t > * walk(vertex_t *) { return new Listwalk_impl < vertex_t > (vl); } 

    Listwalk < macroelement_t > * walk(macroelement_t *) { return new Listwalk_impl < macroelement_t > (mel); }
    
    Listwalk < hbndel_t > * walk( hbndel_t *) { return new Leafwalk < bndel_t > (mbl); }
    
    // von mir dazugeschrieben... (von wem?)
    Listwalk < hbndel_t > * walk( hbndel_t *, int level) { return new Levelwalk < bndel_t > (mbl, level); }

    Hmesh_basic(const Hmesh_basic &);
    
    Hmesh_basic & operator = (const Hmesh_basic &);

 protected:
    void asciiwritetriang(std::ostream &, double, unsigned long int nbr, 
                          int nconfDeg, Refco::tag_t ref_rule);
    
    void asciireadtriang ( std::istream &, const bool = true );

    void setorientation();

  public :
   Hmesh_basic() : 
      vl(this), 
      mel(this), 
      mbl(this),
      _projectVertex( 0 )
   {}

   Hmesh_basic(const ProjectVertex_t* pv) : 
      vl(this), 
      mel(this), 
      mbl(this),
      _projectVertex( pv )
   {}

   virtual ~Hmesh_basic() {}     

   // return number of macro boundary segments 
   size_t numMacroBndSegments() const 
   {
     return mbl.size();
   }

   void projectVertex ( const bndel_t *bndel, const double local, double (&global) [ncoord] ) const
   {
     assert( bndel );
     if( _projectVertex )
       (*_projectVertex)( static_cast< const hbndel_t * >( bndel ), local, global );
   }

   void projectVertex ( const element_t *element, const double (&local) [2], double (&global) [ncoord] ) const
   {
     assert( element );
     if( _projectVertex )
       (*_projectVertex)( static_cast< const helement_t * >( element ), local, global );
   }

   // set vertex projection pointer 
   void setVertexProjection(const ProjectVertex_t* ppv)
   {
     _projectVertex = ppv;
   }

   void setVertexProjection ( const CompatibilityProjectVertex_t *ppv )
   {
     _compatibilityVertexProjection.setVertexProjection( ppv );
     _projectVertex = &_compatibilityVertexProjection;
   }
   
   void makeneighbours();
           
   virtual void refresh() { }
       
   friend class Listwalkptr < helement_t >;
 
   friend class Listwalkptr < vertex_t >;
  
   friend class Listwalkptr < hbndel_t >;

   friend class Listwalkptr < macroelement_t >;

};


template < int N, int NV >
class Hmesh : public Hmesh_basic<N,NV> {

  Hmesh & operator=(const Hmesh &);

  Hmesh(const Hmesh &);

  typedef Hmesh_basic<N,NV> hmesh_basic_t;

  protected :

  enum { ncoord = N, nvtx = NV };

  public:

  typedef Thinelement < ncoord,nvtx > thinelement_t;

  typedef Element < ncoord,nvtx > element_t;
  typedef Bndel < ncoord,nvtx > bndel_t;

  typedef Hier < element_t > helement_t;
  typedef Hier < bndel_t > hbndel_t;

  typedef Macro < element_t > macroelement_t;
  typedef Macro < bndel_t > macrobndel_t;

  typedef Triang < ncoord, nvtx > triang_t;
  typedef Bndel_triang < ncoord, nvtx > bndel_triang_t;
  typedef Bndel_periodic < ncoord, nvtx > bndel_periodic_t;

  typedef Multivertexadapter < ncoord, nvtx > multivertexadapter_t;
  typedef Vertex < ncoord > vertex_t;
  typedef Fullvertex < ncoord > fullvertex_t;

  typedef nconf_vtx < ncoord, nvtx > nconf_vtx_t;

  typedef VtxProjection < ncoord, nvtx > ProjectVertex_t;

  typedef Prolong_basic < ncoord, nvtx > prolong_basic_t;
  typedef Restrict_basic < ncoord, nvtx > restrict_basic_t;

  typedef AdaptRestrictProlong2d < ncoord, nvtx > AdaptRestrictProlong2dType;

  protected :

  using hmesh_basic_t::indexmanager;

  using hmesh_basic_t::vl;

  using hmesh_basic_t::mel;

  using hmesh_basic_t::mbl;

private:
  multivertexadapter_t * adp;

  int _nconfDeg;

  Refco::tag_t refinement_rule;

  prolong_basic_t *_pro_el;
  restrict_basic_t *_rest_el;


  nconf_vtx_t *ncv;

  void setup_grid(const std::string &);
  bool setup_grid ( std::istream &, double &, unsigned long int & );

  bool asciireadtriang ( std::istream &, double &, unsigned long int & );
  void asciiwritetriang(const std::string &, double, unsigned long int);

public:
  Hmesh ();
  Hmesh ( const std::string &, int, Refco::tag_t pref_rule );

  void printMemSize () 
  {
    std::cout << "short int    = " << sizeof(short int) << std::endl;
    std::cout << "Basic        = " << sizeof( Basic  ) << std::endl;
    std::cout << "Edge         = " << sizeof( Edge   ) << std::endl;
    std::cout << "Element      = " << sizeof( element_t ) << std::endl;
    std::cout << "connect      = " << sizeof( typename element_t :: connect_t  ) << std::endl;
    std::cout << "BndEl        = " << sizeof( bndel_t ) << std::endl;
    std::cout << "Triangle     = " << sizeof( triang_t ) << std::endl;
    std::cout << "NonConf Vx   = " << sizeof( nconf_vtx_t ) << std::endl;
    std::cout << "FullVertex   = " << sizeof( fullvertex_t ) << std::endl;
    std::cout << "Vertex       = " << sizeof( Vertex < 3 > ) << std::endl;
    std::cout << "Listagent    = " << sizeof( Listagent < Vertex < 3 > >) << std::endl;
    std::cout << "Multi Vx Adp = " << sizeof( multivertexadapter_t ) << std::endl;
    std::cout << "HElement     = " << sizeof( helement_t ) << std::endl;
  }

  // constructor taking istream with macro triang
  // number of hanging nodes and refinement rule 
  Hmesh ( std::istream &, int, Refco::tag_t pref_rule );
  Hmesh ( const std::string &, Refco::tag_t pref_rule = Refco::ref_1 );
  Hmesh ( const std::string &, int );

  virtual ~Hmesh();

  void storeGrid ( const std::string &, double time = 0, unsigned long int nbr = 0 );
  void storeGrid ( std::ostream &, double time = 0, unsigned long int nbr = 0 );

  bool recoverGrid ( std::istream & );

  void storeIndicies(std::ostream &out);
  void recoverIndicies(std::istream &in);

  void refine();

  // done call notify and loadBalancer
  bool duneAdapt (AdaptRestrictProlong2dType & arp);

  bool checkConf();

  void coarse();

  void refresh();

  void setdata(void (*)(element_t &));
};

#endif // #ifndef __HEADER__HANDLE
