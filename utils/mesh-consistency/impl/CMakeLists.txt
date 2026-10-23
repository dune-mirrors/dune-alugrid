set(meshconsistency_HEADERS edge.hh elements.hh  element_traits.hh  initialization.hh  orientation.hh  reorder_nodes.hh  sort_nodes.hh)
install(FILES ${meshconsistency_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/utils/mesh-consistency/impl)
