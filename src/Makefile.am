LDADD = $(ALL_PKG_LDFLAGS) $(ALL_PKG_LIBS)
AM_CPPFLAGS = $(ALL_PKG_CPPFLAGS) -I$(DUNEADDS)/modules/mpich-1.2.6_SuSE9.2gcc/include
AM_LDFLAGS = -static 

lib_LTLIBRARIES = libALU3dGrid.la
#if HAVE_MPI
AM_CPPFLAGS += -I./serial -I./parallel
#nodist_libALU3dGrid_la_SOURCES = alu3dgrid_parallel.cc 
#else 
nodist_libALU3dGrid_la_SOURCES = alu3dgrid_serial.cc 
#endif

