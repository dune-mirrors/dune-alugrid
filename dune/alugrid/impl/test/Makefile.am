EXTRA_PROGRAMS = adapt backuprestore sizes

ALUSOURCES = \
  $(top_srcdir)/dune/alugrid/impl/binaryio.cc \
  $(top_srcdir)/dune/alugrid/impl/macrofileheader.cc \
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

sizes_SOURCES = sizes.cc $(ALUSOURCES) 
adapt_SOURCES = adapt.cc $(ALUSOURCES) 
backuprestore_SOURCES = backuprestore.cc $(ALUSOURCES) 

include $(top_srcdir)/am/global-rules

CXXFLAGS=-g 

CLEANFILES=file.*
