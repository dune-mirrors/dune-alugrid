GRIDTYPE = ALUGRID_CUBE
GRIDDIM = 3

EXTRA_DIST = 

DATACLEANFILES = dgfparser.log
CLEANFILES = $(DATACLEANFILES)

exampleslbdir = $(includedir)/examples/loadbalanacing
exampleslb_HEADERS = \
  adaptation.hh  loadbalance.hh

# this does not compile with the old ALUGrid version
if ! ALUGRID 
check_PROGRAMS = main

main_SOURCES = main.cc
main_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) 
main_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
main_LDADD = $(LDADD) $(DUNEMPILIBS)
endif

dataclean:
	@rm -f $(DATACLEANFILES)

include $(top_srcdir)/am/global-rules
