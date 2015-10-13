set(TESTS adapt 
          backuprestore 
          sizes
          threadsafety)
set(ALUSOURCES
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/binaryio.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/macrofileheader.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/gitter_geo.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/gitter_hexa_top.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/gitter_impl.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/gitter_mgb.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/gitter_sti.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/gitter_tetra_top.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/mapp_cube_3d.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/mapp_tetra_3d.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/myalloc.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/duneinterface/gitter_dune_impl.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/ghost_info.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/serial/ghost_elements.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/duneinterface/gitter_dune_pll_impl.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/mpAccess.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/gitter_hexa_top_pll.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/gitter_pll_impl.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/gitter_pll_idn.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/gitter_pll_ldb.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/gitter_pll_mgb.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/gitter_pll_sti.cc
  ${dune-alugrid_SOURCE_DIR}/dune/alugrid/impl/parallel/gitter_tetra_top_pll.cc)

# copy data to build source to make tests work
file(COPY alugrid.cfg grids
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(sizes sizes.cc ${ALUSOURCES})
add_executable(adapt adapt.cc ${ALUSOURCES})
add_executable(backuprestore backuprestore.cc ${ALUSOURCES})
add_executable(threadsafety threadsafety.cc ${ALUSOURCES})

foreach(i ${TESTS})
  # target_link_libraries(${i} ${ZLIB_LIBRARIES})
  # libdunalugrid needs symbols from zoltan. Therefore we need to link
  # to Zoltan afterwards.
  dune_target_enable_all_packages(${i})
endforeach(i ${TESTS})
# We do not want want to build the tests during make all
# but just build them on demand
# We also don't want to build them when make test is called
add_dependencies(${_test_target} ${TESTS} ${COMPILE_XFAIL_TESTS})
