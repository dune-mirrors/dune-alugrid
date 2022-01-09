# general stuff
cmake_minimum_required(VERSION 3.13)

# set up project
project("dune-alugrid" C CXX)

#circumvent not building docs
set(BUILD_DOCS 1)
set(DUNE_REENABLE_ADD_TEST TRUE)

# Set the default DUNE_MAX_TEST_CORES that is used in the parallel tests
if(NOT DUNE_MAX_TEST_CORES)
  set(DUNE_MAX_TEST_CORES 4)
endif()

# grid selector in separate file to avoid excessive dependencies
set(ALUGRID_EXTRA_GRIDSELECTOR_FILE TRUE)
set(ALUGRID_DISABLE_METIS TRUE)

# set to true to compile more things into library
set(DUNE_ALUGRID_COMPILE_BINDINGS FALSE)

# variable to be set in config.h
if( DUNE_ALUGRID_COMPILE_BINDINGS )
  set(DUNE_ALUGRID_COMPILE_BINDINGS_IN_LIB 1)
endif()

# guess dune-common build dir
if(NOT (dune-common_DIR OR dune-common_ROOT OR
        "${CMAKE_PREFIX_PATH}" MATCHES ".*dune-common.*"))
    string(REPLACE ${CMAKE_PROJECT_NAME} dune-common dune-common_DIR
      ${PROJECT_BINARY_DIR})
endif()

#find dune-common and set the module path
find_package(dune-common REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${dune-common_MODULE_PATH}
  "${PROJECT_SOURCE_DIR}/cmake/modules")

#include the dune macros
include(DuneMacros)

# start a dune project with information from dune.module
dune_project()

# Define the dune-alugrid library
dune_add_library(dunealugrid)

add_subdirectory(cmake/modules)

add_subdirectory(doc/doxygen)

add_subdirectory(dune)
add_subdirectory(lib)

if(EXISTS "${PROJECT_SOURCE_DIR}/examples")
  add_subdirectory(examples EXCLUDE_FROM_ALL)
endif()
add_subdirectory(utils)

# if Python bindings are enabled, include necessary sub directories.
if( DUNE_ENABLE_PYTHONBINDINGS )
  add_subdirectory("python")
endif()

#look for testgrids.hh header in dune-grid doc sub directory
find_path(DUNE_TESTGRIDS_INCLUDE_DIR testgrids.hh
  PATHS ${dune-grid_INCLUDE_DIRS}/doc/grids/gridfactory
  NO_DEFAULT_PATH
)

if(DUNE_TESTGRIDS_INCLUDE_DIR)
  set(HAVE_DUNE_GRID_TESTGRIDS 1)
endif()

# finalize the dune project, e.g., generate config.h etc.
finalize_dune_project(GENERATE_CONFIG_H_CMAKE)
