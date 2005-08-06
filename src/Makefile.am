# Makefile.am for building the ALU3d-Grid library
AUTOMAKE_OPTIONS = foreign 1.5

SUBDIRS = serial parallel duneinterface 

# here the headers to install are defined 
include serial/Makefile.am.include
include parallel/Makefile.am.include
include duneinterface/Makefile.am.include

# work arround for automake versions < 1.8 
ALU3D_CPPFLAGS = $(ALL_PKG_CPPFLAGS) -I./serial -I./parallel -I./duneinterface 

# build libALU3dGrid 
lib_LTLIBRARIES = libalu3dgrid.la

# headers to install 
nobase_include_HEADERS = alu3dgrid_serial.h alu3dgrid_parallel.h stlheaders.h $(serialhdrs) $(parallelhdrs) $(duneinterfacehdrs)


if MPI 
# if MPI was found the parallel version is build 

# add mpi path to cpplags 
AM_CPPFLAGS = $(ALU3D_CPPFLAGS) $(MPI_CPPFLAGS)
LDADD = $(ALL_PKG_LDFLAGS) $(ALL_PKG_LIBS)

# build lib with parallel support 
dist_libalu3dgrid_la_SOURCES = alu3dgrid_serial.cc alu3dgrid_parallel.cc 
libalu3dgrid_la_LIBADD = $(PARTY_LDFLAGS) $(PARTY_LIBS) $(METIS_LDFLAGS) $(METIS_LIBS) 

else 
# if no mpi build only serial lib 

LDADD = $(ALL_PKG_LDFLAGS) $(ALL_PKG_LIBS)
AM_CPPFLAGS = $(ALU3D_CPPFLAGS)
# build only serial grid lib
dist_libalu3dgrid_la_SOURCES = alu3dgrid_serial.cc 

endif
