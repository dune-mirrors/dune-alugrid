add_python_targets(alugrid
  __init__
  _grids
)

if( ENABLE_PRECOMPILED_PYTHON_MODULES )
  set( DIMS 2 3 )
  foreach( DIM ${DIMS} )
    set( targetname _defaultalucubenonconforming${DIM}${DIM})
    dune_add_pybind11_module(NAME ${targetname})
    set_property(TARGET ${targetname} PROPERTY LINK_LIBRARIES ${DUNE_LIBS} dunealugrid APPEND)
    install(TARGETS ${targetname} LIBRARY DESTINATION python/dune/alugrid)

    set( targetname _defaultalusimplexnonconforming${DIM}${DIM})
    dune_add_pybind11_module(NAME ${targetname})
    set_property(TARGET ${targetname} PROPERTY LINK_LIBRARIES ${DUNE_LIBS} dunealugrid APPEND)
    install(TARGETS ${targetname} LIBRARY DESTINATION python/dune/alugrid)

    set( targetname _defaultalusimplexconforming${DIM}${DIM})
    dune_add_pybind11_module(NAME ${targetname})
    set_property(TARGET ${targetname} PROPERTY LINK_LIBRARIES ${DUNE_LIBS} dunealugrid APPEND)
    install(TARGETS ${targetname} LIBRARY DESTINATION python/dune/alugrid)
  endforeach()
endif()
