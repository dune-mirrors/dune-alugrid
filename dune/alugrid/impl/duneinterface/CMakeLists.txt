set(duneinterfacedir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/impl/duneinterface)
set(duneinterface_HEADERS  gitter_dune_impl.h gitter_dune_pll_impl.h)
set(EXTRA_DIST  gitter_dune_impl.cc gitter_dune_pll_impl.cc)
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${duneinterface_HEADERS} DESTINATION ${duneinterfacedir})
