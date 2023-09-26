# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.20.21032501-MSVC_2)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(gmp-populate NONE)



include(ExternalProject)
ExternalProject_Add(gmp-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://cgal.geometryfactory.com/CGAL/precompiled_libs/auxiliary/x64/GMP/5.0.1/gmp-all-CGAL-3.9.zip" "URL_MD5" "508c1292319c832609329116a8234c9f"
                    SOURCE_DIR          "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/gmp-src"
                    BINARY_DIR          "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/gmp-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)


