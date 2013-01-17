SUBDIRS = serial parallel duneinterface 2d 

# build libALU3dGrid 
lib_LTLIBRARIES = libalugrid.la

# headers to install 
nobase_include_HEADERS = alugrid_serial.h alugrid_parallel.h alugrid_2d.h stlheaders.h indexstack.h projectvertex.h 

# add all package flags 
libalugrid_la_CPPFLAGS = $(ALL_PKG_CPPFLAGS)
libalugrid_la_LIBADD = $(PARMETIS_LDFLAGS) $(PARMETIS_LIBS) $(METIS_LDFLAGS) $(METIS_LIBS) $(MPI_LDFLAGS) $(MPI_LIBS)

if MPI 
# build lib with parallel support 
dist_libalugrid_la_SOURCES = alugrid_serial.cc alugrid_parallel.cc alugrid_2d.cc starmpi.c
else 
# build only serial grid lib
dist_libalugrid_la_SOURCES = alugrid_serial.cc alugrid_2d.cc
endif

include $(top_srcdir)/am/global-rules

