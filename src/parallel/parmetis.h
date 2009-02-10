// (c) --
#ifndef __parmetis_h__

#ifndef PARMETIS_H_INCLUDED
#define PARMETIS_H_INCLUDED

typedef int idxtype ;

void ParMETIS_V3_PartKway(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *, MPI_Comm *);

#endif
#endif
