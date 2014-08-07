GRIDTYPE = ALUGRID_CUBE
GRIDDIM = 3

DATACLEANFILES = dgfparser.log sioncheckpoint checkpoint.* *.*vtu
CLEANFILES = $(DATACLEANFILES)

examplesbrdir = $(includedir)/examples/backuprestore
examplesbr_HEADERS = sionlib.hh paralleldgf.hh

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
