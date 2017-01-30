ALUGRID_SERIAL_SOURCES=\
	$(top_srcdir)/dune/alugrid/impl/binaryio.cc \
	$(top_srcdir)/dune/alugrid/impl/macrofileheader.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/ghost_info.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/gitter_geo.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/gitter_hexa_top.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/gitter_impl.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/gitter_mgb.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/gitter_sti.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/gitter_tetra_top.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/mapp_cube_3d.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/mapp_tetra_3d.cc \
	$(top_srcdir)/dune/alugrid/impl/serial/myalloc.cc \
	$(top_srcdir)/dune/alugrid/impl/duneinterface/gitter_dune_impl.cc

ALUGRID_PARALLEL_SOURCES=\
	$(top_srcdir)/dune/alugrid/impl/serial/ghost_elements.cc \
	$(top_srcdir)/dune/alugrid/impl/duneinterface/gitter_dune_pll_impl.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/mpAccess.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/gitter_hexa_top_pll.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_impl.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_idn.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_ldb.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_mgb.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_sti.cc \
	$(top_srcdir)/dune/alugrid/impl/parallel/gitter_tetra_top_pll.cc

lib_LTLIBRARIES = libdunealugrid.la

libdunealugrid_la_SOURCES = \
  $(ALUGRID_SERIAL_SOURCES) \
  $(ALUGRID_PARALLEL_SOURCES) \
	$(top_srcdir)/dune/alugrid/3d/faceutility.cc \
	$(top_srcdir)/dune/alugrid/3d/mappings.cc \
	$(top_srcdir)/dune/alugrid/3d/topology.cc

libdunealugrid_la_CPPFLAGS = $(AM_CPPFLAGS) 
libdunealugrid_la_LDFLAGS = $(AM_LDFLAGS)
libdunealugrid_la_LIBADD = $(DUNE_LIBS) $(ALL_PKG_LIBS) ${ZOLTAN_LIBS} ${PARMETIS_LIBS} ${METIS_LIB} $(ZLIB_LIBS)

include $(top_srcdir)/am/global-rules
