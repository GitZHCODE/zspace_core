param(
  [string]$EmsdkRoot = (Join-Path $env:USERPROFILE "emsdk"),
  [int]$Jobs = 6
)
$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$EmsdkRoot = (Resolve-Path -LiteralPath $EmsdkRoot).Path
$emscripten = Join-Path $EmsdkRoot "upstream/emscripten"
$pythonDir = Get-ChildItem (Join-Path $EmsdkRoot "python") -Directory |
  Where-Object { Test-Path (Join-Path $_.FullName "python.exe") } |
  Sort-Object Name -Descending | Select-Object -First 1
$nodeDir = Get-ChildItem (Join-Path $EmsdkRoot "node") -Directory |
  Where-Object { Test-Path (Join-Path $_.FullName "node.exe") } |
  Sort-Object Name -Descending | Select-Object -First 1
if (!$pythonDir -or !$nodeDir) { throw "Install Emscripten's Python and Node runtimes first." }
$env:PATH = "$emscripten;$($pythonDir.FullName);$($nodeDir.FullName);$env:PATH"
$env:EMSDK = $EmsdkRoot
$env:EM_CONFIG = Join-Path $EmsdkRoot ".emscripten"
$env:EMSDK_PYTHON = Join-Path $pythonDir.FullName "python.exe"
$env:EM_CACHE = Join-Path $repoRoot "wasm/cache"
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$toolchain = Join-Path $emscripten "cmake/Modules/Platform/Emscripten.cmake"
& $cmake -S (Join-Path $repoRoot "wasm/openusd") -B (Join-Path $repoRoot "wasm/build/openusd") `
  -Upxr_DIR -UTBB_DIR `
  -G Ninja "-DCMAKE_TOOLCHAIN_FILE=$toolchain" `
  "-DCMAKE_BUILD_TYPE=Release"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $cmake --build (Join-Path $repoRoot "wasm/build/openusd") --parallel $Jobs
exit $LASTEXITCODE
