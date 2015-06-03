#ifndef DUNE_ASSERT_HH
#define DUNE_ASSERT_HH

#include <cstdlib>
#include <unistd.h>
#include <iostream>
#if HAVE_MPI
#include <mpi.h>
#endif

// combination of suggestions made in
// http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
// and
// http://www.open-mpi.de/faq/?category=debugging

namespace 
{
  // Call to stop a program so that it is possible to attach a debugger.
  // The process id (PID) and calling host are printed to the error stream
  // If the parameter p is added in a parallel code only the process with
  // that rank will wait to allow a debugger to attach.
  //
  // To attach for example gdb simply call (on the correct host):
  // gdb -pid PID
  // you can continue the program by calling at the gdb prompt:
  // > set var cont = 1
  // > continue
  int attach(int p=-1);
  //
  // dune_assert( bool ) or dune_assert( bool, p )
  // This is a special assert like macro that allows a bit more flexibility
  // then the standard assert. There are three different available
  // behaviors of a failed dune_assert call:
  // - If ASSERTATTACH is defined then attach is called with the optional
  //   process rank p, i.e., the assert is ignored except on the process
  //   with rank p.
  // - If ASSERTATTACH is not defined but ASSERTCONTINUE then the assert
  //   message is printed but the program tries to continue regardless. This
  //   can be usefull for some unit tests
  // - If neither ASSERTATTACH nor ASSERTCONTINUE is defined then the program
  //   will be aborted.
  int attach(int p)
  {
    int cont = 0;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
#if HAVE_MPI
    int mpirank = -1;
    MPI_Comm_rank(MPI_COMM_WORLD,&mpirank);
    if (p >= 0 && p != mpirank)
    {
      std::cout << "[" << mpirank << "] has PID " << getpid() << " on " << hostname << " continuing" << std::endl;
      return 1;
    }
    else
      std::cout << "[" << mpirank << "] has PID " << getpid() << " on " << hostname << " ready to attach" << std::endl;
#else
    std::cout << "PID " << getpid() << " on " << hostname << " ready to attach" << std::endl;
#endif
    while (0 == cont)
        sleep(5);
    return 1;
  }

  // a handle for the dune_assert macro. 
  // The usual output from the C assert (including the mpi rank on a
  // parallel run) is printed to std::cerr.
  typedef int (*AssertHandler)(char const*, char const*, int);
  int default_assert_handler(char const* expr, char const* file, int line)
  { 
#if HAVE_MPI
    int mpirank = -1;
    MPI_Comm_rank(MPI_COMM_WORLD,&mpirank);
    std::cout << "[" << mpirank << "]: ";
#endif
    std::cerr << "Assertion " << expr << " failed in " << file << ":" << line << std::endl; 
    return 1; 
  }
    AssertHandler assert_handler = default_assert_handler;
}


// the ASSERT_HALT macro is available in three flavours depending on some
// defines:
#if defined(ASSERTATTACH)
#define ASSERT_HALT(p) attach(p)
#elif defined(ASSERTCONTINUE)
#define ASSERT_HALT(p) ((void)sizeof(p)), std::cout << "... trying to continue ..." << std::endl
#else
#define ASSERT_HALT(p) ((void)sizeof(p)), abort()
#endif

// The actual implementation fo the dune_assert macro
#ifndef NDEBUG
#define dune_assert_(x,p) ((void)(!(x) && assert_handler(#x, __FILE__, __LINE__) && (ASSERT_HALT(p), 1)))
#else
#define dune_assert_(x,p) ((void)sizeof(x),sizeof(p))
#endif

// Allow for calling dune_assert with one or two arguments 
#define dune_assert0_(x) dune_assert_(x,-1) 
#define dune_assert1_(x,p) dune_assert_(x,p)
#define GET_ASSERT_MACRO(_1,_2,NAME,...) NAME
#define dune_assert(...) GET_ASSERT_MACRO(__VA_ARGS__, dune_assert1_, dune_assert0_)(__VA_ARGS__)

#endif
