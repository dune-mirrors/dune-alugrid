# Module providing convenience functions for using
#
# Provides the following functions:
#
# add_dune_zoltan_flags(target1 target2 ...)
#
# Adds the necessary flags to compile and link the targets with ZOLTAN support.
#
include(AddParMETISFlags)
# Scotch is not supported in CMake with Dune 2.3
if(NOT (("${DUNE_COMMON_VERSION_MAJOR}" STREQUAL "2")
        AND ("${DUNE_COMMON_VERSION_MINOR}" STREQUAL "3")))
  include(AddPTScotchFlags)
endif()

function(add_dune_zoltan_flags _targets)
  if(ZOLTAN_FOUND)
    foreach(_target ${_targets})
      # We use ZOLTAN_LIBRARY instead of ZOLTAN_LIBRARIES
      # Additional libraries are added at the end using add_dune_..._flags
      target_link_libraries(${_target} ${ZOLTAN_LIBRARY})
    endforeach(_target ${_targets})
    set_property(TARGET ${_targets}
      APPEND_STRING
      PROPERTY COMPILE_DEFINITIONS ENABLE_ZOLTAN)
    set_property(TARGET ${_targets} APPEND PROPERTY
      INCLUDE_DIRECTORIES "${ZOLTAN_INCLUDE_DIRS}")
  add_dune_parmetis_flags(${_targets})
  add_dune_ptscotch_flags(${_targets})
  endif(ZOLTAN_FOUND)
endfunction(add_dune_zoltan_flags _targets)
