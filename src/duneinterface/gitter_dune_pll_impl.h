// (c) Robert Kloefkorn 2004 -- 2005 
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
  
//static bool writeLogFile = false;

class GitterDunePll : public GitterBasisPll , public virtual GitterDuneBasis
{
  
protected:  
  bool balanceGrid_;

public:
  typedef Gitter :: Geometric Geometric;
  typedef GitterDuneImpl :: Objects  Objects;
  
  GitterDunePll (const char * filename , MpAccessLocal &mp) 
    : GitterBasisPll (filename,mp) , balanceGrid_ (false) 
  {
    // logfile is defined in gitter_impl.h    
    /*
    char logFileName [32];
    sprintf(logFileName,"logfile.%d",mpAccess().myrank());
    cerr << "open logfile = " << logFileName << "\n";

    logFile.close();
    logFile.open (logFileName);
    logFile << "logfile of processor " << mpAccess().myrank() << "\n";
    */
  };

  ~GitterDunePll () {
    /*
    logFile.close();
    */
  }

  bool refine (); 

  void coarse ();

  // done call notify and loadBalancer  
  bool duneAdapt ();

  // return true if grid has to be balanced again 
  bool duneNotifyNewGrid ();

  bool duneLoadBalance () ; // call loadBalancer 
  bool duneLoadBalance (GatherScatterType & ) ; // call loadBalancer a

  void duneRepartitionMacroGrid (LoadBalancer :: DataBase &, GatherScatterType & gs) ;
  void repartitionMacroGrid (LoadBalancer :: DataBase &) ;
 
  // notifyGridChanges for dune
  void duneNotifyGridChanges (); 
  
  // exchange changed elements  
  void duneExchangeDynamicState ();
  
  // exchange data of dune 
  void duneExchangeData (GatherScatterType &);

  // return indexmanger 
  IndexManagerType & indexManager(int codim)
  {
    return containerPll().indexManager(codim);
  }

};
#endif
