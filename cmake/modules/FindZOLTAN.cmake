# Module that checks whether ZOLTAN is available.
#
# Variables used by this module which you may want to set:
# ZOLTAN_ROOT        Path list to search for ZOLTAN
#
# Sets the following variables
#
# ZOLTAN_FOUND          True if ZOLTAN was found and usable
# HAVE_ZOLTAN           True if ZOLTAN was found and usable
# ZOLTAN_INCLUDE_DIRS   Path to the ZOLTAN include dirs
# ZOLTAN_LIBRARIES      Name of the ZOLTAN libraries
#

set(ZOLTAN_ROOT "" CACHE PATH "Path list to search for ZOLTAN")

# Make sure we have checked for the underlying partitioners.
find_package(PTScotch)
find_package(ParMETIS)

#look for header files at positions given by the user
find_path(ZOLTAN_INCLUDE_DIR zoltan.h
  PATHS ${ZOLTAN_ROOT}
  PATH_SUFFIXES "include"
  NO_DEFAULT_PATH
)

message("Zoltan Dir: ${ZOLTAN_INCLUDE_DIR}")

#if(ZOLTAN_VERSION_AVAILABLE AND ZOLTAN_ROOT_PATH)
  #everything works fine so far...

  #set(ZOLTAN_INCLUDEDIR "${ZOLTAN_INCLUDE_DIR}" CACHE PATH "directory with ZOLTAN headers inside")
  set(ZOLTAN_LIBDIR "${ZOLTAN_ROOT}/lib") # CACHE PATH "directory with ZOLTAN libraries inside")

if(ZOLTAN_INCLUDE_DIR)
  # check header usability
  include(CMakePushCheckState)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} ${MPI_DUNE_COMPILE_FLAGS} -DENABLE_ZOLTAN")
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MPI_DUNE_INCLUDE_PATH} ${ZOLTAN_INCLUDE_DIR}
    ${PTSCOTCH_INCLUDE_DIRS} ${PARMETIS_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}" ${PTSCOTCH_LIBRARIES} ${PARMETIS_LIBRARIES} -L${ZOLTAN_LIBDIR} ${MPI_DUNE_LIBRARIES})

  # include necessary checks
  include(CheckIncludeFileCXX)
  check_include_files(zoltan.h ZOLTAN_HEADER_USABLE)

  #look for library at positions given by the user
  find_library(ZOLTAN_LIBRARY
    NAMES "zoltan"
    PATHS ${ZOLTAN_ROOT}
    PATH_SUFFIXES "lib/.libs" "lib"
    NO_DEFAULT_PATH
  )

  # check if library zoltan works (doesn't work)
  #include(CheckSymbolExists)
  #if(ZOLTAN_LIBRARY)
  #  get_filename_component(ZOLTAN_LIB_PATH ${ZOLTAN_LIBRARY} PATH)
  #  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -L${ZOLTAN_LIBDIR}")
  #  check_library_exists(zoltan Zoltan_LB_Partition ${ZOLTAN_LIBRARY} ZOLTAN_LIB_WORKS)
  #endif(ZOLTAN_LIBRARY)

  cmake_pop_check_state()
endif()

  # behave like a CMake module is supposed to behave
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    "ZOLTAN"
    DEFAULT_MSG
    ZOLTAN_INCLUDE_DIR
    ZOLTAN_LIBRARY
    ZOLTAN_HEADER_USABLE
  )

  mark_as_advanced(ZOLTAN_INCLUDE_DIR ZOLTAN_LIBRARY ZOLTAN_LIB_WORKS ZOLTAN_HEADER_USABLE)

  # if both headers and library are found, store results
  if(ZOLTAN_FOUND)
    set(ZOLTAN_INCLUDE_DIRS ${ZOLTAN_INCLUDE_DIR} ${PARMETIS_INCLUDE_DIRS}
      ${PTSCOTCH_INCLUDE_DIRS})
    set(ZOLTAN_LIBRARIES ${ZOLTAN_LIBRARY} ${PARMETIS_LIBRARIES} ${PTSCOTCH_LIBRARIES})
    set(ZOLTAN_LINK_FLAGS ${PARMETIS_LINK_FLAGS} ${PTSCOTCH_LINK_FLAGS} ${DUNE_MPI_LINK_FLAGS})

    message("Using: ${ZOLTAN_INCLUDE_DIRS} ${ZOLTAN_LIBRARIES}")

    # log result
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining location of ZOLTAN succeded:\n"
      "Include directory: ${ZOLTAN_INCLUDE_DIRS}\n"
      "Library directory: ${ZOLTAN_LIBRARIES}\n\n")
    set(ZOLTAN_DUNE_COMPILE_FLAGS "-I${ZOLTAN_INCLUDE_DIRS}"
      CACHE STRING "Compile Flags used by DUNE when compiling with ZOLTAN programs")
    set(ZOLTAN_DUNE_LIBRARIES ${ZOLTAN_LIBRARIES}
      CACHE STRING "Libraries used by DUNE when linking ZOLTAN programs")
  else(ZOLTAN_FOUND)
    # log errornous result
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Determing location of ZOLTAN failed:\n"
      "Include directory: ${ZOLTAN_INCLUDE_DIRS}\n"
      "Library directory: ${ZOLTAN_LIBRARIES}\n\n")
  endif(ZOLTAN_FOUND)

  #set HAVE_ZOLTAN for config.h
  set(HAVE_ZOLTAN ${ZOLTAN_FOUND})

  # register the flags
  if(ZOLTAN_FOUND)
    dune_register_package_flags(INCLUDE_DIRS ${ZOLTAN_INCLUDE_DIRS}
                                LIBRARIES ${ZOLTAN_LIBRARIES}
                                COMPILE_DEFINITIONS ${ZOLTAN_DUNE_COMPILE_FLAGS})
  endif()
