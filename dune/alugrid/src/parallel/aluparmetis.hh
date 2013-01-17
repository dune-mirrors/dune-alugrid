#ifndef PARMETIS_H_INCLUDED
#define PARMETIS_H_INCLUDED

#include "alumetis.hh"

#if HAVE_PARMETIS
// partitioning libs 
// ParMETIS if not found here then dummy version is included 
extern "C" {
  #include <parmetis.h>
}

#else  
static const char parmetmess [] =
        "**INFO Due to license reasons the library ParMETIS is\n"
        "       not part of the ALUGrid library distribution. \n"
        "       To use this feature get a copy of the ParMETIS library \n"
        "       (see http://www-users.cs.umn.edu/~karypis/metis/metis/ )\n"
        "       and re-configure the ALUGrid library with the \n"
        "       --with-parmetis=PATH_TO_ParMETIS option, \n"
        "       or choose another Graph partitioning method. \n"
        "       Exiting program, bye! \n";
#endif

namespace ALUGridParMETIS 
{

typedef ALUGridMETIS :: idxtype  idxtype ;
typedef ALUGridMETIS :: realtype realtype ;

inline void CALL_ParMETIS_V3_PartKway(
             idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
             idxtype *adjwgt, idxtype *wgtflag, idxtype *numflag, idxtype *ncon, idxtype *nparts,
             realtype *tpwgts, realtype *ubvec, idxtype *options, idxtype *edgecut, idxtype *part,
             MPI_Comm *comm)
{
#if HAVE_PARMETIS
   :: ParMETIS_V3_PartKway(vtxdist, xadj, adjncy, vwgt, adjwgt,
                           wgtflag, numflag, ncon, nparts, tpwgts,
                           ubvec, options, edgecut, part, comm ) ;
#else 
  std::cerr << "**ERROR The use of ParMETIS is not supported, when the ParMETIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  std::cerr << parmetmess << std::endl ;
  exit(1);
#endif
  return ;
}

inline void CALL_ParMETIS_V3_AdaptiveRepart(
       idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
       idxtype *vsize, idxtype *adjwgt, idxtype *wgtflag, idxtype *numflag, idxtype *ncon,
       idxtype *nparts, realtype *tpwgts, realtype *ubvec, realtype *ipc2redist,
       idxtype *options, idxtype *edgecut, idxtype *part, MPI_Comm *comm)
{
#if HAVE_PARMETIS
  :: ParMETIS_V3_AdaptiveRepart(vtxdist, xadj, adjncy, vwgt, vsize, adjwgt,
                                wgtflag, numflag, ncon, nparts, tpwgts,
                                ubvec, ipc2redist, options, edgecut, part, comm ) ;
#else 
  std::cerr << "**ERROR The use of ParMETIS is not supported, when the ParMETIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  std::cerr << parmetmess << std::endl ;
  exit(1);
#endif
  return ;
}

}
#endif
