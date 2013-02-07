EXTRA_DIST = dune.module

SUBDIRS = m4 lib dune examples

DISTCHECK_CONFIGURE_FLAGS = --with-dune-common=$(DUNE_COMMON_ROOT) --dune-geometry=$(DUNE_GEOMETRY_ROOT) --with-dune-grid=$(DUNE_GRID_ROOT) CXX="$(CXX)" CC="$(CC)"

SOURCEFILE_MASK = -name \*.h -o -name \*.hh -o -name \*.c -o -name \*.cc

# add library name, authors list, and license to source files
dist-hook:
	@for SOURCEFILE in `find $(distdir) \( $(SOURCEFILE_MASK) \) -print` ; do \
	  TMPFILE=`mktemp authors.XXXXXX` ; \
	  echo "/******************************************************************************" > $$TMPFILE ; \
	  echo "" >> $$TMPFILE ; \
	  cat $(top_srcdir)/AUTHORS >> $$TMPFILE ; \
	  echo "" >> $$TMPFILE ; \
	  echo "******************************************************************************/" >> $$TMPFILE ; \
	  echo "" >> $$TMPFILE ; \
	  cat $$SOURCEFILE >> $$TMPFILE ; \
	  cp $$TMPFILE $$SOURCEFILE ; \
	  rm -f $$TMPFILE ; \
	done

include $(top_srcdir)/am/top-rules
include $(top_srcdir)/am/global-rules
dune/alugrid/impl/2d/grid.h
dune/alugrid/impl/2d/handle.h
dune/alugrid/impl/2d/listwalk.h
dune/alugrid/impl/2d/triang.h
dune/alugrid/impl/2d/vmmap.h
dune/alugrid/impl/2d/vtx_btree.h
dune/alugrid/impl/2d/vtxprojection.h
