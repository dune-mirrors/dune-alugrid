EXTRA_DIST = gitter_geo.cc  gitter_impl.cc  gitter_mgb.cc  gitter_sti.cc  mapp_cube_3d.cc\
mapp_tetra_3d.cc  myalloc.cc  ghost_elements.cc ghost_info.cc \
gitter_hexa_top.cc gitter_tetra_top.cc

serialdirec = .
serialhdrs = $(serialdirec)/gitter_tetra_top.h $(serialdirec)/mapp_cube_3d.h $(serialdirec)/serialize.h\
	$(serialdirec)/gitter_hexa_top.h $(serialdirec)/headers.h $(serialdirec)/mapp_tetra_3d_ext.h $(serialdirec)/walk.h\
	$(serialdirec)/gitter_impl.h $(serialdirec)/mapp_tetra_3d.h \
	$(serialdirec)/gitter_mgb.h $(serialdirec)/key.h	$(serialdirec)/myalloc.h\
	$(serialdirec)/gitter_sti.h $(serialdirec)/lock.h	$(serialdirec)/parallel.h \
  $(serialdirec)/refinementrules.h \
  $(serialdirec)/ghost_elements.h $(serialdirec)/ghost_info.h 

nobase_include_HEADERS = $(serialhdrs)

include $(top_srcdir)/am/global-rules
