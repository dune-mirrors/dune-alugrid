#ifndef DUNE_TIMER_HH
#define DUNE_TIMER_HH

//#define TIMER_USE_STD_CLOCK

#ifndef TIMER_USE_STD_CLOCK
// headers for getrusage(2)
#include <sys/resource.h>
#endif

#include <ctime>

// headers for stderror(3)
#include <cstring>

// access to errno in C++
#include <cerrno>

using namespace std;

/** @addtogroup Common
 @{
*/

/*! \file
    \brief A simple timing class.
*/

    /** \brief A simple stop watch

  This class reports the elapsed user-time, i.e. time spent computing,
  after the last call to Timer::reset(). The results are seconds and
  fractional seconds. Note that the resolution of the timing depends
  on your OS kernel which should be somewhere in the milisecond range.

  The class is basically a wrapper for the libc-function getrusage()

  */
class Timer
{
public:
	//! A new timer, start immediately
  inline Timer ()
	{
	  reset();
	}

	//! Reset timer
	inline void reset()
	{
#ifdef TIMER_USE_STD_CLOCK
	  cstart = std::clock();
#else
	  rusage ru;
    getrusage(RUSAGE_SELF, &ru);
	  cstart = ru.ru_utime;
#endif
	}

	//! Get elapsed user-time in seconds
	inline double elapsed () const
  {
#ifdef TIMER_USE_STD_CLOCK
	  return (std::clock()-cstart) / static_cast<double>(CLOCKS_PER_SEC);
#else
	  rusage ru;
    getrusage(RUSAGE_SELF, &ru);
	  return 1.0 * (ru.ru_utime.tv_sec - cstart.tv_sec) + (ru.ru_utime.tv_usec - cstart.tv_usec) / (1000.0 * 1000.0);
#endif

	}

  //! return elapsed in milli seconds
  inline double convElapsed() const
  {
    size_t time = (size_t) (1e3 * elapsed());
    return (double) time;
  }

private:
#ifdef TIMER_USE_STD_CLOCK
  clock_t cstart;
#else
  struct timeval cstart;
#endif
}; // end class Timer

/** @} end documentation */

#endif
