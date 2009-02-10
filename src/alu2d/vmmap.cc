// +----------------------------------------------------------------------------+
// |	ParGiz Software Project :: MicroGrid 3.0 ++ :: 				|
// |	Na endlich :-) 								|
// +----------------------------------------------------------------------------+

#include "grid.h"
#include "handle.h"
#include "vmmap.h"

Multivertexadapter::Multivertexadapter() {

  edmaps.reserve(30) ;

  f4maps.reserve(30) ;

  edmaps.push_back(map_t ()) ; 

  f4maps.push_back(map_t()) ;

}

void Multivertexadapter::refresh( Listwalk < Macro < Element > > & walk) {

  while(edmaps.size() > 0) edmaps.pop_back() ;

  edmaps.push_back(map_t()) ;

  while(f4maps.size() > 0) f4maps.pop_back() ;

  f4maps.push_back(map_t()) ;

  
  for( walk.first() ; ! walk.done() ; walk.next()) {

    for(int e = 0 ; e < walk.getitem()->numedges() ; e ++) {

      Vertex * edg [2] ;

      walk.getitem()->edge_vtx(e, edg) ;

      vector < Vertex * > v ;

      v.push_back(edg[0]) ;

      v.push_back(edg[1]) ;

      sort(v.begin(), v.end()) ;

      edmaps[0][v].b ++ ;

    }

  }

}

Vertex * Multivertexadapter::find( Vertex * a, Vertex * b, int l) {

  vector < Vertex * > e ;

  e.push_back(a) ;

  e.push_back(b) ;

  sort(e.begin(), e.end()) ;

  if(! l < edmaps.size()) edmaps.push_back(map_t ()) ;

  map_t & map = edmaps[l] ;

  map_t::iterator edge = map.find(e) ;

  return edge == map.end() ? 0 : (Vertex *) (*edge).second.a ;

}

Vertex * Multivertexadapter::find(Vertex * a, Vertex * b, 

	Vertex * c, Vertex * d, int l) {

  vector < Vertex * > v ;

  v.push_back(a) ; 

  v.push_back(b) ;

  v.push_back(c) ; 

  v.push_back(d) ;

  sort(v.begin(), v.end()) ;

  if(! l < f4maps.size()) f4maps.push_back(map_t ()) ;

  map_t & map = f4maps[l] ;

  map_t::iterator face = map.find(v) ;

  if(face == map.end()) return 0 ;

  else {

    Vertex * hit = (Vertex *) (*face).second.a ;

    map.erase(face) ;

    return hit ;

  }

}

void Multivertexadapter::insert( Vertex * a, Vertex * b, 

	Vertex * ev, int l) {

  vector < Vertex * > e ;

  e.push_back(a) ;

  e.push_back(b) ;

  sort(e.begin(), e.end()) ;  

  edmaps[l][e] = val_t(ev,0) ;

}

void Multivertexadapter::insert(Vertex * a, Vertex * b,

	Vertex * c, Vertex * d, Vertex * cv, int l) {

  vector < Vertex * > v ;

  v.push_back(a) ;

  v.push_back(b) ;

  v.push_back(c) ;

  v.push_back(d) ;

  sort(v.begin(), v.end()) ;

  f4maps[l][v] = val_t(cv,0) ;

}
