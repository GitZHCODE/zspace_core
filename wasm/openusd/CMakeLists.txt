cmake_minimum_required(VERSION 3.24)
project(zspace_core_openusd_wasm LANGUAGES CXX)
if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "Configure this target with the Emscripten toolchain")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
get_filename_component(ZSPACE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(ZSPACE_OPENUSD_ROOT "" CACHE PATH "Headless OpenUSD WASM SDK root")
include("${ZSPACE_ROOT}/cmake/ZSpaceOpenUSD.cmake")

file(GLOB_RECURSE sources CONFIGURE_DEPENDS
  "${ZSPACE_ROOT}/src/zCore/*.cpp"
  "${ZSPACE_ROOT}/src/zInterface/*.cpp"
  "${ZSPACE_ROOT}/src/zIO/*.cpp")
# Match the established WASM source set; field/compute implementations are not
# supported by the existing bridge build.
list(FILTER sources EXCLUDE REGEX "/(zFnComputeMesh|zObjectComputeMesh|zFnMeshField|zFnPointField|zObjectMeshField|zObjectPointField|zObjectComputeField2D)\\.cpp$")
add_executable(zspace_core ${sources}
  "${ZSPACE_ROOT}/wasm/bridge/zspace_core_wasm_bridge.cpp"
  "${ZSPACE_ROOT}/wasm/bridge/default_live_sketch.cpp")
target_include_directories(zspace_core PRIVATE
  "${ZSPACE_ROOT}/wasm/bridge" "${ZSPACE_ROOT}" "${ZSPACE_ROOT}/include")
target_include_directories(zspace_core SYSTEM PRIVATE
  "${ZSPACE_ROOT}/third_party" "${ZSPACE_ROOT}/third_party/depends")
target_compile_definitions(zspace_core PRIVATE ZSPACE_STATIC_LIBRARY ZSPACE_IO_OPENUSD)
target_compile_options(zspace_core PRIVATE -fexceptions -pthread
  -include "${ZSPACE_ROOT}/wasm/bridge/zspace_wasm_compat.h")
zspace_link_openusd(zspace_core)
target_link_options(zspace_core PRIVATE -fexceptions -pthread --use-port=zlib
  "-sMODULARIZE=1" "-sEXPORT_ES6=1" "-sENVIRONMENT=web,node"
  "-sALLOW_MEMORY_GROWTH=1" "-sFORCE_FILESYSTEM=1" "-sPTHREAD_POOL_SIZE=2"
  "-sEXPORTED_RUNTIME_METHODS=['HEAPF32','HEAPU32','UTF8ToString','FS','ccall','PThread']")
set_target_properties(zspace_core PROPERTIES SUFFIX ".js"
  RUNTIME_OUTPUT_DIRECTORY "${ZSPACE_ROOT}/wasm/out/openusd")

add_custom_command(TARGET zspace_core POST_BUILD
  COMMAND "${CMAKE_COMMAND}" "-DROOT=$<TARGET_FILE_DIR:zspace_core>"
    -P "${ZSPACE_ROOT}/cmake/WriteWasmManifest.cmake" VERBATIM)
