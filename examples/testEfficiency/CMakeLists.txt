set(testefficiencydir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/testEfficiency)
set(testefficiency_HEADERS adaptation.hh  
                           diagnostics.hh  
                           paralleldgf.hh        
                           problem-ball.hh   
                           problem-transport.hh
                           datamap.hh     
                           fvscheme.hh     
                           piecewisefunction.hh  
                           problem-euler.hh  
                           problem.hh)

add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")
add_definitions("-DWORLDDIM=3")

configure_file(check-efficiency.sh ${CMAKE_CURRENT_BINARY_DIR}/check-efficiency.sh COPYONLY)

foreach(variant transport ball euler)
  string(TOUPPER ${variant} variant_upper)
  add_executable(main_${variant}_eff main.cc)
  dune_target_enable_all_packages(main_${variant}_eff)
  target_compile_definitions(main_${variant}_eff PRIVATE "${variant_upper}")
  set_property(TARGET main_${variant}_eff APPEND PROPERTY LINK_LIBRARIES dunealugrid)
endforeach()

install(FILES ${examples_HEADERS} DESTINATION ${examplesdir})
