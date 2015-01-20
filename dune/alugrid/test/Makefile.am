SUBDIRS = dgf

check_PROGRAMS = test-alugrid test-twists test-fromtogridfactory test-threadsafety test-geometries test-entities test-iterators
TESTS = test-alugrid test-twists

AM_CPPFLAGS += $(DUNEMPICPPFLAGS)
AM_LDFLAGS += $(DUNEMPILDFLAGS)
LDADD += $(DUNEMPILIBS)

test_alugrid_SOURCES = test-alugrid.cc
test_twists_SOURCES = test-twists.cc
test_fromtogridfactory_SOURCES = test-fromtogridfactory.cc
test_threadsafety_SOURCES = test-threadsafety.cc
test_geometries_SOURCES = test-geometries.cc
test_entities_SOURCES = test-entities.cc
test_iterators_SOURCES = test-iterators.cc

test_threadsafety_CPPFLAGS = $(AM_CPPFLAGS) -fopenmp
test_threadsafety_LDFLAGS = $(AM_LDFLAGS) -fopenmp

CLEANFILES = dgfparser.log

include $(top_srcdir)/am/global-rules
