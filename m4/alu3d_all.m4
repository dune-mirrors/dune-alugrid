# this meta-check calls everything needed for Dune to work and all
# possible components. Applications should use this so that
# Dune-updates enable new features automagically

# the entries are more or less copied from an "autoscan"-run in the
# dune-directory

AC_DEFUN([ALU3D_CHECK_ALL],[
  AC_LANG_PUSH([C++])
dnl check for programs
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CPP])
  AC_REQUIRE([ALU3D_CHECK_COMPILER])
  AC_REQUIRE([AC_PROG_INSTALL])
  AC_REQUIRE([AC_PROG_LN_S])
  AC_REQUIRE([AC_PROG_MAKE_SET])
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_PROG_LIBTOOL])

dnl checks for header files.
  AC_REQUIRE([AC_HEADER_STDC])
  AC_CHECK_HEADERS([malloc.h string.h])

dnl checks for typedefs, structures, and compiler characteristics.
#  doesn't work, but we don't need it currently
#  AC_REQUIRE([AC_HEADER_STDBOOL])
  AC_REQUIRE([AC_C_CONST])
  AC_REQUIRE([AC_C_INLINE])
  AC_REQUIRE([AC_TYPE_SIZE_T])
  AC_REQUIRE([AC_STRUCT_TM])

dnl check for library functions
  AC_REQUIRE([AC_FUNC_MALLOC])
#  doesn't work, but we don't need it currently
#  AC_REQUIRE([AC_FUNC_REALLOC])

  AC_LANG_PUSH([C++])
  AC_CHECK_LIB([m], [pow])
  AC_CHECK_FUNCS([sqrt strchr])
  AC_LANG_POP([C++])

dnl check all components
  AC_REQUIRE([ALU3D_PATH_METIS])
  AC_REQUIRE([ALU3D_PATH_PARTY])
  AC_REQUIRE([ALU3D_MPI])
  AC_REQUIRE([ALU3D_XDR])

  # convenience-variables if every found package should be used
  AC_SUBST(ALL_PKG_LIBS, "$LIBS $ALU3D_PKG_LIBS")
  AC_SUBST(ALL_PKG_LDFLAGS, "$LDFLAGS $ALU3D_PKG_LDFLAGS")
  AC_SUBST(ALL_PKG_CPPFLAGS, "$CPPFLAGS $ALU3D_PKG_CPPFLAGS")
  AC_LANG_POP([C++])
])

AC_DEFUN([ALU3D_SUMMARY_ALL],[
  # show search results

  echo
  echo "Found the following components for ALU3dGrid (parallel) : "
  echo
  echo "-----------------------------"
  echo  
  echo "METIS............: $with_metis"
  echo "PARTY............: $with_party"
  echo "MPI..............: $with_mpi"
  echo
  echo "-----------------------------"
  echo
  echo "See ./configure --help and config.log for reasons why a component wasn't found"
  echo

])
