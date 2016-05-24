set(EXTRA_PROGRAMS  test-compatibility)
set(compatibilitydir  ${CMAKE_INSTALL_INCLUDEDIR}/utils/bisection-compatibility)
set(compatibility_HEADERS  bisectioncompatibility.hh)
add_executable(test-compatibility test-compatibility.cc)
dune_target_enable_all_packages( test-compatibility )
# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${compatibility_HEADERS} DESTINATION ${compatibilitydir})
