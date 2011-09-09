# Makefile.am for building the example program 
AUTOMAKE_OPTIONS = foreign 1.5

#CXXFLAGS=-O3 -funroll-loops -finline-functions -DNDEBUG 
CXXFLAGS=-g -Wall -Wfatal-errors
#CXXFLAGS=-g -Wall 
CXX=mpiCC

#MPI_LIBS=-pthread -L/usr/lib64 -lmpi_cxx -lmpi -lopen-rte -lopen-pal -ldl -Wl,--export-dynamic -lnsl -lutil -lm -ldl

# work arround for automake versions < 1.8 
SOURCE_DIR=../../src
AM_CPPFLAGS = $(ALL_PKG_CPPFLAGS) $(MPI_CPPFLAGS)  -I$(SOURCE_DIR) -I$(SOURCE_DIR)/serial -I$(SOURCE_DIR)/parallel -I$(SOURCE_DIR)/duneinterface 
LDADD = $(PARTY_LDFLAGS) $(PARTY_LIBS) $(PARMETIS_LDFLAGS) $(PARMETIS_LIBS) $(METIS_LDFLAGS) $(METIS_LIBS) 

# headers to install 
#noinst_PROGRAMS = loadlb sizes triang adapt # example sizes # adapt
noinst_PROGRAMS = sizes loadlb # example sizes # adapt
#example_SOURCES = example.cc $(SOURCE_DIR)/alugrid_serial.cc 
sizes_SOURCES = sizes.cc $(SOURCE_DIR)/alugrid_serial.cc $(SOURCE_DIR)/alugrid_parallel.cc
triang_SOURCES = triang.cc $(SOURCE_DIR)/alugrid_2d.cc 
#sizes_SOURCES = sizes.cc $(SOURCE_DIR)/alugrid_serial.cc
adapt_SOURCES = adapt.cc $(SOURCE_DIR)/alugrid_2d.cc $(SOURCE_DIR)/alugrid_serial.cc
loadlb_SOURCES = loadlb.cc  $(SOURCE_DIR)/alugrid_serial.cc $(SOURCE_DIR)/alugrid_parallel.cc
loadlb_LDADD = $(PARTY_LDFLAGS) $(PARTY_LIBS) $(PARMETIS_LDFLAGS) $(PARMETIS_LIBS) $(METIS_LDFLAGS) $(METIS_LIBS) 

#sizes_SOURCES = sizes.cc $(SOURCE_DIR)/alugrid_serial.cc $(SOURCE_DIR)/alugrid_parallel.cc 

EXTRA_DIST=square.triangle 
