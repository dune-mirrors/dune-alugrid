// (c) --
#ifndef METISTITLE

#warning  -- Compile parallel version without METIS !!!

#ifndef METIS_H_INCLUDED
#define METIS_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
#else
  #include <iostream.h>
#endif

typedef int idxtype ;

static const char metmess [] = 	"**INFO Aus lizenzrechtlichen Gr\"unden steht die Bibliothek\n"
				"       metis 3.0 nicht im Rahmen der Quelldateien auf diesem\n"
				"       Datentr\"ager zur Verf\"ugung. Weitere Hinweise sind in\n"
				"       der Datei " __FILE__ " enthalten." ;

inline void METIS_PartGraphKway(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *) {
  cerr << metmess << endl ;
  return ;
}

inline void METIS_PartGraphRecursive(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *) {
  cerr << metmess << endl ;
  return ;
}

#endif
#endif
