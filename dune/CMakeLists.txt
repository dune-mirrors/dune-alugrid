set(dunedir  ${CMAKE_INSTALL_INCLUDEDIR}/dune)
set(SUBDIRS  alugrid)
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
foreach(i ${SUBDIRS})
  if(${i} STREQUAL "test")
    set(opt EXCLUDE_FROM_ALL)
  endif(${i} STREQUAL "test")
  add_subdirectory(${i} ${opt})
  unset(opt)
endforeach(i ${SUBDIRS})

# if Python bindings are enabled, include necessary sub directories.
if( DUNE_ENABLE_PYTHONBINDINGS )
  add_subdirectory("python")
endif()
