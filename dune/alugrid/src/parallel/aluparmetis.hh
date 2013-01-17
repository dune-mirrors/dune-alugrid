#ifndef PARMETIS_H_INCLUDED
#define PARMETIS_H_INCLUDED

#include "alumetis.hh"

#include "mpAccess.h"

#if HAVE_PARMETIS
// partitioning libs 
// ParMETIS if not found here then dummy version is included 
extern "C" {
  #include <parmetis.h>
}
#else // #if HAVE_PARMETIS
static const char parmetmess[]
  = "**INFO Due to license reasons the library ParMETIS is\n"
    "       not part of the ALUGrid library distribution. \n"
    "       To use this feature get a copy of the ParMETIS library \n"
    "       (see http://www-users.cs.umn.edu/~karypis/metis/metis/ )\n"
    "       and re-configure the ALUGrid library with the \n"
    "       --with-parmetis=PATH_TO_ParMETIS option, \n"
    "       or choose another Graph partitioning method. \n"
    "       Exiting program, bye! \n";
#endif // #else // #if HAVE_PARMETIS

namespace ALUGridParMETIS 
{

  typedef ALUGridMETIS :: idxtype  idxtype ;
  typedef ALUGridMETIS :: realtype realtype ;

  inline void CALL_ParMETIS_V3_PartKway(
               idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
               idxtype *adjwgt, idxtype *wgtflag, idxtype *numflag, idxtype *ncon, idxtype *nparts,
               realtype *tpwgts, realtype *ubvec, idxtype *options, idxtype *edgecut, idxtype *part,
               MpAccessGlobal &mpa )
  {
#if HAVE_PARMETIS
    // get communincator (see mpAccess_MPI.cc)
    MPI_Comm comm = getMPICommunicator( mpa.communicator() );
     :: ParMETIS_V3_PartKway(vtxdist, xadj, adjncy, vwgt, adjwgt,
                             wgtflag, numflag, ncon, nparts, tpwgts,
                             ubvec, options, edgecut, part, &comm ) ;
#else // #if HAVE_PARMETIS
    std::cerr << "**ERROR The use of ParMETIS is not supported, when the ParMETIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
    std::cerr << parmetmess << std::endl ;
    exit( 1 );
#endif // #else // #if HAVE_PARMETIS
  }

  inline void CALL_ParMETIS_V3_AdaptiveRepart(
         idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
         idxtype *vsize, idxtype *adjwgt, idxtype *wgtflag, idxtype *numflag, idxtype *ncon,
         idxtype *nparts, realtype *tpwgts, realtype *ubvec, realtype *ipc2redist,
         idxtype *options, idxtype *edgecut, idxtype *part,
         MpAccessGlobal &mpa )
  {
#if HAVE_PARMETIS
    // get communincator (see mpAccess_MPI.cc)
    MPI_Comm comm = getMPICommunicator( mpa.communicator() );
    :: ParMETIS_V3_AdaptiveRepart(vtxdist, xadj, adjncy, vwgt, vsize, adjwgt,
                                  wgtflag, numflag, ncon, nparts, tpwgts,
                                  ubvec, ipc2redist, options, edgecut, part, &comm ) ;
#else // #if HAVE_PARMETIS
    std::cerr << "**ERROR The use of ParMETIS is not supported, when the ParMETIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
    std::cerr << parmetmess << std::endl ;
    exit( 1 );
#endif // #else // #if HAVE_PARMETIS
  }

}

#endif // #ifndef PARMETIS_H_INCLUDED
