# check wether ALUGrid can be used for serial and for parallel compilation 

AC_DEFUN([ALUGRID_SERIAL_PARALLEL],[
  AC_PREREQ(2.50) dnl for AC_LANG_CASE

  with_mpi="no"
  # implicitly sets the HAVE_MPI-define and the MPICXX-substitution
  # if MPI was found yes is returned 
  CHECK_MPI([with_mpi="yes"])

  # get compilation script
  AC_LANG_CASE([C++],[
    MPICOMP="$MPICXX"
  ])
  
  LD=$CXX
  AC_ARG_VAR(LD,[linker program])
  AC_SUBST(LD)

  REM_CPPFLAGS=$CPPFLAGS
  REM_CXXFLAGS=$CXXFLAGS
  ALUGRID_INC_FLAG_PARA="-I./src -I./src/serial -I./src/parallel -I./src/duneinterface"
  CXXFLAGS="$CPPFLAGS $ALUGRID_INC_FLAG_PARA"
  CPPFLAGS="$CPPFLAGS $ALUGRID_INC_FLAG_PARA"

  with_parallel="none"
  with_serial="no"
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADER([alugrid_serial.h],[with_serial="yes"],
    AC_MSG_WARN([alugrid_serial.h could not be found or compiled!]) 
  )

 if test x$with_serial != xno ; then 
    with_parallel="serial"
    if test "x$with_mpi" = "xyes"; then 
    AC_CHECK_HEADER([alugrid_parallel.h],[with_parallel="parallel"],
      AC_MSG_WARN([alugrid_parallel.h could not be found or compiled! 
      Maybe you should rerun configure with the parameter CXX=$MPICXX 
      instead the choosen way! See the README file for more 
      information on compilers!]) 
    )
    else 
      AC_MSG_WARN([MPI not found, not checking alugrid_parallel.h!]) 
    fi
 fi
    
  AC_MSG_CHECKING([for which type of run mode ALUGrid can be used])
  AC_MSG_RESULT([$with_parallel])

  AC_LANG_POP

  CPPFLAGS="$REM_CPPFLAGS"
  CXXFLAGS="$REM_CXXFLAGS"
  REM_CPPFLAGS=
      
  AM_CONDITIONAL(MPI, test x$with_parallel = xparallel)
])
