# Makefile.am for building the example program 
AUTOMAKE_OPTIONS = foreign 1.5

#CXXFLAGS=-O3 -funroll-loops -finline-functions -DNDEBUG 
CXXFLAGS=-g -Wall -Wfatal-errors
#CXXFLAGS=-g -Wall 

# work arround for automake versions < 1.8 
AM_CPPFLAGS = $(ALL_PKG_CPPFLAGS) $(DUNEMPICPPFLAGS)  
AM_LDFLAGS = $(ALL_PKG_LDFLAGS) $(DUNEMPILDFLAGS) $(DUNEMPILIBS)
# LDADD = $(LDADD) $(DUNEMPILIBS)

# headers to install 
noinst_PROGRAMS = sizes backuprestore 
# (following prog do not work at the moment - and did not in alugrid-dev):
# loadlb triang periodic adapt
sizes_SOURCES = sizes.cc
backuprestore_SOURCES = backuprestore.cc 
#periodic_SOURCES = periodic.cc 
#triang_SOURCES = triang.cc 
#adapt_SOURCES = adapt.cc 
#loadlb_SOURCES = loadlb.cc  

EXTRA_DIST=square.triangle 

include $(top_srcdir)/am/global-rules

