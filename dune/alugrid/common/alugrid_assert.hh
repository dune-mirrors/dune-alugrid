#ifndef ALUGRID_ASSERT_HH
#define ALUGRID_ASSERT_HH

#include <cassert>

// this is only of interest when NDEBUG is not set
// NOTE: defining NO_ALUGRID_DEBUG will disable all ALUGrid asserts
#ifndef NDEBUG

// enable ALUGrid debug mode by default unless NO_ALUGRID_DEBUG is set
#ifndef NO_ALUGRID_DEBUG
#define ALUGRIDDEBUG
#endif

#endif // NDEBUG

#ifndef ALUGRIDDEBUG
# define alugrid_assert(EX) (static_cast<void>(0))
#else
# define alugrid_assert(EX) assert(EX)
#endif

#endif // ALUGRID_ASSERT_HH
