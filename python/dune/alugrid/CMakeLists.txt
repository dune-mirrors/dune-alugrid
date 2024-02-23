add_python_targets(alugrid
  __init__
  _grids
)

set(ALUGRID_SOURCES _alugrid.cc)
if( DUNE_ENABLE_PYTHONMODULE_PRECOMPILE )
  set(ALUGRID_SOURCES ${ALUGRID_SOURCES} registeralugrid_22_simplex.cc registeralugrid_33_simplex.cc )
  set(ALUGRID_SOURCES ${ALUGRID_SOURCES} registeralugrid_22_cube.cc    registeralugrid_33_cube.cc )
endif()

dune_add_pybind11_module(NAME _alugrid SOURCES ${ALUGRID_SOURCES})
set_property(TARGET _alugrid PROPERTY LINK_LIBRARIES Dune::ALUGrid APPEND)
if(SKBUILD)
  install(TARGETS _alugrid LIBRARY DESTINATION python/dune/alugrid)
endif()
