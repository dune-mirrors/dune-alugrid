add_subdirectory(backuprestore)
add_subdirectory(callback)
add_subdirectory(communication)
add_subdirectory(dgf)
add_subdirectory(loadbalance)
add_subdirectory(quality)
add_subdirectory(testEfficiency)

set(DEFAULT_GRID_TYPE ALUGRID_CUBE)

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

# enable all flags and link against alugrid library
link_libraries(Dune::ALUGrid)
dune_enable_all_packages()

foreach(variant ball euler transport)
  string(TOUPPER ${variant} variant_upper)
  add_executable(main_${variant} main.cc )
  target_compile_definitions(main_${variant} PRIVATE ${variant_upper})
  if("${variant}" STREQUAL "ball")
    target_compile_definitions(main_${variant} PRIVATE "ALUGRID_COUNT_GLOBALCOMM")
  endif()
  target_compile_definitions(main_${variant} PUBLIC "${DEFAULT_GRID_TYPE}" "GRIDDIM=2" "WORLDDIM=2" "USE_ALUGRID_SFC_ORDERING")

  foreach(type cube simplex conform)
    string(TOUPPER ${type} type_upper)
    foreach(dim RANGE 2 3)
      add_executable(main_${variant}_${type}_${dim}d EXCLUDE_FROM_ALL main.cc)
      target_compile_definitions(main_${variant}_${type}_${dim}d PRIVATE "${variant_upper}" "ALUGRID_${type_upper}" "GRIDDIM=${dim}" "WORLDDIM=${dim}")
    endforeach()
  endforeach()
endforeach()

install(FILES ${examples_HEADERS} DESTINATION ${examplesdir})
