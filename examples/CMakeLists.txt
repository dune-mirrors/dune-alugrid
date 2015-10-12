set(SUBDIRS callback dgf loadbalance backuprestore communication testEfficiency)

set(examplesdir  ${CMAKE_INSTALL_INCLUDEDIR}/examples)
set(examples_HEADERS adaptation.hh  
                     diagnostics.hh  
                     paralleldgf.hh        
                     problem-ball.hh   
                     problem-transport.hh
                     datamap.hh     
                     fvscheme.hh     
                     piecewisefunction.hh  
                     problem-euler.hh  
                     problem.hh)

configure_file(alugrid.cfg ${CMAKE_CURRENT_BINARY_DIR}/alugrid.cfg COPYONLY)

foreach( variant ball euler transport )
  string( TOUPPER ${variant} variant_upper )
  set( prog "main_${variant}" )
  add_executable( ${prog} main.cc )
  dune_target_enable_all_packages( ${prog} )
  target_compile_definitions( ${prog} PRIVATE "${variant_upper}" )
  target_compile_definitions( ${prog} PUBLIC "ALUGRID_CUBE" "GRIDDIM=3" "WORLDDIM=3" )
  # dune_target_link_libraries( ${prog} "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}" )

  foreach( type cube simplex )
    string( TOUPPER ${type} type_upper )
    foreach( dim RANGE 2 3 )
      set( prog "main_${variant}_${type}_${dim}d" )
      add_executable( ${prog} EXCLUDE_FROM_ALL main.cc )
      target_compile_definitions( ${prog} PRIVATE "${variant_upper}" "ALUGRID_${type_upper}" "GRIDDIM=${dim}" "WORLDDIM=${dim}" )
      dune_target_enable_all_packages( ${prog} )
      # dune_target_link_libraries( ${prog} "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}" )
    endforeach()
  endforeach()
endforeach()

install(FILES ${examples_HEADERS} DESTINATION ${examplesdir})

foreach(i ${SUBDIRS})
  if(${i} STREQUAL "test")
    set(opt EXCLUDE_FROM_ALL)
  endif(${i} STREQUAL "test")
  add_subdirectory(${i} ${opt})
  unset(opt)
endforeach(i ${SUBDIRS})
