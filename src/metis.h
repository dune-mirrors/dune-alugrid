        // (c) --
        
        // $Source$
        // $Revision$
        // $Name$
        // $State$

	// Diese Datei enth"alt nur inaktive Prozeduren, die der
	// Aufrufkonvention der Partitionierungsbibliothek metis
	// folgen. Dadurch wird die testweise "Ubersetzung des
	// Programms m"oglich, wobei aber die Funktionalit"at der
	// Graphenpartitionierungsverfahren aus metis 3.0 nicht
	// zur Verf"ugung steht.
	// (c) metis 3.0 und parmetis 2.0 sind frei verf"ugbare
	// Softwareprodukte und k"onnen ohne Lizensierung bezogen
	// werden "uber die untenstehende Internetadresse:
	//
	// http://www-users.cs.umn.edu/~karypis/metis/main.shtml
	//

/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:39:54  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.3  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.2  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef METIS_H_INCLUDED
#define METIS_H_INCLUDED

static volatile char RCSId_metis_h [] = "$Id$" ;

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

void METIS_PartGraphRecursive(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *) {
  cerr << metmess << endl ;
  return ;
}

#endif
