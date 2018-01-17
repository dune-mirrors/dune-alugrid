add_subdirectory(alugrid)

if(dune-python_FOUND)
  add_python_targets(dune
    __init__
  )
endif()
