GRIDTYPE = ALUGRID_CUBE
GRIDDIM = 3

DATACLEANFILES = dgfparser.log
CLEANFILES = $(DATACLEANFILES)

# possible problems are BALL, TRANSPORT, EULER
PROBLEM = -DBALL
# extra flags: -DUSE_VECTOR_FOR_PWF -DNO_OUTPUT
EXTRAFLAGS = 
AM_CPPFLAGS += $(EXTRAFLAGS) $(PROBLEM)
AM_LDFLAGS += $(DUNEMPILDFLAGS)
LDADD += $(DUNEMPILIBS)

EXTRA_DIST = alugrid.cfg 

exampleslbdir = $(includedir)/examples/loadbalanacing
exampleslb_HEADERS = \
    adaptation.hh  diagnostics.hh  loadbalance_simple.hh  paralleldgf.hh        problem-ball.hh   problem-transport.hh \
    datamap.hh     fvscheme.hh     loadbalance_zoltan.hh  piecewisefunction.hh  problem-euler.hh  problem.hh

# this does not compile with the old ALUGrid version
if ! ALUGRID 

if ZOLTAN
ZOLTAN_PROG=main_zoltan
endif
check_PROGRAMS = main_internal main_weights main_simple $(ZOLTAN_PROG)


main_internal_SOURCES = main.cc
main_internal_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) 

main_weights_SOURCES = main.cc
main_weights_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DUSE_WEIGHTS=1

main_simple_SOURCES = main.cc
main_simple_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DUSE_SIMPLELB=1 

if ZOLTAN
main_zoltan_SOURCES = main.cc
main_zoltan_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DUSE_ZOLTANLB=1
endif

endif

dataclean:
	@rm -f $(DATACLEANFILES)

include $(top_srcdir)/am/global-rules
