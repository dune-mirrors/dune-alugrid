lib_LTLIBRARIES = libalugrid_serial.la libdunealugrid.la

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

libdunealugrid_la_SOURCES = \
	$(top_srcdir)/dune/alugrid/2d/intersection.cc \
	$(top_srcdir)/dune/alugrid/2d/alu2dgridfactory.cc \
	$(top_srcdir)/dune/alugrid/3d/faceutility.cc \
	$(top_srcdir)/dune/alugrid/3d/mappings.cc \
	$(top_srcdir)/dune/alugrid/3d/topology.cc

libdunealugrid_la_CPPFLAGS = $(AM_CPPFLAGS)
libdunealugrid_la_LDFLAGS = $(AM_LDFLAGS)
libdunealugrid_la_LIBADD = $(DUNE_LIBS)

include $(top_srcdir)/am/global-rules
