AC_DEFUN([DUNE_ALUGRID_CHECKS],[

  dnl check for the ParMETIS library
  AC_REQUIRE([DUNE_PATH_PARMETIS])
  dnl check for the METIS library (check needs to after ParMETIS check)
  AC_REQUIRE([IMMDX_LIB_METIS])

  dnl add to global list
  dnl DUNE_ADD_ALL_PKG([PARMETIS], [\${PARMETIS_CPPFLAGS}],
  dnl                 [\${PARMETIS_LDFLAGS}], [\${PARMETIS_LIBS}])

  dnl add to global list
  dnl DUNE_ADD_ALL_PKG([METIS], [\${METIS_INCLUDE}],
  dnl                  [\${METIS_LDFLAGS}], [\${METIS_LIB}])

  dnl check whether ALUGrid was found by the dune-grid module 
  dnl this conflicts with this package
  if test "x$ALUGRID_CPPFLAGS" != "x" ; then 
    AC_MSG_ERROR([--with-alugrid conflicts with dune-alugrid module, remove the --with-alugrid from the configure options and rebuild dune-grid and dune-alugrid!]);
  fi

  DUNE_DEFINE_GRIDTYPE([ALUGRID_CONFORM],[],[Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::conforming >],[dune/alugrid/grid.hh],[dune/alugrid/dgf.hh])
  DUNE_DEFINE_GRIDTYPE([ALUGRID_CUBE],[],[Dune::ALUGrid< dimgrid, dimworld, Dune::cube, Dune::nonconforming >],[dune/alugrid/grid.hh],[dune/alugrid/dgf.hh])
  DUNE_DEFINE_GRIDTYPE([ALUGRID_SIMPLEX],[],[Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::nonconforming >],[dune/alugrid/grid.hh],[dune/alugrid/dgf.hh])
])

AC_DEFUN([DUNE_ALUGRID_CHECK_MODULE],[
  AC_MSG_NOTICE([Searching for dune-alugrid...])
  DUNE_CHECK_MODULES([dune-alugrid],[alugrid/grid.hh],[main])
])
