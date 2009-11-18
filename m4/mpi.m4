# check for MPI components 

AC_DEFUN([CHECK_MPI], [
AC_PREREQ(2.50) dnl requires autoconf version 2.50 or higher

AC_LANG_PUSH([C])
AC_ARG_VAR(MPICC,[MPI C compiler command])
AC_CHECK_PROGS(MPICC, mpicc hcc mpcc mpcc_r mpxlc cmpicc, $CC)
acx_mpi_save_CC="$CC"
REM_CC="$CC"
CC="$MPICC"
AC_SUBST(MPICC)

# use the mpi compiler script in the following 

REM_LDFLAGS="$LDFLAGS"

LDFLAGS=`$MPICC -showme:link 2> /dev/null`

# test whether the following code compiles 
# and if use the mpicc -showme:link libs for linking
AC_MSG_CHECKING([for MPI_Finalize]) 
AC_TRY_COMPILE([#include <mpi.h>],
[{
  MPI_Finalize();
  exit(0);
}], 
[MPI_LIBS="$LDFLAGS"], [MPI_LIBS=""])
if test x = x"$MPI_LIBS"; then
  AC_MSG_RESULT(no)
else 
  # get MPI CFLAGS from compiler and -DMPIPP_H to avoid C++ bindings  
  MPI_CPPFLAGS="`$MPICC -showme:compile 2> /dev/null`"
  AC_MSG_RESULT(yes)
fi 

if test x = x"$MPI_LIBS"; then
  AC_CHECK_LIB(mpi, MPI_Finalize, [MPI_LIBS="-lmpi"])
fi
if test x = x"$MPI_LIBS"; then
  AC_CHECK_LIB(mpich, MPI_Finalize, [MPI_LIBS="-lmpich"])
fi

# reset previous compiler 
LDFLAGS="$REM_LDFLAGS"
CC="$REM_CC"
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
