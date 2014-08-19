SUBDIRS = dgf

check_PROGRAMS = test-alugrid test-geometries test-entities
TESTS = test-alugrid test-geometries test-entities

AM_CPPFLAGS += $(DUNEMPICPPFLAGS)
AM_LDFLAGS += $(DUNEMPILDFLAGS)
LDADD += $(DUNEMPILIBS)

test_alugrid_SOURCES = test-alugrid.cc
test_geometries_SOURCES = test-geometries.cc
test_entities_SOURCES = test-entities.cc

CLEANFILES = dgfparser.log

include $(top_srcdir)/am/global-rules
