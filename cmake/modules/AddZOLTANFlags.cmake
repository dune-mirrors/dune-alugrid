# Module providing convenience functions for using 
#
# Provides the following functions:
#
# add_dune_zoltan_flags(target1 target2 ...)
#
# Adds the necessary flags to compile and link the targets with ZOLTAN support.
#
function(add_dune_zoltan_flags _targets)
  if(ZOLTAN_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${ZOLTAN_LIBRARIES})
    endforeach(_target ${_targets})
    set_property(TARGET ${_targets}
      APPEND_STRING
      PROPERTY COMPILE_FLAGS ENABLE_ZOLTAN=1 )
    set_property(TARGET ${_targets} APPEND PROPERTY
      INCLUDE_DIRECTORIES "${ZOLTAN_INCLUDE_DIRS}")
  endif(ZOLTAN_FOUND)
endfunction(add_dune_zoltan_flags _targets)
