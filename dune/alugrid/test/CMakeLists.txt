set(SUBDIRS dgf)
set(TESTS
  test-alugrid
  test-adaptivity
  test-fromtogridfactory
  test-twists
  test-hierarchic
  test-ball-projection
)

# copy data to build source to make tests work
file(COPY dgf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(test-alugrid test-alugrid.cc)
add_executable(test-adaptivity test-adaptivity.cc)
add_executable(test-fromtogridfactory test-fromtogridfactory.cc)
add_executable(test-twists test-twists.cc)
add_executable(test-hierarchic test-hierarchic.cc)
add_executable(test-ball-projection test-ball-projection.cc)

foreach(i ${SUBDIRS})
  if(${i} STREQUAL "test")
    set(opt EXCLUDE_FROM_ALL)
  endif(${i} STREQUAL "test")
  add_subdirectory(${i} ${opt})
  unset(opt)
endforeach(i ${SUBDIRS})

dune_register_package_flags(LIBRARIES "${DUNE_ALUGRID_LIBRARY}"
                            INCLUDE_DIRS "${DUNE_ALUGRID_INCLUDEDIRS}")

# enable all package flags for all tests
foreach(i ${TESTS})
  dune_target_enable_all_packages( ${i} )
  add_test(${i} ${i})
endforeach(i ${TESTS})
