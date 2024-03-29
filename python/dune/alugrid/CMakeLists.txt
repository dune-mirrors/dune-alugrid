add_python_targets(alugrid
  __init__
  _grids
)

dune_add_pybind11_module(NAME _alugrid SOURCES _alugrid.cc)
if( DUNE_ENABLE_PYTHONMODULE_PRECOMPILE )
  set(DIMS 2 3)
  set(GEOMTYPES simplex cube)
  foreach( DIM ${DIMS} )
    foreach( GT ${GEOMTYPES} )
      dune_add_pybind11_submodule(NAME alugrid_${GT}_${DIM}
                                  MODULE _alugrid SOURCES registeralugrid.cc
                                  COMPILE_DEFINITIONS DIM=${DIM} GEOMTYPE=Dune::${GT})
    endforeach()
  endforeach()
endif()
set_property(TARGET _alugrid PROPERTY LINK_LIBRARIES Dune::ALUGrid APPEND)

if(SKBUILD)
  install(TARGETS _alugrid LIBRARY DESTINATION python/dune/alugrid)
endif()
