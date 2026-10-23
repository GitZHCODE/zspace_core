@echo off
rem Typed USD scene exports are discovered from ZSPACE_WASM_EXPORT declarations.
rem The SDK-free runtime reports disabled support; build-openusd-wasm.ps1 supplies USD.
setlocal EnableExtensions

set "REPO_ROOT=%~dp0..\.."
for %%I in ("%REPO_ROOT%") do set "REPO_ROOT=%%~fI"

set "WASM_ROOT=%REPO_ROOT%\wasm"
set "BRIDGE_DIR=%WASM_ROOT%\bridge"
if not defined ZSPACE_WASM_BUILD_DIR set "ZSPACE_WASM_BUILD_DIR=%WASM_ROOT%\build\standard"
set "WASM_BUILD=%ZSPACE_WASM_BUILD_DIR%"
set "WASM_TMP=%WASM_ROOT%\tmp"
set "WASM_CACHE=%WASM_ROOT%\cache"
if not defined ZSPACE_WASM_OUT_DIR set "ZSPACE_WASM_OUT_DIR=%WASM_ROOT%\out"
set "WASM_OUT=%ZSPACE_WASM_OUT_DIR%"
set "EMSDK_DIR=%USERPROFILE%\emsdk"
set "EMSCRIPTEN_DIR=%EMSDK_DIR%\upstream\emscripten"
set "EMSDK_PYTHON_DIR=%EMSDK_DIR%\python\3.13.3_64bit"
set "EMSDK_PYTHON_EXE=%EMSDK_PYTHON_DIR%\python.exe"

if exist "%EMSCRIPTEN_DIR%\em++.exe" set "PATH=%EMSCRIPTEN_DIR%;%PATH%"
if exist "%EMSDK_DIR%\node\24.19.0_64bit\bin" set "PATH=%EMSDK_DIR%\node\24.19.0_64bit\bin;%PATH%"
if exist "%EMSDK_PYTHON_EXE%" set "PATH=%EMSDK_PYTHON_DIR%;%PATH%"

if exist "%EMSDK_DIR%\.emscripten" (
  set "EMSDK=%EMSDK_DIR%"
  set "EM_CONFIG=%EMSDK_DIR%\.emscripten"
)
if exist "%EMSDK_PYTHON_EXE%" (
  set "PYTHON=%EMSDK_PYTHON_EXE%"
  set "EMSDK_PYTHON=%EMSDK_PYTHON_EXE%"
)

where em++ >nul 2>nul || (
  echo ERROR: em++ was not found. Install Emscripten or run emsdk_env.bat first.
  exit /b 1
)

if not exist "%WASM_BUILD%" mkdir "%WASM_BUILD%"
if not exist "%WASM_TMP%" mkdir "%WASM_TMP%"
if not exist "%WASM_CACHE%" mkdir "%WASM_CACHE%"
if not exist "%WASM_OUT%" mkdir "%WASM_OUT%"

set "TEMP=%WASM_TMP%"
set "TMP=%WASM_TMP%"
set "EMCC_TEMP_DIR=%WASM_TMP%"
set "EM_CACHE=%WASM_CACHE%"

rem Ninja retains project objects and compiler-generated header dependencies.
rem Use available processors up to sixteen; callers can tune throughput/memory.
set "ZSPACE_WASM_DEFAULT_JOBS=6"
if defined NUMBER_OF_PROCESSORS set "ZSPACE_WASM_DEFAULT_JOBS=%NUMBER_OF_PROCESSORS%"
if %ZSPACE_WASM_DEFAULT_JOBS% GTR 16 set "ZSPACE_WASM_DEFAULT_JOBS=16"
if not defined ZSPACE_WASM_JOBS set "ZSPACE_WASM_JOBS=%ZSPACE_WASM_DEFAULT_JOBS%"
if not defined ZSPACE_WASM_USE_PCH set "ZSPACE_WASM_USE_PCH=ON"
if not defined ZSPACE_WASM_FAST_LINK set "ZSPACE_WASM_FAST_LINK=OFF"
echo(%ZSPACE_WASM_JOBS%| findstr /r /x "[1-9][0-9]*" >nul || (
  echo ERROR: ZSPACE_WASM_JOBS must be a positive integer.
  exit /b 1
)
where cmake >nul 2>nul || (
  echo ERROR: cmake was not found. Install CMake and add it to PATH.
  exit /b 1
)
where ninja >nul 2>nul || (
  echo ERROR: ninja was not found. Install Ninja and add it to PATH.
  exit /b 1
)
where emcmake >nul 2>nul || (
  echo ERROR: emcmake was not found. Run emsdk_env.bat first.
  exit /b 1
)

set "ZSPACE_WASM_STAMP_FILE=%WASM_BUILD%\zspace_wasm_config.stamp"
set "ZSPACE_WASM_EXPECTED_STAMP=PCH=%ZSPACE_WASM_USE_PCH%;OUT=%WASM_OUT%;FAST_LINK=%ZSPACE_WASM_FAST_LINK%"
set "ZSPACE_WASM_CURRENT_STAMP="
if exist "%ZSPACE_WASM_STAMP_FILE%" set /p ZSPACE_WASM_CURRENT_STAMP=<"%ZSPACE_WASM_STAMP_FILE%"
set "ZSPACE_WASM_NEED_CONFIGURE=1"
if not defined ZSPACE_WASM_FORCE_CONFIGURE if exist "%WASM_BUILD%\build.ninja" if "%ZSPACE_WASM_CURRENT_STAMP%"=="%ZSPACE_WASM_EXPECTED_STAMP%" set "ZSPACE_WASM_NEED_CONFIGURE=0"

rem Explicitly avoid CMake Release's -O3/-DNDEBUG defaults and inherited CXXFLAGS.
rem The target owns the existing -O2, C++17, exception and export settings.
if "%ZSPACE_WASM_NEED_CONFIGURE%"=="1" (
  call emcmake cmake -S "%WASM_ROOT%" -B "%WASM_BUILD%" -G Ninja ^
    "-DCMAKE_BUILD_TYPE=" "-DCMAKE_CXX_FLAGS=" ^
    "-DZSPACE_WASM_USE_PCH=%ZSPACE_WASM_USE_PCH%" ^
    "-DZSPACE_WASM_FAST_LINK=%ZSPACE_WASM_FAST_LINK%" ^
    "-DZSPACE_WASM_OUT_DIR=%WASM_OUT%"
  if errorlevel 1 exit /b 1
  >"%ZSPACE_WASM_STAMP_FILE%" echo(%ZSPACE_WASM_EXPECTED_STAMP%
)

cmake --build "%WASM_BUILD%" --parallel %ZSPACE_WASM_JOBS%
if errorlevel 1 exit /b 1
exit /b 0
