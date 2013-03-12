if ! ALUGRID

libalugrid_2d_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/impl/2d/grid.cc \
	$(top_srcdir)/dune/alugrid/impl/2d/handle.cc \
	$(top_srcdir)/dune/alugrid/impl/2d/hdlrw.cc \
	$(top_srcdir)/dune/alugrid/impl/2d/mknbrs.cc \
	$(top_srcdir)/dune/alugrid/impl/2d/triang.cc \
	$(top_srcdir)/dune/alugrid/impl/2d/vmmap.cc \
	$(top_srcdir)/dune/alugrid/impl/2d/vtx_btree.cc

libalugrid_2d_la_CPPFLAGS = $(AM_CPPFLAGS)
libalugrid_2d_la_LDFLAGS = $(AM_LDFLAGS)
libalugrid_2d_la_LIBADD = 

libalugrid_serial_la_SOURCES = \
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

libalugrid_serial_la_CPPFLAGS = $(AM_CPPFLAGS)
libalugrid_serial_la_LDFLAGS = $(AM_LDFLAGS)
libalugrid_serial_la_LIBADD = 

libalugrid_parallel_la_SOURCES = \
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

libalugrid_parallel_la_CPPFLAGS = $(AM_CPPFLAGS)
libalugrid_parallel_la_LDFLAGS = $(AM_LDFLAGS)
libalugrid_parallel_la_LIBADD = $(ALL_PKG_LIBS) libalugrid_serial.la ${ZOLTAN_LIBS} ${PARMETIS_LIBS} ${METIS_LIB}

ALUGRID_IMPL_LIBS=libalugrid_2d.la libalugrid_parallel.la libalugrid_serial.la 
lib_LTLIBRARIES = $(ALUGRID_IMPL_LIBS) libdunealugrid.la

libdunealugrid_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/2d/intersection.cc \
	$(top_srcdir)/dune/alugrid/2d/gridfactory.cc \
	$(top_srcdir)/dune/alugrid/3d/faceutility.cc \
	$(top_srcdir)/dune/alugrid/3d/mappings.cc \
	$(top_srcdir)/dune/alugrid/3d/topology.cc


libdunealugrid_la_CPPFLAGS = $(AM_CPPFLAGS) 
libdunealugrid_la_LDFLAGS = $(AM_LDFLAGS)
libdunealugrid_la_LIBADD = $(DUNE_LIBS) $(ALUGRID_IMPL_LIBS)

include $(top_srcdir)/am/global-rules

endif
