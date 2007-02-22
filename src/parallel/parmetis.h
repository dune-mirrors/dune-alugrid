// (c) --
#ifndef __parmetis_h__

#ifndef PARMETIS_H_INCLUDED
#define PARMETIS_H_INCLUDED

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

void ParMETIS_V3_PartKway(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *, MPI_Comm *);

#endif
#endif
