set(communicationdir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/communication)
set(communication_HEADERS adaptation.hh  
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

configure_file(check-communication.sh ${CMAKE_CURRENT_BINARY_DIR}/check-communication.sh COPYONLY)

foreach(variant transport ball euler)
  string(TOUPPER ${variant} variant_upper)
  add_executable(main_${variant}_comm main.cc)
  dune_target_enable_all_packages(main_${variant}_comm)
  target_compile_definitions(main_${variant}_comm PRIVATE "${variant_upper}" "NON_BLOCKING")
  set_property(TARGET main_${variant}_comm APPEND PROPERTY LINK_LIBRARIES dunealugrid)
endforeach()

install(FILES ${communication_HEADERS} DESTINATION ${communicationdir})
