#ifndef ALUGRID_ASSERT_HH
#define ALUGRID_ASSERT_HH

#include <cassert>

#ifndef ALUGRIDDEBUG
# define alugrid_assert(EX) (__ASSERT_VOID_CAST (0))
#else
# define alugrid_assert(EX) assert(EX)
#endif

#endif // ALUGRID_ASSERT_HH

