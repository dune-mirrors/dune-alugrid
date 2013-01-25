# AM_CPPFLAGS = ${ALL_PKG_CPPFLAGS} ${DUNEMPICPPFLAGS} 
# LDADD = ${LDADD} ${ALL_PKG_LIBS} ${DUNEMPILDFLAGS} ${DUNEMPILIBS} -ldunealugrid -lalugrid_serial -lalugrid_parallel -L../lib

# headers to install 
check_PROGRAMS = aluball
aluball_SOURCES = aluball.cc
test_aluball_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
test_aluball_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
test_aluball_LDADD = $(LDADD) $(DUNEMPILIBS)

include $(top_srcdir)/am/global-rules

