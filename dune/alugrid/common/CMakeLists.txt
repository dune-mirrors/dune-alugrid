set(alucommondir  ${CMAKE_INSTALL_INCLUDEDIR}/dune/alugrid/common)
set(alucommon_HEADERS alugrid_assert.hh
                      backuprestore.hh
                      bndprojection.hh
                      capabilities.hh
                      declaration.hh
                      defaultindexsets.hh
                      geostorage.hh
                      hsfc.hh
                      intersectioniteratorwrapper.hh
                      interfaces.hh
                      ldbhandleif.hh
                      macrogridview.hh
                      memory.hh
                      persistentcontainer.hh
                      structuredgridfactory.hh
                      transformation.hh
                      twists.hh
                      typetraits.hh
                      writeparalleldgf.hh
   )
# installation of the header files                  
install(FILES ${alucommon_HEADERS} DESTINATION ${alucommondir})
