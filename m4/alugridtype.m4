AC_DEFUN([DUNE_DEFINE_ALUGRIDTYPE],[AH_BOTTOM(dnl
[
#if ! HAVE_ALUGRID
  /* add GRIDTYPE typedef for grid implementation $3:
    defining $1 during compilation typedefs this grid implementation as GridType
    in namespace Dune::GridSelector;
    also integer constants dimgrid and dimworld are set in this namespace.
    The required headers for this grid implementation are also included.
  */
 #if HAVE_DUNE_GRID && defined $1 && ! defined USED_$1_GRIDTYPE
  #if HAVE_GRIDTYPE
   #error "Ambiguous definition of GRIDTYPE."
  #endif 

  #ifndef WORLDDIM
    #define WORLDDIM GRIDDIM
  #endif
  #if not (WORLDDIM >= GRIDDIM)
    #error "WORLDDIM < GRIDDIM does not make sense."
  #endif
]dnl
m4_if([$2],[],[],[
  #if ! ($2)
    #error "Preprocessor assertion $2 failed."
  #endif
])
DUNE_DEFINE_GRIDTYPE_INCLUDE(m4_shift(m4_shift(m4_shift($@))))dnl
[
  namespace Dune
  {
    namespace GridSelector
    {
      const int dimgrid = GRIDDIM;
      const int dimworld = WORLDDIM;
      typedef $3 GridType;
    }
  }
  #define HAVE_GRIDTYPE 1
  #define USED_$1_GRIDTYPE 1
#endif // #if HAVE_DUNE_GRID && defined $1 && ..
#endif // #if ! HAVE_ALUGRID
]dnl
)])
