#ifndef __HEADER__VMMAP
#define __HEADER__VMMAP

static volatile char RCSId_vmmap_h [] = "$Id$";

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iterator>
  #include <algorithm>
  #include <map>
  #include <vector>
#else
  #include <iterator.h>
  #include <algo.h>
  #include <map.h>
  #include <multimap.h>
  #include <vector.h>
#endif

class Multivertexadapter {

  typedef struct value {

    void * a ;

    void * d ;

    int b ;

    int c ;

    value(Vertex * x = 0, int y = 0) : a(x), b(y), c(0) { }

   ~value() { }

  } val_t ;

  typedef map < vector < Vertex * > , val_t , less < vector < Vertex * > > > map_t ;

  vector < map_t > edmaps ;

  vector < map_t > f4maps ;

  Multivertexadapter(const Multivertexadapter &) ;

  Multivertexadapter & operator=(const Multivertexadapter &) ;

  public :

    Multivertexadapter() ;

   ~Multivertexadapter() { }

    void refresh(Listwalk < Macro < Element > > & ) ;

    Vertex * find( Vertex *, Vertex *, int ) ;

    Vertex * find( Vertex *, Vertex *, Vertex *, Vertex *, int ) ;

    void insert( Vertex *, Vertex *, Vertex *, int ) ;

    void insert( Vertex *, Vertex *, Vertex *, Vertex *, Vertex *, int ) ;

} ;

#endif
