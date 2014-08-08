#GRIDTYPE = YASPGRID
#GRIDTYPE = SGRID
GRIDTYPE = ALUGRID_CUBE
#GRIDTYPE = ALUGRID_SIMPLEX
#GRIDTYPE = ALUGRID_CONFORM
GRIDDIM = 3

SUBDIRS = callback loadbalance dgf backuprestore communication testEfficiency

EXTRA_DIST = alugrid.cfg 

DATACLEANFILES = dgfparser.log
CLEANFILES = $(DATACLEANFILES)

#USE_OMP=-fopenmp

# extra flags: -DUSE_VECTOR_FOR_PWF -DNO_OUTPUT -DCALLBACK_ADAPTATION
EXTRAFLAGS = 

AM_CPPFLAGS += $(EXTRAFLAGS)

examplesdir = $(includedir)/examples
examples_HEADERS = \
    adaptation.hh  diagnostics.hh  paralleldgf.hh        problem-ball.hh   problem-transport.hh \
    datamap.hh     fvscheme.hh     piecewisefunction.hh  problem-euler.hh  problem.hh

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
