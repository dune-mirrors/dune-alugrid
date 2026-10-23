set(meshconsistencyimpldir ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/common/mesh-consistency/impl)
set(meshconsistencyimpl_HEADERS edge.hh  elements.hh  element_traits.hh  initialization.hh  orientation.hh  reorder_nodes.hh  sort_nodes.hh)

# installation of the header files
install(FILES ${meshconsistencyimpl_HEADERS} DESTINATION ${meshconsistencyimpldir})
