// (c) bernhard schupp 1998
// modifications for dune
// (c) robert kloefkorn 2007
#ifndef MYALLOC_H_INCLUDED
#define MYALLOC_H_INCLUDED

#include <cstddef>
#include <string>
#include <iostream>

// if defined the memory allocation from dlmalloc is used
#if HAVE_DLMALLOC
#define ALUGRID_USES_DLMALLOC
#endif

namespace ALUGrid
{
  class ALUGridException
  {
  protected:
    ALUGridException () {}
  public:
    virtual ~ALUGridException () {}
    virtual std::string what () const = 0 ;
  };


#ifdef ALUGRID_USES_DLMALLOC
  class DLMalloc
  {
    // true if initialized
    static bool _initialized ;

  public :
    static const bool ALUGridUsesDLMalloc = true ;

    class Initializer
    {
      // initializer versucht, die statischen Objekte der Speicherverwaltung
      // vor allem anderen zu initialisieren, damit keine Fehler auftreten,
      // falls statische Objekte irgendwo Instanzen mit MyAlloc als Basis-
      // klasse angelegen.
      public :
        Initializer () ;
       ~Initializer () ;
    } ;

    class OutOfMemoryException : public ALUGridException
    {
    public:
      virtual std::string what () const { return "OutOfMemoryException"; }
    };
    friend class Initializer;

    // if called, freeing objects is allowed again
    static void unlockFree(void *);

    // if called free of objects is not allowed
    static void lockFree (void *);

    // try to make free memory available for the system
    static void clearFreeMemory () ;

    // return size of allocated memory
    static size_t allocatedMemory () ;

  protected :
    DLMalloc () {}
   ~DLMalloc () {}

  public :

    // new version of operator new
    void * operator new (size_t);
    // corresponding version of operator delete
    void operator delete (void *,size_t);
  } ;

  static DLMalloc :: Initializer allocatorInitializer ;

  typedef DLMalloc MyAlloc;

#else

  // dummy class defaulting to standard new and delete
  class MyAlloc
  {
  public:
    class OutOfMemoryException : public ALUGridException
    {
    public:
      virtual std::string what () const { return "OutOfMemoryException"; }
    };
    // this is false here anyway
    static const bool ALUGridUsesDLMalloc = false ;

    // if called, freeing objects is allowed again
    inline static void unlockFree(void *) {}

    // if called free of objects is not allowed
    inline static void lockFree (void *) {}

    // try to make free memory available for the system
    inline static void clearFreeMemory () {}

    // return size of allocated memory
    inline static size_t allocatedMemory () { return 0; }

  };

#endif // #ifdef ALUGRID_USES_DLMALLOC

} // namespace ALUGrid

#endif // #ifndef MYALLOC_H_INCLUDED
