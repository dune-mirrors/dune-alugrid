# Makefile.am for building the example program 
AUTOMAKE_OPTIONS = foreign 1.5

CXXFLAGS=-O3 -funroll-loops -finline-functions -DNDEBUG 

# work arround for automake versions < 1.8 
SOURCE_DIR=../../src
AM_CPPFLAGS = -I$(SOURCE_DIR) -I$(SOURCE_DIR)/serial -I$(SOURCE_DIR)/parallel -I$(SOURCE_DIR)/duneinterface 

# headers to install 
noinst_PROGRAMS = example sizes adapt
example_SOURCES = example.cc $(SOURCE_DIR)/alugrid_serial.cc 
sizes_SOURCES = sizes.cc $(SOURCE_DIR)/alugrid_serial.cc 
adapt_SOURCES = adapt.cc $(SOURCE_DIR)/alugrid_2d.cc 
#sizes_SOURCES = sizes.cc $(SOURCE_DIR)/alugrid_serial.cc $(SOURCE_DIR)/alugrid_parallel.cc 
