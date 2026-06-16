# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.20.21032501-MSVC_2)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(mpfr-populate NONE)



include(ExternalProject)
ExternalProject_Add(mpfr-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://cgal.geometryfactory.com/CGAL/precompiled_libs/auxiliary/x64/MPFR/3.0.0/mpfr-all-CGAL-3.9.zip" "URL_MD5" "48840454eef0ff18730050c05028734b"
                    SOURCE_DIR          "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/mpfr-src"
                    BINARY_DIR          "C:/Users/vishu.b/source/repos/libigl/precompile/_deps/mpfr-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)


