set(modules 
    ALUGridType.cmake
    CMakeLists.txt
    DuneAlugridMacros.cmake
    FindDLMalloc.cmake
    FindPThreads.cmake
    FindSIONlib.cmake
    FindZOLTAN.cmake
)
install(FILES ${modules} DESTINATION ${DUNE_INSTALL_MODULEDIR})
