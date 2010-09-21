# Makefile.am for building the ALU3d-Grid library
AUTOMAKE_OPTIONS = foreign 1.5

SUBDIRS = serial parallel duneinterface alu2d 

# here the headers to install are defined 
include serial/Makefile.am.include
include parallel/Makefile.am.include
include duneinterface/Makefile.am.include
include alu2d/Makefile.am.include

# work arround for automake versions < 1.8 
ALUGRID_CPPFLAGS = $(ALL_PKG_CPPFLAGS) -I./serial -I./parallel -I./duneinterface -I./alu2d

# build libALU3dGrid 
lib_LTLIBRARIES = libalugrid.la

# headers to install 
nobase_include_HEADERS = alugrid_serial.h alugrid_parallel.h alugrid_2d.h stlheaders.h indexstack.h projectvertex.h $(serialhdrs) $(parallelhdrs) $(duneinterfacehdrs) $(alu2dhdrs)

# add all package flags 
AM_CPPFLAGS = -Wfatal-errors $(ALUGRID_CPPFLAGS)
libalugrid_la_CPPFLAGS = $(ALUGRID_CPPFLAGS) $(ALL_PKG_CPPFLAGS)
libalugrid_la_LIBADD = $(PARMETIS_LDFLAGS) $(PARMETIS_LIBS) $(METIS_LDFLAGS) $(METIS_LIBS) $(MPI_LDFLAGS) $(MPI_LIBS)

if MPI 
# if MPI was found the parallel version is build 
include_HEADERS = parallel/alugrid_defineparallel.h

# build lib with parallel support 
dist_libalugrid_la_SOURCES = alugrid_serial.cc alugrid_parallel.cc alugrid_2d.cc starmpi.c

else 
# if no mpi build only serial lib 
include_HEADERS = serial/alugrid_defineparallel.h

# build only serial grid lib
dist_libalugrid_la_SOURCES = alugrid_serial.cc alugrid_2d.cc
endif
