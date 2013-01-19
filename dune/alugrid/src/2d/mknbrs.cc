#include <config.h>

#include <map>

#include "grid.h"
#include "handle.h"

// workaround for new Silicon-CC

  template < int N, int NV >
  struct k {

    Thinelement < N,NV > * a;

    int b;

    k(const struct k & l) : a(l.a), b(l.b) { }

    k() { }

    k(Thinelement < N,NV > * x, int y) : a(x), b(y) { }

  };

template <int N,int NV>
void Hmesh_basic<N,NV>::makeneighbours ()
{
/*
#ifndef NDEBUG
  int start = clock();
#endif
*/

  typedef k < ncoord, nvtx > value_t;
  //typedef vector < vertex_t * > key_t;
  typedef std::pair< vertex_t *, vertex_t * > key_t;
  typedef std::map< key_t, value_t > map_t;

  int count = 0;

  map_t m;

  {

    Levelwalk < element_t > lwe (mel,0);

    Levelwalk < bndel_t > lwb (mbl,0);

    Alignwalk < helement_t, hbndel_t, thinelement_t > walk (lwe, lwb);

    for(walk.first(); !walk.done(); walk.next())
    {
      thinelement_t &e = walk.getitem();
      int numfce = (e.thinis(thinelement_t::element_like))?((element_t&)e).numfaces():1;

      for(int fce = 0; fce < numfce; ++fce)
      {
        assert( e.numfacevertices(fce) == 2 );
        key_t key(e.vertex(fce, 0), e.vertex(fce, 1));
        if( key.first < key.second )
          swap( key.first, key.second );

        /*
        int npv = e.numfacevertices(fce);
        key_t key(npv);
        for(int j = 0; j < npv; ++j )
          key[j] = e.vertex(fce, j);
        sort(key.begin(), key.end());
        */

        typename map_t::iterator hit = m.find(key);
        if(hit != m.end())
        {
          thinelement_t &nb = *hit->second.a;
          int bck = hit->second.b;
          e.nbconnect(fce, &nb, bck);
          nb.nbconnect(bck, &e, fce);
          m.erase(hit);
          ++count;
        }
        else
          m[key] = value_t(&e, fce);
      }
    }

    if(!m.empty())
    {
      std::cerr << "Wrong connectivity:" << std::endl;
      typename map_t::iterator end = m.end();
      for(typename map_t::iterator it = m.begin(); it != end; ++it)
      {
        const key_t &key = it->first;
        std::cerr << "key:" << key.first->getIndex() 
             << " " << key.second->getIndex() << std::endl;
      }
      abort();
    }

  /*
#ifndef NDEBUG
  float used = (float)(clock() - start)/(float)(CLOCKS_PER_SEC);
  std::cerr << "\n  Hmesh_basic::makeneighbours(?) resulted in " << count << " hits, ";
  std::cerr << m.size() << " faults, used time: " << (float)(used) << "\n" << std::endl;
#endif
  */
  }
}
