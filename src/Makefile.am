# Makefile.am for building the ALU3d-Grid library

# here the headers to install are defined 
include serial/Makefile.am.include
include parallel/Makefile.am.include
include duneinterface/Makefile.am.include

# use the subdirectories as include dir 
LDADD = $(ALL_PKG_LDFLAGS) $(ALL_PKG_LIBS)
AM_CPPFLAGS = $(ALL_PKG_CPPFLAGS) -I./serial -I./parallel -I./duneinterface 

# build libALU3dGrid 
lib_LTLIBRARIES = libalu3dgrid.la

# headers to install 
nobase_include_HEADERS = alu3dgrid_serial.h alu3dgrid_parallel.h stlheaders.h $(serialhdrs) $(parallelhdrs) $(duneinterfacehdrs)

if MPI 
# if MPI was found the parallel version is build 

# add mpi path to cpplags 
AM_CPPFLAGS += $(MPI_CPPFLAGS)
# build lib with parallel support 
nodist_libalu3dgrid_la_SOURCES = alu3dgrid_serial.cc alu3dgrid_parallel.cc 

else 

# build only serial grid lib
nodist_libalu3dgrid_la_SOURCES = alu3dgrid_serial.cc 
endif
