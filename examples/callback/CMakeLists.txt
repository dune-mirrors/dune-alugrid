set(GRIDTYPE  ALUGRID_CUBE)
set(GRIDDIM  3)
set(WORLDDIM  3)

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

set(EXAMPLES main_transport_cb 
             main_ball_cb 
             main_euler_cb)



add_definitions("-D${GRIDTYPE}")
add_definitions("-DGRIDDIM=${GRIDDIM}")
add_definitions("-DWORLDDIM=${WORLDDIM}")

configure_file(check-adaptation.sh ${CMAKE_CURRENT_BINARY_DIR}/check-adaptation.sh COPYONLY)


add_executable(main_transport_cb main.cc)
dune_target_enable_all_packages(main_transport_cb )
#dune_target_link_libraries(main_transport_cb "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_transport_cb APPEND PROPERTY 
  COMPILE_DEFINITIONS "TRANSPORT" )

add_executable(main_ball_cb main.cc)
dune_target_enable_all_packages(main_ball_cb )
#dune_target_link_libraries(main_ball_cb "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_ball_cb APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL" )

add_executable(main_euler_cb main.cc)
dune_target_enable_all_packages(main_euler_cb )
#dune_target_link_libraries(main_euler_cb "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_euler_cb APPEND PROPERTY 
  COMPILE_DEFINITIONS "EULER" )

install(FILES ${callback_HEADERS} DESTINATION ${callbackdir})
