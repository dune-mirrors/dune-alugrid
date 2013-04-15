SUBDIRS = dgf

check_PROGRAMS = test-alugrid
TESTS = test-alugrid

test_alugrid_SOURCES = test-alugrid.cc
test_alugrid_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
test_alugrid_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
test_alugrid_LDADD = $(LDADD) $(DUNEMPILIBS)

CLEANFILES = dgfparser.log

include $(top_srcdir)/am/global-rules
