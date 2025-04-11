set(modules
    CMakeLists.txt
    DuneAlugridMacros.cmake
    FindDLMalloc.cmake
    FindPThreads.cmake
    FindSIONLib.cmake
    FindZOLTAN.cmake
    AlugridTortureTests.cmake
)
install(FILES ${modules} DESTINATION ${DUNE_INSTALL_MODULEDIR})
