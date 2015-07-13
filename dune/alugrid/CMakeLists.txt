set(SUBDIRS
  3d
  common
  impl
  test)

foreach(i ${SUBDIRS})
  if(${i} STREQUAL "test")
    set(opt EXCLUDE_FROM_ALL)
  endif(${i} STREQUAL "test")
  add_subdirectory(${i} ${opt})
  unset(opt)
endforeach(i ${SUBDIRS})

set(alugrid_HEADERS dgf.hh grid.hh)
install(FILES ${alugrid_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid)
