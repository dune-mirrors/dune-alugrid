#include <config.h>

#ifndef DONT_USE_ALUGRID_ALLOC
#warning "Using ALUGrid's internal memory management!"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <stack>

//#define ONLY_MSPACES 1 

#include "myalloc.h"

// MyAlloc initialize flag                                                   
bool MyAlloc :: _initialized = false;

const size_t MyAlloc :: MAX_HOLD_ADD  = 40000 ;  // max MAX_HOLD_ADD Objekte werden gespeichert
const double MyAlloc :: MAX_HOLD_MULT = 0.25 ;   // max das MAX_HOLD_MULT-fache der momentan
                                                 // aktiven Objekte werden gespeichert
// if true objects could be freeed  
bool MyAlloc :: _freeAllowed = true ;

#if ONLY_MSPACES 
#warning "Using DL malloc"
#include "dlmalloc.c"
static void*  ALUGridMemorySpace = 0 ;
static size_t ALUGridMemSpaceAllocated = 0;
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

  size_t N ;

  typedef std::stack< void * > memstack_t ;
  memstack_t  S ;

  AllocEntry () : N (0), S () {}

  AllocEntry(const AllocEntry& other) 
    : N(other.N) , S(other.S) 
  {} 
  
  ~AllocEntry () 
  {
    // only free memory here if garbage collection is disabled
    while (!S.empty ()) 
    {
      std::free (S.top ()) ;
      S.pop () ;
    }
  }
};

// map holding AllocEntries for sizes 
typedef std::map< std::size_t, AllocEntry > memorymap_t ;
static  memorymap_t *freeStore = 0;
static  std::set< void * > myAllocFreeLockers;

#endif // end else of #if ONLY_MSPACES

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
    cout << "On exit: " ;
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
    instance()._allocatedBytes += i ;
  }
  static void substract ( size_t i ) 
  {
    instance()._allocatedBytes -= i ;
  }

  static AllocCounter& instance () 
  {
    static AllocCounter obj ;
    return obj ;
  }
};

void printMemoryBytesUsed () 
{
  AllocCounter::print();
  cout << "rusage: " << getMemoryUsage() << endl;
}
*/

void MyAlloc :: lockFree (void * addr) 
{
#if ! ONLY_MSPACES
  // remember address of locker 
  myAllocFreeLockers.insert( addr );
  _freeAllowed = true ; 
#endif
}

void MyAlloc :: unlockFree (void * addr) 
{
#if ! ONLY_MSPACES
  myAllocFreeLockers.erase( addr );
  // only if no-one else has locked 
  if( myAllocFreeLockers.empty () )
  {
    _freeAllowed = false ; 
  }
#endif

  // make free memory available to the system again 
  clearFreeMemory();
}

#if ! ONLY_MSPACES
void memAllocate( AllocEntry& fs, const size_t s, void* mem[], const size_t request)
{
  assert( s > 0 );
  {
    fs.N += request ;
    const std::size_t fsSize  = fs.S.size();
    const std::size_t popSize = request > fsSize ? fsSize : request ;
    const std::size_t newSize = request - popSize ;
    for( std::size_t i = 0; i<newSize; ++i )
    {
      mem[ i ] = malloc ( s ) ;
      assert( mem[ i ] );
    }

    // pop the rest from the stack
    for( std::size_t i = newSize; i<popSize; ++i ) 
    {
      // get pointer from stack 
      mem[ i ] = fs.S.top () ;
      fs.S.pop () ;
    }
  }
}

#ifdef USE_MALLOC_AT_ONCE
void MyAlloc :: allocate( const size_t s, void* mem[], const size_t request) throw (OutOfMemoryException) 
{
  AllocEntry & fs ((*freeStore) [s]) ;
  memAllocate( fs, s, mem, request );
}
#endif // end #ifdef USE_MALLOC_AT_ONCE
#endif // end #if ! ONLY_MSPACES

void* MyAlloc :: operator new ( size_t s ) throw (OutOfMemoryException) 
{
#ifndef DONT_USE_ALUGRID_ALLOC

#if ONLY_MSPACES
  assert( s > 0 );
  ++ ALUGridMemSpaceAllocated ;
  return mspace_malloc( ALUGridMemorySpace, s );
#else
  assert(s > 0);
  {
    AllocEntry & fs = ((*freeStore) [s]) ;
    ++ fs.N ;
    if ( fs.S.empty () ) 
    {
      // else simply allocate block 
      void * p = malloc (s) ;
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
      void * p = fs.S.top () ;
      fs.S.pop () ;
      return p ;
    }
  }
#endif // ONLY_MSPACES 
#endif // DONT_USE_ALUGRID_ALLOC
}

// operator delete, put pointer to stack 
void MyAlloc :: operator delete (void *ptr, size_t s) 
{
#ifndef DONT_USE_ALUGRID_ALLOC

#if ONLY_MSPACES
 // defined in dlmalloc.c 
 mspace_free( ALUGridMemorySpace, ptr );
 -- ALUGridMemSpaceAllocated ;
#else  
  // get stack for size s 
  AllocEntry & fs ((*freeStore) [s]) ;
  // push pointer to stack 
  assert (fs.N > 0) ;
  --fs.N ;
  fs.S.push (ptr) ;
 
  // if free of objects is allowd 
  // if( _freeAllowed )
  {
    // check if max size is exceeded 
    const size_t stackSize = fs.S.size ();
    if ( ( stackSize >= MAX_HOLD_ADD ) && 
         ( double (stackSize) >= MAX_HOLD_MULT * double (fs.N) )
       ) 
    {
      assert (!fs.S.empty()) ;
      free ( fs.S.top () ) ;
      fs.S.pop() ;
    }
  }
#endif // end #if     ONLY_MSPACES 
#endif // end #ifndef DONT_USE_ALUGRID_ALLOC
}

// operator delete, put pointer to stack 
void MyAlloc :: clearFreeMemory () 
{
#if ONLY_MSPACES
  // if no objects are allocated clear memory space and reallocate
  // this will free memory to the system 
  if ( ALUGridMemSpaceAllocated == 0 ) 
  {
    destroy_mspace( ALUGridMemorySpace );
    ALUGridMemorySpace = create_mspace( 0, 0 );
  }
#endif
}

MyAlloc :: Initializer :: Initializer () 
{
  if ( ! MyAlloc :: _initialized ) 
  {
#if ONLY_MSPACES 
    ALUGridMemorySpace = create_mspace( 0, 0 );
#else
    freeStore = new memorymap_t () ;
    assert (freeStore) ;
#endif

    MyAlloc :: _initialized = true;
  }
  return ;
}

MyAlloc :: Initializer :: ~Initializer () 
{
  if ( MyAlloc :: _initialized ) 
  {
#if ONLY_MSPACES 
    if( ALUGridMemorySpace ) 
    {
      destroy_mspace( ALUGridMemorySpace );
      ALUGridMemorySpace = 0;
    }
#else
    if(freeStore) 
    {
      delete freeStore ;
      freeStore = 0 ;
    }
#endif

    MyAlloc :: _initialized = false;
  }
  return ;
}
#endif // DONT_USE_ALUGRID_ALLOC 
