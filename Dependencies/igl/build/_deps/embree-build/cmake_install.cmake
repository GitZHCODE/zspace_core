# Install script for directory: C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/libigl")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE FILES
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/VC/Redist/MSVC/14.29.30133/x64/Microsoft.VC142.CRT/msvcp140.dll"
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/VC/Redist/MSVC/14.29.30133/x64/Microsoft.VC142.CRT/vcruntime140_1.dll"
    "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/VC/Redist/MSVC/14.29.30133/x64/Microsoft.VC142.CRT/vcruntime140.dll"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src/include/embree3")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/libigl" TYPE FILE FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src/LICENSE.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/libigl" TYPE FILE FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src/CHANGELOG.md")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/libigl" TYPE FILE FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src/README.md")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/libigl" TYPE FILE FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src/readme.pdf")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/libigl" TYPE FILE FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src/third-party-programs.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/libigl" TYPE FILE FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-src/third-party-programs-TBB.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/embree-3.13.3" TYPE FILE RENAME "embree-config.cmake" FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-build/embree-config-install.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/embree-3.13.3" TYPE FILE FILES "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-build/embree-config-version.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-build/common/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/libigl/precompile/_deps/embree-build/kernels/cmake_install.cmake")

endif()

