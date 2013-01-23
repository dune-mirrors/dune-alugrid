#ifndef DUNE_ALUGRID_CHECKPARALLEL_HH
#define DUNE_ALUGRID_CHECKPARALLEL_HH

#if HAVE_MPI
  // if this variable is defined, 
  // then parallel version of ALUGrid is compiled
  #if HAVE_DUNE_ALUGRID_MPI
    #define ALU3DGRID_PARALLEL 1
  #else 
    #warning "The ALUGrid-library wasn't compiled for parallel usage. Reconfigure\
 using the MPI compiler script or compile Dune without the MPI support!\
 Defaulting to serial ALUGrid!"
    #define ALU3DGRID_PARALLEL 0
  #endif
#else  // #else  #if HAVE_MPI
  #define ALU3DGRID_PARALLEL 0
#endif // #endif // #if HAVE_MPI

#endif // #ifndef DUNE_ALUGRID_CHECKPARALLEL_HH
