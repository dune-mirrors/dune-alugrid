SUBDIRS = dgf

check_PROGRAMS = test-alugrid test-geometries
TESTS = test-alugrid test-geometries

AM_CPPFLAGS += $(DUNEMPICPPFLAGS)
AM_LDFLAGS += $(DUNEMPILDFLAGS)
LDADD += $(DUNEMPILIBS)

test_alugrid_SOURCES = test-alugrid.cc
test_geometries_SOURCES = test-geometries.cc

CLEANFILES = dgfparser.log

include $(top_srcdir)/am/global-rules
