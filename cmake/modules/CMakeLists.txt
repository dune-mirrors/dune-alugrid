set(modules 
    AddSIONlibFlags.cmake
    AddZOLTANFlags.cmake
    ALUGridType.cmake
    CMakeLists.txt
    DuneAlugridMacros.cmake
    FindDLMalloc.cmake
    FindSIONlib.cmake
    FindZOLTAN.cmake
)
install(FILES "${modules}" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/modules)
