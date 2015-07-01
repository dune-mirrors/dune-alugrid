set(SUBDIRS callback loadbalance backuprestore communication testEfficiency)

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

set(EXAMPLES main_transport 
             main_ball 
             main_euler)

# set default grid type and dimensions
add_definitions("-DALUGRID_CUBE")
add_definitions("-DGRIDDIM=3")
add_definitions("-DWORLDDIM=3")

configure_file(alugrid.cfg ${CMAKE_CURRENT_BINARY_DIR}/alugrid.cfg COPYONLY)


add_executable(main_transport main.cc)
add_dune_mpi_flags(main_transport )
dune_target_link_libraries(main_transport "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_transport APPEND PROPERTY 
  COMPILE_DEFINITIONS "TRANSPORT" )

add_executable(main_ball main.cc)
add_dune_mpi_flags(main_ball )
dune_target_link_libraries(main_ball "${DUNE_LIBS};${LIBRARIES};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_ball APPEND PROPERTY 
  COMPILE_DEFINITIONS "BALL" )

add_executable(main_euler main.cc)
add_dune_mpi_flags(main_euler )
dune_target_link_libraries(main_euler "${DUNE_LIBS};${DUNE_ALUGRID_LIBRARY}")
set_property(TARGET main_euler APPEND PROPERTY 
  COMPILE_DEFINITIONS "EULER" )

install(FILES ${examples_HEADERS} DESTINATION ${examplesdir})
foreach(i ${SUBDIRS})
  if(${i} STREQUAL "test")
    set(opt EXCLUDE_FROM_ALL)
  endif(${i} STREQUAL "test")
  add_subdirectory(${i} ${opt})
  unset(opt)
endforeach(i ${SUBDIRS})
