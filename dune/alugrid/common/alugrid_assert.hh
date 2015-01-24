#ifndef ALUGRID_ASSERT_HH
#define ALUGRID_ASSERT_HH

#include "duneassert.hh"

// this is only of interest when NDEBUG is not set
// NOTE: defining NO_ALUGRID_DEBUG will disable all ALUGrid asserts
#ifndef NDEBUG

// enable ALUGrid debug mode by default unless NO_ALUGRID_DEBUG is set
#if defined(NO_ALUGRID_DEBUG) && defined(ALUGRIDDEBUG)
#undef ALUGRIDDEBUG
#endif

#endif // NDEBUG

#ifndef ALUGRIDDEBUG
# define alugrid_assert(EX) ((void)sizeof(EX))
#else
# define alugrid_assert(EX) dune_assert(EX)
#endif

#endif // ALUGRID_ASSERT_HH
