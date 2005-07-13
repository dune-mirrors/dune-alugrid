#!gmake
########################################################################
#   Sample ALBERT Makefile                                             #
########################################################################

#   uncomment line, if ALBERT_LIB_PATH isn't an environment variable

#   uncomment line, for 
CC = gcc
ALBERTA_PATH=../../alberta-1.2-SuSE9.2

ALBERT_LIB_PATH = $(ALBERTA_PATH)/lib
ALBERT_INCLUDE_PATH = $(ALBERTA_PATH)/include

DEFAULT = al2bsgrid 
DEBUG = 0  # may be 0 or 1

include Makefile.al

CFLAGS = -g -Wall -I$(ALBERT_INCLUDE_PATH)
OFLAG = -O2

########################################################################
#   DFLAGS: DIM, DIM_OF_WORLD, NEIGH_IN_EL, EL_INDEX                   #
########################################################################

DIM = 3
DIM_OF_WORLD = 3
EL_INDEX = 0
NEIGH_IN_EL = 0
XDR=1

########################################################################
#   and now the user's files                                           #
########################################################################

Albert2BSGrid = albert2BSGrid.o
al2bsgrid: $(Albert2BSGrid)
	$(LD) $(MOFLAG) -o $@ $(Albert2BSGrid) $(LIBS)


albert2BSGrid.o : albert2BSGrid.c trans2BSMacro.c 


clean:
			-rm *.o al2bsgrid
												
