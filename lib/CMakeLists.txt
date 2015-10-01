# build library libdunealugrid

# OBJECT is needed since CMake 3.0.0
# but it conflicts with shared libs
set(_OBJECT_FLAG "OBJECT")
if(BUILD_SHARED_LIBS)
  set(_OBJECT_FLAG "")
endif()

dune_add_library(dunealugrid
  _DUNE_TARGET_OBJECTS:serial_
  _DUNE_TARGET_OBJECTS:parallel_
  ${PROJECT_SOURCE_DIR}/dune/alugrid/3d/faceutility.cc
  ${PROJECT_SOURCE_DIR}/dune/alugrid/3d/mappings.cc
  ${PROJECT_SOURCE_DIR}/dune/alugrid/3d/topology.cc
  ADD_LIBS "${DUNE_LIBS};${ZLIB_LIBRARIES}")

# This _is_ needed for the correct compile flags.
# (e.g. to not use the MPI C++ bindings, include path etc.)
add_dune_zoltan_flags(dunealugrid)

####################################################################
#### general warning: avoid such and similar commands here 
#### as they will not influence anything after the library
#### is build!!! (as long as you do not call the export() function
#add_dune_zlib_flags(dunealugrid)
#####################################################################
          
install(FILES dunealugridam2cmake.lib
  DESTINATION ${CMAKE_INSTALL_BINDIR}/../lib)
