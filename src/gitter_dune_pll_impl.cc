#ifndef GITTER_DUNE_PLL_IMPL_CC_INCLUDED
#define GITTER_DUNE_PLL_IMPL_CC_INCLUDED

#include "gitter_dune_pll_impl.h"
#include "gitter_dune_pll_mgb.cc"

bool GitterDunePll :: duneAdapt () 
{
  __STATIC_myrank = mpAccess ().myrank () ;
  __STATIC_turn ++ ;
  assert (debugOption (20) ? (cout << "**INFO GitterPll :: adapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  int start = clock () ;
  bool refined = refine () ;
  int lap = clock () ;
  coarse () ;
  int end = clock () ;
  if (debugOption (1)) 
  {
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
  if (neu) 
  {
    if (mpAccess ().gmax (_ldbMethod)) 
    {
      duneRepartitionMacroGrid (db, gs) ;
      notifyMacroGridChanges () ;
    }
  }
  return true;
}

void GitterDunePll :: duneExchangeDynamicState () 
{
  // Die Methode wird jedesmal aufgerufen, wenn sich der dynamische
  // Zustand des Gitters ge"andert hat: Verfeinerung und alle Situationen
  // die einer "Anderung des statischen Zustands entsprechen. Sie wird in
  // diesem Fall NACH dem Update des statischen Zustands aufgerufen, und
  // kann demnach von einem korrekten statischen Zustand ausgehen. F"ur
  // Methoden die noch h"aufigere Updates erfordern m"ussen diese in der
  // Regel hier eingeschleift werden.
  {
  int mallocedsize;
  const int nl = mpAccess ().nlinks () ;
  const int start = clock () ;
  try 
  {
    typedef Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
                TreeIterator < hface_STI, is_def_true < hface_STI > > > InnerIteratorType;
    typedef Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
                TreeIterator < hface_STI, is_def_true < hface_STI > > > OuterIteratorType;
                
    vector < ObjectStream > osv (nl) ;
    {
      for (int l = 0 ; l < nl ; l ++) 
      {
        {
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;

          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
          }
        
          OuterIteratorType wo (mof);
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
          }
        }
      } 
    }
    
    osv = mpAccess ().exchange (osv) ;
    
    { 
      for (int l = 0 ; l < nl ; l ++ ) 
      {
        {
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
        
          OuterIteratorType wo (mof) ;
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
          }
        
          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
          }
        }
      } 
    }
  } 
  catch (Parallel ::  AccessPllException) 
  {
    cerr << "  FEHLER Parallel :: AccessPllException entstanden in: " << __FILE__ << " " << __LINE__ << endl ;
  }
  assert (debugOption (20) ? (cout << "**INFO GitterPll :: exchangeDynamicState () used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << endl, 1) : 1 ) ;
  }
}

void GitterDunePll :: duneExchangeDynamicState (GatherScatterType & gs) 
{
  // Die Methode wird jedesmal aufgerufen, wenn sich der dynamische
  // Zustand des Gitters ge"andert hat: Verfeinerung und alle Situationen
  // die einer "Anderung des statischen Zustands entsprechen. Sie wird in
  // diesem Fall NACH dem Update des statischen Zustands aufgerufen, und
  // kann demnach von einem korrekten statischen Zustand ausgehen. F"ur
  // Methoden die noch h"aufigere Updates erfordern m"ussen diese in der
  // Regel hier eingeschleift werden.
  {
  int mallocedsize;
  const int nl = mpAccess ().nlinks () ;
  const int start = clock () ;
  try 
  {
    typedef Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
                TreeIterator < hface_STI, is_def_true < hface_STI > > > InnerIteratorType;
    typedef Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
                TreeIterator < hface_STI, is_def_true < hface_STI > > > OuterIteratorType;
                
    vector < ObjectStream > osv (nl) ;
    {
      for (int l = 0 ; l < nl ; l ++) 
      {
        {
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;

          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
            p.first->writeDynamicState (osv [l], gs) ;
          }
        
          OuterIteratorType wo (mof);
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
            p.first->writeDynamicState (osv [l], gs) ;
          }
        }
      } 
    }
    
    osv = mpAccess ().exchange (osv) ;
    
    { 
      for (int l = 0 ; l < nl ; l ++ ) 
      {
        {
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
        
          OuterIteratorType wo (mof) ;
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
            p.first->readDynamicState (osv [l], gs) ;
          }
        
          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
            p.first->readDynamicState (osv [l], gs ) ;
          }
        }
      } 
    }
  } 
  catch (Parallel ::  AccessPllException) 
  {
    cerr << "  FEHLER Parallel :: AccessPllException entstanden in: " << __FILE__ << " " << __LINE__ << endl ;
  }
  assert (debugOption (20) ? (cout << "**INFO GitterPll :: exchangeDynamicState () used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << endl, 1) : 1 ) ;
  }
}

#endif
