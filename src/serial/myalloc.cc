#ifndef DONT_USE_ALUGRID_ALLOC

#include "myalloc.h"

const long   MyAlloc :: MAX_HOLD_ADD  = 40000 ;  // max MAX_HOLD_ADD Objekte werden gespeichert
const double MyAlloc :: MAX_HOLD_MULT = 0.25 ;   // max das MAX_HOLD_MULT-fache der momentan
                                                 // aktiven Objekte werden gespeichert
// MyAlloc initialize flag                                                   
bool MyAlloc :: _initialized = false;

// if true objects could be freeed  
bool MyAlloc :: _freeAllowed = true;

// class to store items of same size in a stack 
// also number of used items outside is stored 
class AllocEntry {

public:

  // N verfolgt die Anzahl der angelegten Objekte der entsprechenden
  // Gr"osse, die in Gebrauch sind, auf dem Stack liegen die Objekte,
  // f"ur die delete aufgerufen wurde, die aber nicht an free () zur"uck-
  // gegeben werden um Fragmentierung zu vermeiden.

  long N ;

  stack < void * > S ;

  AllocEntry () : N (0), S () {}

  AllocEntry(const AllocEntry& other) 
    : N(other.N) , S(other.S) 
  {} 
  
  ~AllocEntry () 
  {
    while (!S.empty ()) 
    {
      free (S.top ()) ;
      S.pop () ;
    }
    return ;
  }
};

// map holding AllocEntries for sizes 
static map < size_t, AllocEntry, less < size_t > > * freeStore = 0 ;
static set < void * > myAllocFreeLockers;

void MyAlloc :: lockFree (void * addr) 
{
  // remember address of locker 
  myAllocFreeLockers.insert( addr );
  _freeAllowed = false; 
}

void MyAlloc :: unlockFree (void * addr) 
{
  myAllocFreeLockers.erase( addr );
  // only if no-one else has locked 
  if( myAllocFreeLockers.empty () )
  {
    _freeAllowed = true; 
  }
}

void * MyAlloc :: operator new (size_t s) throw (OutOfMemoryException) 
{
  assert(s > 0);
  {
    AllocEntry & fs = ((*freeStore) [s]) ;
    ++ fs.N ;
    if (fs.S.empty ()) {
      void * p = malloc (s) ;
      if (p == NULL) {
        perror ("**ERROR (FATAL) in MyAlloc :: operator new ()") ;
        cerr << "**INFO MyAlloc :: operator new (" << s << "): No more memory available " << endl; 
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
}

// operator delete, put pointer to stack 
void MyAlloc :: operator delete (void *ptr, size_t s) 
{
  // get stack for size s 
  AllocEntry & fs ((*freeStore) [s]) ;
  // push pointer to stack 
  assert (fs.N > 0) ;
  --fs.N ;
  fs.S.push (ptr) ;
 
  // if free of objects is allowd 
  if( _freeAllowed )
  {
    // check if max size is exceeded 
    const size_t stackSize = fs.S.size ();
    if ( ( stackSize >= (unsigned) MAX_HOLD_ADD ) && 
         ( double (stackSize) >= MAX_HOLD_MULT * double (fs.N) )
       ) 
    {
      assert (!fs.S.empty()) ;
      free (fs.S.top ()) ;
      fs.S.pop() ;
    }
  }
  return ;
}

MyAlloc :: Initializer :: Initializer () 
{
  if ( ! MyAlloc :: _initialized ) 
  {
    freeStore = new map < size_t, AllocEntry, less < size_t > > ;
    assert (freeStore) ;

    MyAlloc :: _initialized = true;
  }
  return ;
}

MyAlloc :: Initializer :: ~Initializer () {
  if ( MyAlloc :: _initialized ) 
  {
    if(freeStore) delete freeStore ;
    freeStore = 0 ;
    MyAlloc :: _initialized = false;
  }
  return ;
}
#endif // DONT_USE_ALUGRID_ALLOC 
