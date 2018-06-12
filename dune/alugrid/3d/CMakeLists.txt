set(alu3dgriddir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/3d)
set(alu3dgrid_HEADERS  alu3dinclude.hh communication.hh
  datacollectorcaps.hh datahandle.hh grid.hh grid_imp.cc grid_inline.hh
  mappings.hh mappings_imp.cc topology.hh topology.cc
  entity.hh entity_imp.cc entity_inline.hh entityseed.hh
  faceutility.hh faceutility_imp.cc geometry.hh geometry_imp.cc
        gridfactory.hh gridfactory.cc gridview.hh
  indexsets.hh iterator.hh iterator.cc iterator_imp.cc alu3diterators.hh
  alugrid.hh)
set(headercheck_IGNORE  ${alu3dgrid_HEADERS})
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${alu3dgrid_HEADERS} DESTINATION ${alu3dgriddir})
