GRIDTYPE = ALUGRID_CUBE
GRIDDIM = 3

EXTRA_DIST = *.cfg

DATACLEANFILES = dgfparser.log
CLEANFILES = $(DATACLEANFILES)

exampleslbdir = $(includedir)/examples/loadbalanacing
exampleslb_HEADERS = \
  adaptation.hh  loadbalance.hh loadbalance_inline.hh

headercheck_IGNORE = loadbalance_inline.hh

# this does not compile with the old ALUGrid version
if ! ALUGRID 
check_PROGRAMS = main_simple main_zoltan partition

partition_SOURCES = partition.cc
partition_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) -DUSE_ZOLTANLB
partition_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
partition_LDADD = $(LDADD) $(DUNEMPILIBS)

main_simple_SOURCES = main.cc
main_simple_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) $(GRIDDIM_CPPFLAGS) 
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
