cmake_minimum_required(VERSION 3.21)

project(zspace_core
  VERSION 3.0.0
  LANGUAGES CXX
)

option(ZSPACE_BUILD_INTERFACE "Build zSpace interface DLL" ON)
option(ZSPACE_BUILD_IO "Build zSpace IO DLL" ON)
option(ZSPACE_BUILD_DISPLAY "Build zSpace display DLL" ON)
option(ZSPACE_BUILD_INTEROP "Build zSpace interop DLL" OFF)
option(ZSPACE_BUILD_TESTS "Build zSpace CLI smoke tests" OFF)
option(ZSPACE_BUILD_SHARED "Build zSpace libraries as DLLs" ON)
option(ZSPACE_WITH_OPENGL "Link bundled OpenGL helper libraries" ON)
option(ZSPACE_IO_WITH_TINYUSDZ "Enable TinyUSDZ mesh IO" ON)
option(ZSPACE_WITH_MAYA "Enable Autodesk Maya interop code" OFF)
option(ZSPACE_WITH_RHINO "Enable Rhino interop code" OFF)
option(ZSPACE_WITH_UNREAL "Enable Unreal interop code" OFF)
option(ZSPACE_WITH_USD "Enable USD/Omniverse interop code" OFF)
option(ZSPACE_RUN_RHINO_RUNTIME_TESTS "Run tests that load Rhino runtime DLLs" OFF)

set(ZSPACE_MAYA_DIR "" CACHE PATH "Autodesk Maya SDK root")
set(ZSPACE_RHINO_DIR "" CACHE PATH "Rhino SDK root")
set(ZSPACE_RHINO_RUNTIME_DIR "" CACHE PATH "Rhino runtime System folder")
set(ZSPACE_UNREAL_DIR "" CACHE PATH "Unreal Engine root")
set(ZSPACE_USD_DIR "" CACHE PATH "USD/Omniverse dependency root")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ZSPACE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(ZSPACE_OUTPUT_DIR "${CMAKE_BINARY_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ZSPACE_OUTPUT_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${ZSPACE_OUTPUT_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ZSPACE_OUTPUT_DIR}/bin")

if(ZSPACE_BUILD_SHARED)
  set(ZSPACE_LIBRARY_TYPE SHARED)
else()
  set(ZSPACE_LIBRARY_TYPE STATIC)
endif()

set(ZSPACE_COMMON_INCLUDE_DIRS
  "${ZSPACE_ROOT}"
  "${ZSPACE_ROOT}/include"
  "${ZSPACE_ROOT}/third_party"
  "${ZSPACE_ROOT}/third_party/depends"
  "${ZSPACE_ROOT}/third_party/depends/Armadillo"
  "${ZSPACE_ROOT}/third_party/depends/alglib/cpp/src"
)

set(ZSPACE_INSTALL_TARGETS)

function(zspace_configure_public_includes target)
  target_include_directories(${target}
    PUBLIC
      $<BUILD_INTERFACE:${ZSPACE_ROOT}>
      $<BUILD_INTERFACE:${ZSPACE_ROOT}/include>
      $<INSTALL_INTERFACE:include>
  )
  target_include_directories(${target}
    SYSTEM PUBLIC
      $<BUILD_INTERFACE:${ZSPACE_ROOT}/third_party>
      $<BUILD_INTERFACE:${ZSPACE_ROOT}/third_party/depends>
      $<BUILD_INTERFACE:${ZSPACE_ROOT}/third_party/depends/Armadillo>
      $<BUILD_INTERFACE:${ZSPACE_ROOT}/third_party/depends/alglib/cpp/src>
      $<INSTALL_INTERFACE:include/depends>
  )
endfunction()

file(GLOB_RECURSE ZSPACE_CORE_SOURCES CONFIGURE_DEPENDS
  "${ZSPACE_ROOT}/src/zCore/*.cpp"
  "${ZSPACE_ROOT}/third_party/depends/lodePNG/lodepng.cpp"
  "${ZSPACE_ROOT}/third_party/depends/tooJPEG/toojpeg.cpp"
)
list(REMOVE_ITEM ZSPACE_CORE_SOURCES
  "${ZSPACE_ROOT}/src/zCore/zCore.cpp"
)

add_library(zSpace_Core ${ZSPACE_LIBRARY_TYPE} ${ZSPACE_CORE_SOURCES})
add_library(zspace::core ALIAS zSpace_Core)
set_target_properties(zSpace_Core PROPERTIES EXPORT_NAME core)
zspace_configure_public_includes(zSpace_Core)
list(APPEND ZSPACE_INSTALL_TARGETS zSpace_Core)

target_compile_definitions(zSpace_Core
  PUBLIC
    $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_DYNAMIC_LIBRARY>
    $<$<NOT:$<BOOL:${ZSPACE_BUILD_SHARED}>>:ZSPACE_STATIC_LIBRARY>
  PRIVATE
    $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_CORE_EXPORTS>
)

if(MSVC)
  target_compile_options(zSpace_Core PRIVATE /MP /W0 /permissive-)
  target_compile_definitions(zSpace_Core PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(ZSPACE_BUILD_INTERFACE)
  file(GLOB_RECURSE ZSPACE_INTERFACE_SOURCES CONFIGURE_DEPENDS
    "${ZSPACE_ROOT}/src/zInterface/*.cpp"
  )
  list(REMOVE_ITEM ZSPACE_INTERFACE_SOURCES
    "${ZSPACE_ROOT}/src/zInterface/functionsets/zFnComputeMesh.cpp"
    "${ZSPACE_ROOT}/src/zInterface/objects/zObjectComputeMesh.cpp"
  )

  add_library(zSpace_Interface ${ZSPACE_LIBRARY_TYPE} ${ZSPACE_INTERFACE_SOURCES})
  add_library(zspace::interface ALIAS zSpace_Interface)
  set_target_properties(zSpace_Interface PROPERTIES EXPORT_NAME interface)
  zspace_configure_public_includes(zSpace_Interface)
  list(APPEND ZSPACE_INSTALL_TARGETS zSpace_Interface)

  target_compile_definitions(zSpace_Interface
    PUBLIC
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_DYNAMIC_LIBRARY>
      $<$<NOT:$<BOOL:${ZSPACE_BUILD_SHARED}>>:ZSPACE_STATIC_LIBRARY>
    PRIVATE
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_INTERFACE_EXPORTS>
  )

  if(MSVC)
    target_compile_options(zSpace_Interface PRIVATE /MP /W0 /permissive-)
    target_compile_definitions(zSpace_Interface PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()

  target_link_libraries(zSpace_Interface PUBLIC zspace::core)

endif()

if(ZSPACE_BUILD_IO)
  if(NOT TARGET zSpace_Interface)
    message(FATAL_ERROR "ZSPACE_BUILD_IO requires ZSPACE_BUILD_INTERFACE=ON")
  endif()

  file(GLOB_RECURSE ZSPACE_IO_SOURCES CONFIGURE_DEPENDS
    "${ZSPACE_ROOT}/src/zIO/*.cpp"
  )

  add_library(zSpace_IO ${ZSPACE_LIBRARY_TYPE} ${ZSPACE_IO_SOURCES})
  add_library(zspace::io ALIAS zSpace_IO)
  set_target_properties(zSpace_IO PROPERTIES EXPORT_NAME io)
  zspace_configure_public_includes(zSpace_IO)
  list(APPEND ZSPACE_INSTALL_TARGETS zSpace_IO)

  target_compile_definitions(zSpace_IO
    PUBLIC
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_DYNAMIC_LIBRARY>
      $<$<NOT:$<BOOL:${ZSPACE_BUILD_SHARED}>>:ZSPACE_STATIC_LIBRARY>
    PRIVATE
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_IO_EXPORTS>
  )

  if(MSVC)
    target_compile_options(zSpace_IO PRIVATE /MP /W0 /permissive-)
    target_compile_definitions(zSpace_IO PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()

  target_link_libraries(zSpace_IO PUBLIC zspace::interface)

  if(ZSPACE_IO_WITH_TINYUSDZ)
    include(FetchContent)

    set(TINYUSDZ_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_C_API OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_BUILTIN_IMAGE_LOADER OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_TYDRA OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_USDMTLX OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_JSON OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_USD_TO_GLTF OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_USDOBJ OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_USDFBX OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_AUDIO OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_ALAC_AUDIO OFF CACHE BOOL "" FORCE)
    set(TINYUSDZ_WITH_PXR_COMPAT_API OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(tinyusdz
      GIT_REPOSITORY https://github.com/lighttransport/tinyusdz.git
      GIT_TAG 11a2d361a9b6a0c4bf8918027233858d80ed17c5
      GIT_SHALLOW TRUE
      EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(tinyusdz)

    target_compile_definitions(zSpace_IO PRIVATE ZSPACE_IO_TINYUSDZ)
    target_include_directories(zSpace_IO PRIVATE "${tinyusdz_SOURCE_DIR}/src")
    target_link_libraries(zSpace_IO PRIVATE tinyusdz::tinyusdz_static)
  endif()
endif()

if(ZSPACE_BUILD_DISPLAY)
  if(NOT TARGET zSpace_Interface)
    message(FATAL_ERROR "ZSPACE_BUILD_DISPLAY requires ZSPACE_BUILD_INTERFACE=ON")
  endif()

  file(GLOB_RECURSE ZSPACE_DISPLAY_SOURCES CONFIGURE_DEPENDS
    "${ZSPACE_ROOT}/src/zDisplay/*.cpp"
  )

  add_library(zSpace_Display ${ZSPACE_LIBRARY_TYPE} ${ZSPACE_DISPLAY_SOURCES})
  add_library(zspace::display ALIAS zSpace_Display)
  set_target_properties(zSpace_Display PROPERTIES EXPORT_NAME display)
  zspace_configure_public_includes(zSpace_Display)
  list(APPEND ZSPACE_INSTALL_TARGETS zSpace_Display)

  target_compile_definitions(zSpace_Display
    PUBLIC
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_DYNAMIC_LIBRARY>
      $<$<NOT:$<BOOL:${ZSPACE_BUILD_SHARED}>>:ZSPACE_STATIC_LIBRARY>
    PRIVATE
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_DISPLAY_EXPORTS>
  )

  if(MSVC)
    target_compile_options(zSpace_Display PRIVATE /MP /W0 /permissive-)
    target_compile_definitions(zSpace_Display PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()

  if(ZSPACE_WITH_OPENGL)
    target_link_directories(zSpace_Display PRIVATE
      "${ZSPACE_ROOT}/third_party/depends/openGL/lib"
      "${ZSPACE_ROOT}/third_party/depends/freeglut"
    )
    target_link_libraries(zSpace_Display PRIVATE opengl32 glew32 freeglut)
  endif()

  target_link_libraries(zSpace_Display PUBLIC zspace::interface)
endif()

include(CMakePackageConfigHelpers)

install(TARGETS ${ZSPACE_INSTALL_TARGETS}
  EXPORT zspaceTargets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION include
)

install(DIRECTORY "${ZSPACE_ROOT}/include/zspace"
  DESTINATION include
)

foreach(zspace_dependency Eigen Eigen_Unsupported Armadillo nlohmann quickhull lodePNG tooJPEG)
  if(EXISTS "${ZSPACE_ROOT}/third_party/depends/${zspace_dependency}")
    install(DIRECTORY "${ZSPACE_ROOT}/third_party/depends/${zspace_dependency}"
      DESTINATION include/depends
    )
  endif()
endforeach()

configure_package_config_file(
  "${ZSPACE_ROOT}/cmake/zspaceConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/zspaceConfig.cmake"
  INSTALL_DESTINATION lib/cmake/zspace
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/zspaceConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(EXPORT zspaceTargets
  FILE zspaceTargets.cmake
  NAMESPACE zspace::
  DESTINATION lib/cmake/zspace
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/zspaceConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/zspaceConfigVersion.cmake"
  DESTINATION lib/cmake/zspace
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/version.txt" "${PROJECT_VERSION}\n")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version.txt" DESTINATION .)

if(ZSPACE_BUILD_TESTS)
  if(NOT TARGET zSpace_Display)
    message(FATAL_ERROR "ZSPACE_BUILD_TESTS requires ZSPACE_BUILD_DISPLAY=ON")
  endif()

  include(CTest)

  add_executable(zspace_smoke
    "${ZSPACE_ROOT}/tests/smoke/zspace_smoke.cpp"
  )
  target_link_libraries(zspace_smoke PRIVATE zspace::interface)

  add_executable(zspace_display_smoke
    "${ZSPACE_ROOT}/tests/smoke/zspace_display_smoke.cpp"
  )
  target_link_libraries(zspace_display_smoke PRIVATE zspace::display)

  if(TARGET zSpace_IO)
    add_executable(zspace_io_smoke
      "${ZSPACE_ROOT}/tests/smoke/zspace_io_smoke.cpp"
    )
    target_link_libraries(zspace_io_smoke PRIVATE zspace::io)
  endif()

  add_library(zspace_display_headers OBJECT
    "${ZSPACE_ROOT}/tests/headers/zDisplayGraph_header.cpp"
    "${ZSPACE_ROOT}/tests/headers/zDisplayMesh_header.cpp"
    "${ZSPACE_ROOT}/tests/headers/zDisplayScene_header.cpp"
  )
  target_include_directories(zspace_display_headers PRIVATE
    "${ZSPACE_ROOT}/include"
  )
  target_include_directories(zspace_display_headers SYSTEM PRIVATE
    "${ZSPACE_ROOT}/third_party/depends"
  )
  target_compile_definitions(zspace_display_headers PRIVATE
    $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_DYNAMIC_LIBRARY>
    $<$<NOT:$<BOOL:${ZSPACE_BUILD_SHARED}>>:ZSPACE_STATIC_LIBRARY>
  )

  if(MSVC)
    target_compile_options(zspace_smoke PRIVATE /W4 /permissive-)
    target_compile_options(zspace_display_smoke PRIVATE /W4 /permissive-)
    if(TARGET zspace_io_smoke)
      target_compile_options(zspace_io_smoke PRIVATE /W4 /permissive-)
    endif()
    target_compile_options(zspace_display_headers PRIVATE /W4 /WX /permissive-)
  endif()

  add_test(NAME zspace.smoke COMMAND zspace_smoke)
  if(TARGET zspace_io_smoke)
    add_test(NAME zspace.io.smoke COMMAND zspace_io_smoke)
  endif()
endif()

if(ZSPACE_BUILD_INTEROP)
  if(NOT TARGET zSpace_Interface)
    message(FATAL_ERROR "ZSPACE_BUILD_INTEROP requires ZSPACE_BUILD_INTERFACE=ON")
  endif()

  if(NOT ZSPACE_WITH_RHINO)
    message(FATAL_ERROR "ZSPACE_BUILD_INTEROP requires ZSPACE_WITH_RHINO=ON because the current zInterOp plane/arc/nurbs API uses OpenNURBS types directly")
  endif()

  file(GLOB ZSPACE_INTEROP_SOURCES CONFIGURE_DEPENDS
    "${ZSPACE_ROOT}/src/zInterOp/objects/*.cpp"
    "${ZSPACE_ROOT}/src/zInterOp/functionsets/zFn*.cpp"
    "${ZSPACE_ROOT}/src/zInterOp/functionsets/zRhinoFn*.cpp"
    "${ZSPACE_ROOT}/src/zInterOp/io/*.cpp"
    "${ZSPACE_ROOT}/src/zInterOp/core/zRhinoCore.cpp"
  )

  if(ZSPACE_WITH_MAYA)
    file(GLOB ZSPACE_INTEROP_MAYA_SOURCES CONFIGURE_DEPENDS
      "${ZSPACE_ROOT}/src/zInterOp/functionsets/zMayaFn*.cpp"
    )
    list(APPEND ZSPACE_INTEROP_SOURCES ${ZSPACE_INTEROP_MAYA_SOURCES})
  endif()

  if(ZSPACE_WITH_UNREAL)
    list(APPEND ZSPACE_INTEROP_SOURCES
      "${ZSPACE_ROOT}/src/zInterOp/functionsets/zUnrealFnMesh.cpp"
    )
  endif()

  if(ZSPACE_WITH_MAYA AND ZSPACE_WITH_RHINO)
    file(GLOB ZSPACE_INTEROP_CONVERTER_SOURCES CONFIGURE_DEPENDS
      "${ZSPACE_ROOT}/src/zInterOp/converters/*.cpp"
    )
    list(APPEND ZSPACE_INTEROP_SOURCES ${ZSPACE_INTEROP_CONVERTER_SOURCES})
  endif()

  if(ZSPACE_WITH_USD)
    file(GLOB_RECURSE ZSPACE_INTEROP_USD_SOURCES CONFIGURE_DEPENDS
      "${ZSPACE_ROOT}/src/zInterOp/core/omniUtils/*.cpp"
    )
    list(APPEND ZSPACE_INTEROP_SOURCES
      "${ZSPACE_ROOT}/src/zInterOp/core/zOmniCore.cpp"
      ${ZSPACE_INTEROP_USD_SOURCES}
    )
  endif()

  add_library(zSpace_InterOp ${ZSPACE_LIBRARY_TYPE} ${ZSPACE_INTEROP_SOURCES})
  add_library(zspace::interop ALIAS zSpace_InterOp)

  target_include_directories(zSpace_InterOp
    PUBLIC
      ${ZSPACE_COMMON_INCLUDE_DIRS}
  )

  target_compile_definitions(zSpace_InterOp
    PUBLIC
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_DYNAMIC_LIBRARY>
      $<$<NOT:$<BOOL:${ZSPACE_BUILD_SHARED}>>:ZSPACE_STATIC_LIBRARY>
    PRIVATE
      $<$<BOOL:${ZSPACE_BUILD_SHARED}>:ZSPACE_INTEROP_EXPORTS>
  )

  if(MSVC)
    target_compile_options(zSpace_InterOp PRIVATE /MP /W0 /permissive- /UWIN32)
    target_compile_definitions(zSpace_InterOp PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()

  if(ZSPACE_WITH_MAYA)
    if(NOT ZSPACE_MAYA_DIR)
      message(FATAL_ERROR "ZSPACE_WITH_MAYA=ON requires ZSPACE_MAYA_DIR")
    endif()
    target_compile_definitions(zSpace_InterOp PUBLIC ZSPACE_MAYA_INTEROP)
    target_include_directories(zSpace_InterOp PUBLIC "${ZSPACE_MAYA_DIR}/include")
    target_link_directories(zSpace_InterOp PUBLIC "${ZSPACE_MAYA_DIR}/lib")
    target_link_libraries(zSpace_InterOp PUBLIC
      OpenMayaRender OpenMayaFX OpenMayaAnim OpenMaya OpenMayaUI Foundation
    )
  endif()

  if(NOT ZSPACE_RHINO_DIR)
    message(FATAL_ERROR "ZSPACE_BUILD_INTEROP requires ZSPACE_RHINO_DIR")
  endif()
  if(NOT ZSPACE_RHINO_RUNTIME_DIR)
    string(REGEX REPLACE " SDK$" "" ZSPACE_RHINO_INSTALL_DIR "${ZSPACE_RHINO_DIR}")
    if(EXISTS "${ZSPACE_RHINO_INSTALL_DIR}/System")
      set(ZSPACE_RHINO_RUNTIME_DIR "${ZSPACE_RHINO_INSTALL_DIR}/System" CACHE PATH "Rhino runtime System folder" FORCE)
    endif()
  endif()
  target_compile_definitions(zSpace_InterOp PUBLIC ZSPACE_RHINO_INTEROP WIN64 UNICODE _UNICODE _HAS_STD_BYTE=0)
  target_include_directories(zSpace_InterOp PUBLIC "${ZSPACE_RHINO_DIR}/inc")
  target_link_directories(zSpace_InterOp PUBLIC "${ZSPACE_RHINO_DIR}/lib/Release")
  target_link_libraries(zSpace_InterOp PUBLIC opennurbs RhinoCore RhinoLibrary)

  if(ZSPACE_WITH_UNREAL)
    if(NOT ZSPACE_UNREAL_DIR)
      message(FATAL_ERROR "ZSPACE_WITH_UNREAL=ON requires ZSPACE_UNREAL_DIR")
    endif()
    target_compile_definitions(zSpace_InterOp PUBLIC ZSPACE_UNREAL_INTEROP)
    target_include_directories(zSpace_InterOp PUBLIC "${ZSPACE_UNREAL_DIR}/Engine/Source")
  endif()

  if(ZSPACE_WITH_USD)
    if(NOT ZSPACE_USD_DIR)
      message(FATAL_ERROR "ZSPACE_WITH_USD=ON requires ZSPACE_USD_DIR")
    endif()
    target_compile_definitions(zSpace_InterOp PUBLIC ZSPACE_USD_INTEROP)
    target_include_directories(zSpace_InterOp PUBLIC
      "${ZSPACE_USD_DIR}/usd/release/include"
      "${ZSPACE_USD_DIR}/omni_client_library/include"
      "${ZSPACE_USD_DIR}/omni_usd_resolver/include"
      "${ZSPACE_USD_DIR}/tinytoml/include"
    )
    target_link_directories(zSpace_InterOp PUBLIC
      "${ZSPACE_USD_DIR}/usd/release/lib"
      "${ZSPACE_USD_DIR}/omni_client_library/release"
      "${ZSPACE_USD_DIR}/omni_usd_resolver/release"
    )
  endif()

  if(TARGET zSpace_IO)
    target_link_libraries(zSpace_InterOp PUBLIC zspace::io)
  else()
    target_link_libraries(zSpace_InterOp PUBLIC zspace::interface)
  endif()

  if(ZSPACE_BUILD_TESTS AND ZSPACE_WITH_RHINO)
    add_library(zspace_interop_headers OBJECT
      "${ZSPACE_ROOT}/tests/headers/zInterOpObjects_header.cpp"
    )
    target_link_libraries(zspace_interop_headers PRIVATE zspace::interop)
    if(MSVC)
      target_compile_options(zspace_interop_headers PRIVATE /W4 /permissive- /UWIN32)
    endif()
  endif()

  if(ZSPACE_BUILD_TESTS AND ZSPACE_RUN_RHINO_RUNTIME_TESTS)
    add_executable(zspace_interop_io_smoke
      "${ZSPACE_ROOT}/tests/smoke/zspace_interop_io_smoke.cpp"
    )
    target_link_libraries(zspace_interop_io_smoke PRIVATE zspace::interop)
    if(MSVC)
      target_compile_options(zspace_interop_io_smoke PRIVATE /W4 /permissive- /UWIN32)
    endif()
    add_test(NAME zspace.interop.io.smoke COMMAND zspace_interop_io_smoke)
    if(ZSPACE_RHINO_RUNTIME_DIR)
      set_tests_properties(zspace.interop.io.smoke PROPERTIES
        ENVIRONMENT_MODIFICATION
          "PATH=path_list_prepend:$<TARGET_FILE_DIR:zSpace_InterOp>;PATH=path_list_prepend:${ZSPACE_RHINO_RUNTIME_DIR}"
      )
    endif()
  endif()
endif()
