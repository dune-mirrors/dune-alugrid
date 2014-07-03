bin_PROGRAMS =

EXTRA_PROGRAMS = convert

if ZOLTAN
bin_PROGRAMS += $(EXTRA_PROGRAMS)
endif

convertdir = $(includedir)/utils/convert-macrogrid
convert_HEADERS = partition.hh

convert_SOURCES = main.cc

include $(top_srcdir)/am/global-rules
