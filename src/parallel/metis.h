// (c) --
#if defined METISTITLE || defined METIS_H 
#define METIS_H_INCLUDED
#endif

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

void METIS_PartGraphKway(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *);
void METIS_PartGraphRecursive(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *);

#endif
