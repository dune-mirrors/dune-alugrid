set(modules
    CMakeLists.txt
    DuneAlugridMacros.cmake
    FindDLMalloc.cmake
    FindPThreads.cmake
    FindSIONlib.cmake
    FindZOLTAN.cmake
    AlugridTortureTests.cmake
)
install(FILES ${modules} DESTINATION ${DUNE_INSTALL_MODULEDIR})
