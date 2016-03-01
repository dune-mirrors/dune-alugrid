# set up project
project("dune-alugrid" C CXX)

#circumvent not building docs
set(BUILD_DOCS 1)
set(DUNE_REENABLE_ADD_TEST TRUE)

# general stuff
cmake_minimum_required(VERSION 2.8.12)

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

find_package(dune-grid REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${dune-grid_MODULE_PATH}
  "${PROJECT_SOURCE_DIR}/cmake/modules")

#include the dune macros
include(DuneMacros)
include(GridType) 

# start a dune project with information from dune.module
dune_project()

add_subdirectory(cmake/modules)
add_subdirectory(m4)

add_subdirectory(doc/doxygen)

add_subdirectory(dune)
add_subdirectory(lib)

add_subdirectory(examples EXCLUDE_FROM_ALL)
add_subdirectory(utils)

# finalize the dune project, e.g., generate config.h etc.
finalize_dune_project(GENERATE_CONFIG_H_CMAKE)
