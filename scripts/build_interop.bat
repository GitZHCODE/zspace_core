@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%.."
set "CONFIGURATION=Release"
set "RHINO_DIR=%~1"

if "%RHINO_DIR%"=="" if not "%RhinoDir%"=="" set "RHINO_DIR=%RhinoDir%"

if "%RHINO_DIR%"=="" (
  echo Usage: scripts\build_interop.bat "C:\Path\To\Rhino\SDK"
  echo.
  echo zSpace_InterOp currently requires Rhino/OpenNURBS headers and libraries.
  echo.
  echo Expected Rhino SDK paths:
  echo   Include: %%RhinoDir%%\inc
  echo   Library: %%RhinoDir%%\lib\Release
  echo   Libraries: opennurbs.lib RhinoCore.lib RhinoLibrary.lib
  echo.
  echo Set RhinoDir before running:
  echo   set "RhinoDir=C:\Path\To\Rhino\SDK"
  echo   scripts\build_interop.bat
  echo.
  echo Or pass the SDK root directly:
  echo   scripts\build_interop.bat "C:\Path\To\Rhino\SDK"
  echo.
  echo Change the path above to the folder that contains:
  echo   inc\
  echo   lib\Release\
  echo.
  pause
  exit /b 1
)

if not exist "%RHINO_DIR%\inc" (
  echo Rhino include folder not found:
  echo   "%RHINO_DIR%\inc"
  echo.
  echo Update RhinoDir or pass the SDK root that contains inc\ and lib\Release\.
  echo.
  pause
  exit /b 1
)

if not exist "%RHINO_DIR%\lib\Release" (
  echo Rhino library folder not found:
  echo   "%RHINO_DIR%\lib\Release"
  echo.
  echo Update RhinoDir or pass the SDK root that contains inc\ and lib\Release\.
  echo.
  pause
  exit /b 1
)

set "CMAKE_EXE=cmake.exe"
where cmake.exe >nul 2>nul
if errorlevel 1 (
  set "CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)

if not exist "%CMAKE_EXE%" (
  echo cmake.exe was not found on PATH or at:
  echo "%CMAKE_EXE%"
  echo.
  pause
  exit /b 1
)

set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
  echo vcvars64.bat was not found. Install Visual Studio Build Tools or update this script.
  echo.
  pause
  exit /b 1
)

call "%VCVARS%"
if errorlevel 1 (
  set "BUILD_ERROR=%ERRORLEVEL%"
  echo.
  pause
  exit /b %BUILD_ERROR%
)

pushd "%REPO_ROOT%"
"%CMAKE_EXE%" --preset ninja-msvc -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DZSPACE_BUILD_INTEROP=ON -DZSPACE_WITH_RHINO=ON -DZSPACE_RHINO_DIR="%RHINO_DIR%"
if errorlevel 1 (
  set "BUILD_ERROR=%ERRORLEVEL%"
  popd
  echo.
  pause
  exit /b %BUILD_ERROR%
)

"%CMAKE_EXE%" --build --preset ninja-msvc-release --parallel
set "BUILD_ERROR=%ERRORLEVEL%"
popd
echo.
pause
exit /b %BUILD_ERROR%
