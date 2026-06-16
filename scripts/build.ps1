param(
  [ValidateSet("Debug", "Release")]
  [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$preset = "ninja-msvc"
$buildPreset = if ($Configuration -eq "Debug") { "ninja-msvc-debug" } else { "ninja-msvc-release" }

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
  $cmakeExe = $cmake.Source
} else {
  $cmakeExe = "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
}

if (!(Test-Path $cmakeExe)) {
  throw "cmake.exe was not found on PATH or at the Visual Studio bundled location: $cmakeExe"
}

$vcvars = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
if (!(Test-Path $vcvars)) {
  throw "vcvars64.bat was not found. Install Visual Studio Build Tools or update scripts/build.ps1 with your compiler environment setup."
}

$command = "`"$vcvars`" && `"$cmakeExe`" --preset $preset -DCMAKE_BUILD_TYPE=$Configuration && `"$cmakeExe`" --build --preset $buildPreset --parallel"
Push-Location $repoRoot
try {
  cmd.exe /d /s /c $command
} finally {
  Pop-Location
}
if ($LASTEXITCODE -ne 0) {
  exit $LASTEXITCODE
}
