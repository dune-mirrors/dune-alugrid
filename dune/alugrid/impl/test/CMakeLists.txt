set(TESTS
  adapt
  backuprestore
  sizes
  threadsafety
)

# copy data to build source to make tests work
file(COPY alugrid.cfg grids DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

foreach(TEST ${TESTS})
  dune_add_test(NAME ${TEST} SOURCES ${TEST}.cc LINK_LIBRARIES dunealugrid)
endforeach()
