        // (c) --
        
        // $Source$
        // $Revision$
        // $Name$
        // $State$

	// Diese Datei enth"alt nur inaktive Prozeduren, die der
	// Aufrufkonvention der Partitionierungsbibliothek PARTY
	// folgen. Dadurch wird die testweise "Ubersetzung des
	// Programms m"oglich, wobei aber die Funktionalit"at der
	// Graphenpartitionierungsverfahren aus PARTY nicht zur
	// Verf"ugung steht.
	// (c) PARTY ist urheberrechtliches Eigentum der 
	// Universit"at Paderborn und kann zu wissenschaftlichen
	// Zwecken von dort kostenlos bezogen werden.
	//
	// http://www.uni-paderborn.de/fachbereich/AG/monien/RESEARCH/PART/party.html
	//
	// Ansprechpartner:
	// Robert Preis 	<preis@hni.uni-paderborn.de>
	//
	// 
	// /**********************************************************\
	// *
	// *  PARTY PARTITIONING LIBRARY            party_lib.h
	// *
	// *  Robert Preis
	// *  HEINZ NIXDORF INSTITUT
	// *  Universit\"at Paderborn, Germany
	// *  preis@hni.uni-paderborn.de
	// *
	// \**********************************************************/
	// #define VERSION "PARTY Version 1.1, 16 September 1996"

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

#ifndef PARTY_LIB_H_INCLUDED
#define PARTY_LIB_H_INCLUDED

static volatile char RCSId_party_lib_h [] = "$Id$" ;

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
#else
  #include <iostream.h>
#endif

static const char parmess [] = 	"**INFO Aus lizenzrechtlichen Gr\"unden steht die Bibliothek\n"
				"       PARTY 1.1 nicht im Rahmen der Quelldateien auf diesem\n"
				"       Datentr\"ager zur Verf\"ugung. Weitere Hinweise sind in\n"
				"       der Datei " __FILE__ " enthalten." ;


inline int global_linear (int,float *,int,float,int *) {
  cerr << parmess << endl ;
  return 0 ;
}

inline int global_scattered (int,float *,int,float,int *) {
  cerr << parmess << endl ;
  return 0 ;
}

inline int global_gain (int,float *,int *,int *,int *,int,float,int *) {
  cerr << parmess << endl ;
  return 0 ;
}

inline int global_farhat (int,float *,int *,int *,int *,int,float,int *) {
  cerr << parmess << endl ;
  return 0 ;
}

inline int local_kl (int,float *,int *,int *,int *,int,float, int *,int) {
  cerr << parmess << endl ;
  return 0 ;
}

inline int local_hs (int n, float *vertex_w, int *edge_p, int *edge, int *edge_w,int p, float add_bal, int *part,int Output) {
  cerr << parmess << endl ;
  return 0 ;
}

#endif
