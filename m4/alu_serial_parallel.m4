# check wether ALUGrid can be used for serial and for parallel compilation 

AC_DEFUN([ALU3D_SERIAL_PARALLEL],[
  AC_PREREQ(2.50) dnl for AC_LANG_CASE

  # implicitly sets the HAVE_MPI-define and the MPICXX-substitution
  ACX_MPI()

  # get compilation script
  AC_LANG_CASE([C],[
    MPICOMP="$MPICC"
  ],
  [C++],[
    MPICOMP="$MPICXX"
  ])

  with_parallel="no"
  REM_CPPFLAGS=$CPPFLAGS
  ALU3D_INC_FLAG_PARA="-I./src -I./src/serial -I./src/parallel -I./src/duneinterface $MPI_CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $ALU3D_INC_FLAG_PARA"
  
  with_serial="no"
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADER([alu3dgrid_serial.h],[with_serial="yes"],
    AC_MSG_WARN([alu3dgrid_serial.h could not be found or compiled!]) 
  )

  AC_MSG_CHECKING([serial ALUGrid usability])
  AC_MSG_RESULT([$with_serial])

  
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADER([alu3dgrid_parallel.h],[with_parallel="yes"],
    AC_MSG_WARN([alu3dgrid_parallel.h could not be found or compiled!]) 
  )
  # set flags
  AC_MSG_CHECKING([parallel ALUGrid usability])
  if test x$with_parallel != xno ; then
    AC_SUBST(MPI_CPPFLAGS, $MPI_CPPFLAGS)
    AC_SUBST(MPI_LDFLAGS, $MPI_LDFLAGS)
  else
    AC_SUBST(MPI_CPPFLAGS, "")
    AC_SUBST(MPI_LDFLAGS, "")
  fi
  AC_MSG_RESULT([$with_parallel])
  AC_LANG_POP

  CPPFLAGS="$REM_CPPFLAGS"
  REM_CPPFLAGS=
      
  AM_CONDITIONAL(MPI, test x$with_parallel != xno)
])
