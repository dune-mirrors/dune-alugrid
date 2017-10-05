set(qualitydir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/quality)

configure_file(alugrid.cfg ${CMAKE_CURRENT_BINARY_DIR}/alugrid.cfg COPYONLY)

foreach(variant quality)
  string(TOUPPER ${variant} variant_upper)
  add_executable(main_${variant} main.cc )
  dune_target_enable_all_packages(main_${variant})
  set_property(TARGET main_${variant} APPEND PROPERTY LINK_LIBRARIES dunealugrid)
  target_compile_definitions(main_${variant} PRIVATE ${variant_upper})
  target_compile_definitions(main_${variant} PRIVATE "ALUGRID_COUNT_GLOBALCOMM")
  target_compile_definitions(main_${variant} PUBLIC "ALUGRID_CONFORM" "GRIDDIM=3" "WORLDDIM=3")
endforeach()

add_executable(gmsh2dgf gmsh2dgf.cc )
dune_target_enable_all_packages(gmsh2dgf)
set_property(TARGET gmsh2dgf APPEND PROPERTY LINK_LIBRARIES dunealugrid)
target_compile_definitions(gmsh2dgf PUBLIC "ALUGRID_SIMPLEX" "GRIDDIM=3" "WORLDDIM=3")

add_executable(estclo estimate-closure.cc)
dune_target_enable_all_packages(estclo)
set_property(TARGET estclo APPEND PROPERTY LINK_LIBRARIES dunealugrid)
target_compile_definitions(estclo PUBLIC "ALUGRID_CONFORM" "GRIDDIM=3" "WORLDDIM=3")

install(FILES ${examples_HEADERS} DESTINATION ${examplesdir})
