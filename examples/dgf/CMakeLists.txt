set( DGFS
  cube3d.dgf
  cube_hc_32768.dgf
  cube_hc_4096.dgf
  cube_hc_512.dgf
  ffs2d.dgf
  ffs3d.dgf
  ffs3d_fine.dgf
  sb2d.dgf
  sb3d_1024.dgf
  sb3d_128.dgf
  sb3d_16.dgf
  sb3d_65536.dgf
  sb3d_8192.dgf
  sb3d.dgf
  unitcube2d.dgf
  unitcube3d.dgf
  tet.0.msh.dgf
  tet.00.msh.dgf
  tet.1.msh.dgf
  tet.2.msh.dgf
  tet.3.msh.dgf
  tet.4.msh.dgf
  tet.5.msh.dgf
  tet.6.msh.dgf
)

foreach( dgf ${DGFS} )
  configure_file( ${dgf} ${CMAKE_CURRENT_BINARY_DIR}/${dgf} COPYONLY )
endforeach()
