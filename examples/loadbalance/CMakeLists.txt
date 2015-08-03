set(exampleslbdir  ${CMAKE_INSTALL_INCLUDEDIR}/examples/loadbalanacing)
set(exampleslb_HEADERS adaptation.hh  
                       diagnostics.hh  
                       loadbalance_simple.hh  
                       paralleldgf.hh        
                       problem-ball.hh   
                       problem-transport.hh
                       datamap.hh     
                       fvscheme.hh     
                       loadbalance_zoltan.hh 
                       piecewisefunction.hh  
                       problem-euler.hh  
                       problem.hh)

set(EXAMPLES main_transport_lb 
             main_ball_lb 
             main_euler_lb)

add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")

configure_file(alugrid.cfg ${CMAKE_CURRENT_BINARY_DIR}/alugrid.cfg COPYONLY)


add_executable(main_transport_lb main.cc)
add_dune_mpi_flags(main_transport_lb )
dune_target_link_libraries(main_transport_lb "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_transport_lb APPEND PROPERTY 
  COMPILE_DEFINITIONS "TRANSPORT" )

add_executable(main_ball_lb main.cc)
add_dune_mpi_flags(main_ball_lb )
dune_target_link_libraries(main_ball_lb "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_ball APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL" )

add_executable(main_euler_lb main.cc)
add_dune_mpi_flags(main_euler_lb )
dune_target_link_libraries(main_euler_lb "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_euler_lb APPEND PROPERTY 
  COMPILE_DEFINITIONS "EULER" )

install(FILES ${exampleslb_HEADERS} DESTINATION ${exampleslbdir})


