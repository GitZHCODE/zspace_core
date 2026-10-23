file(MAKE_DIRECTORY "${ZSPACE_PYTHON_PACKAGE_DIR}/core")
set(_python_runtime "${ZSPACE_PYTHON_PACKAGE_DIR}/runtime")
file(MAKE_DIRECTORY "${_python_runtime}")
file(WRITE "${ZSPACE_PYTHON_PACKAGE_DIR}/__init__.py" "import os, sys\nfrom pathlib import Path\nif sys.platform == 'win32':\n    _dll_dir = os.add_dll_directory(str(Path(__file__).parent / 'runtime'))\nfrom . import core\n")
file(WRITE "${ZSPACE_PYTHON_PACKAGE_DIR}/core/__init__.py" "from ._core import *\n")
# Native geometry bindings linked against the headless core source build.
pybind11_add_module(_core
    src/module.cpp
    src/bind_core_types.cpp
    src/bind_obj_mesh.cpp
    src/bind_fn_mesh.cpp
    src/bind_obj_graph.cpp
    src/bind_fn_graph.cpp
)

target_link_libraries(_core PRIVATE zspace::core zspace::interface)
set(ZSPACE_PYTHON_PACKAGE_DIR "${ZSPACE_PYTHON_PACKAGE_DIR}/core")
# Plain LIBRARY_/RUNTIME_OUTPUT_DIRECTORY get a per-config subdirectory
# appended by multi-config generators (VS: Release/, Debug/, ...) unless the
# per-config properties are set explicitly, so set both.
set_target_properties(_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${ZSPACE_PYTHON_PACKAGE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${ZSPACE_PYTHON_PACKAGE_DIR}"
)
foreach(_cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(_core PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_${_cfg} "${ZSPACE_PYTHON_PACKAGE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${ZSPACE_PYTHON_PACKAGE_DIR}"
    )
endforeach()

# All native runtime dependencies share one package directory.
add_custom_command(TARGET _core POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:zSpace_Core>" "$<TARGET_FILE:zSpace_Interface>"
    "${_python_runtime}" VERBATIM)

if(ZSPACE_PYTHON_WITH_USD)
  target_compile_definitions(_core PRIVATE ZSPACE_PYTHON_WITH_USD)
  target_link_libraries(_core PRIVATE zspace::io)
  add_custom_command(TARGET _core POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:zSpace_IO>" "${_python_runtime}" VERBATIM)
  zspace_deploy_runtime(TARGET _core COMPONENTS io DESTINATION "${_python_runtime}" PLUGIN_DESTINATION "${_python_runtime}/plugin")
endif()

if(ZSPACE_PYTHON_WITH_USD)
  file(MAKE_DIRECTORY "${ZSPACE_PYTHON_PACKAGE_DIR}/../io")
  file(WRITE "${ZSPACE_PYTHON_PACKAGE_DIR}/../io/__init__.py" "from zspace.core import USDdoc\n")
endif()

add_custom_command(TARGET _core POST_BUILD COMMAND ${CMAKE_COMMAND}
  "-DPACKAGE_ROOT=${ZSPACE_PYTHON_PACKAGE_DIR}/.."
  -P "${CMAKE_CURRENT_SOURCE_DIR}/../CleanLegacyRuntime.cmake" VERBATIM)
