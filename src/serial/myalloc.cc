#ifndef DONT_USE_ALUGRID_ALLOC

#include "myalloc.h"

const size_t MyAlloc :: MAX_HOLD_ADD  = 40000 ;  // max MAX_HOLD_ADD Objekte werden gespeichert
const double MyAlloc :: MAX_HOLD_MULT = 0.25 ;   // max das MAX_HOLD_MULT-fache der momentan
                                                 // aktiven Objekte werden gespeichert
// MyAlloc initialize flag                                                   
bool MyAlloc :: _initialized = false;

// if true objects could be freeed  
bool MyAlloc :: _freeAllowed = true ;

#ifdef ALUGRID_GARBAGE_COLLECTION
class MemoryBlock 
{
  //static const size_t _memSize = 32768 ; // 2^15 
  static const size_t _memSize = 131072; // 2^17
  void*  _mem ;

  typedef unsigned int uint_t ;
  uint_t _pos ;
  uint_t _counter ;

public:  
  MemoryBlock() : _mem( 0 ), _pos( 0 ), _counter( 0 )
  {
    _mem = std::malloc( _memSize );
    //assert( sizeof(char) == 1 );
    assert( _mem );
  }

  ~MemoryBlock() 
  {
    std::free( _mem );
    // all allocted entries should have been freed.
    assert( empty() );
  }

  void* alloc( const size_t request ) 
  {
    if( (_pos + request) > _memSize ) 
      return 0;
    else 
    {
      assert( sizeof( char ) == 1 );
      // get pointer 
      char* p = ((char *) _mem) + _pos ;
      _pos += request ; 
      _counter += request ;
      return p;
    }
  }

  bool inside( char* p ) const 
  {
    char* mem = (char *) _mem;
    return ( p >= mem ) && ( p < ( mem + _memSize ) ); 
  }

  void free( void* p, const size_t psize ) 
  {
    if( inside( (char*) p ) ) 
    {
      assert( _counter >= psize );
      _counter -= psize ;
    }
  }

  bool empty () const 
  {
    return _counter == 0 ;
  }

  //! comparison operator for map storage 
  bool operator < ( const MemoryBlock& mb ) const 
  {
    return _mem < mb._mem ;
  }

  template <class memoryvec_t>
  static MemoryBlock* find( memoryvec_t& memvec, void* p ) 
  {
    typedef typename memoryvec_t :: iterator iterator ;
    const iterator end = memvec.end(); 
    for( iterator it = memvec.begin(); it != end; ++it ) 
    {
      if( (*it)->inside( (char*) p ) )
        return (*it);
    }
    // not found is not an option 
    abort();
    return 0;
  }
};
// global memory block list 
typedef list< MemoryBlock* > MemoryBlockVector_t ;
static MemoryBlockVector_t* memoryBlocks = 0 ;

#endif // end ifdef ALUGRID_GARBAGE_COLLECTION

// class to store items of same size in a stack 
// also number of used items outside is stored 
class AllocEntry {

public:

  // N verfolgt die Anzahl der angelegten Objekte der entsprechenden
  // Gr"osse, die in Gebrauch sind, auf dem Stack liegen die Objekte,
  // f"ur die delete aufgerufen wurde, die aber nicht an free () zur"uck-
  // gegeben werden um Fragmentierung zu vermeiden.

  size_t N ;

  typedef stack < void * > memstack_t ;
  memstack_t  S ;

  AllocEntry () : N (0), S () {}

  AllocEntry(const AllocEntry& other) 
    : N(other.N) , S(other.S) 
  {} 
  
  ~AllocEntry () 
  {
#ifndef ALUGRID_GARBAGE_COLLECTION
    // only free memory here if garbage collection is disabled
    while (!S.empty ()) 
    {
      free (S.top ()) ;
      S.pop () ;
    }
#endif
    return ;
  }
};

// map holding AllocEntries for sizes 
typedef map < size_t, AllocEntry, less < size_t > > memorymap_t ;
static memorymap_t* freeStore = 0 ;
static set < void * > myAllocFreeLockers;

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
  // remember address of locker 
  myAllocFreeLockers.insert( addr );
  _freeAllowed = true ; 
}

void MyAlloc :: unlockFree (void * addr) 
{
  myAllocFreeLockers.erase( addr );
  // only if no-one else has locked 
  if( myAllocFreeLockers.empty () )
  {
    _freeAllowed = false ; 
  }

  // make free memory available to the system again 
  clearFreeMemory();
}

void memAllocate( AllocEntry& fs, const size_t s, void* mem[], const size_t request)
{
  assert(s > 0);
  {
    fs.N += request ;
    const size_t fsSize  = fs.S.size();
    const size_t popSize = request > fsSize ? fsSize : request ;
    const size_t newSize = request - popSize ;
    for( size_t i = 0; i<newSize; ++i )
    {
      mem[ i ] = malloc ( s ) ;
      assert( mem[ i ] );
    }

    // pop the rest from the stack
    for( size_t i = newSize; i<popSize; ++i ) 
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
#endif

void* MyAlloc :: operator new ( size_t s ) throw (OutOfMemoryException) 
{
#ifndef DONT_USE_ALUGRID_ALLOC
  assert(s > 0);
  {
    AllocEntry & fs = ((*freeStore) [s]) ;
    ++ fs.N ;
    if ( fs.S.empty () ) 
    {
#ifdef ALUGRID_GARBAGE_COLLECTION
      assert( memoryBlocks->size() > 0 );
      MemoryBlock* current = memoryBlocks->back();
      assert( current );
      void* p = current->alloc( s );
      if( p == 0 ) 
      {
        MemoryBlock* newBlock = new MemoryBlock();
        assert( newBlock );
        memoryBlocks->push_back( newBlock );
        p = newBlock->alloc( s );
        // this should not be NULL
        assert( p );
      }
#else
      // else simply allocate block 
      void * p = malloc (s) ;
#endif
      if (p == 0) 
      {
        perror ("**ERROR (FATAL) in MyAlloc :: operator new ()") ;
        cerr << "**INFO MyAlloc :: operator new (" << s << "): Out of memory!!" << endl;
        throw OutOfMemoryException () ;
      }
      return p ;
    }
    else
    {
      // get pointer from stack 
      void * p = fs.S.top () ;
      fs.S.pop () ;
      return p ;
    }
  }
#endif
}

// operator delete, put pointer to stack 
void MyAlloc :: operator delete (void *ptr, size_t s) 
{
#ifndef DONT_USE_ALUGRID_ALLOC
  // get stack for size s 
  AllocEntry & fs ((*freeStore) [s]) ;
  // push pointer to stack 
  assert (fs.N > 0) ;
  --fs.N ;
  fs.S.push (ptr) ;
 
#ifndef ALUGRID_GARBAGE_COLLECTION
  // if free of objects is allowd 
  if( _freeAllowed )
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
#endif // end #ifndef ALUGRID_GARBAGE_COLLECTION
#endif // end #ifndef DONT_USE_ALUGRID_ALLOC
}

// operator delete, put pointer to stack 
void MyAlloc :: clearFreeMemory () 
{
#ifdef ALUGRID_GARBAGE_COLLECTION
  // make a copy of the memory map 
  // memorymap_t backup( *freeStore );
  memorymap_t& backup = *freeStore ;

  map< MemoryBlock*, memorymap_t > freePointer ;

  typedef memorymap_t :: iterator  iterator ;
  const iterator end = backup.end();
  for( iterator it = backup.begin(); it != end ; ++it ) 
  {
    AllocEntry& fs = (*it).second ;
    const size_t blockSize = (*it).first ;
    while( ! fs.S.empty() )
    {
      void* p = fs.S.top();
      fs.S.pop();
      MemoryBlock* mb = MemoryBlock :: find( *memoryBlocks, p );
      assert( mb );
      memorymap_t& freeMap = freePointer[ mb ];
      AllocEntry& ae = freeMap[ blockSize ];
      ae.S.push( p );
      mb->free( p, blockSize );
    }
  }

  {
    assert( memoryBlocks->size() > 0 );
    typedef MemoryBlockVector_t :: iterator  stiterator ;
    const stiterator end = memoryBlocks->end();
    for( stiterator it =  memoryBlocks->begin(); it != end; ++it ) 
    {
      MemoryBlock* mb = (*it);
      if( mb->empty() ) 
      {
        memoryBlocks->erase( it++ );
        delete mb ;
        std::cout << "Delete memory block" << endl;
      }
      else 
      {
        memorymap_t& freeMap = freePointer[ mb ];
        const iterator endm = freeMap.end();
        for( iterator itm = freeMap.begin(); itm != endm; ++itm ) 
        {
          const size_t blockSize = (*itm).first ;
          AllocEntry& ae = (*itm).second ;
          AllocEntry& fs = backup[ blockSize ];
          while ( ae.S.empty() ) 
          {
            fs.S.push( ae.S.top() );
            ae.S.pop();
          }
        }
      }
    }
  }
#endif
}

MyAlloc :: Initializer :: Initializer () 
{
  if ( ! MyAlloc :: _initialized ) 
  {
    freeStore = new memorymap_t () ;
    assert (freeStore) ;

#ifdef ALUGRID_GARBAGE_COLLECTION
    memoryBlocks = new MemoryBlockVector_t ();
    MemoryBlock* block = new MemoryBlock();
    memoryBlocks->push_back( block );
#endif

    MyAlloc :: _initialized = true;
  }
  return ;
}

MyAlloc :: Initializer :: ~Initializer () 
{
  if ( MyAlloc :: _initialized ) 
  {
    if(freeStore) delete freeStore ;
    freeStore = 0 ;

#ifdef ALUGRID_GARBAGE_COLLECTION
    while( ! memoryBlocks->empty() ) 
    {
      MemoryBlock* mb = memoryBlocks->back();
      memoryBlocks->pop_back();
      delete mb;
    }
#endif

    MyAlloc :: _initialized = false;
  }
  return ;
}
#endif // DONT_USE_ALUGRID_ALLOC 
