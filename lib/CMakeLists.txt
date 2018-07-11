# build library libdunealugrid
dune_add_library(dunealugrid
  _DUNE_TARGET_OBJECTS:serial_
  _DUNE_TARGET_OBJECTS:parallel_
  ${PROJECT_SOURCE_DIR}/dune/alugrid/3d/faceutility.cc
  ${PROJECT_SOURCE_DIR}/dune/alugrid/3d/mappings.cc
  ${PROJECT_SOURCE_DIR}/dune/alugrid/3d/topology.cc)

# add mpi flags to compile library
add_dune_mpi_flags( dunealugrid )

# on macOS, we have to explicitly link against the Dune libraries
if(CMAKE_HOST_APPLE)
  target_link_libraries(dunealugrid ${DUNE_LIBS})
endif()

# METIS dependencies
if(METIS_FOUND)
  target_link_libraries(dunealugrid ${METIS_LIBRARIES})
  set_property(TARGET dunealugrid APPEND PROPERTY
              INCLUDE_DIRECTORIES "${METIS_INCLUDE_DIRS}")
endif()
# zoltan dependencies
if(ZOLTAN_FOUND)
  target_link_libraries(dunealugrid ${ZOLTAN_LIBRARIES})
  set_property(TARGET dunealugrid APPEND PROPERTY
               INCLUDE_DIRECTORIES "${ZOLTAN_INCLUDE_DIRS}")
endif()
# zlib dependencies
if(ZLIB_FOUND)
  target_link_libraries(dunealugrid ${ZLIB_LIBRARIES})
  set_property(TARGET dunealugrid APPEND PROPERTY
               INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
endif()

# workaround for clang 6, which runs into an infinite loop on gitter_mgb.cc and gitter_pll_mgb.cc
# when optimization level is -O2 or higher
if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 6.0.0)
  string(TOUPPER ${CMAKE_BUILD_TYPE} _build_type_upper)
  set(_flags "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${_build_type_upper}}")
  string(REGEX MATCH " -O(2|3|fast) " _opt_level ${_flags})
  if (NOT _opt_level STREQUAL "")
    message(STATUS "detected clang 6 and optimized build: overriding optimization level for gitter_mgb.cc and gitter_pll_mgb.cc")
    set_source_files_properties(
      ../dune/alugrid/impl/serial/gitter_mgb.cc
      ../dune/alugrid/impl/parallel/gitter_pll_mgb.cc
      PROPERTIES
        COMPILE_FLAGS -O1
      )
    endif()
endif()

####################################################################
#### general warning: avoid such and similar commands here
#### as they will not influence anything after the library
#### is build!!! (as long as you do not call the export() function
#add_dune_zlib_flags(dunealugrid)
#####################################################################

install(FILES dunealugridam2cmake.lib
  DESTINATION ${CMAKE_INSTALL_BINDIR}/../lib)
