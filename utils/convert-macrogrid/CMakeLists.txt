set(convertdir  ${CMAKE_INSTALL_INCLUDEDIR}/utils/convert-macrogrid)
set(convert_HEADERS  partition.hh)

add_executable(convert main.cc)
dune_target_enable_all_packages( convert )

install(FILES ${convert_HEADERS} DESTINATION ${convertdir})
