// (c) bernhard schupp 1998
// modifications for dune 
// (c) robert kloefkorn 2007 
#ifndef MYALLOC_H_INCLUDED
#define MYALLOC_H_INCLUDED

#ifndef DONT_USE_ALUGRID_ALLOC 

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <memory>      // Def. von size_t, malloc (), free ()
  #include <cassert>
#else
  #include <memory.h>    // Def. von size_t, malloc (), free ()
  #include <assert.h>
#endif

class MyAlloc {
  // max number of storable items per stack 
  static const long MAX_HOLD_ADD ;
  // overestimation factor 
  static const double MAX_HOLD_MULT ;

  // true if initialized 
  static bool _initialized ;

  // if true objects are not free, only pushed to stack 
  static bool _freeAllowed ;
  
  public :
    class Initializer {
	// initializer versucht, die statischen Objekte der Speicherverwaltung
	// vor allem anderen zu initialisieren, damit keine Fehler auftreten,
	// falls statische Objekte irgendwo Instanzen mit MyAlloc als Basis-
	// klasse angelegen.
      public :
        Initializer () ;
       ~Initializer () ;
    } ;
    
    class OutOfMemoryException { };
    friend class Initializer;

    // if called, freeing objects is allowed again 
    static void unlockFree(void *);

    // if called free of objects is not allowed 
    static void lockFree (void *); 
  protected :
    MyAlloc () {}
   ~MyAlloc () {}

  protected: 
#ifdef USE_MALLOC_AT_ONCE
    // malloc same size of memory at once 
    void mallocAtOnce(size_t , void* v[] , size_t ) throw (OutOfMemoryException) ; 
    // operator new that gets memory from outside 
    void * operator new (size_t, void * p ) { assert(p); return p; }
#endif

  public :
    // new version of operator new 
    void * operator new (size_t) throw (OutOfMemoryException) ;
    // corresponding version of operator delete 
    void operator delete (void *,size_t) ;
} ;

static MyAlloc :: Initializer allocatorInitializer ;

#else // DONT_USE_ALUGRID_ALLOC 

// dummy class 
class MyAlloc {};

#endif // DONT_USE_ALUGRID_ALLOC

#endif // MYALLOC_H_INCLUDED
