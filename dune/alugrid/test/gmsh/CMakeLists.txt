set(GMSHFILES
  circle2ndorder.msh
  curved2d.msh
  telescope.msh
  unitsquare_quads_2x2.msh
)

foreach(gmshfile ${GMSHFILES})
  configure_file( ${gmshfile} ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
endforeach()

install(FILES ${GMSHFILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/test/gmsh)
