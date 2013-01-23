check_PROGRAMS = test-alugrid
TESTS = test-alugrid

test_alugrid_SOURCES = test-alugrid.cc
test_alugrid_CPPFLAGS = $(AM_CPPFLAGS) $(ALL_PKG_CPPFLAGS) $(DUNEMPICPPFLAGS)
test_alugrid_LDFLAGS = $(AM_LDFLAGS) $(ALL_PKG_LDFLAGS) $(DUNEMPILDFLAGS)
test_alugrid_LDADD = $(LDADD) $(ALL_PKG_LIBS) $(DUNEMPILIBS)

CLEANFILES = dgfparser.log

include $(top_srcdir)/am/global-rules
