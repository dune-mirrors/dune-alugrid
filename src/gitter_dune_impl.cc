#ifndef GITTER_DUNE_IMPL_CC_INCLUDED
#define GITTER_DUNE_IMPL_CC_INCLUDED

#include "gitter_dune_impl.h"

// wird von Dune verwendet 
void GitterDuneBasis :: duneBackup (const char * fileName)
{
  // diese Methode wird von der Dune Schnittstelle aufgerufen und ruft
  // intern lediglich backup (siehe oben) und backupCMode des Makrogitters
  // auf, allerdings wird hier der path und filename in einer variablen
  // uebergeben 

  assert (debugOption (20) ? (cout << "**INFO GitterDuneImpl :: duneBackup (const char * = \""
                       << fileName << "\") " << endl, 1) : 1) ;

  ofstream out (fileName) ;
  if (!out) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneImpl :: duneBackup (const char *, double) Fehler beim Anlegen von < "
         << (fileName ? fileName : "null") << " >" << endl ;
  }
  else
  {
    FSLock lock (fileName) ;
    this->backup (out) ;

    {
      char *fullName = new char[strlen(fileName)+20];
      if(!fullName)
      {
        cerr << "**WARNUNG GitterDuneImpl :: duneBackup (, const char *, double) :";
        cerr << "couldn't allocate fullName! " << endl;
        abort();
      }
      sprintf(fullName,"%s.macro",fileName);
      ofstream macro (fullName) ;

      if(!macro)
      {
        cerr << "**WARNUNG (IGNORIERT) GitterDuneImpl :: duneBackup (const char *, const char *) Fehler beim Anlegen von < "
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

  assert (debugOption (20) ? (cout << "**INFO GitterDuneImpl :: duneRestore (const char * = \""
                 << fileName << "\") " << endl, 1) : 1) ;

  ifstream in (fileName) ;
  if (!in) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneImpl :: duneRestore (const char *, double & ) Fehler beim \"Offnen von < "
         << (fileName ? fileName : "null") << " > " << endl ;
  } else {
    this->restore (in) ;
  }
  return ;
}


#endif
