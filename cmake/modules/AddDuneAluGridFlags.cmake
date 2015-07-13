# A convenience macro for using dune-alugrid
#
# add_dune_dune_alugrid_flags(targets)
#
# This will add flags for ZLib, Zoltan and SIONLIb,
# if those are found.

macro(add_dune_dune_alugrid_flags targets)
  foreach(target ${targets})
    if(ZLIB_FOUND)
      set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${ZLIB_INCLUDE_DIRS})
    endif()
    add_dune_sionlib_flags(target)
    add_dune_zoltan_flags(target)
  endforeach()
endmacro()
