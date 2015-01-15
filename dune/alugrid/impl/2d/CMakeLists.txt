set(impl2ddir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/impl/2d)
set(impl2d_HEADERS  grid.h handle.h listwalk.h triang.h vmmap.h vtx_btree.h vtxprojection.h)
set(EXTRA_DIST  grid.cc mknbrs.cc vmmap.cc hdlrw.h handle.cc triang.cc vtx_btree.cc grid_imp.cc)
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${impl2d_HEADERS} DESTINATION ${impl2ddir})

dune_add_library(alugrid2d OBJECT
        grid.cc
        handle.cc
        hdlrw.h
        mknbrs.cc
        triang.cc
        vmmap.cc
        vtx_btree.cc)