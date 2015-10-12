# Module that checks whether DLMALLOC is available.
#
# Variables used by this module which you may want to set:
# DLMALLOC_ROOT        Path list to search for DLMALLOC
#
# Sets the following variables
#
# DLMALLOC_FOUND          True if DLMALLOC was found and usable
# HAVE_DLMALLOC           True if DLMALLOC was found and usable
# DLMALLOC_INCLUDE_DIRS   Path to the DLMALLOC include dirs

set(DLMALLOC_ROOT "" CACHE PATH "Path list to search for DLMALLOC")
mark_as_advanced(DLMALLOC_ROOT)

#message("dlmalloc: ${DLMALLOC_ROOT}")

#look for header files at positions given by the user
find_path(DLMALLOC_INCLUDE_DIR malloc.c
  PATHS ${DLMALLOC_DIR} ${DLMALLOC_ROOT}
  NO_DEFAULT_PATH
)

IF(DLMALLOC_INCLUDE_DIR)
  set(DLMALLOC_SOURCE_INCLUDE "\"${DLMALLOC_INCLUDE_DIR}/malloc.c\"")
ELSE()
  #look for header files at positions given by the user
  find_path(DLMALLOC_INCLUDE_DIR malloc-2.8.6.c
    PATHS ${DLMALLOC_DIR} ${DLMALLOC_ROOT}
    NO_DEFAULT_PATH
  )
  IF(DLMALLOC_INCLUDE_DIR)
    set(DLMALLOC_SOURCE_INCLUDE "\"${DLMALLOC_INCLUDE_DIR}/malloc-2.8.6.c\"")
  ENDIF()
ENDIF()

# check if dlmalloc can be compiled
CHECK_C_SOURCE_COMPILES( "#include ${DLMALLOC_SOURCE_INCLUDE}
                          int main () { return 0; }" DLMALLOC_SOURCE_USABLE )


# behave like a CMake module is supposed to behave
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "DLMalloc"
  DEFAULT_MSG
  DLMALLOC_INCLUDE_DIR
  DLMALLOC_SOURCE_USABLE
)

mark_as_advanced(DLMALLOC_INCLUDE_DIR)

# if found, store some results
if(DLMALLOC_FOUND)
    message(STATUS "${DLMALLOC_SOURCE_INCLUDE} found.")
endif(DLMALLOC_FOUND)

#set HAVE_DLMALLOC for config.h
set(HAVE_DLMALLOC ${DLMALLOC_FOUND})

# register package
if(DLMALLOC_FOUND)
  dune_register_package_flags(INCLUDE_DIRS ${DLMALLOC_SOURCE_INCLUDE})
endif()
