set(MESH_CONSISTENCY_ROOT "" CACHE PATH "Path with mesh-consistency package inside")
mark_as_advanced(MESH_CONSISTENCY_ROOT)

set(MESH_CONSISTENCY_INCLUDE_DIR "MESH_CONSISTENCY_INCLUDE_DIR-NOTFOUND")
if( ${MESH_CONSISTENCY_ROOT} )
  # search mesh-consistency header
  find_path(MESH_CONSISTENCY_INCLUDE_DIR
    NAMES "mesh-consistency.hh"
    PATHS ${MESH_CONSISTENCY_ROOT}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH
    DOC "Include directory of mesh-consistency headers.")
endif()

# download mesh-consistency if not found
if( ALUGRID_DOWNLOAD_MESH_CONSISTENCY )
  if ( MESH_CONSISTENCY_INCLUDE_DIR STREQUAL "MESH_CONSISTENCY_INCLUDE_DIR-NOTFOUND" )
    message (STATUS "Downloading mesh-consistency")
    include (ExternalProject)
    externalProject_Add (mesh-consistency
      # original repo does not work on duneci
      #GIT_REPOSITORY https://zivgitlab.uni-muenster.de/marckoch/mesh-consistency.git
      GIT_REPOSITORY https://gitlab.dune-project.org/robert.kloefkorn/mesh-consistency.git
      GIT_TAG master
      PREFIX mesh-consistency
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
    )
    set(MESH_CONSISTENCY_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/mesh-consistency/src/mesh-consistency)

    # search mesh-consistency header
    find_path(MESH_CONSISTENCY_INCLUDE_DIR
      NAMES "mesh-consistency.hh"
      PATHS ${MESH_CONSISTENCY_DOWNLOAD_DIR}
      PATH_SUFFIXES include
      NO_DEFAULT_PATH
      DOC "Include directory of mesh-consistency headers.")
  endif()

  # exclude mesh-consistency if it was put here
  file(GLOB_RECURSE list RELATIVE ${MESH_CONSISTENCY_DOWNLOAD_DIR} "*.hh")
  foreach(item ${list})
    exclude_from_headercheck( ${item} )
  endforeach()
endif ()

# configure the library dune-alugrid
if( DUNE_ALUGRID_COMPILE_BINDINGS )
  target_compile_options(dunealugrid PRIVATE -include${CMAKE_BINARY_DIR}/config.h)
endif()

# add mpi flags to compile library
add_dune_mpi_flags( dunealugrid )

# link against the Dune libraries
target_link_libraries(dunealugrid PUBLIC ${DUNE_LIBS})

include(AddMETISFlags)
# METIS dependencies
add_dune_metis_flags(dunealugrid)

# zoltan dependencies
if(ZOLTAN_FOUND)
  target_link_libraries(dunealugrid PUBLIC ${ZOLTAN_LIBRARIES})
  set_property(TARGET dunealugrid APPEND PROPERTY
               INCLUDE_DIRECTORIES "${ZOLTAN_INCLUDE_DIRS}")
endif()
# zlib dependencies
if(ZLIB_FOUND)
  target_link_libraries(dunealugrid PUBLIC ${ZLIB_LIBRARIES})
  set_property(TARGET dunealugrid APPEND PROPERTY
               INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
endif()

# workaround for clang 6, which runs into an infinite loop on gitter_mgb.cc and gitter_pll_mgb.cc
# when optimization level is -O2 or higher
if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 5.9.9 AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 7.0.0)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_type_upper)
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
