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

set(EXAMPLES main_transport_comm 
             main_ball_comm 
             main_euler_comm)

add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")
add_definitions("-DWORLDDIM=3")

configure_file(check-communication.sh ${CMAKE_CURRENT_BINARY_DIR}/check-communication.sh COPYONLY)

add_executable(main_transport_comm main.cc)
dune_target_enable_all_packages(main_transport_comm )
#dune_target_link_libraries(main_transport_comm "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_transport_comm APPEND PROPERTY 
  COMPILE_DEFINITIONS "TRANSPORT;NON_BLOCKING" )

add_executable(main_ball_comm main.cc)
dune_target_enable_all_packages(main_ball_comm )
#dune_target_link_libraries(main_ball_comm "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_ball_comm APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL;NON_BLOCKING" )

add_executable(main_euler_comm main.cc)
dune_target_enable_all_packages(main_euler_comm )
#dune_target_link_libraries(main_euler_comm "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_euler_comm APPEND PROPERTY 
  COMPILE_DEFINITIONS "EULER;NON_BLOCKING" )

install(FILES ${communication_HEADERS} DESTINATION ${communicationdir})
