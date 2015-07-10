set(paralleldir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/impl/parallel)
set(parallel_HEADERS  alumetis.hh alusfc.hh aluzoltan.hh
                        gitter_hexa_top_pll.h gitter_pll_impl.h
                        gitter_pll_ldb.h gitter_pll_mgb.h
                        gitter_pll_sti.h gitter_tetra_top_pll.h mpAccess.h
                        mpAccess_MPI.h mpAccess_MPI_inline.h zcurve.hh)
set(EXTRA_DIST  gitter_hexa_top_pll.cc gitter_pll_idn.cc gitter_pll_impl.cc
                                                 gitter_pll_ldb.cc gitter_pll_mgb.cc gitter_pll_sti.cc
                                                 gitter_tetra_top_pll.cc mpAccess.cc)
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${parallel_HEADERS} DESTINATION ${paralleldir})
