set(alu2dgriddir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/2d)
set(alu2dgrid_HEADERS  alu2dinclude.hh  entity_imp.cc  geometry_imp.cc
  grid_imp.cc  intersection.hh  iterator.hh  entity.hh  geometry.hh   grid.hh
  indexsets.hh  intersection_imp.cc  iterator_imp.cc datahandle.hh
  gridfactory.hh  gridfactory.cc  alugrid.hh  bndprojection.hh
  intersection.cc entityseed.hh)
set(headercheck_IGNORE  ${alu2dgrid_HEADERS})
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${alu2dgrid_HEADERS} DESTINATION ${alu2dgriddir})