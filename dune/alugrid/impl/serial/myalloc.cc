#include <config.h>

#include <dune/alugrid/common/alugrid_assert.hh>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <stack>

#include "myalloc.h"

#ifdef ALUGRID_USES_DLMALLOC

namespace ALUGrid
{

  // DLMalloc initialize flag
  bool DLMalloc::_initialized = false;

  static size_t ALUGridMemSpaceAllocated = 0;

#define ONLY_MSPACES 1
#warning "Using DL malloc"
#include DLMALLOC_SOURCE_INCLUDE
#undef ONLY_MSPACES
  static void*  ALUGridMemorySpace = 0;

  void DLMalloc::lockFree (void * addr)
  {
  }

  void DLMalloc::unlockFree (void * addr)
  {

    // make free memory available to the system again
    clearFreeMemory();
  }

  void* DLMalloc::operator new ( size_t s )
  {
    alugrid_assert ( s > 0 );
    return mspace_malloc( ALUGridMemorySpace, s );
  }

  // operator delete, put pointer to stack
  void DLMalloc::operator delete (void *ptr, size_t s)
  {
   // decrease memory usage counter
   ALUGridMemSpaceAllocated -= s ;

   // defined in dlmalloc.c
   mspace_free( ALUGridMemorySpace, ptr );
  }

  // operator delete, put pointer to stack
  void DLMalloc::clearFreeMemory ()
  {
    // if no objects are allocated clear memory space and reallocate
    // this will free memory to the system
    if ( ALUGridMemSpaceAllocated == 0 )
    {
      destroy_mspace( ALUGridMemorySpace );
      ALUGridMemorySpace = create_mspace( 0, 0 );
    }
  }

  // operator delete, put pointer to stack
  size_t DLMalloc::allocatedMemory ()
  {
    return ALUGridMemSpaceAllocated ;
  }

  DLMalloc::Initializer::Initializer ()
  {
    if ( ! DLMalloc::_initialized )
    {
      ALUGridMemorySpace = create_mspace( 0, 0 );
      DLMalloc::_initialized = true;
    }
    return;
  }

  DLMalloc::Initializer::~Initializer ()
  {
    if ( DLMalloc::_initialized )
    {
      if( ALUGridMemorySpace )
      {
        destroy_mspace( ALUGridMemorySpace );
        ALUGridMemorySpace = 0;
      }

      DLMalloc::_initialized = false;
    }
  }

} // namespace ALUGrid

#endif // #ifdef ALUGRID_USES_DLMALLOC
