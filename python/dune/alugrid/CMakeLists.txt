add_python_targets(alugrid
  __init__
  _grids
)

dune_add_pybind11_module(NAME _alugrid SOURCES _alugrid.cc)
if( DUNE_ENABLE_PYTHONMODULE_PRECOMPILE )
  include(AddMETISFlags)
  set(DIMS 2 3)
  set(GEOMTYPES simplex cube)
  foreach( DIM ${DIMS} )
    foreach( GT ${GEOMTYPES} )
      add_library(alugrid_${GT}_${DIM} OBJECT registeralugrid.cc)
      add_dune_mpi_flags( alugrid_${GT}_${DIM} )
      add_dune_metis_flags(dunealugrid)
      target_compile_definitions(alugrid_${GT}_${DIM} PRIVATE DIM=${DIM} GEOMTYPE=Dune::${GT})
      if(ZOLTAN_FOUND)
        set_property(TARGET alugrid_${GT}_${DIM} APPEND PROPERTY INCLUDE_DIRECTORIES "${ZOLTAN_INCLUDE_DIRS}")
      endif()
      # minimal c++ standard required
      target_compile_features(alugrid_${GT}_${DIM} PUBLIC cxx_std_17)
      target_link_libraries(_alugrid PUBLIC alugrid_${GT}_${DIM})
    endforeach()
  endforeach()
endif()
set_property(TARGET _alugrid PROPERTY LINK_LIBRARIES Dune::ALUGrid APPEND)

if(SKBUILD)
  install(TARGETS _alugrid LIBRARY DESTINATION python/dune/alugrid)
endif()
