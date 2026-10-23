param(
  [Parameter(Mandatory=$true)][string]$PythonExecutable,
  [string]$Pybind11Dir,
  [string]$BuildDirectory,
  [switch]$WithUSD
)
$ErrorActionPreference = "Stop"
$source = $PSScriptRoot
$version = & $PythonExecutable -c "import sys; print(str(sys.version_info.major)+str(sys.version_info.minor))"
if ($LASTEXITCODE -ne 0) { throw "Python could not run" }
if ($version -notin @("310", "313")) { throw "Use Python 3.10 or 3.13" }
if (-not $BuildDirectory) { $BuildDirectory = Join-Path $source "build_headless$version" }
if (-not $Pybind11Dir) {
  $Pybind11Dir = & $PythonExecutable -m pybind11 --cmakedir
  if ($LASTEXITCODE -ne 0) { throw "Install pybind11 or specify Pybind11Dir" }
}
$usdOption = if ($WithUSD) { "ON" } else { "OFF" }
cmake -S $source -B $BuildDirectory -G "Visual Studio 17 2022" -A x64 "-DPython_EXECUTABLE=$PythonExecutable" "-Dpybind11_DIR=$Pybind11Dir" "-DZSPACE_PYTHON_WITH_USD=$usdOption"
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
cmake --build $BuildDirectory --config Release --parallel 4
if ($LASTEXITCODE -ne 0) { throw "Build failed" }
Write-Output "Package: $BuildDirectory/package"
