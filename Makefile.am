# don't follow the full GNU-standard
AUTOMAKE_OPTIONS = foreign 1.5

# do everything in the subdirs
SUBDIRS = src misc 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = alugrid.pc

# add library name, authors list, and license to progam files
dist-hook:
	find $(distdir) -name "*.h" -exec ./addauthors.sh ./AUTHORS {} \;
	find $(distdir) -name "*.hh" -exec ./addauthors.sh ./AUTHORS {} \;
	find $(distdir) -name "*.c" -exec ./addauthors.sh ./AUTHORS {} \;
	find $(distdir) -name "*.cc" -exec ./addauthors.sh ./AUTHORS {} \;
