#ifndef GITTER_DUNE_PLL_IMPL_CC_INCLUDED
#define GITTER_DUNE_PLL_IMPL_CC_INCLUDED

#include "gitter_dune_pll_impl.h"
#include "gitter_dune_pll_mgb.cc"

bool GitterDunePll :: duneAdapt () {
  __STATIC_myrank = mpAccess ().myrank () ;
  __STATIC_turn ++ ;
  assert (debugOption (20) ? (cout << "**INFO GitterPll :: adapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  int start = clock () ;
  bool refined = refine () ;
  int lap = clock () ;
  coarse () ;
  int end = clock () ;
  if (debugOption (1)) {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO GitterPll :: adapt () [ref (loops)|cse|all] " << u1 << " ("
         << _refineLoops << ") " << u2 << " " << u3 << endl ;
  }
  notifyGridChanges () ;
  return refined;
}

bool GitterDunePll :: duneLoadBalance () 
{
  loadBalancerGridChangesNotify () ;
  return true;
}

bool GitterDunePll :: duneLoadBalance (GatherScatterType & gs) {
  assert (debugOption (20) ? (cout << "**GitterPll :: loadBalancerGridChangesNotify () " << endl, 1) : 1) ;
  const int start = clock (), me = mpAccess ().myrank (), np = mpAccess ().psize () ;
  LoadBalancer :: DataBase db ;
  {
    AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphEdge (db) ;
  }
  {
    AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphVertex (db) ;
  }
  bool neu = false ;
  {
  // Kriterium, wann eine Lastneuverteilung vorzunehmen ist:
  // 
  // load  - eigene ElementLast
  // mean  - mittlere ElementLast
  // nload - Lastverh"altnis

  
    double load = db.accVertexLoad () ;
    vector < double > v (mpAccess ().gcollect (load)) ;
    double mean = accumulate (v.begin (), v.end (), 0.0) / double (np) ;

    for (vector < double > :: iterator i = v.begin () ; i != v.end () ; i ++)
      neu |= (*i > mean ? (*i > (_ldbOver * mean) ? true : false) : (*i < (_ldbUnder * mean) ? true : false));
  }
  if (neu) {
    if (mpAccess ().gmax (_ldbMethod)) {
      duneRepartitionMacroGrid (db, gs) ;
      notifyMacroGridChanges () ;
    }
  }
  return true;
}
#endif
