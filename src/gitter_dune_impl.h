#ifndef GITTER_DUNE_IMPL_H_INCLUDED
#define GITTER_DUNE_IMPL_H_INCLUDED

#include "gitter_impl.h"
#include "lock.h"

class GitterDuneBasis : public GitterBasis {

public:
  // write status of grid  
  virtual void duneBackup  (const char*) ; 

  // read status of grid 
  virtual void duneRestore (const char*) ;
};


class GitterDuneImpl : public GitterBasisImpl , public GitterDuneBasis 
{
public :
  // Constructor getting macro file name 
  inline GitterDuneImpl (const char *filename) : GitterBasisImpl ( filename ) {}
};

#endif
