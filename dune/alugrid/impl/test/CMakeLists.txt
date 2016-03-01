set(TESTS adapt
          backuprestore
          sizes
          threadsafety)

# copy data to build source to make tests work
file(COPY alugrid.cfg grids
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(sizes sizes.cc)
add_executable(adapt adapt.cc)
add_executable(backuprestore backuprestore.cc)
add_executable(threadsafety threadsafety.cc)

dune_register_package_flags(LIBRARIES "${DUNE_ALUGRID_LIBRARY}"
                            INCLUDE_DIRS "${DUNE_ALUGRID_INCLUDEDIRS}")

foreach(i ${TESTS})
  # target_link_libraries(${i} ${ZLIB_LIBRARIES})
  # libdunalugrid needs symbols from zoltan. Therefore we need to link
  # to Zoltan afterwards.
  dune_target_enable_all_packages(${i})
endforeach(i ${TESTS})
# We do not want want to build the tests during make all
# but just build them on demand
# We also don't want to build them when make test is called
# add_dependencies(${_test_target} ${TESTS} ${COMPILE_XFAIL_TESTS})
