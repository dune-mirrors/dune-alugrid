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

#if HAVE_PARTY 
extern "C" {
  // include PARTY lib header 
  #include <party_lib.h>
}
#else 
static const char parmess [] =  "**INFO Due to license reasons the library PARTY is\n"
        "       not part of the ALUGrid library distribution. \n"
        "       To use this feature get a copy of the PARTY library \n"
        "       (see http://wwwcs.upb.de/fachbereich/AG/monien/RESEARCH/PART/party.html)\n"
        "       and re-configure the ALUGrid library with the \n"
        "       --with-party=PATH_TO_PARTY option, \n"
        "       or choose another Graph partitioning method. \n"
        "       Exiting program, bye! \n";
#endif

inline int CALL_global_lin (int nel, float *vertex_w, int np, int *neu)
{
#if HAVE_PARTY 
  return :: global_lin (nel, vertex_w, np, neu) ;
#else
  cerr << "**ERROR The use of global_lin is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1); 
  return 0 ;
#endif
}

inline int CALL_global_sca (int nel, float *vertex_w, int np, int *neu)
{
#if HAVE_PARTY 
  return :: global_sca (nel, vertex_w, np, neu) ;
#else
  cerr << "**ERROR The use of global_sca is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1); 
  return 0 ;
#endif
}

inline int CALL_global_ran (int nel, float *vertex_w, int np, int *neu)
{
#if HAVE_PARTY 
  return :: global_ran (nel, vertex_w, np, neu) ;
#else
  cerr << "**ERROR The use of global_ran is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1); 
  return 0 ;
#endif
}

inline int CALL_global_gbf (int nel, float *vertex_w, int *edge_p,
                            int *edge, int *edge_w, int np, int *neu)
{
#if HAVE_PARTY 
  return :: global_gbf (nel, vertex_w, edge_p, edge, edge_w, np, neu) ;
#else
  cerr << "**ERROR The use of global_gbf is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1); 
  return 0 ;
#endif
}

inline int CALL_global_gcf (int nel, float *vertex_w, int *edge_p,
                            int *edge, int *edge_w, int np, int *neu)
{
#if HAVE_PARTY 
  return :: global_gcf (nel, vertex_w, edge_p, edge, edge_w, np, neu) ;
#else
  cerr << "**ERROR The use of global_gcf is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1); 
  return 0 ;
#endif
}

inline int CALL_local_kl (int nel, float *vertex_w, int *edge_p, 
                          int *edge, int *edge_w, int np, int * neu, int output)
{
#if HAVE_PARTY 
  return :: local_kl (nel, vertex_w, edge_p, edge, edge_w, np,  neu, output) ;
#else
  cerr << "**ERROR The use of local_kl is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1); 
  return 0 ;
#endif
}

inline int CALL_local_hs (int nel, float *vertex_w, int *edge_p, 
                          int *edge, int *edge_w, int np, int * neu, int output)
{
#if HAVE_PARTY 
  return :: local_hs (nel, vertex_w, edge_p, edge, edge_w, np,  neu, output) ;
#else
  cerr << "**ERROR The use of local_hs is not supported, when the PARTY library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmess << endl ;
  exit(1); 
  return 0 ;
#endif
}
#endif
