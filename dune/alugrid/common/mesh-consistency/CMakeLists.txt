add_subdirectory(impl)

set(meshconsistencydir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/common/mesh-consistency)
set(meshconsistency_HEADERS element_types.hh mesh-consistency.hh)

# installation of the header files
install(FILES ${meshconsistency_HEADERS} DESTINATION ${meshconsistencydir})
