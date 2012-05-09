# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.50)

# current version of ALUGrid
AC_INIT(ALUGrid, 1.95, alugrid@mathematik.uni-freiburg.de)
AM_INIT_AUTOMAKE(ALUGrid, 1.95, alugrid@mathematik.uni-freiburg.de)

# default install path is current directory 
AC_PREFIX_DEFAULT([`pwd`])

# build static library as default (enable with the --enable-shared option) 
AC_DISABLE_SHARED

# check if we're in the right directory
AC_CONFIG_SRCDIR([src/serial/gitter_sti.h])
AC_CONFIG_MACRO_DIR([m4])
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

# generate Makefiles when existent 
AC_CHECK_FILE([misc/examples/Makefile.am],
              [AC_CONFIG_FILES([misc/examples/Makefile
              ])],
              [])

# generate Makefiles
AC_CONFIG_FILES([Makefile
                 src/serial/Makefile
                 src/parallel/Makefile
                 src/duneinterface/Makefile
                 src/alu2d/Makefile
                 src/Makefile
                 misc/Makefile
                 misc/macrogrids/Makefile
                 misc/versioncheck/Makefile
                 alugrid.pc
                 ])

AC_OUTPUT

# show search results
ALUGRID_SUMMARY_ALL
