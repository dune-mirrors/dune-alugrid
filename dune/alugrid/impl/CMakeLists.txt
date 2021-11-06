set(impldir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/impl)
set(impl_HEADERS  binaryio.hh byteorder.hh indexstack.h macrofileheader.hh projectvertex.h)
set(SUBDIRS  serial parallel duneinterface test)
set(EXTRA_DIST  binaryio.cc macrofileheader.cc)
# include not needed for CMake
# include $(top_srcdir)/am/global-rules

# serial sources
target_sources(dunealugrid PRIVATE
  binaryio.cc
  macrofileheader.cc
  serial/ghost_info.cc
  serial/gitter_geo.cc
  serial/gitter_hexa_top.cc
  serial/gitter_impl.cc
  serial/gitter_mgb.cc
  serial/gitter_sti.cc
  serial/gitter_tetra_top.cc
  serial/mapp_cube_3d.cc
  serial/mapp_tetra_3d.cc
  serial/myalloc.cc
  duneinterface/gitter_dune_impl.cc)

# parallel sources
target_sources(dunealugrid PRIVATE
  serial/ghost_elements.cc
  duneinterface/gitter_dune_pll_impl.cc
  parallel/mpAccess.cc
  parallel/gitter_hexa_top_pll.cc
  parallel/gitter_pll_impl.cc
  parallel/gitter_pll_idn.cc
  parallel/gitter_pll_ldb.cc
  parallel/gitter_pll_mgb.cc
  parallel/gitter_pll_sti.cc
  parallel/gitter_tetra_top_pll.cc)

install(FILES ${impl_HEADERS} DESTINATION ${impldir})
foreach(i ${SUBDIRS})
  if(${i} STREQUAL "test")
    set(opt EXCLUDE_FROM_ALL)
  endif(${i} STREQUAL "test")
  add_subdirectory(${i} ${opt})
  unset(opt)
endforeach(i ${SUBDIRS})
