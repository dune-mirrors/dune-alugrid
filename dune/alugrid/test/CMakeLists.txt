set(SUBDIRS dgf)
set(TESTS
  test-alugrid
  test-fromtogridfactory
  test-twists
)

# copy data to build source to make tests work
file(COPY dgf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(test-alugrid test-alugrid.cc)
add_executable(test-fromtogridfactory test-fromtogridfactory.cc)
add_executable(test-twists test-twists.cc)

foreach(i ${SUBDIRS})
  if(${i} STREQUAL "test")
    set(opt EXCLUDE_FROM_ALL)
  endif(${i} STREQUAL "test")
  add_subdirectory(${i} ${opt})
  unset(opt)
endforeach(i ${SUBDIRS})

foreach(i ${TESTS})
  dune_target_enable_all_packages( ${i} )
  add_test(${i} ${i})
endforeach(i ${TESTS})

# We do not want want to build the tests during make all
# but just build them on demand
add_directory_test_target(_test_target)
add_dependencies(${_test_target} ${TESTS} ${COMPILE_XFAIL_TESTS})
