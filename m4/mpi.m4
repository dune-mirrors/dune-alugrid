# check for MPI components 

AC_DEFUN([CHECK_MPI], [
AC_PREREQ(2.50) dnl requires autoconf version 2.50 or higher

AC_REQUIRE([AC_PROG_CXX])

AC_LANG_PUSH([C++])
AC_ARG_VAR(MPICXX,[MPI C++ compiler command])

# check different mpic compiler scripts 
AC_CHECK_PROGS(MPICXX, mpic++ mpiCC mpCC hcp mpxlC mpxlC_r cmpic++, $CXX)
REM_CXX="$CXX"
CXX="$MPICXX"
AC_SUBST(MPICXX)
# use the mpi compiler script in the following 


# test whether the following code compiles 
if test x = x"$MPILIBS"; then
AC_MSG_CHECKING([for MPI_Finalize]) 
AC_TRY_COMPILE([#include <mpi.h>],
[{
  MPI_Finalize();
  exit(0);
}], 
[MPILIBS=" "], [MPILIBS=""])
fi

if test x = x"$MPILIBS"; then
  AC_MSG_RESULT(no)
else 
  AC_MSG_RESULT(yes)
fi 

if test x = x"$MPILIBS"; then
        AC_CHECK_LIB(mpi, MPI_Finalize, [MPILIBS="-lmpi"])
fi
if test x = x"$MPILIBS"; then
        AC_CHECK_LIB(mpich, MPI_Finalize, [MPILIBS="-lmpich"])
fi

if test x != x"$MPILIBS"; then
        AC_MSG_CHECKING([for mpi.h])
        AC_TRY_COMPILE([#include <mpi.h>],[],[AC_MSG_RESULT(yes)], [MPILIBS=""
                AC_MSG_RESULT(no)])
fi

# reset previous compiler 
CXX="$REM_CXX"
AC_SUBST(MPILIBS)
AC_LANG_POP

# return the option if MPILIBS where found 
if test x != x"$MPILIBS"; then
  ifelse([$1],,[AC_DEFINE(HAVE_MPI,1,[Defines whether you have the MPI library or not.])],[$1])
fi
])
