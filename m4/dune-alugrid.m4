AC_DEFUN([DUNE_ALUGRID_CHECKS],[
  # check for the ParMetis library 
  AC_REQUIRE([DUNE_PATH_PARMETIS])

  # check whether ALUGrid was found by the dune-grid module 
  # this conflicts with this package
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
