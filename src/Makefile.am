# Makefile.am for building the ALU3d-Grid library
AUTOMAKE_OPTIONS = foreign 1.5

SUBDIRS = serial parallel duneinterface 

# here the headers to install are defined 
include serial/Makefile.am.include
include parallel/Makefile.am.include
include duneinterface/Makefile.am.include

# work arround for automake versions < 1.8 
ALUGRID_CPPFLAGS = $(ALL_PKG_CPPFLAGS) -I./serial -I./parallel -I./duneinterface 

# build libALU3dGrid 
lib_LTLIBRARIES = libalugrid.la

# headers to install 
nobase_include_HEADERS = alugrid_serial.h alugrid_parallel.h stlheaders.h $(serialhdrs) $(parallelhdrs) $(duneinterfacehdrs)


if MPI 
# if MPI was found the parallel version is build 

# add mpi path to cpplags 
AM_CPPFLAGS = $(ALUGRID_CPPFLAGS) $(MPI_CPPFLAGS)
LDADD = $(ALL_PKG_LDFLAGS) $(ALL_PKG_LIBS)

# build lib with parallel support 
dist_libalugrid_la_SOURCES = alugrid_serial.cc alugrid_parallel.cc 
libalugrid_la_LIBADD = $(PARTY_LDFLAGS) $(PARTY_LIBS) $(METIS_LDFLAGS) $(METIS_LIBS) 

else 
# if no mpi build only serial lib 

LDADD = $(ALL_PKG_LDFLAGS) $(ALL_PKG_LIBS)
AM_CPPFLAGS = $(ALUGRID_CPPFLAGS)
# build only serial grid lib
dist_libalugrid_la_SOURCES = alugrid_serial.cc 

endif
