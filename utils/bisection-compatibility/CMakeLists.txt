set(compatibilitydir  ${CMAKE_INSTALL_INCLUDEDIR}/utils/bisection-compatibility)
set(compatibility_HEADERS  bisectioncompatibility.hh)

# set(EXTRA_PROGRAMS  test-compatibility estimate-closure )
# add_executable(test-compatibility test-compatibility.cc EXCLUDE_FROM_ALL)
# add_executable(estimate-closure estimate-closure.cc EXCLUDE_FROM_ALL)
# dune_target_enable_all_packages( test-compatibility estimate-closure )

# include not needed for CMake
# include $(top_srcdir)/am/global-rules
install(FILES ${compatibility_HEADERS} DESTINATION ${compatibilitydir})
