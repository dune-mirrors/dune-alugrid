set(GRIDTYPE  ALUGRID_CUBE)
set(GRIDDIM  3)
set(WORLDDIM  3)

set(examplesbrdir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/backuprestore)
set(examplesbr_HEADERS sionlib.hh 
                       paralleldgf.hh)

set(EXAMPLES main_br )



add_definitions("-D${GRIDTYPE}")
add_definitions("-DGRIDDIM=${GRIDDIM}")
add_definitions("-DWORLDDIM=${WORLDDIM}")

if( NOT HAVE_ALUGRID )
  add_executable(main_br main.cc)
  #dune_target_link_libraries(main_br "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
  dune_target_enable_all_packages( main_br )
endif( NOT HAVE_ALUGRID  )

install(FILES ${examplesbr_HEADERS} DESTINATION ${examplesbrdir})
