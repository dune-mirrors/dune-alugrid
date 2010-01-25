# check for thing needed to compile the ALUGrid library. 

AC_DEFUN([ALUGRID_CHECK_ALL],[

dnl check for programs
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CXXCPP])

  AC_REQUIRE([AC_PROG_INSTALL])
  AC_REQUIRE([AC_PROG_LN_S])
  AC_REQUIRE([AC_PROG_MAKE_SET])
  AC_REQUIRE([AC_PROG_RANLIB])


dnl disable F77 support (only C++ and C (for METIS) is needed)
  m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
  AC_REQUIRE([AC_PROG_LIBTOOL])

dnl checks for typedefs, structures, and compiler characteristics.
  AC_REQUIRE([AC_TYPE_SIZE_T])
  AC_REQUIRE([AC_STRUCT_TM])

dnl C header files 
  AC_REQUIRE([AC_HEADER_STDC])

dnl check for library functions
  AC_LANG_PUSH([C++])

dnl checks for header files.
  AC_CHECK_HEADERS([iostream stack map])

  AC_REQUIRE([AC_FUNC_MALLOC])
  AC_CHECK_LIB([m], [pow])
  AC_CHECK_FUNCS([sqrt strchr])
  AC_LANG_POP([C++])

dnl check all components
  AC_REQUIRE([ALUGRID_SERIAL_PARALLEL])
  AC_REQUIRE([ALUGRID_PATH_METIS])
  AC_REQUIRE([ALUGRID_PATH_PARMETIS])
  AC_REQUIRE([ALUGRID_PATH_PARTY])

dnl convenience-variables if every found package should be used
  AC_SUBST(ALL_PKG_LIBS, "$LIBS $ALUGRID_PKG_LIBS")
  AC_SUBST(ALL_PKG_LDFLAGS, "$LDFLAGS $ALUGRID_PKG_LDFLAGS")
  AC_SUBST(ALL_PKG_CPPFLAGS, "$CPPFLAGS $ALUGRID_PKG_CPPFLAGS")
])


AC_DEFUN([ALUGRID_SUMMARY_ALL],[
  # show search results
  echo
if test x$with_parallel = xnone ; then
  echo "ALUGrid is not ready to compile for computations!"
else
  echo "ALUGrid is ready to compile for $with_parallel computations!"
fi

if test "x$with_mpi" = "xno" ; then 
  with_mpi="no $mpi_help_string"
fi  

echo
echo The following components where found: 
echo "-------------------------------------"
echo  
echo "METIS............: $with_metis"
echo "ParMETIS.........: $with_parmetis"
echo "PARTY............: $with_party"
echo "MPI..............: $with_mpi"
echo
echo "-------------------------------------"
echo
echo "See ./configure --help and config.log for reasons why a component wasn't found"
echo

])
