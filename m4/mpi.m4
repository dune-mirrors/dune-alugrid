# check for MPI components 

AC_DEFUN([CHECK_MPI], [
AC_PREREQ(2.50) dnl requires autoconf version 2.50 or higher

AC_LANG_PUSH([C])
AC_ARG_VAR(MPICC,[MPI C compiler command])
AC_CHECK_PROGS(MPICC, mpicc hcc mpcc mpcc_r mpxlc cmpicc, $CC)
AC_SUBST(MPICC)

AC_LANG_POP
AC_LANG_PUSH([C++])
AC_ARG_VAR(MPICXX,[MPI C++ compiler command])
AC_CHECK_PROGS(MPICXX, mpic++ mpicxx mpiCC mpCC hcp mpxlC mpxlC_r cmpic++, $CXX)
AC_SUBST(MPICXX)

mpi_help_string="(run again with ./configure CXX=$MPICXX .... or use '$MPICC -show' to obtain CXXFLAGS and LDFLAGS)"

REM_LDFLAGS="$LDFLAGS"
REM_CXXFLAGS="$CXXFLAGS"
# get MPI CFLAGS from compiler  
MPI_CPPFLAGS="`$MPICC -showme:compile 2> /dev/null`"
LDFLAGS="$LDFLAGS `$MPICC -showme:link 2> /dev/null`"
CXXFLAGS="$CXXFLAGS $MPI_CPPFLAGS" 

# test whether the following code compiles 
# and if use the mpicc -showme:link libs for linking

# now check whether we can compile C++ with C-MPI interface
# we need this for alugrid_parallel.cc 
AC_MSG_CHECKING([for MPI_Finalize]) 
AC_TRY_COMPILE([
  #if defined(__cplusplus) 
  #define rem__cplusplus __cplusplus
  #undef __cplusplus
  #endif
  #if defined(c_plusplus) 
  #define remc_plusplus c_plusplus
  #undef c_plusplus
  #endif

  extern "C" {
    #include <stdlib.h>
    // the message passing interface (MPI) headers for C 
    #include <mpi.h>
  }

  // restore defines 
  #if defined(rem__cplusplus) 
  #define __cplusplus rem__cplusplus
  #undef rem__cplusplus
  #endif

  #if defined(c_plusplus) 
  #define c_plusplus remc_plusplus
  #undef remc_plusplus
  #endif
  
  ],
[{
  MPI_Finalize();
  exit(0);
}], 
[MPI_LIBS="$LDFLAGS"], [MPI_LIBS=""])

if test x = x"$MPI_LIBS"; then
  AC_MSG_RESULT(no)
else 
  AC_MSG_RESULT(yes)
fi 

# reset previous compiler 
LDFLAGS="$REM_LDFLAGS"
CXXFLAGS="$REM_CXXFLAGS"

AC_SUBST(MPI_LIBS)
AC_LANG_POP

# return the option if MPI_LIBS where found 
if test x != x"$MPI_LIBS"; then
  AC_SUBST(MPI_LIBS, $MPI_LIBS)
  AC_SUBST(MPI_LDFLAGS, $MPI_LDFLAGS)
  AC_SUBST(MPI_CPPFLAGS, $MPI_CPPFLAGS)

  # add to global list
  ALUGRID_PKG_LDFLAGS="$ALUGRID_PKG_LDFLAGS $MPI_LDFLAGS"
  ALUGRID_PKG_LIBS="$ALUGRID_PKG_LIBS $MPI_LIBS"
  ALUGRID_PKG_CPPFLAGS="$ALUGRID_PKG_CPPFLAGS $MPI_CPPFLAGS"
fi

if test x != x"$MPI_LIBS"; then
  ifelse([$1],,[AC_DEFINE(HAVE_MPI,1,[Defines whether you have the MPI library or not.])],[$1])
fi
])
