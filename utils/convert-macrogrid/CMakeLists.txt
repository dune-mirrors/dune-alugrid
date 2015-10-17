set(EXTRA_PROGRAMS  convert)
set(convertdir  ${CMAKE_INSTALL_INCLUDEDIR}/utils/convert-macrogrid)
set(convert_HEADERS  partition.hh)
add_executable(convert main.cc)
dune_target_enable_all_packages( convert )
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${convert_HEADERS} DESTINATION ${convertdir})
