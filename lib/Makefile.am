lib_LTLIBRARIES = libdunealugrid.la

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
