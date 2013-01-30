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
#include "mpAccess_MPI.h"
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
#if HAVE_METIS
  typedef ALUGridMETIS :: idxtype  idxtype ;
  typedef ALUGridMETIS :: realtype realtype ;
#elif HAVE_PARMETIS // #if HAVE_METIS
#if PARMETIS_MAJOR_VERSION >= 4
  typedef idx_t  idxtype ;
  typedef real_t realtype ;
#else // #if PARMETIS_MAJOR_VERSION >= 4
  typedef ::idxtype idxtype;
  typedef float     realtype;
#endif // #else // #if PARMETIS_MAJOR_VERSION >= 4
#else // #elif HAVE_PARMETIS // #if HAVE_METIS
  typedef int   idxtype ;
  typedef float realtype ;
#endif // #else // #elif HAVE_PARMETIS // #if HAVE_METIS

  inline void CALL_ParMETIS_V3_PartKway(
               idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
               idxtype *adjwgt, idxtype *wgtflag, idxtype *numflag, idxtype *ncon, idxtype *nparts,
               realtype *tpwgts, realtype *ubvec, idxtype *options, idxtype *edgecut, idxtype *part,
               ALUGrid::MpAccessGlobal &mpa )
  {
#if HAVE_PARMETIS
    ALUGrid::MpAccessMPI* mpaMPI = dynamic_cast<ALUGrid::MpAccessMPI *> (&mpa);
    if( mpaMPI == 0 ) 
    {
      std::cerr << "ERROR: wrong mpAccess object, couldn't convert to MpAccessMPI!! in: " << __FILE__ << " line: " << __LINE__ << std::endl;
      abort();
    }

    // get communincator (see mpAccess_MPI.cc
    MPI_Comm comm = mpaMPI->communicator();

     :: ParMETIS_V3_PartKway(vtxdist, xadj, adjncy, vwgt, adjwgt,
                             wgtflag, numflag, ncon, nparts, tpwgts,
                             ubvec, options, edgecut, part, &comm ) ;
#else // #if HAVE_PARMETIS
    std::cerr << "**ERROR The use of ParMETIS is not supported, when the ParMETIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << std::endl;
    std::cerr << parmetmess << std::endl ;
    exit( 1 );
#endif // #else // #if HAVE_PARMETIS
  }

  inline void CALL_ParMETIS_V3_AdaptiveRepart(
         idxtype *vtxdist, idxtype *xadj, idxtype *adjncy, idxtype *vwgt,
         idxtype *vsize, idxtype *adjwgt, idxtype *wgtflag, idxtype *numflag, idxtype *ncon,
         idxtype *nparts, realtype *tpwgts, realtype *ubvec, realtype *ipc2redist,
         idxtype *options, idxtype *edgecut, idxtype *part,
         ALUGrid::MpAccessGlobal &mpa )
  {
#if HAVE_PARMETIS
    // get communincator (see mpAccess_MPI.cc)
    ALUGrid::MpAccessMPI* mpaMPI = dynamic_cast<ALUGrid::MpAccessMPI *> (&mpa);
    if( mpaMPI == 0 ) 
    {
      std::cerr << "ERROR: wrong mpAccess object, couldn't convert to MpAccessMPI!! in: " << __FILE__ << " line: " << __LINE__ << std::endl;
      abort();
    }

    // get communincator (see mpAccess_MPI.cc)
    MPI_Comm comm = mpaMPI->communicator();

    :: ParMETIS_V3_AdaptiveRepart(vtxdist, xadj, adjncy, vwgt, vsize, adjwgt,
                                  wgtflag, numflag, ncon, nparts, tpwgts,
                                  ubvec, ipc2redist, options, edgecut, part, &comm ) ;
#else // #if HAVE_PARMETIS
    std::cerr << "**ERROR The use of ParMETIS is not supported, when the ParMETIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
    std::cerr << parmetmess << std::endl ;
    exit( 1 );
#endif // #else // #if HAVE_PARMETIS
  }

} // namespace ALUGridParMETIS 

#endif // #ifndef PARMETIS_H_INCLUDED
