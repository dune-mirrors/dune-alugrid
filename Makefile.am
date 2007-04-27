# don't follow the full GNU-standard
AUTOMAKE_OPTIONS = foreign 1.5

# do everything in the subdirs
SUBDIRS = src misc 

# add library name, authors list, and licence to progam files
dist-hook:
	find $(distdir) -name "*.h*" -exec ./addauthors.sh ./AUTHORS {} \;
	find $(distdir) -name "*.c*" -exec ./addauthors.sh ./AUTHORS {} \;
