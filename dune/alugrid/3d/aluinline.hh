#ifndef DUNE_ALUGRID_INLINE_HH
#define DUNE_ALUGRID_INLINE_HH


//////////////////////////////////////////////////////////////////////
// compile imp.cc into lib (1 yes, 0 no)
// if you change this, you'll get what you deserve
//////////////////////////////////////////////////////////////////////
#if DUNE_ALUGRID_COMPILE_BINDINGS_IN_LIB
#define COMPILE_ALUGRID_LIB 1
#else
#define COMPILE_ALUGRID_LIB 0
#endif


#if COMPILE_ALUGRID_LIB
  #define COMPILE_ALUGRID_INLINE 0
#else
  #define COMPILE_ALUGRID_INLINE 1
#endif

#if COMPILE_ALUGRID_INLINE
#define alu_inline inline
#else
#define alu_inline
#endif
/////////////////////////////////////////////////////////////////////


#endif //DUNE_ALUGRID_INLINE_HH
