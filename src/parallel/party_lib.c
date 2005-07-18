// (c) --
#ifndef PARTY_LIB_C_INCLUDED
#define PARTY_LIB_C_INCLUDED

#warning  -- Compiling parallel version without PARTY_LIB !!! 

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


int global_linear (int,float *,int,float,int *) {
  cerr << parmess << endl ;
  abort();
  return 0 ;
}

int global_scattered (int,float *,int,float,int *) {
  cerr << parmess << endl ;
  abort();
  return 0 ;
}

int global_gain (int,float *,int *,int *,int *,int,float,int *) {
  cerr << parmess << endl ;
  abort();
  return 0 ;
}

int global_farhat (int,float *,int *,int *,int *,int,float,int *) {
  cerr << parmess << endl ;
  abort();
  return 0 ;
}

int local_kl (int,float *,int *,int *,int *,int,float, int *,int) {
  cerr << parmess << endl ;
  abort();
  return 0 ;
}

int local_hs (int n, float *vertex_w, int *edge_p, int *edge, int *edge_w,int p, float add_bal, int *part,int Output) {
  cerr << parmess << endl ;
  return 0 ;
}

#endif
