#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "embree" for configuration "Debug"
set_property(TARGET embree APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(embree PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX;RC"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/embree3.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS embree )
list(APPEND _IMPORT_CHECK_FILES_FOR_embree "${_IMPORT_PREFIX}/lib/embree3.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
