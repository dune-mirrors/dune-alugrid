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
  
//static ofstream logFile;
static bool writeLogFile = false;

class GitterDunePll : public GitterBasisPll , public GitterDuneBasis 
{
public:
  GitterDunePll (const char * filename , MpAccessLocal &mp) : GitterBasisPll (filename,mp) 
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

  virtual bool duneAdapt () ; // done call notify and loadBalancer  

  virtual bool duneLoadBalance () ; // call loadBalancer 
  virtual bool duneLoadBalance (GatherScatterType & ) ; // call loadBalancer a

  virtual void duneRepartitionMacroGrid (LoadBalancer :: DataBase &, GatherScatterType & gs) ;
  
  void duneExchangeDynamicState ();
  void duneExchangeDynamicState (GatherScatterType &);

  IndexManagerType & indexManager(int codim)
  {
    return containerPll().indexManager(codim);
  }

};
#endif
