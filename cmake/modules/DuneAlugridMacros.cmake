# do not ignore <PACKAGE>_ROOT variables
cmake_policy(SET CMP0074 NEW)

# from dune-grid/cmake/modules
include(GridType)
#define available alugrid types
dune_define_gridtype(GRIDSELECTOR_GRIDS GRIDTYPE ALUGRID_CONFORM
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::conforming >"
    HEADERS dune/alugrid/grid.hh dune/alugrid/dgf.hh)
dune_define_gridtype(GRIDSELECTOR_GRIDS GRIDTYPE ALUGRID_CUBE
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, Dune::cube, Dune::nonconforming >"
    HEADERS dune/alugrid/grid.hh dune/alugrid/dgf.hh)
dune_define_gridtype(GRIDSELECTOR_GRIDS GRIDTYPE ALUGRID_SIMPLEX
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::nonconforming >"
    HEADERS dune/alugrid/grid.hh dune/alugrid/dgf.hh)
dune_define_gridtype(GRIDSELECTOR_GRIDS GRIDTYPE ALUGRID_CONFORM_NOCOMM
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm >"
    HEADERS dune/alugrid/grid.hh dune/alugrid/dgf.hh)
dune_define_gridtype(GRIDSELECTOR_GRIDS GRIDTYPE ALUGRID_CUBE_NOCOMM
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm >"
    HEADERS dune/alugrid/grid.hh dune/alugrid/dgf.hh)
dune_define_gridtype(GRIDSELECTOR_GRIDS GRIDTYPE ALUGRID_SIMPLEX_NOCOMM
    DUNETYPE "Dune::ALUGrid< dimgrid, dimworld, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm >"
    HEADERS dune/alugrid/grid.hh dune/alugrid/dgf.hh)

# for ALUGrid module we write a separate grid selector file to avoid
# dependencies of the library files to all headers, for all other module
# the grid selection defs are written to config.h
if(DUNE_GRID_GRIDTYPE_SELECTOR AND ALUGRID_EXTRA_GRIDSELECTOR_FILE)
  file(WRITE "${CMAKE_BINARY_DIR}/gridselector.hh" "#include <config.h>\n${GRIDSELECTOR_GRIDS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include${CMAKE_BINARY_DIR}/gridselector.hh")
else()
  set(ALUGRID_CONFIG_H_BOTTOM "${ALUGRID_CONFIG_H_BOTTOM} ${GRIDSELECTOR_GRIDS}")
endif()

# avoid conflicts with normal ALUGrid
if( ALUGRID_CPPFLAGS )
  message(ERROR "--with-alugrid conflicts with dune-alugrid module,
  remove the --with-alugrid from the configure options,
  use the --without-alugrid configure option,
  and rebuild dune-grid and dune-alugrid!")
endif()

set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "-DENABLE_ALUGRID")
foreach(dir ${ALUGRID_INCLUDES})
  set_property(GLOBAL APPEND PROPERTY ALL_PKG_FLAGS "-I${dir}")
endforeach()

# contained in cmake system modules
find_package(ZLIB)
#set HAVE_ZLIB for config.h
set(HAVE_ZLIB ${ZLIB_FOUND})
if(ZLIB_FOUND)
  dune_register_package_flags(INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} LIBRARIES ${ZLIB_LIBRARIES})
endif()

find_package(SIONlib)
find_package(DLMalloc)

# set ZOLTAN_ROOT from environment variable if set
# -DZOLTAN_ROOT overrules the env variable
if( NOT ZOLTAN_ROOT )
  set(ZOLTAN_ROOT $ENV{ZOLTAN_ROOT})
endif()

find_package(ZOLTAN)
find_package(METIS)
if( METIS_FOUND AND ALUGRID_DISABLE_METIS )
  unset( HAVE_METIS )
endif()

# check for phtreads
include(FindPThreads)

# torture tests for extended testing
include(AlugridTortureTests)
