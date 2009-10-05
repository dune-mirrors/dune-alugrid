// (c) --
#ifndef __parmetis_h__

#warning  -- Compiling parallel version without ParMETIS !!!

#ifndef PARMETIS_C_INCLUDED
#define PARMETIS_C_INCLUDED

using namespace std; 

static const char parmetmess [] = 
        "**INFO Due to license reasons the library ParMETIS is\n"
				"       not part of the ALUGrid library distribution. \n"
				"       To use this feature get a copy of the ParMETIS library \n"
        "       (see http://www-users.cs.umn.edu/~karypis/metis/metis/ )\n"
        "       and re-configure the ALUGrid library with the \n"
        "       --with-parmetis=PATH_TO_ParMETIS option, \n"
        "       or choose another Graph partitioning method. \n"
        "       Exiting program, bye! \n";

void parMetisErroMSG() 
{
  cerr << "**ERROR The use of ParMETIS is not supported, when the ParMETIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmetmess << endl ;
  exit(1); 
  return ;
}

// dummy ParMETIS_V3_PartKway, when ParMETIS library is missing 
void ParMETIS_V3_PartKway(
             idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
             idxtype *adjwgt, int *wgtflag, int *numflag, int *ncon, int *nparts,
             float *tpwgts, float *ubvec, int *options, int *edgecut, idxtype *part,
             MPI_Comm *comm)
{
  parMetisErroMSG();
}

// dummy ParMETIS_V3_AdaptiveRepart, when ParMETIS library is missing 
void ParMETIS_V3_AdaptiveRepart(
       idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
       idxtype *vsize, idxtype *adjwgt, int *wgtflag, int *numflag, int *ncon,
       int *nparts, float *tpwgts, float *ubvec, float *ipc2redist,
       int *options, int *edgecut, idxtype *part, MPI_Comm *comm)
{
  parMetisErroMSG();
}

#endif
#endif
