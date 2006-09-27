# Makefile.am for building the example program 
AUTOMAKE_OPTIONS = foreign 1.5

METIS_DIR=$(HOME)/DuneAdds/modules/metis-4.0
# work arround for automake versions < 1.8 
SOURCE_DIR=../../src
AM_CPPFLAGS = -I$(METIS_DIR)/Lib -I$(SOURCE_DIR) -I$(SOURCE_DIR)/serial -I$(SOURCE_DIR)/parallel -I$(SOURCE_DIR)/duneinterface 
LDADD = $(METIS_DIR)/libmetis.a

# headers to install 
noinst_PROGRAMS = example
example_SOURCES = example.cc $(SOURCE_DIR)/alugrid_serial.cc $(SOURCE_DIR)/alugrid_parallel.cc
