#include <config.h>

#if HAVE_STARMPI
// compile STAR-MPI object file
#if defined(__cplusplus)
extern "C" {
#endif

// include STAR-MPI sources to compile object file
#include <star-mpi.c>

#if defined(__cplusplus)
} 
#endif

#endif
