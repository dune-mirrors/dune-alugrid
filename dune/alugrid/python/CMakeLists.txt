set(alupythondir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/python)
set(alupython_HEADERS registergrid.hh)
# installation of the header files
install(FILES ${alupython_HEADERS} DESTINATION ${alupythondir})
