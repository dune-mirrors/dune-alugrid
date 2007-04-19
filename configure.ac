# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.50)

# current version of ALUGrid
AC_INIT(ALUGrid, 0.5, alugrid@mathematik.uni-freiburg.de)
AM_INIT_AUTOMAKE(ALUGrid, 0.5, alugrid@mathematik.uni-freiburg.de)

# default install path is current directory 
AC_PREFIX_DEFAULT([`pwd`])

# build static library as default (enable with the --enable-shared option) 
AC_DISABLE_SHARED

# check if we're in the right directory
AC_CONFIG_SRCDIR([src/serial/gitter_sti.h])
# which header-file is created
AM_CONFIG_HEADER(config.h)

# do the check for the neccessary components for ALUGrid 
ALUGRID_CHECK_ALL

# defines libtool to build the lib
AC_PROG_LIBTOOL 

# implicitly set the ALU3dGrid-include everywhere
CPPFLAGS="$CPPFLAGS $ALUGRID_CPPFLAGS"

# implicitly set the ALUGrid-flags everywhere
LDFLAGS="$ALUGRID_LDFLAGS"
LIBS="$ALUGRID_LIBS"


# generate Makefiles
AC_CONFIG_FILES([Makefile
                 src/serial/Makefile
                 src/parallel/Makefile
                 src/duneinterface/Makefile
                 src/alu2d/Makefile
                 src/Makefile
                 misc/Makefile
                 misc/macrogrids/Makefile
                 misc/examples/Makefile
                 misc/versioncheck/Makefile
                 alugrid.pc
                 ])

AC_OUTPUT

# show search results
ALUGRID_SUMMARY_ALL
