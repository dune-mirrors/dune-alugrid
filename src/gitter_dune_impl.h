#ifndef GITTER_DUNE_IMPL_H_INCLUDED
#define GITTER_DUNE_IMPL_H_INCLUDED

#include "gitter_impl.h"
#include "lock.h"

class GitterDuneBasis :  public virtual GitterBasis 
{
protected:
  void backupIndices  (ostream & out);
  void restoreIndices (istream & in );

  inline void goDownHelement( helement_STI & el , vector<bool> & idxcheck);
  
public:
  // write status of grid  
  virtual void duneBackup  (const char*) ; 

  // read status of grid 
  virtual void duneRestore (const char*) ;
  // Constructor getting macro file name 
};

class GitterDuneImpl : public GitterBasisImpl , public GitterDuneBasis 
{
public:
  inline GitterDuneImpl (const char *filename) : GitterBasisImpl ( filename ) {}
};

#endif
