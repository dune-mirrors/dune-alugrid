set(TESTS
  test-alugrid
  test-adaptivity
  test-fromtogridfactory
  test-iterators
  test-structuredgridfactory
  test-twists
  test-hierarchic
  test-ball-projection
  test-backup-restore
  test-gmsh
)

foreach(TEST ${TESTS})
  dune_add_test(NAME ${TEST} SOURCES ${TEST}.cc LINK_LIBRARIES dunealugrid)
endforeach()

add_subdirectory(dgf)
add_subdirectory(gmsh)

set(alutestdir ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/test)
set(alugridtest_HEADERS checkintersectionit.hh  checktwists.hh  insertgrid.hh )

install( FILES ${alugridtest_HEADERS} DESTINATION ${alutestdir} )
