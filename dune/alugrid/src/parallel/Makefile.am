EXTRA_DIST = gitter_pll_idn.cc   gitter_pll_ldb.cc  gitter_pll_sti.cc\
mpAccess_MPI.cc  mpAccess_STAR.cc  gitter_pll_impl.cc  gitter_pll_mgb.cc  mpAccess.cc \
gitter_hexa_top_pll.cc gitter_tetra_top_pll.cc

paralleldir = .
parallelhdrs = $(paralleldir)/gitter_pll_ldb.h $(paralleldir)/gitter_tetra_top_pll.h $(paralleldir)/mpAccess_MPI.h\
        $(paralleldir)/mpAccess_STAR.h $(paralleldir)/gitter_hexa_top_pll.h $(paralleldir)/gitter_pll_mgb.h \
        $(paralleldir)/alusfc.hh $(paralleldir)/alumetis.hh $(paralleldir)/aluparmetis.hh $(paralleldir)/gitter_pll_impl.h $(paralleldir)/gitter_pll_sti.h  $(paralleldir)/mpAccess.h

nobase_include_HEADERS = $(parallelhdrs)


include $(top_srcdir)/am/global-rules
