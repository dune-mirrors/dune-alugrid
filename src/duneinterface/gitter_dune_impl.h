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

  AdaptRestrictProlongType * _arp;

  // call preCoarsening and postRefinement of arp
  virtual int preCoarsening  (Gitter::helement_STI &);
  virtual int postRefinement (Gitter::helement_STI &);

  virtual void setAdaptRestrictProlongOp ( AdaptRestrictProlongType & arp );
  virtual void removeAdaptRestrictProlongOp ();

  // maxlevel of the grid 
  int maxlevel_;
public:
  GitterDuneBasis() : _arp(0) , maxlevel_(0) {}
  
  // write status of grid  
  virtual void duneBackup  (const char*) ; 

  // read status of grid 
  virtual void duneRestore (const char*) ;
  // Constructor getting macro file name 
  
  bool refine ();

  void coarse ();

  // done call notify and loadBalancer  
  bool duneAdapt (AdaptRestrictProlongType & arp);
};

class GitterDuneImpl : public GitterBasisImpl , public GitterDuneBasis 
{
public:
  inline GitterDuneImpl (const char *filename) : GitterBasisImpl ( filename ) {}
};

#endif
