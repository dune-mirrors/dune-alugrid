set(serialdir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/impl/serial)
set(serial_HEADERS  gatherscatter.hh ghost_elements.h ghost_info.h
                 gitter_hexa_top.h gitter_impl.h gitter_mgb.h gitter_sti.h
                 gitter_tetra_top.h iterator_sti.h key.h lock.h mapp_cube_3d.h
                 mapp_tetra_3d_ext.h mapp_tetra_3d.h myalloc.h parallel.h
                 refcount.hh refinementrules.h serialize.h walk.h)
set(EXTRA_DIST  ghost_elements.cc ghost_info.cc gitter_geo.cc gitter_hexa_top.cc
             gitter_impl.cc gitter_mgb.cc gitter_sti.cc gitter_tetra_top.cc
             mapp_cube_3d.cc mapp_tetra_3d.cc myalloc.cc)
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${serial_HEADERS} DESTINATION ${serialdir})
