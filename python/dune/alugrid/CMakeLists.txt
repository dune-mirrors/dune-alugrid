add_python_targets(alugrid
  __init__
  _grids
)

dune_add_pybind11_module(NAME _alugrid SOURCES _alugrid.cc)
target_link_libraries(_alugrid PUBLIC Dune::ALUGrid)

if( DUNE_ENABLE_PYTHONMODULE_PRECOMPILE )
  set(DIMS 2 3)
  set(GEOMTYPES simplex cube)
  foreach( DIM ${DIMS} )
    foreach( GT ${GEOMTYPES} )
      add_library(alugrid_${GT}_${DIM} OBJECT registeralugrid.cc)
      target_compile_definitions(alugrid_${GT}_${DIM} PRIVATE DIM=${DIM} GEOMTYPE=Dune::${GT})
      target_link_libraries(alugrid_${GT}_${DIM} PRIVATE Dune::ALUGrid)
      dune_target_enable_all_packages( alugrid_${GT}_${DIM} )

      target_link_libraries(_alugrid PRIVATE alugrid_${GT}_${DIM})
    endforeach()
  endforeach()
endif()

if(SKBUILD)
  install(TARGETS _alugrid LIBRARY DESTINATION python/dune/alugrid)
endif()
