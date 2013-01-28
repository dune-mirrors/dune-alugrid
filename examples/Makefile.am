SUBDIRS = fv 

check_PROGRAMS = aluball
aluball_SOURCES = aluball.cc
aluball_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
aluball_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
aluball_LDADD = $(LDADD) $(DUNEMPILIBS)

include $(top_srcdir)/am/global-rules

