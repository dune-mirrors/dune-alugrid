#ifndef __HEADER__VMMAP
#define __HEADER__VMMAP

template < int N, int NV >
class Multivertexadapter {

  public:
    typedef Vertex < N > vertex_t;
    typedef Element < N, NV > element_t;

    typedef Macro < element_t > macroelement_t;

  private:

  typedef struct value {

    void * a ;

    void * d ;

    int b ;

    int c ;

    value(vertex_t * x = 0, int y = 0) : a(x), b(y), c(0) { }

   ~value() { }

  } val_t ;

  typedef map < vector < vertex_t * > , val_t , less < vector < vertex_t * > > > map_t ;

  vector < map_t > edmaps ;

  vector < map_t > f4maps ;

  Multivertexadapter(const Multivertexadapter &) ;

  Multivertexadapter & operator=(const Multivertexadapter &) ;

  public :

    Multivertexadapter() ;

   ~Multivertexadapter() { }

    void refresh(Listwalk < macroelement_t > & ) ;

    vertex_t * find( vertex_t *, vertex_t *, int ) ;

    vertex_t * find( vertex_t *, vertex_t *, vertex_t *, vertex_t *, int ) ;

    void insert( vertex_t *, vertex_t *, vertex_t *, int ) ;

    void insert( vertex_t *, vertex_t *, vertex_t *, vertex_t *, vertex_t *, int ) ;

} ;

#endif
