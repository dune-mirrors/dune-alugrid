# check wether ALUGrid can be used for serial and for parallel compilation 

AC_DEFUN([ALU3D_SERIAL_PARALLEL],[
  AC_PREREQ(2.50) dnl for AC_LANG_CASE

  with_mpi="no"
  # implicitly sets the HAVE_MPI-define and the MPICXX-substitution
  ACX_MPI([with_mpi="yes"])

  # get compilation script
  AC_LANG_CASE([C],[
    MPICOMP="$MPICC"
  ],
  [C++],[
    MPICOMP="$MPICXX"
  ])

  REM_CPPFLAGS=$CPPFLAGS
  REM_CXXFLAGS=$CXXFLAGS
  ALU3D_INC_FLAG_PARA="-I./src -I./src/serial -I./src/parallel -I./src/duneinterface"
  CXXFLAGS="$CPPFLAGS $ALU3D_INC_FLAG_PARA"
  CPPFLAGS="$CPPFLAGS $ALU3D_INC_FLAG_PARA"

  with_parallel="none"
  with_serial="no"
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADER([alu3dgrid_serial.h],[with_serial="yes"],
    AC_MSG_WARN([alu3dgrid_serial.h could not be found or compiled!]) 
  )

 if test x$with_serial != xno ; then 
  with_parallel="serial"
  if test x$with_mpi != xno ; then
    AC_CHECK_HEADER([alu3dgrid_parallel.h],[with_parallel="parallel"],
      AC_MSG_WARN([alu3dgrid_parallel.h could not be found or compiled! 
      Maybe you should use  < $MPICXX > instead of < $CXX > ! 
      See the README file for more information on compilers!]) 
    )
  fi
 fi
    
  AC_MSG_CHECKING([for which type of run mode ALU-Grid can be used])
  AC_MSG_RESULT([$with_parallel])

  AC_LANG_POP

  CPPFLAGS="$REM_CPPFLAGS"
  CXXFLAGS="$REM_CXXFLAGS"
  REM_CPPFLAGS=
      
  AM_CONDITIONAL(MPI, test x$with_parallel != xno)
])
