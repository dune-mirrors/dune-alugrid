set(TESTS
  test-alugrid
  test-structuredgridfactory
  test-backup-restore
  test-dgf
)

set(EXTRA_TESTS
  test-adaptivity
  test-fromtogridfactory
  test-periodic
  test-iterators
  test-twists
  test-hierarchic
  test-ball-projection
  test-gmsh
)

link_libraries(dunealugrid)

foreach(TEST ${TESTS})
  dune_add_test(NAME ${TEST} SOURCES ${TEST}.cc MPI_RANKS 1 2 3 4 TIMEOUT 3600)
  dune_target_enable_all_packages( ${TEST} )
endforeach()

# the extra tests are only enabled in torture mode
if(ALUGRID_TORTURE_TESTS)
  foreach(TEST ${EXTRA_TESTS})
    dune_add_test(NAME ${TEST} SOURCES ${TEST}.cc)
    dune_target_enable_all_packages( ${TEST} )
  endforeach()
endif()

add_subdirectory(dgf)
add_subdirectory(gmsh)

set(alutestdir ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/test)
set(alugridtest_HEADERS checkintersectionit.hh  checktwists.hh  insertgrid.hh )

install( FILES ${alugridtest_HEADERS} DESTINATION ${alutestdir} )
