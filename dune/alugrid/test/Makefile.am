SUBDIRS = dgf

check_PROGRAMS = test-alugrid test-geometries test-entities test-iterators
TESTS = test-alugrid test-geometries test-entities test-iterators

AM_CPPFLAGS += $(DUNEMPICPPFLAGS)
AM_LDFLAGS += $(DUNEMPILDFLAGS)
LDADD += $(DUNEMPILIBS)

test_alugrid_SOURCES = test-alugrid.cc 
test_geometries_SOURCES = test-geometries.cc
test_entities_SOURCES = test-entities.cc
test_iterators_SOURCES = test-iterators.cc

CLEANFILES = dgfparser.log

include $(top_srcdir)/am/global-rules
