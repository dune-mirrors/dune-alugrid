# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.50)

# current version of ALU3d-Grid
AC_INIT(ALU3d-Grid, 0.1, alu3dgridk@mathematik.uni-freiburg.de)
AM_INIT_AUTOMAKE(ALU3dGrid, 0.1, alu3dgrid@mathematik.uni-freiburg.de)

# default install path is current directory 
AC_PREFIX_DEFAULT([`pwd`])

# build static library as default (enable with the --enable-shared option) 
AC_DISABLE_SHARED

# check if we're in the right directory
AC_CONFIG_SRCDIR([src/serial/gitter_sti.h])
# which header-file is created
AM_CONFIG_HEADER(config.h)

# do the check for the neccessary components for ALU3d-Grid 
ALU3D_CHECK_ALL

# defines libtool to build the lib
AC_PROG_LIBTOOL 

# implicitly set the ALU3dGrid-include everywhere
CPPFLAGS="$CPPFLAGS $ALU3D_CPPFLAGS"
# implicitly set the Dune-flags everywhere
LDFLAGS="$ALU3D_LDFLAGS"
LIBS="$ALU3D_LIBS"


# generate Makefiles
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 ])

AC_OUTPUT

# show search results
ALU3D_SUMMARY_ALL
