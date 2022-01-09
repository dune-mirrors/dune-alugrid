set(alu3dgriddir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/3d)
set(alu3dgrid_HEADERS  aluinline.hh alu3dinclude.hh communication.hh
    datacollectorcaps.hh datahandle.hh grid.hh grid_imp.cc grid_inline.hh
    mappings.hh mappings_imp.cc topology.hh topology.cc
    entity.hh entity_imp.cc entity_inline.hh entityseed.hh
    faceutility.hh faceutility_imp.cc geometry.hh geometry_imp.cc
    gridfactory.hh gridfactory.cc gridview.hh
    indexsets.hh indexsets.cc iterator.hh iterator.cc iterator_imp.cc alu3diterators.hh
    alu3diterators_imp.cc
    alugrid.hh)
exclude_from_headercheck( gridfactory.hh gridfactory.cc )

# Add sources to the dunealugrid library
if(DUNE_ALUGRID_COMPILE_BINDINGS)
  target_sources(dunealugrid PRIVATE
    iterator.cc
    indexsets.cc
    mappings_imp.cc
    grid_imp.cc
    entity_imp.cc)
endif()

target_sources(dunealugrid PRIVATE
  faceutility.cc
  mappings.cc
  topology.cc)

# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${alu3dgrid_HEADERS} DESTINATION ${alu3dgriddir})
