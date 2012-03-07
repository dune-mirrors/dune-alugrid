# don't follow the full GNU-standard
AUTOMAKE_OPTIONS = foreign 1.5

# do everything in the subdirs
SUBDIRS = src misc 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = alugrid.pc

SOURCEFILE_MASK = -name \*.h -o -name \*.hh -o -name \*.c -o -name \*.cc

# add library name, authors list, and license to progam files
dist-hook:
	if test -f ./addauthors.sh ; then \
		find $(distdir) \( $(SOURCEFILE_MASK) \) -exec ./addauthors.sh ./AUTHORS {} \; ; \
	fi
