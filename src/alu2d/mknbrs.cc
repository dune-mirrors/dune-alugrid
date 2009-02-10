#include "grid.h"
#include "handle.h"

// workaround for new Silicon-CC

 struct k {

    Thinelement * a ;

    int b ;

    k(const struct k & l) : a(l.a), b(l.b) { }

    k() { }

    k(Thinelement * x, int y) : a(x), b(y) { }

  };


void Hmesh_basic::makeneighbours() {
/*
#ifndef NDEBUG
  int start = clock() ;
#endif
*/

  int count = 0 ;

  map < vector < Vertex * > , struct k , less < vector < Vertex * > > > m ;

  {

    Levelwalk < Element > lwe (mel,0) ;

    Levelwalk < Bndel > lwb (mbl,0) ;

    Alignwalk < helement_t, hbndel_t, Thinelement > walk (lwe, lwb) ;

    for(walk.first() ; !walk.done() ; walk.next()) {

      Thinelement & e = walk.getitem() ;

      for(int fce = 0 ; fce < e.numfaces() ; fce ++ ) {

        int npv = e.numfacevertices(fce) ;

        vector < Vertex * > v ;

        for(int j = 0 ; j < npv ; j ++ )

          v.push_back(e.vertex(fce, j)) ;

        sort(v.begin(), v.end()) ;

        map < vector < Vertex * > , struct k , less < vector < Vertex * > > > :: iterator hit = m.find(v) ;

        if(hit == m.end()) m [v] = k( & e, fce) ;

        else {

          count ++ ;

          e.nbconnect(fce, (*hit).second.a,(*hit).second.b) ;

          (*hit).second.a->nbconnect((*hit).second.b, & e, fce) ;

          m.erase(hit) ;

        }

      }

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
