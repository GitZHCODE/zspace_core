if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
# Runs in script mode so runtime inspection does not occur in project mode.
set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM windows+pe)
set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL dumpbin)
set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND "${DUMPBIN}")
file(GET_RUNTIME_DEPENDENCIES LIBRARIES ${SEEDS}
  DIRECTORIES "${ROOT}/bin" "${ROOT}/lib"
  PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
  POST_EXCLUDE_REGEXES ".*[Ww][Ii][Nn][Dd][Oo][Ww][Ss]/[Ss][Yy][Ss][Tt][Ee][Mm]32/.*"
  RESOLVED_DEPENDENCIES_VAR resolved UNRESOLVED_DEPENDENCIES_VAR unresolved)
if(unresolved)
  message(FATAL_ERROR "Core OpenUSD unresolved runtime dependencies: ${unresolved}")
endif()
# Also normalize explicitly for CMake versions predating CMP0207.
set(filtered)
foreach(path IN LISTS resolved)
  file(TO_CMAKE_PATH "${path}" path)
  string(TOLOWER "${path}" lower)
  file(TO_CMAKE_PATH "$ENV{SystemRoot}/System32/" system_dir)
  string(TOLOWER "${system_dir}" system_dir)
  string(FIND "${lower}" "${system_dir}" system_index)
  if(NOT system_index EQUAL 0)
    list(APPEND filtered "${path}")
  endif()
endforeach()
set(resolved ${filtered})
list(APPEND resolved ${SEEDS})
list(REMOVE_DUPLICATES resolved)
file(WRITE "${OUTPUT}" "set(ZSPACE_USD_RUNTIME_FILES [==[${resolved}]==])\n")
