// (c) --
#ifndef __parmetis_h__

#ifndef PARMETIS_H_INCLUDED
#define PARMETIS_H_INCLUDED

typedef int idxtype ;

extern 
void ParMETIS_V3_PartKway(
             idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
             idxtype *adjwgt, int *wgtflag, int *numflag, int *ncon, int *nparts,
             float *tpwgts, float *ubvec, int *options, int *edgecut, idxtype *part,
             MPI_Comm *comm);

extern 
void ParMETIS_V3_AdaptiveRepart(
       idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
       idxtype *vsize, idxtype *adjwgt, int *wgtflag, int *numflag, int *ncon,
       int *nparts, float *tpwgts, float *ubvec, float *ipc2redist,
       int *options, int *edgecut, idxtype *part, MPI_Comm *comm);

#endif
#endif
