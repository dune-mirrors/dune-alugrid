// Robert Kloefkorn (c) 2004 - 2005 
#ifndef GITTER_DUNE_IMPL_CC_INCLUDED
#define GITTER_DUNE_IMPL_CC_INCLUDED

#include "gitter_dune_impl.h"

IteratorSTI < Gitter :: helement_STI > * GitterDuneImpl :: 
leafIterator (const helement_STI *) 
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > (container ()) ;
}

IteratorSTI < Gitter :: helement_STI > * GitterDuneImpl :: 
leafIterator (const IteratorSTI < helement_STI > * p) 
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > 
  (*(const Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > *) p) ;
}

// wird von Dune verwendet 
void GitterDuneBasis :: duneBackup (const char * fileName) 
{
  // diese Methode wird von der Dune Schnittstelle aufgerufen und ruft
  // intern lediglich backup (siehe oben) und backupCMode des Makrogitters
  // auf, allerdings wird hier der path und filename in einer variablen
  // uebergeben 

  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneBackup (const char * = \""
                       << fileName << "\") " << endl, 1) : 1) ;

  ofstream out (fileName) ;
  if (!out) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneBackup (const char *, double) Fehler beim Anlegen von < "
         << (fileName ? fileName : "null") << " >" << endl ;
  }
  else
  {
    Gitter :: backup (out) ;

    backupIndices ( out ) ;

    {
      char *fullName = new char[strlen(fileName)+20];
      if(!fullName)
      {
        cerr << "**WARNUNG GitterDuneBasis :: duneBackup (, const char *, double) :";
        cerr << "couldn't allocate fullName! " << endl;
        abort();
      }
      sprintf(fullName,"%s.macro",fileName);
      ofstream macro (fullName) ;

      if(!macro)
      {
        cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneBackup (const char *, const char *) Fehler beim Anlegen von < "
         << (fullName ? fullName : "null") << " >" << endl ;
      }
      else
      {
        container ().backupCMode (macro) ;
      }
      delete [] fullName;
    }
  }
  return ;
}

// wird von Dune verwendet 
void GitterDuneBasis :: duneRestore (const char * fileName)
{
  // diese Methode wird von der Dune Schnittstelle aufgerufen 
  // diese Methode ruft intern restore auf, hier wird lediglich 
  // der path und filename in einer variablen uebergeben

  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneRestore (const char * = \""
                 << fileName << "\") " << endl, 1) : 1) ;

  ifstream in (fileName) ;
  if (!in) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneRestore (const char *, double & ) Fehler beim \"Offnen von < "
         << (fileName ? fileName : "null") << " > " << endl ;
  } else {
    Gitter :: restore (in) ;
    this->restoreIndices (in);
  }
  return ;
}

int GitterDuneBasis :: preCoarsening  (Gitter::helement_STI & elem)
{
  // if _arp is set then the extrenal preCoarsening is called 
  return (_arp) ? (*_arp).preCoarsening(elem) : 0;
}

int GitterDuneBasis :: postRefinement (Gitter::helement_STI & elem)
{
  // if _arp is set then the extrenal postRefinement is called 
  return (_arp) ? (*_arp).postRefinement(elem) : 0;
}

int GitterDuneBasis :: preCoarsening  (Gitter::hbndseg_STI & bnd)
{
  // if _arp is set then the extrenal preCoarsening is called 
  return (_arp) ? (*_arp).preCoarsening( bnd ) : 0;
}

int GitterDuneBasis :: postRefinement (Gitter::hbndseg_STI & bnd)
{
  // if _arp is set then the extrenal postRefinement is called 
  return (_arp) ? (*_arp).postRefinement( bnd ) : 0;
}

void GitterDuneBasis ::
setAdaptRestrictProlongOp( AdaptRestrictProlongType & arp )
{
  if(_arp) 
  {
    cerr<< "WARNING: GitterDuneBasis :: setAdaptRestrictProlongOp: _arp not NULL! in:" <<  __FILE__ << " line="<<__LINE__ << endl; 
  }
  _arp = &arp;
}

void GitterDuneBasis :: removeAdaptRestrictProlongOp()
{
  _arp = 0;
}

bool GitterDuneBasis :: duneAdapt (AdaptRestrictProlongType & arp) 
{
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneAdapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  const int start = clock () ;

  setAdaptRestrictProlongOp(arp); 
  bool refined = this->refine ();
  if (!refined) {
    cerr << "**WARNUNG (IGNORIERT) Verfeinerung nicht vollst\"andig (warum auch immer)\n" ;
    cerr << "  diese Option ist eigentlich dem parallelen Verfeinerer vorbehalten.\n" ;
    cerr << "  Der Fehler trat auf in " << __FILE__ << " " << __LINE__ << endl ;
  }

  int lap = clock () ;
  this->coarse () ;
  int end = clock () ;
  if (debugOption (1)) {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO GitterDuneBasis :: duneAdapt () [ref|cse|all] " << u1 << " " << u2 << " " << u3 << endl ;
  }
  // sets pointer to zero 
  removeAdaptRestrictProlongOp ();

  return refined;
}
#endif
