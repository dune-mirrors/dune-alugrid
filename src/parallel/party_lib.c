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

static const char parmess [] = 	"**INFO Due to license reasons the library PARTY is\n"
				"       not part of the ALUGrid library distribution. \n"
				"       To use this feature get a copy of the PARTY library \n"
        "       (see http://wwwcs.upb.de/fachbereich/AG/monien/RESEARCH/PART/party.html)\n"
        "       and re-configure the ALUGrid library with the \n"
        "       --with-party=PATH_TO_PARTY option, \n"
        "       or choose another Graph partitioning method. \n"
        "       Exiting program, bye! \n";

// dummy method, when library missing        
int global_linear (int,float *,int,float,int *) {
  cerr << "**ERROR The use of global_linear is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1);
  return 0 ;
}

// dummy method, when library missing        
int global_scattered (int,float *,int,float,int *) {
  cerr << "**ERROR The use of global_scattered is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1);
  return 0 ;
}

// dummy method, when library missing        
int global_gain (int,float *,int *,int *,int *,int,float,int *) {
  cerr << "**ERROR The use of global_gain is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1);
  return 0 ;
}

// dummy method, when library missing        
int global_farhat (int,float *,int *,int *,int *,int,float,int *) {
  cerr << "**ERROR The use of global_farhat is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1);
  return 0 ;
}

// dummy method, when library missing        
int local_kl (int,float *,int *,int *,int *,int,float, int *,int) {
  cerr << "**ERROR The use of local_kl is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1);
  return 0 ;
}

// dummy method, when library missing        
int local_hs (int n, float *vertex_w, int *edge_p, int *edge, int *edge_w,int p, float add_bal, int *part,int Output) {
  cerr << "**ERROR The use of local_hs is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1);
  return 0 ;
}

#endif
