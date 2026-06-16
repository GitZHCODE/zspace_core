#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "lexers" for configuration "Release"
set_property(TARGET lexers APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(lexers PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/lexers.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS lexers )
list(APPEND _IMPORT_CHECK_FILES_FOR_lexers "${_IMPORT_PREFIX}/lib/lexers.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
