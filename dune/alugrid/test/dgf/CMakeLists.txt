set(DGFS
  2-3-testgrid.dgf
  ball.dgf
  cube-testgrid-2-2.dgf
  cube-testgrid-2-2-inconsistent.dgf
  cube-testgrid-2-3.dgf
  cube-testgrid-3-3-inconsistent.dgf
  grid2d_str1d.dgf
  periodic-2d.dgf
  periodic-3d.dgf
  periodic3.dgf
  periodic3.dgf
  sphere.dgf
  simplex-testgrid-2-2.dgf
  simplex-testgrid-2-3.dgf
  simplex-testgrid-2-3-noproj.dgf
  simplex-testgrid-3-3.dgf
  reference-cube-2-2.dgf
  reference-cube-3-3.dgf
  reference-simplex-2-2.dgf
  reference-simplex-3-3.dgf
)

foreach(dgf ${DGFS})
  configure_file( ${dgf} ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
endforeach()

install(FILES ${DGFS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/test/dgf)
