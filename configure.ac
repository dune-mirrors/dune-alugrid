AC_PREREQ(2.50)
DUNE_AC_INIT
AM_INIT_AUTOMAKE([foreign 1.5])
AC_CONFIG_SRCDIR([dune/grid/spgrid.hh])
AM_CONFIG_HEADER([config.h])


DUNE_CHECK_ALL

AC_SUBST(AM_CPPFLAGS, $DUNE_CPPFLAGS)
AC_SUBST(AM_LDFLAGS, $DUNE_LDFLAGS)
LIBS="$DUNE_LIBS"

# generate Makefiles
AC_CONFIG_FILES([
  Makefile
  m4/Makefile
  misc/Makefile
  misc/macrogrids/Makefile
  misc/versioncheck/Makefile
  src/serial/Makefile
  src/parallel/Makefile
  src/duneinterface/Makefile
  src/alu2d/Makefile
  src/Makefile
  dune-alugrid.pc
])

AC_OUTPUT
DUNE_SUMMARY_ALL
