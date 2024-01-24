#----------------------------------------------------------------
# Generated CMake target import file for configuration "release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "MaterialXCore" for configuration "release"
set_property(TARGET MaterialXCore APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXCore PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXCore.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXCore.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXCore )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXCore "${_IMPORT_PREFIX}/lib/MaterialXCore.lib" "${_IMPORT_PREFIX}/bin/MaterialXCore.dll" )

# Import target "MaterialXFormat" for configuration "release"
set_property(TARGET MaterialXFormat APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXFormat PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXFormat.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXFormat.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXFormat )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXFormat "${_IMPORT_PREFIX}/lib/MaterialXFormat.lib" "${_IMPORT_PREFIX}/bin/MaterialXFormat.dll" )

# Import target "MaterialXGenShader" for configuration "release"
set_property(TARGET MaterialXGenShader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXGenShader PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXGenShader.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXGenShader.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXGenShader )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXGenShader "${_IMPORT_PREFIX}/lib/MaterialXGenShader.lib" "${_IMPORT_PREFIX}/bin/MaterialXGenShader.dll" )

# Import target "MaterialXGenGlsl" for configuration "release"
set_property(TARGET MaterialXGenGlsl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXGenGlsl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXGenGlsl.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXGenGlsl.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXGenGlsl )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXGenGlsl "${_IMPORT_PREFIX}/lib/MaterialXGenGlsl.lib" "${_IMPORT_PREFIX}/bin/MaterialXGenGlsl.dll" )

# Import target "MaterialXGenOsl" for configuration "release"
set_property(TARGET MaterialXGenOsl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXGenOsl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXGenOsl.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXGenOsl.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXGenOsl )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXGenOsl "${_IMPORT_PREFIX}/lib/MaterialXGenOsl.lib" "${_IMPORT_PREFIX}/bin/MaterialXGenOsl.dll" )

# Import target "MaterialXGenMdl" for configuration "release"
set_property(TARGET MaterialXGenMdl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXGenMdl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXGenMdl.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXGenMdl.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXGenMdl )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXGenMdl "${_IMPORT_PREFIX}/lib/MaterialXGenMdl.lib" "${_IMPORT_PREFIX}/bin/MaterialXGenMdl.dll" )

# Import target "MaterialXRender" for configuration "release"
set_property(TARGET MaterialXRender APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXRender PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXRender.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXRender.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXRender )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXRender "${_IMPORT_PREFIX}/lib/MaterialXRender.lib" "${_IMPORT_PREFIX}/bin/MaterialXRender.dll" )

# Import target "MaterialXRenderOsl" for configuration "release"
set_property(TARGET MaterialXRenderOsl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXRenderOsl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXRenderOsl.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXRenderOsl.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXRenderOsl )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXRenderOsl "${_IMPORT_PREFIX}/lib/MaterialXRenderOsl.lib" "${_IMPORT_PREFIX}/bin/MaterialXRenderOsl.dll" )

# Import target "MaterialXRenderHw" for configuration "release"
set_property(TARGET MaterialXRenderHw APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXRenderHw PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXRenderHw.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXRenderHw.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXRenderHw )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXRenderHw "${_IMPORT_PREFIX}/lib/MaterialXRenderHw.lib" "${_IMPORT_PREFIX}/bin/MaterialXRenderHw.dll" )

# Import target "MaterialXRenderGlsl" for configuration "release"
set_property(TARGET MaterialXRenderGlsl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(MaterialXRenderGlsl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/MaterialXRenderGlsl.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/MaterialXRenderGlsl.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS MaterialXRenderGlsl )
list(APPEND _IMPORT_CHECK_FILES_FOR_MaterialXRenderGlsl "${_IMPORT_PREFIX}/lib/MaterialXRenderGlsl.lib" "${_IMPORT_PREFIX}/bin/MaterialXRenderGlsl.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
