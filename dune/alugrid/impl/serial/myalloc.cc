#include <config.h>

#include <dune/alugrid/common/alugrid_assert.hh>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <stack>

#include "myalloc.h"

#ifndef DONT_USE_ALUGRID_ALLOC
#warning "Using ALUGrid's internal memory management!"

namespace ALUGrid
{

  // MyAlloc initialize flag
  bool MyAlloc::_initialized = false;

  const size_t MyAlloc::MAX_HOLD_ADD  = 40000;  // max MAX_HOLD_ADD Objekte werden gespeichert
  const double MyAlloc::MAX_HOLD_MULT = 0.25;   // max das MAX_HOLD_MULT-fache der momentan
                                                   // aktiven Objekte werden gespeichert
  // if true objects could be freeed
  bool MyAlloc::_freeAllowed = true;

  static size_t ALUGridMemSpaceAllocated = 0;

#ifdef ALUGRID_USES_DLMALLOC
#define ONLY_MSPACES 1
#warning "Using DL malloc"
#include DLMALLOC_SOURCE_INCLUDE
#undef ONLY_MSPACES
  static void*  ALUGridMemorySpace = 0;
#else

  // class to store items of same size in a stack
  // also number of used items outside is stored
  class AllocEntry
  {
  public:

    // N verfolgt die Anzahl der angelegten Objekte der entsprechenden
    // Gr"osse, die in Gebrauch sind, auf dem Stack liegen die Objekte,
    // f"ur die delete aufgerufen wurde, die aber nicht an free () zur"uck-
    // gegeben werden um Fragmentierung zu vermeiden.

    size_t N;

    typedef std::stack< void * > memstack_t;
    memstack_t  S;

    AllocEntry () : N (0), S () {}

    AllocEntry(const AllocEntry& other)
      : N(other.N) , S(other.S)
    {}

    ~AllocEntry ()
    {
      // only free memory here if garbage collection is disabled
      while (!S.empty ())
      {
        std::free (S.top ());
        S.pop ();
      }
    }
  };

  // map holding AllocEntries for sizes
  typedef std::map< std::size_t, AllocEntry > memorymap_t;
  static  memorymap_t *freeStore = 0;
  static  std::set< void * > myAllocFreeLockers;

#endif

  /*
  //! get memory in MB
  double getMemoryUsage()
  {
    struct rusage info;
    getrusage( RUSAGE_SELF, &info );
    return (info.ru_maxrss / 1024.0);
  }

  class AllocCounter
  {
    size_t _allocatedBytes;
    AllocCounter () : _allocatedBytes( 0 ) {}
    ~AllocCounter()
    {
      cout << "On exit: ";
      print();
    }
  public:
    static void print()
    {
      cout << "bytes allocated = " << instance()._allocatedBytes << endl;
      cout << "rusage: " << getMemoryUsage() << endl;
    }
    static void add( size_t i )
    {
      instance()._allocatedBytes += i;
    }
    static void substract ( size_t i )
    {
      instance()._allocatedBytes -= i;
    }

    static AllocCounter& instance ()
    {
      static AllocCounter obj;
      return obj;
    }
  };

  void printMemoryBytesUsed ()
  {
    AllocCounter::print();
    cout << "rusage: " << getMemoryUsage() << endl;
  }
  */

  void MyAlloc::lockFree (void * addr)
  {
#ifndef ALUGRID_USES_DLMALLOC
    // remember address of locker
    myAllocFreeLockers.insert( addr );
    _freeAllowed = true;
#endif
  }

  void MyAlloc::unlockFree (void * addr)
  {
#ifndef ALUGRID_USES_DLMALLOC
    myAllocFreeLockers.erase( addr );
    // only if no-one else has locked
    if( myAllocFreeLockers.empty () )
    {
      _freeAllowed = false;
    }
#endif

    // make free memory available to the system again
    clearFreeMemory();
  }

#ifndef ALUGRID_USES_DLMALLOC
  void memAllocate( AllocEntry& fs, const size_t s, void* mem[], const size_t request)
  {
    alugrid_assert ( s > 0 );
    {
      fs.N += request;
      const std::size_t fsSize  = fs.S.size();
      const std::size_t popSize = request > fsSize ? fsSize : request;
      const std::size_t newSize = request - popSize;
      for( std::size_t i = 0; i<newSize; ++i )
      {
        mem[ i ] = malloc ( s );
        alugrid_assert ( mem[ i ] );
      }

      // pop the rest from the stack
      for( std::size_t i = newSize; i<popSize; ++i )
      {
        // get pointer from stack
        mem[ i ] = fs.S.top ();
        fs.S.pop ();
      }
    }
  }
#endif // end ALUGRID_USES_DLMALLOC

  void* MyAlloc::operator new ( size_t s )
  {
#ifndef DONT_USE_ALUGRID_ALLOC
    // increase memory usage counter
    ALUGridMemSpaceAllocated += s ;

#ifdef ALUGRID_USES_DLMALLOC
    alugrid_assert ( s > 0 );
    return mspace_malloc( ALUGridMemorySpace, s );
#else
    alugrid_assert (s > 0);
    {
      AllocEntry & fs = ((*freeStore) [s]);
      ++ fs.N;
      if ( fs.S.empty () )
      {
        // else simply allocate block
        void * p = malloc (s);
        if( !p )
        {
          std::cerr << "ERROR: Out of memory." << std::endl;
          throw OutOfMemoryException();
        }
        return p;
      }
      else
      {
        // get pointer from stack
        void * p = fs.S.top ();
        fs.S.pop ();
        return p;
      }
    }
#endif // ALUGRID_USES_DLMALLOC
#endif // DONT_USE_ALUGRID_ALLOC
  }

  // operator delete, put pointer to stack
  void MyAlloc::operator delete (void *ptr, size_t s)
  {
#ifndef DONT_USE_ALUGRID_ALLOC
   // decrease memory usage counter
   ALUGridMemSpaceAllocated -= s ;

#ifdef ALUGRID_USES_DLMALLOC
   // defined in dlmalloc.c
   mspace_free( ALUGridMemorySpace, ptr );
#else
    // get stack for size s
    AllocEntry & fs ((*freeStore) [s]);
    // push pointer to stack
    alugrid_assert (fs.N > 0);
    --fs.N;
    fs.S.push (ptr);

    // if free of objects is allowd
    // if( _freeAllowed )
    {
      // check if max size is exceeded
      const size_t stackSize = fs.S.size ();
      if ( ( stackSize >= MAX_HOLD_ADD ) &&
           ( double (stackSize) >= MAX_HOLD_MULT * double (fs.N) )
         )
      {
        alugrid_assert (!fs.S.empty());
        free ( fs.S.top () );
        fs.S.pop();
      }
    }
#endif // end #ifdef ALUGRID_USES_DLMALLOC
#endif // end #ifndef DONT_USE_ALUGRID_ALLOC
  }

  // operator delete, put pointer to stack
  void MyAlloc::clearFreeMemory ()
  {
#ifdef ALUGRID_USES_DLMALLOC
    // if no objects are allocated clear memory space and reallocate
    // this will free memory to the system
    if ( ALUGridMemSpaceAllocated == 0 )
    {
      destroy_mspace( ALUGridMemorySpace );
      ALUGridMemorySpace = create_mspace( 0, 0 );
    }
#endif
  }

  // operator delete, put pointer to stack
  size_t MyAlloc::allocatedMemory ()
  {
    return ALUGridMemSpaceAllocated ;
  }

  MyAlloc::Initializer::Initializer ()
  {
    if ( ! MyAlloc::_initialized )
    {
#ifdef ALUGRID_USES_DLMALLOC
      ALUGridMemorySpace = create_mspace( 0, 0 );
#else
      freeStore = new memorymap_t ();
      alugrid_assert (freeStore);
#endif

      MyAlloc::_initialized = true;
    }
    return;
  }

  MyAlloc::Initializer::~Initializer ()
  {
    if ( MyAlloc::_initialized )
    {
#ifdef ALUGRID_USES_DLMALLOC
      if( ALUGridMemorySpace )
      {
        destroy_mspace( ALUGridMemorySpace );
        ALUGridMemorySpace = 0;
      }
#else
      if(freeStore)
      {
        delete freeStore;
        freeStore = 0;
      }
#endif

      MyAlloc::_initialized = false;
    }
  }

} // namespace ALUGrid

#endif // #ifndef DONT_USE_ALUGRID_ALLOC
