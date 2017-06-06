set(examplesbrdir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/backuprestore)
set(examplesbr_HEADERS sionlib.hh 
                       paralleldgf.hh)

add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")
add_definitions("-DWORLDDIM=3")

if( NOT HAVE_ALUGRID )
  add_executable(main_br main.cc)
  #dune_target_link_libraries(main_br "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
  dune_target_enable_all_packages( main_br )
endif( NOT HAVE_ALUGRID  )

install(FILES ${examplesbr_HEADERS} DESTINATION ${examplesbrdir})
