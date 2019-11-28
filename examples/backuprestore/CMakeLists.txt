set(examplesbrdir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/backuprestore)
set(examplesbr_HEADERS sionlib.hh
                       paralleldgf.hh)

add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")
add_definitions("-DWORLDDIM=3")

add_executable(main_br main.cc)
dune_target_enable_all_packages(main_br)
set_property(TARGET main_br APPEND PROPERTY LINK_LIBRARIES dunealugrid)

install(FILES ${examplesbr_HEADERS} DESTINATION ${examplesbrdir})
