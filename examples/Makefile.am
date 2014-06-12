#GRIDTYPE = YASPGRID
#GRIDTYPE = SGRID
GRIDTYPE = ALUGRID_CUBE
#GRIDTYPE = ALUGRID_SIMPLEX
#GRIDTYPE = ALUGRID_CONFORM
GRIDDIM = 3

SUBDIRS = loadbalance dgf

EXTRA_DIST = alugrid.cfg 

DATACLEANFILES = dgfparser.log
CLEANFILES = $(DATACLEANFILES)

#USE_OMP=-fopenmp

examplesdir = $(includedir)/examples
examples_HEADERS = \
  adaptation.hh        diagnostics.hh  piecewisefunction.hh  problem-euler.hh \
  problem-transport.hh datamap.hh      fvscheme.hh           problem-ball.hh \
  problem.hh           paralleldgf.hh

check_PROGRAMS = main_transport main_ball main_euler 

main_transport_SOURCES = main.cc
main_transport_CPPFLAGS = $(AM_CPPFLAGS) $(USE_OMP) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DTRANSPORT
main_transport_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS) $(USE_OMP) 
main_transport_LDADD = $(LDADD) $(DUNEMPILIBS) 

main_ball_SOURCES = main.cc
main_ball_CPPFLAGS = $(AM_CPPFLAGS) $(USE_OMP) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DBALL 
main_ball_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS) $(USE_OMP) 
main_ball_LDADD = $(LDADD) $(DUNEMPILIBS) 

main_euler_SOURCES = main.cc
main_euler_CPPFLAGS = $(AM_CPPFLAGS) $(USE_OMP) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DEULER
main_euler_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS) $(USE_OMP) 
main_euler_LDADD = $(LDADD) $(DUNEMPILIBS) 

dataclean:
	@rm -f $(DATACLEANFILES)

include $(top_srcdir)/am/global-rules
