# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(alu3dgrid, 0.0, robertk@mathematik.uni-freiburg.de)
AM_INIT_AUTOMAKE(alu3dgrid, 0.0, robertk@mathematik.uni-freiburg.de)

# check if we're in the right directory
#AC_CONFIG_SRCDIR([scr/Makefile.am])
# which header-file is created
AM_CONFIG_HEADER(config.h)

# we need no more than the standard for ALU3dGrid Parallel
ALU3D_CHECK_ALL

# implicitly set the ALU3dGrid-include everywhere
CPPFLAGS="$CPPFLAGS $ALU3D_CPPFLAGS"

# generate Makefiles
AC_CONFIG_FILES([Makefile
                 ])
AC_OUTPUT

# show search results
ALU3D_SUMMARY_ALL
