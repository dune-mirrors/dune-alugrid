set(TESTS
  adapt
  backuprestore
  sizes
  threadsafety
)

# copy data to build source to make tests work
file(COPY alugrid.cfg grids DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(ALUGRID_TORTURE_TESTS)
  foreach(TEST ${TESTS})
    dune_add_test(NAME ${TEST} SOURCES ${TEST}.cc MPI_RANKS 1 2 3 4 TIMEOUT 3600 LINK_LIBRARIES dunealugrid)
  endforeach()
endif()
