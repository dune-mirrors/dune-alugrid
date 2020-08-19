add_subdirectory(backuprestore)
add_subdirectory(callback)
add_subdirectory(communication)
add_subdirectory(dgf)
add_subdirectory(loadbalance)
add_subdirectory(quality)
add_subdirectory(testEfficiency)

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

foreach(variant ball euler transport)
  string(TOUPPER ${variant} variant_upper)
  add_executable(main_${variant} main.cc )
  dune_target_enable_all_packages(main_${variant})
  set_property(TARGET main_${variant} APPEND PROPERTY LINK_LIBRARIES dunealugrid)
  target_compile_definitions(main_${variant} PRIVATE ${variant_upper})
  if("${variant}" STREQUAL "ball")
    target_compile_definitions(main_${variant} PRIVATE "ALUGRID_COUNT_GLOBALCOMM")
  endif()
  target_compile_definitions(main_${variant} PUBLIC "ALUGRID_CUBE" "GRIDDIM=2" "WORLDDIM=2")

  foreach(type cube simplex conform)
    string(TOUPPER ${type} type_upper)
    foreach(dim RANGE 2 3)
      add_executable(main_${variant}_${type}_${dim}d EXCLUDE_FROM_ALL main.cc)
      target_compile_definitions(main_${variant}_${type}_${dim}d PRIVATE "${variant_upper}" "ALUGRID_${type_upper}" "GRIDDIM=${dim}" "WORLDDIM=${dim}")
      dune_target_enable_all_packages(main_${variant}_${type}_${dim}d)
      set_property(TARGET main_${variant}_${type}_${dim}d APPEND PROPERTY LINK_LIBRARIES dunealugrid)
    endforeach()
  endforeach()
endforeach()

install(FILES ${examples_HEADERS} DESTINATION ${examplesdir})
