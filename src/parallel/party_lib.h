  // (c) --
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
	// **********************************************************
	// *
	// *  PARTY PARTITIONING LIBRARY            party_lib.h
	// *
	// *  Robert Preis
	// *  HEINZ NIXDORF INSTITUT
	// *  Universit\"at Paderborn, Germany
	// *  preis@hni.uni-paderborn.de
	// *
	// **********************************************************

#ifndef PARTY_LIB_H_INCLUDED
#define PARTY_LIB_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
#else
  #include <iostream.h>
#endif

int global_linear (int,float *,int,float,int *);
int global_scattered (int,float *,int,float,int *);
int global_gain (int,float *,int *,int *,int *,int,float,int *);
int global_farhat (int,float *,int *,int *,int *,int,float,int *);
int local_kl (int,float *,int *,int *,int *,int,float, int *,int);
int local_hs (int n, float *vertex_w, int *edge_p, int *edge, int *edge_w,int p, float add_bal, int *part,int Output);

#endif
