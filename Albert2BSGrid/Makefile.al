########################################################################
#   master makefile, included by application Makefiles                 #
#   automatically generated during ALBERT installation                 #
#   by:       kunibert                                                 #
#   on host:  serv1.mathepool.uni-freiburg.de                          #
#   date:     Thu May 11 12:58:42 2000                                 #
########################################################################

########################################################################
#   C and FORTRAN compilers and loader                                 #
########################################################################

CC = cc
FC = f77
LD = cc

########################################################################
#   default settings                                                   #
########################################################################

MOFLAG = 
#CFLAGS = 
OFLAG = -O2
DEBUG = 0
XDR=1


########################################################################
#   define flags and compile commands                                  #
########################################################################

DFLAGS = -DXDR=$(XDR) -DDIM=$(DIM) -DDIM_OF_WORLD=$(DIM_OF_WORLD) -DNEIGH_IN_EL=$(NEIGH_IN_EL) -DEL_INDEX=$(EL_INDEX)
.c.o:
	$(CC) -c $(MOFLAG) $(OFLAG) $(CFLAGS) $(DFLAGS) -I$(ALBERT_LIB_PATH) $<
.f.o:
	$(FC) -c $(MOFLAG) $(FFLAGS) $(OFLAG) $<

########################################################################
#   the ALBERT library and libraries used by ALBERT                    #
########################################################################

ALBERT_LIB = libALBERTA$(DIM)$(DIM_OF_WORLD)_0.a
LIBS = $(ALBERT_LIB_PATH)/$(ALBERT_LIB) $(ALBERT_LIB_PATH)/libalberta_util.a -lm

########################################################################
#   some often used targets                                            #
########################################################################

default: $(DEFAULT)

new: cleano default

########################################################################
#   end of master makefile                                             #
########################################################################
