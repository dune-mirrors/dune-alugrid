#ifndef GITTER_DUNE_PLL_IMPL_H_INCLUDED
#define GITTER_DUNE_PLL_IMPL_H_INCLUDED

#ifdef _ANSI_HEADER
  using namespace std;
  #include <numeric>
#else
#endif

#include "gitter_dune_impl.h"
#include "gitter_pll_impl.h"
#include "gitter_pll_ldb.h"
  
static bool writeLogFile = false;

class GitterDunePll : public GitterBasisPll , public virtual GitterDuneBasis
{
  
protected:  
  bool balanceGrid_;
public:
  //class Objects   : public GitterBasisPll :: Objects   {};
  //class Geometric : public GitterBasisPll :: Geomietric {};

  GitterDunePll (const char * filename , MpAccessLocal &mp) 
    : GitterBasisPll (filename,mp) , balanceGrid_ (false) 
  {
    char logFileName [32];
    sprintf(logFileName,"logfile.%d",mpAccess().myrank());
    cerr << "open logfile = " << logFileName << "\n";

    logFile.close();
    logFile.open (logFileName);
    logFile << "logfile of processor " << mpAccess().myrank() << "\n";
  };

  ~GitterDunePll () {
    logFile.close();
  }

  bool refine (); 

  void coarse ();

  bool duneAdapt ()   // done call notify and loadBalancer  
  {
    //cout << "duneAdapt \n";
    __STATIC_myrank = mpAccess ().myrank () ;
    __STATIC_turn ++ ;
    assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: duneAdapt ()" << endl, 1) : 1) ;
    assert (! iterators_attached ()) ;
    int start = clock () ;
    bool refined = this-> refine () ;
    int lap = clock () ;
    this-> coarse () ;
    int end = clock () ;
    if (debugOption (1))
    {
      float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
      float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
      float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
      cout << "**INFO GitterDunePll :: adapt () [ref (loops)|cse|all] " << u1 << " ("
           << _refineLoops << ") " << u2 << " " << u3 << endl ;
    }
    duneNotifyGridChanges () ;
    balanceGrid_ = duneNotifyNewGrid();
  return refined;
  }

  // return true if grid has to be balanced again 
  bool duneNotifyNewGrid () 
  {
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
        neu |= (*i > mean ? (*i > (_ldbOver * mean) ? true : false) : (*i < (_ldbUnder * mean) ? true : false)) ;
    }
    return neu; 
  }

  bool duneLoadBalance () ; // call loadBalancer 
  bool duneLoadBalance (GatherScatterType & ) ; // call loadBalancer a

  void duneRepartitionMacroGrid (LoadBalancer :: DataBase &, GatherScatterType & gs) ;
  void repartitionMacroGrid (LoadBalancer :: DataBase &) ;
  
  void duneNotifyGridChanges ()
  {
    //logFile << "call of notifyGridChanges \n";
    Gitter :: notifyGridChanges () ;
    writeLogFile = true;
    duneExchangeDynamicState () ;
    writeLogFile = false;
    return ;
  };
  
  void duneExchangeDynamicState ();
  void duneExchangeData (GatherScatterType &);
  

  IndexManagerType & indexManager(int codim)
  {
    return containerPll().indexManager(codim);
  }

  // write status of grid  
  void duneBackup  (const char*) ;

  // read status of grid 
  void duneRestore (const char*) ;

};
#endif
