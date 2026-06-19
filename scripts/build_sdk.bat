@echo off
setlocal

set "CONFIG=Release"
set "BUILD_DIR=%~dp0..\build\sdk-msvc"
set "INSTALL_DIR=%~dp0..\build\sdk"
set "CMAKE_EXE=cmake"
set "BUILD_TOOL_ARGS=/m /clp:ErrorsOnly"

where cmake >nul 2>nul
if errorlevel 1 (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    ) else (
        echo [zspace] CMake was not found.
        goto :fail
    )
)

pushd "%~dp0.."
if errorlevel 1 goto :fail

echo.
echo [zspace] Building binary SDK
echo [zspace] install: "%INSTALL_DIR%"
echo.

"%CMAKE_EXE%" -S . -B "%BUILD_DIR%" -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" -DZSPACE_BUILD_INTERFACE=ON -DZSPACE_BUILD_IO=ON -DZSPACE_BUILD_DISPLAY=OFF -DZSPACE_BUILD_INTEROP=OFF -DZSPACE_BUILD_TESTS=OFF -DZSPACE_BUILD_SHARED=ON -DZSPACE_WITH_OPENGL=OFF
if errorlevel 1 goto :fail_pop

"%CMAKE_EXE%" --build "%BUILD_DIR%" --config %CONFIG% -- %BUILD_TOOL_ARGS%
if errorlevel 1 goto :fail_pop

"%CMAKE_EXE%" --install "%BUILD_DIR%" --config %CONFIG%
if errorlevel 1 goto :fail_pop

echo.
echo [zspace] SDK created successfully at "%INSTALL_DIR%".
popd
pause
exit /b 0

:fail_pop
popd

:fail
echo.
echo [zspace] SDK build failed.
pause
exit /b 1
