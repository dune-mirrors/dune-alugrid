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
set(EXAMPLES main_transport_eff
             main_ball_eff 
             main_euler_eff)

add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")
add_definitions("-DWORLDDIM=3")

configure_file(check-efficiency.sh ${CMAKE_CURRENT_BINARY_DIR}/check-efficiency.sh COPYONLY)

add_executable(main_transport_eff main.cc)
dune_target_enable_all_packages(main_transport_eff )
# dune_target_link_libraries(main_transport_eff "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_transport_eff APPEND PROPERTY 
  COMPILE_DEFINITIONS "TRANSPORT" )

add_executable(main_ball_eff main.cc)
dune_target_enable_all_packages(main_ball_eff )
# dune_target_link_libraries(main_ball_eff "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_ball_eff APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL" )

add_executable(main_euler_eff main.cc)
dune_target_enable_all_packages(main_euler_eff )
# dune_target_link_libraries(main_euler_eff "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_euler_eff APPEND PROPERTY 
  COMPILE_DEFINITIONS "EULER" )

install(FILES ${examples_HEADERS} DESTINATION ${examplesdir})
