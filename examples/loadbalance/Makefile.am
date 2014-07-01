GRIDTYPE = ALUGRID_CUBE
GRIDDIM = 3

DATACLEANFILES = dgfparser.log
CLEANFILES = $(DATACLEANFILES)

# possible problems are BALL, TRANSPORT, EULER
PROBLEM = -DTRANSPORT
# extra flags: -DUSE_VECTOR_FOR_PWF -DNO_OUTPUT
EXTRAFLAGS = 
AM_CPPFLAGS += $(EXTRAFLAGS) $(PROBLEM)

exampleslbdir = $(includedir)/examples/loadbalanacing
exampleslb_HEADERS = \
    adaptation.hh  diagnostics.hh  loadbalance_simple.hh  paralleldgf.hh        problem-ball.hh   problem-transport.hh \
    datamap.hh     fvscheme.hh     loadbalance_zoltan.hh  piecewisefunction.hh  problem-euler.hh  problem.hh

# this does not compile with the old ALUGrid version
if ! ALUGRID 
check_PROGRAMS = main_internal main_simple main_zoltan 

main_internal_SOURCES = main.cc
main_internal_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) 
main_internal_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
main_internal_LDADD = $(LDADD) $(DUNEMPILIBS)

main_simple_SOURCES = main.cc
main_simple_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DUSE_SIMPLELB 
main_simple_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
main_simple_LDADD = $(LDADD) $(DUNEMPILIBS)

main_zoltan_SOURCES = main.cc
main_zoltan_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DUSE_ZOLTANLB 
main_zoltan_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
main_zoltan_LDADD = $(LDADD) $(DUNEMPILIBS)
endif

dataclean:
	@rm -f $(DATACLEANFILES)

include $(top_srcdir)/am/global-rules
