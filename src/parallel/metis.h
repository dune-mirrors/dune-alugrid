// (c) --
#if defined METISTITLE || defined METIS_H 
#define METIS_H_INCLUDED
#endif

#ifndef METIS_H_INCLUDED
#define METIS_H_INCLUDED

typedef int idxtype ;

void METIS_PartGraphKway(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *);
void METIS_PartGraphRecursive(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *);

#endif
