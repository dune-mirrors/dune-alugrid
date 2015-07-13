set(M4FILES  dune-alugrid.m4 alugridtype.m4 dlmalloc.m4 sionlib.m4 zlib.m4 zoltan.m4)
set(aclocaldir  ${CMAKE_INSTALL_DATADIR}/aclocal)
set(aclocal_DATA  ${M4FILES})
set(EXTRA_DIST  ${M4FILES})
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${aclocal_DATA} DESTINATION ${aclocaldir})