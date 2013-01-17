alugriddir = $(includedir)/dune/alugrid
alugrid_HEADERS = grid.hh

SUBDIRS = common 2d 3d

noinst_LTLIBRARIES = libalugrid.la

libalugrid_la_SOURCES =
libalugrid_la_LIBADD = 2d/libalugrid2d.la 3d/libalugrid3d.la $(DUNE_LIBS)

nodist_EXTRA_libalugrid_la_SOURCES = dummy.cc
sourcescheck_DUMMY = dummy.cc

include $(top_srcdir)/am/global-rules
