GRIDTYPE = ALUGRID_CUBE
GRIDDIM = 3

SUBDIRS = loadbalancing

EXTRA_DIST = alugrid.cfg

DATACLEANFILES = dgfparser.log
CLEANFILES = $(DATACLEANFILES)

examplesdir = $(includedir)/examples
examples_HEADERS = \
  adaptation.hh        diagnostics.hh  piecewisefunction.hh  problem-euler.hh \
  problem-transport.hh datamap.hh      fvscheme.hh           problem-ball.hh \
  problem.hh           threadmanager.hh

check_PROGRAMS = main_transport main_ball main_euler

main_transport_SOURCES = main.cc
main_transport_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DTRANSPORT $(ZOLTAN_CPPFLAGS)
main_tranaport_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
main_transport_LDADD = $(LDADD) $(DUNEMPILIBS)

main_ball_SOURCES = main.cc
main_ball_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DBALL $(ZOLTAN_CPPFLAGS)
main_ball_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
main_ball_LDADD = $(LDADD) $(DUNEMPILIBS)

main_euler_SOURCES = main.cc
main_euler_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DEULER $(ZOLTAN_CPPFLAGS)
main_euler_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
main_euler_LDADD = $(LDADD) $(DUNEMPILIBS)

dataclean:
	@rm -f $(DATACLEANFILES)

include $(top_srcdir)/am/global-rules
