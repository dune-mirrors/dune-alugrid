check_PROGRAMS = sizes

sizes_SOURCES = sizes.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/gitter_geo.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/gitter_hexa_top.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/gitter_impl.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/gitter_mgb.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/gitter_sti.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/gitter_tetra_top.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/mapp_cube_3d.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/mapp_tetra_3d.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/myalloc.cc \
  $(top_srcdir)/dune/alugrid/impl/duneinterface/gitter_dune_impl.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/ghost_info.cc \
  $(top_srcdir)/dune/alugrid/impl/serial/ghost_elements.cc \
  $(top_srcdir)/dune/alugrid/impl/duneinterface/gitter_dune_pll_impl.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/mpAccess.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/gitter_hexa_top_pll.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_impl.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_idn.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_ldb.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_mgb.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/gitter_pll_sti.cc \
  $(top_srcdir)/dune/alugrid/impl/parallel/gitter_tetra_top_pll.cc

sizes_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) -Wfatal-errors -DALUGRIDDEBUG
sizes_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)
sizes_LDADD = ${ZOLTAN_LIBS} ${PARMETIS_LIBS} ${METIS_LIB} $(DUNEMPILIBS)

include $(top_srcdir)/am/global-rules

CXXFLAGS=-g 
