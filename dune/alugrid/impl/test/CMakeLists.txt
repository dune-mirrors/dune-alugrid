set(TESTS
  adapt
  backuprestore
  sizes
  threadsafety
)

# copy data to build source to make tests work
file(COPY alugrid.cfg grids DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#dune_add_test(NAME sizes SOURCES sizes.cc LINK_LIBRARIES dunealugrid)
# dune 2.5 and higher
#if(NOT (("${DUNE_COMMON_VERSION_MAJOR}" STREQUAL "2")
#           AND ("${DUNE_COMMON_VERSION_MINOR}" STREQUAL "4")))
#  foreach(TEST ${TESTS})
#    dune_add_test(NAME ${TEST} SOURCES ${TEST}.cc LINK_LIBRARIES dunealugrid)
#  endforeach()
#endif()
