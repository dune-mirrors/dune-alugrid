lib_LTLIBRARIES = libalugrid_2d.la libalugrid_serial.la libalugrid_parallel.la libalugrid_mpi.la libdunealugrid.la

libalugrid_2d_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/src/2d/grid.cc \
	$(top_srcdir)/dune/alugrid/src/2d/handle.cc \
	$(top_srcdir)/dune/alugrid/src/2d/hdlrw.cc \
	$(top_srcdir)/dune/alugrid/src/2d/mknbrs.cc \
	$(top_srcdir)/dune/alugrid/src/2d/triang.cc \
	$(top_srcdir)/dune/alugrid/src/2d/vmmap.cc \
	$(top_srcdir)/dune/alugrid/src/2d/vtx_btree.cc

libalugrid_2d_la_CPPFLAGS = $(AM_CPPFLAGS)
libalugrid_2d_la_LDFLAGS = $(AM_LDFLAGS)
libalugrid_2d_la_LIBADD = 

libalugrid_serial_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/src/serial/ghost_elements.cc \
	$(top_srcdir)/dune/alugrid/src/serial/ghost_info.cc \
	$(top_srcdir)/dune/alugrid/src/serial/gitter_geo.cc \
	$(top_srcdir)/dune/alugrid/src/serial/gitter_hexa_top.cc \
	$(top_srcdir)/dune/alugrid/src/serial/gitter_impl.cc \
	$(top_srcdir)/dune/alugrid/src/serial/gitter_mgb.cc \
	$(top_srcdir)/dune/alugrid/src/serial/gitter_sti.cc \
	$(top_srcdir)/dune/alugrid/src/serial/gitter_tetra_top.cc \
	$(top_srcdir)/dune/alugrid/src/serial/mapp_cube_3d.cc \
	$(top_srcdir)/dune/alugrid/src/serial/mapp_tetra_3d.cc \
	$(top_srcdir)/dune/alugrid/src/serial/myalloc.cc \
	$(top_srcdir)/dune/alugrid/src/duneinterface/gitter_dune_impl.cc

libalugrid_serial_la_CPPFLAGS = $(AM_CPPFLAGS)
libalugrid_serial_la_LDFLAGS = $(AM_LDFLAGS)
libalugrid_serial_la_LIBADD = 

libalugrid_parallel_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/src/duneinterface/gitter_dune_pll_impl.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/mpAccess.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/gitter_hexa_top_pll.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/gitter_pll_impl.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/gitter_pll_idn.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/gitter_pll_ldb.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/gitter_pll_mgb.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/gitter_pll_sti.cc \
	$(top_srcdir)/dune/alugrid/src/parallel/gitter_tetra_top_pll.cc

libalugrid_parallel_la_CPPFLAGS = $(AM_CPPFLAGS)
libalugrid_parallel_la_LDFLAGS = $(AM_LDFLAGS)
libalugrid_parallel_la_LIBADD = 

libalugrid_mpi_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/src/parallel/mpAccess_MPI.cc

libalugrid_mpi_la_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
libalugrid_mpi_la_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
libalugrid_mpi_la_LIBADD = $(DUNEMPILIBS)

libdunealugrid_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/3d/faceutility.cc \
	$(top_srcdir)/dune/alugrid/3d/mappings.cc \
	$(top_srcdir)/dune/alugrid/3d/topology.cc

#	$(top_srcdir)/dune/alugrid/2d/intersection.cc
#	$(top_srcdir)/dune/alugrid/2d/alu2dgridfactory.cc

libdunealugrid_la_CPPFLAGS = $(AM_CPPFLAGS)
libdunealugrid_la_LDFLAGS = $(AM_LDFLAGS)
libdunealugrid_la_LIBADD = $(DUNE_LIBS)

include $(top_srcdir)/am/global-rules
