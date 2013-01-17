AC_PREREQ(2.50)
DUNE_AC_INIT
AM_INIT_AUTOMAKE([foreign 1.5])
AC_CONFIG_SRCDIR([dune/alugrid/grid.hh])
AM_CONFIG_HEADER([config.h])


DUNE_CHECK_ALL

AC_SUBST(AM_CPPFLAGS, $DUNE_CPPFLAGS)
AC_SUBST(AM_LDFLAGS, $DUNE_LDFLAGS)
LIBS="$DUNE_LIBS"

# generate Makefiles
AC_CONFIG_FILES([
  Makefile
  dune/Makefile
  dune/alugrid/Makefile
  dune/alugrid/2d/Makefile
  dune/alugrid/3d/Makefile
  dune/alugrid/common/Makefile
  dune/alugrid/src/Makefile
  dune/alugrid/src/2d/Makefile
  dune/alugrid/src/duneinterface/Makefile
  dune/alugrid/src/parallel/Makefile
  dune/alugrid/src/serial/Makefile
  lib/Makefile
  m4/Makefile
  misc/Makefile
  misc/macrogrids/Makefile
  misc/versioncheck/Makefile
  dune-alugrid.pc
])

AC_OUTPUT
DUNE_SUMMARY_ALL
