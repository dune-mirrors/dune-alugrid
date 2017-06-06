set(callbackdir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/callback)
set(callback_HEADERS adaptation.hh  
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

#add_definitions("-DUSE_VECTOR_FOR_PWF")

configure_file(check-adaptation.sh ${CMAKE_CURRENT_BINARY_DIR}/check-adaptation.sh COPYONLY)

foreach(variant transport ball euler)
  string(TOUPPER ${variant} variant_upper)
  add_executable(main_${variant}_cb main.cc)
  dune_target_enable_all_packages(main_${variant}_cb)
  target_compile_definitions(main_${variant}_cb PRIVATE "${variant_upper}" "CALLBACK_ADAPTATION")
  set_property(TARGET main_${variant}_cb APPEND PROPERTY LINK_LIBRARIES dunealugrid)
endforeach()

install(FILES ${callback_HEADERS} DESTINATION ${callbackdir})
