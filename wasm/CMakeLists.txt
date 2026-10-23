cmake_minimum_required(VERSION 3.21)
project(zspace_core_wasm LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "Configure this target with emcmake and the Emscripten toolchain")
endif()

get_filename_component(ZSPACE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(ZSPACE_WASM_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/out" CACHE PATH
  "Standard SDK-free JavaScript/WASM output directory")

# Keep the batch build's source set. CONFIGURE_DEPENDS makes
# additions/removals trigger regeneration even when building Ninja directly.
file(GLOB_RECURSE core_sources CONFIGURE_DEPENDS "${ZSPACE_ROOT}/src/zCore/*.cpp")
file(GLOB_RECURSE interface_sources CONFIGURE_DEPENDS "${ZSPACE_ROOT}/src/zInterface/*.cpp")
list(FILTER interface_sources EXCLUDE REGEX
  "/(zFnComputeMesh|zObjectComputeMesh|zFnMeshField|zFnPointField|zObjectMeshField|zObjectPointField|zObjectComputeField2D)\\.cpp$")
file(GLOB_RECURSE io_sources CONFIGURE_DEPENDS "${ZSPACE_ROOT}/src/zIO/*.cpp")

# Match the Windows batch traversal: files in each directory before its
# subdirectories, with case-insensitive ordering. Keep static initialization
# and linker layout in the established order instead of glob's global order.
function(zspace_wasm_order_sources result)
  set(directories)
  foreach(source IN LISTS ARGN)
    get_filename_component(directory "${source}" DIRECTORY)
    list(APPEND directories "${directory}")
  endforeach()
  list(REMOVE_DUPLICATES directories)
  list(SORT directories CASE INSENSITIVE)
  set(ordered)
  foreach(directory IN LISTS directories)
    set(local_sources)
    foreach(source IN LISTS ARGN)
      get_filename_component(parent "${source}" DIRECTORY)
      if(parent STREQUAL directory)
        list(APPEND local_sources "${source}")
      endif()
    endforeach()
    list(SORT local_sources CASE INSENSITIVE)
    list(APPEND ordered ${local_sources})
  endforeach()
  set(${result} "${ordered}" PARENT_SCOPE)
endfunction()
zspace_wasm_order_sources(core_sources ${core_sources})
zspace_wasm_order_sources(interface_sources ${interface_sources})
zspace_wasm_order_sources(io_sources ${io_sources})

# All objects still reach the linker in the established traversal order.
set(wasm_sources ${core_sources} ${interface_sources} ${io_sources}
  "${CMAKE_CURRENT_SOURCE_DIR}/bridge/zspace_core_wasm_bridge.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/bridge/default_live_sketch.cpp")

function(zspace_wasm_configure_compilation target)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
  target_include_directories(${target} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge" "${ZSPACE_ROOT}"
    "${ZSPACE_ROOT}/include" "${ZSPACE_ROOT}/third_party"
    "${ZSPACE_ROOT}/third_party/depends")
  target_compile_definitions(${target} PRIVATE ZSPACE_STATIC_LIBRARY)
  target_compile_options(${target} PRIVATE -O2 -fexceptions
    -include "${CMAKE_CURRENT_SOURCE_DIR}/bridge/zspace_wasm_compat.h")
endfunction()

option(ZSPACE_WASM_SCHEDULE_HEAVY_FIRST
  "Start expensive compilation units early while preserving link order" ON)
if(ZSPACE_WASM_SCHEDULE_HEAVY_FIRST)
  # Object targets separate Ninja's compilation order from the link order.
  # Split only around expensive units; retain contiguous groups for the rest.
  set(heavy_sources
    "${ZSPACE_ROOT}/src/zCore/utilities/zUtilsCore.cpp"
    "${ZSPACE_ROOT}/src/zInterface/functionsets/zFnMesh.cpp"
    "${ZSPACE_ROOT}/src/zIO/codecs/zCodecJSON.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/zspace_core_wasm_bridge.cpp")
  set(chunk 0)
  set(heavy_chunks)
  foreach(source IN LISTS wasm_sources)
    if(source IN_LIST heavy_sources)
      if(chunk_${chunk})
        math(EXPR chunk "${chunk} + 1")
      endif()
      set(chunk_${chunk} "${source}")
      list(APPEND heavy_chunks ${chunk})
      math(EXPR chunk "${chunk} + 1")
    else()
      list(APPEND chunk_${chunk} "${source}")
    endif()
  endforeach()
  set(normal_chunks)
  set(link_objects)
  foreach(index RANGE 0 ${chunk})
    if(chunk_${index})
      if(NOT index IN_LIST heavy_chunks)
        list(APPEND normal_chunks ${index})
      endif()
      list(APPEND link_objects "$<TARGET_OBJECTS:zspace_wasm_part_${index}>")
    endif()
  endforeach()
  # Ninja sees the heavy compilation edges before the remaining groups.
  foreach(index IN LISTS heavy_chunks normal_chunks)
    add_library(zspace_wasm_part_${index} OBJECT ${chunk_${index}})
    zspace_wasm_configure_compilation(zspace_wasm_part_${index})
  endforeach()
  add_executable(zspace_core ${link_objects})
else()
  add_executable(zspace_core ${wasm_sources})
  zspace_wasm_configure_compilation(zspace_core)
endif()
option(ZSPACE_WASM_FAST_LINK
  "Skip binaryen wasm-opt pass at link time for faster incremental development builds" OFF)
set(zspace_wasm_link_opt -O2)
if(ZSPACE_WASM_FAST_LINK)
  set(zspace_wasm_link_opt -O0)
endif()

set_target_properties(zspace_core PROPERTIES
  LINKER_LANGUAGE CXX SUFFIX ".js"
  RUNTIME_OUTPUT_DIRECTORY "${ZSPACE_WASM_OUT_DIR}")
target_link_options(zspace_core PRIVATE ${zspace_wasm_link_opt} -fexceptions
  "-sMODULARIZE=1" "-sEXPORT_ES6=1" "-sENVIRONMENT=web"
  "-sALLOW_MEMORY_GROWTH=1" "-sFORCE_FILESYSTEM=1"
  "-sEXPORTED_RUNTIME_METHODS=['HEAPF32','HEAPU32','UTF8ToString','FS','ccall']")

# Emscripten emits both files in one link. Track the secondary output too, so
# deleting the WASM binary causes a relink instead of a false no-op build.
add_custom_command(TARGET zspace_core POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E true
  BYPRODUCTS "${ZSPACE_WASM_OUT_DIR}/zspace_core.wasm")

# Reuse one PCH across all object groups with identical compiler settings.
# The helper object is a build dependency only and is never linked into WASM.
option(ZSPACE_WASM_USE_PCH "Reuse precompiled Eigen and JSON headers" ON)
if(ZSPACE_WASM_USE_PCH)
  set(zspace_wasm_pch_headers
    <depends/Eigen/Dense>
    <depends/Eigen/Sparse>
    <depends/nlohmann/json.hpp>
    <zspace/zCore/base/zTypeDef.h>)
  if(ZSPACE_WASM_SCHEDULE_HEAVY_FIRST)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/zspace_wasm_pch_stub.cpp"
      CONTENT "// Unlinked helper for the shared precompiled header.\n")
    add_library(zspace_wasm_pch OBJECT "${CMAKE_CURRENT_BINARY_DIR}/zspace_wasm_pch_stub.cpp")
    zspace_wasm_configure_compilation(zspace_wasm_pch)
    target_precompile_headers(zspace_wasm_pch PRIVATE ${zspace_wasm_pch_headers})
    foreach(index IN LISTS heavy_chunks normal_chunks)
      target_precompile_headers(zspace_wasm_part_${index} REUSE_FROM zspace_wasm_pch)
    endforeach()
  else()
    target_precompile_headers(zspace_core PRIVATE ${zspace_wasm_pch_headers})
  endif()
endif()
