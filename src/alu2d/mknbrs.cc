#include "grid.h"
#include "handle.h"

// workaround for new Silicon-CC

  template < int N, int NV >
  struct k {

    Thinelement < N,NV > * a ;

    int b ;

    k(const struct k & l) : a(l.a), b(l.b) { }

    k() { }

    k(Thinelement < N,NV > * x, int y) : a(x), b(y) { }

  };

template <int N,int NV>
void Hmesh_basic<N,NV>::makeneighbours() {
/*
#ifndef NDEBUG
  int start = clock() ;
#endif
*/

  typedef k < ncoord, nvtx > value_t;
  typedef vector < vertex_t * > key_t;
  typedef map < key_t , value_t , less < key_t > > map_t ;

  int count = 0 ;

  map_t m ;

  {

    Levelwalk < element_t > lwe (mel,0) ;

    Levelwalk < bndel_t > lwb (mbl,0) ;

    Alignwalk < helement_t, hbndel_t, thinelement_t > walk (lwe, lwb) ;

    for(walk.first() ; !walk.done() ; walk.next()) {

      thinelement_t & e = walk.getitem() ;

      int numfce = (e.thinis(thinelement_t::element_like))?((element_t&)e).numfaces():1;

      for(int fce = 0 ; fce < numfce ; fce ++ ) {

        int npv = e.numfacevertices(fce) ;
        key_t v(npv) ;
        for(int j = 0 ; j < npv ; j ++ )
          v[j] = e.vertex(fce, j) ;
        sort(v.begin(), v.end()) ;

        typename map_t :: iterator hit = m.find(v) ;
        if(hit == m.end()) m [v] = value_t ( & e, fce) ;
        else {
          count ++ ;
          e.nbconnect(fce, (*hit).second.a,(*hit).second.b) ;
          (*hit).second.a->nbconnect((*hit).second.b, & e, fce) ;
          m.erase(hit) ;
        }
      }
    }

    if ( !m.empty() )
    {
      cerr << "Wrong connectivity:" << endl;
      typename map_t :: iterator end = m.end() ;
      for ( typename map_t :: iterator it = m.begin() ; it != end ; ++it )
      {
        cerr << "key:" << it->first[0]->getIndex() 
             << " " << it->first[1]->getIndex() << endl;
      }
      abort();
    }

  /*
#ifndef NDEBUG
  float used = (float)(clock() - start)/(float)(CLOCKS_PER_SEC) ;
  cerr << "\n  Hmesh_basic::makeneighbours(?) resulted in " << count << " hits, " ;
  cerr << m.size() << " faults, used time: " << (float)(used) << "\n" << endl ;
#endif
  */
  }
}
