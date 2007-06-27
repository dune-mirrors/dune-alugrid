// (c) bernhard schupp 1998
// modifications for dune 
// (c) robert kloefkorn 2007 
#ifndef MYALLOC_H_INCLUDED
#define MYALLOC_H_INCLUDED

#if USE_ALUGRID_ALLOC 

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <memory>      // Def. von size_t, malloc (), free ()
#else
  #include <memory.h>    // Def. von size_t, malloc (), free ()
#endif

class MyAlloc {
  static const long MAX_HOLD_ADD ;
  static const double MAX_HOLD_MULT ;
  static long _init ;
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
  protected :
    MyAlloc () {}
   ~MyAlloc () {}
  public :
    // operator new that gets memory from outside 
    //void * operator new (size_t, void * p ) { return p; }

    void * operator new (size_t) throw (OutOfMemoryException) ;
    void operator delete (void *,size_t) ;
} ;

static MyAlloc :: Initializer allocatorInitializer ;

#else // USE_ALUGRID_ALLOC 

// dummy class 
class MyAlloc {};

#endif // USE_ALUGRID_ALLOC

#endif // MYALLOC_H_INCLUDED
