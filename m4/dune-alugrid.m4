AC_DEFUN([DUNE_ALUGRID_CHECKS],[
  # check for the ParMetis library 
  AC_REQUIRE([DUNE_PATH_PARMETIS])

  DUNE_DEFINE_GRIDTYPE([ALUGRID_CONFORM],[],[Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::conforming >],[dune/alugrid/grid.hh],[dune/alugrid/dgf.hh])
  DUNE_DEFINE_GRIDTYPE([ALUGRID_CUBE],[],[Dune::ALUGrid< dimgrid, dimworld, Dune::cube, Dune::nonconforming >],[dune/alugrid/grid.hh],[dune/alugrid/dgf.hh])
  DUNE_DEFINE_GRIDTYPE([ALUGRID_SIMPLEX],[],[Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::nonconforming >],[dune/alugrid/grid.hh],[dune/alugrid/dgf.hh])
])

AC_DEFUN([DUNE_ALUGRID_CHECK_MODULE],[
  AC_MSG_NOTICE([Searching for dune-alugrid...])
  DUNE_CHECK_MODULES([dune-alugrid],[alugrid/grid.hh],[main])

  dnl not used 
  if test x = x1 ; then  
    ac_save_LDFLAGS="$LDFLAGS"
    ac_save_CPPFLAGS="$CPPFLAGS"
    ac_save_LIBS="$LIBS"

    CPPFLAGS="$ac_save_CPPFLAGS $DUNE_ALUGRID_CPPFLAGS $DUNEMPICPPFLAGS" 
    LDFLAGS="$ac_save_LDFLAGS $DUNE_ALUGRID_LDFLAGS -L$DUNE_ALUGRID_LIBDIR/.libs $DUNEMPILDFLAGS"
    LIBS="$ac_save_LIBS $DUNE_ALUGRID_LIBS $DUNEMPILIBS"
    AC_LANG_PUSH([C++])
    AC_TRY_LINK(dnl
      [#]include<dune/alugrid/grid.hh>,
      getMPICommunicator, 
      [HAVE_DUNE_ALUGRID_MPI="1"],
      [HAVE_DUNE_ALUGRID_MPI="0"])
    AC_LANG_POP([C++])

    CPPFLAGS="$ac_save_CPPFLAGS"
    LDFLAGS="$ac_save_LDFLAGS"
    LIBS="$ac_save_LIBS"
  fi 

  dnl # simple check
  ALUGRID_COMPILED_PARALLEL=`grep libalugrid_mpi.la $DUNE_ALUGRID_ROOT/lib/libdunealugrid.la`
  dnl if test x"$HAVE_DUNE_ALUGRID_MPI" = x1 ; then 
  if test x"$ALUGRID_COMPILED_PARALLEL" != x ; then 
    AC_MSG_NOTICE([Found parallel DUNE-ALUGrid version (MPI).])
    AC_DEFINE(HAVE_DUNE_ALUGRID_MPI, 1, [Define to 1 if DUNE_ALUGRID was build with MPI support])
  else  
    AC_MSG_NOTICE([Found serial DUNE-ALUGrid version.])
  fi  
])
