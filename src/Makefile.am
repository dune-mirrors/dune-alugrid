
# here the headers to install are defined 
include serial/Makefile.am.include
include parallel/Makefile.am.include
include duneinterface/Makefile.am.include

# use the subdirectories as include dir 
LDADD = $(ALL_PKG_LDFLAGS) $(ALL_PKG_LIBS)
AM_CPPFLAGS = $(ALL_PKG_CPPFLAGS) -I./serial -I./parallel -I./duneinterface -I$(DUNEADDS)/modules/mpich-1.2.6_SuSE9.2gcc/include
#AM_LDFLAGS = -static 

# build libALU3dGrid 
lib_LTLIBRARIES = libALU3dGrid.la

# headers to install 
nobase_include_HEADERS = alu3dgrid_serial.h alu3dgrid_parallel.h stlheaders.h $(serialhdrs) $(parallelhdrs) $(duneinterfacehdrs)

if MPI
# build lib with parallel support 
nodist_libALU3dGrid_la_SOURCES = alu3dgrid_parallel.cc 
else 
# build only serial grid lib
nodist_libALU3dGrid_la_SOURCES = alu3dgrid_serial.cc 
endif
