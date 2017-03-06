set(TESTS
  test-alugrid
  test-adaptivity
  test-fromtogridfactory
  test-twists
  test-hierarchic
  test-ball-projection
  test-levelintersection
)

# 2.5 and higher
if(NOT (("${DUNE_COMMON_VERSION_MAJOR}" STREQUAL "2")
         AND ("${DUNE_COMMON_VERSION_MINOR}" STREQUAL "4")))
  foreach(TEST ${TESTS})
      dune_add_test(NAME ${TEST} SOURCES ${TEST}.cc LINK_LIBRARIES dunealugrid)
  endforeach()
else()
  # dune 2.4
  dune_register_package_flags(LIBRARIES "${DUNE_ALUGRID_LIBRARY}"
                              INCLUDE_DIRS "${DUNE_ALUGRID_INCLUDEDIRS}")

  foreach(TEST ${TESTS})
    add_executable(${TEST} ${TEST}.cc)
    dune_target_enable_all_packages( ${TEST} )
  endforeach()
  endif()

add_subdirectory(dgf)
