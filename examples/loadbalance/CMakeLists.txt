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

set(EXAMPLES main_internal
             main_weights 
             main_simple 
             main_zoltan)

add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")

configure_file(alugrid.cfg ${CMAKE_CURRENT_BINARY_DIR}/alugrid.cfg COPYONLY)


add_executable(main_internal main.cc)
dune_target_enable_all_packages( main_internal )
#dune_target_link_libraries(main_internal "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_internal APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL" )

add_executable(main_weights main.cc)
dune_target_enable_all_packages( main_weights )
#dune_target_link_libraries(main_weights "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_weights APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL;USE_WEIGHTS=1" )

add_executable(main_simple main.cc)
dune_target_enable_all_packages( main_simple )
#dune_target_link_libraries(main_simple "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_simple APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL;USE_SIMPLELB=1" )

add_executable(main_zoltan main.cc)
dune_target_enable_all_packages( main_zoltan )
#dune_target_link_libraries(main_zoltan "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_zoltan APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL;USE_ZOLTANLB=1" )

install(FILES ${exampleslb_HEADERS} DESTINATION ${exampleslbdir})


