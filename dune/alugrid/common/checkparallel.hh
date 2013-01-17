#ifndef DUNE_ALUGRID_CHECKPARALLEL_HH
#define DUNE_ALUGRID_CHECKPARALLEL_HH

#include <dune/alugrid/src/parallel/alugrid_defineparallel.h>

#if HAVE_MPI
  // if this variable is defined, 
  // // then parallel version of ALUGrid is compiled
  #if ALU3DGRID_BUILD_FOR_PARALLEL == 0
    #warning "The ALUGrid-library wasn't compiled for parallel usage. Reconfigure\
 using the MPI compiler script or compile Dune without the MPI support!\
 Defaulting to serial ALUGrid!"
    #define ALU3DGRID_PARALLEL 0
  #else 
    #define ALU3DGRID_PARALLEL 1
  #endif
#else  // #if HAVE_MPI
  #define ALU3DGRID_PARALLEL 0
#endif // #else  // #if HAVE_MPI

#endif // #ifndef DUNE_ALUGRID_CHECKPARALLEL_HH
