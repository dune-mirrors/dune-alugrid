#ifndef ALUGRID_ASSERT_HH
#define ALUGRID_ASSERT_HH

#include <cassert>

#ifndef ALUGRIDDEBUG
# define alugrid_assert(EX) (static_cast<void>(0))
#else
#ifdef NDEBUG
#warning defined ALUGRIDDEBUG and NDEBUG both leads to many warnings and is probably not reasonable...
#endif
# define alugrid_assert(EX) assert(EX)
#endif

#endif // ALUGRID_ASSERT_HH

