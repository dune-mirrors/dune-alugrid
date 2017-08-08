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
  if( "${variant}" STREQUAL "ball" )
    target_compile_definitions( ${prog} PRIVATE "${variant_upper};ALUGRID_COUNT_GLOBALCOMM" )
  else()
    target_compile_definitions( ${prog} PRIVATE "${variant_upper}" )
  endif()
  target_compile_definitions( ${prog} PUBLIC "ALUGRID_CONFORM" "GRIDDIM=3" "WORLDDIM=3" )

  foreach( type cube simplex conform )
    string( TOUPPER ${type} type_upper )
    foreach( dim RANGE 2 3 )
      set( prog "main_${variant}_${type}_${dim}d" )
      add_executable( ${prog} EXCLUDE_FROM_ALL main.cc )
      target_compile_definitions( ${prog} PRIVATE "${variant_upper}" "ALUGRID_${type_upper}" "GRIDDIM=${dim}" "WORLDDIM=${dim}" )
      dune_target_enable_all_packages( ${prog} )
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
