AC_DEFUN([DUNE_ALUGRID_CHECKS],[
  DUNE_DEFINE_GRIDTYPE([ALUGRID_CONFORM],[],[Dune::ALUGrid< dimgrid, dimworld, simplex, conforming >],[dune/grid/alugrid.hh],[dune/grid/io/file/dgfparser/dgfalu.hh])
  DUNE_DEFINE_GRIDTYPE([ALUGRID_CUBE],[],[Dune::ALUGrid< dimgrid, dimworld, cube, nonconforming >],[dune/grid/alugrid.hh],[dune/grid/io/file/dgfparser/dgfalu.hh])
  DUNE_DEFINE_GRIDTYPE([ALUGRID_SIMPLEX],[],[Dune::ALUGrid< dimgrid, dimworld, simplex, nonconforming >],[dune/grid/alugrid.hh],[dune/grid/io/file/dgfparser/dgfalu.hh])
])

AC_DEFUN([DUNE_ALUGRID_CHECK_MODULE],[
  AC_MSG_NOTICE([Searching for dune-alugrid...])
  DUNE_CHECK_MODULES([dune-alugrid],[grid/alugrid.hh])
])
