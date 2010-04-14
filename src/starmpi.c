#include <config.h>

#if HAVE_STARMPI
// compile STAR-MPI object file
#if defined(__cplusplus)
extern "C" {
#endif

// include STAR-MPI sources to compile object file
#include <star-mpi.c>

void InitializeStarMPI() 
{
  static int initialized = 0;
  if( ! initialized ) 
  {
    STAR_Init();
    initialized = 1;
  }
}

void FinalizeStarMPI() 
{
  static int finalized = 0;
  if( ! finalized ) 
  {
    STAR_Finalize();
    finalized = 1;
  }
}

#if defined(__cplusplus)
} 
#endif

#endif
