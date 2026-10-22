param(
  [Parameter(Mandatory = $true)][string]$SourceRoot,
  [Parameter(Mandatory = $true)][string]$OpenUSDSourceRoot,
  [Parameter(Mandatory = $true)][string]$TbbSourceRoot,
  [Parameter(Mandatory = $true)][string]$OpenUSDCommit,
  [Parameter(Mandatory = $true)][string]$OutputZip,
  [string]$Version = '26.08'
)

$ErrorActionPreference = 'Stop'
$SourceRoot = [IO.Path]::GetFullPath($SourceRoot)
$OutputZip = [IO.Path]::GetFullPath($OutputZip)
$stage = Join-Path "$PSScriptRoot/../build_zspace_v3" ('openusd-package-stage-' + [guid]::NewGuid().ToString('N'))

foreach ($required in @(
  "$SourceRoot/pxrConfig.cmake",
  "$SourceRoot/lib/pkgconfig/tbb.pc",
  "$SourceRoot/lib/usd/plugInfo.json",
  "$OpenUSDSourceRoot/LICENSE.txt",
  "$OpenUSDSourceRoot/NOTICE.txt",
  "$TbbSourceRoot/LICENSE.txt"
)) {
  if (!(Test-Path -LiteralPath $required)) { throw "Missing package input: $required" }
}
if (Test-Path -LiteralPath $OutputZip) { throw "Package already exists: $OutputZip" }

New-Item -ItemType Directory -Path $stage -Force | Out-Null
Get-ChildItem -LiteralPath $SourceRoot -Force | Copy-Item -Destination $stage -Recurse -Force

# The locally installed SDK embeds its original install path in these two files.
# Rewrite both to refer to the extracted SDK root before publishing the archive.
$sourcePrefix = $SourceRoot.Replace('\', '/')
$configPath = Join-Path $stage 'pxrConfig.cmake'
$config = [IO.File]::ReadAllText($configPath)
if (!$config.Contains($sourcePrefix)) { throw "Expected SDK prefix not found in $configPath" }
$config = $config.Replace($sourcePrefix, '${PXR_CMAKE_DIR}')
[IO.File]::WriteAllText($configPath, $config, [Text.UTF8Encoding]::new($false))

$pkgConfigPath = Join-Path $stage 'lib/pkgconfig/tbb.pc'
$pkgConfig = [IO.File]::ReadAllText($pkgConfigPath)
if (!$pkgConfig.Contains("prefix=$sourcePrefix")) { throw "Expected SDK prefix not found in $pkgConfigPath" }
$pkgConfig = $pkgConfig.Replace("prefix=$sourcePrefix", 'prefix=${pcfiledir}/../..')
[IO.File]::WriteAllText($pkgConfigPath, $pkgConfig, [Text.UTF8Encoding]::new($false))

$openusdLicenses = Join-Path $stage 'licenses/OpenUSD'
$tbbLicenses = Join-Path $stage 'licenses/oneTBB'
New-Item -ItemType Directory -Path $openusdLicenses, $tbbLicenses -Force | Out-Null
Copy-Item -LiteralPath "$OpenUSDSourceRoot/LICENSE.txt", "$OpenUSDSourceRoot/NOTICE.txt" -Destination $openusdLicenses
Copy-Item -LiteralPath "$TbbSourceRoot/LICENSE.txt" -Destination $tbbLicenses

$provenance = [ordered]@{
  artifact = "openusd-native-$Version-windows-x64"
  createdUtc = [DateTime]::UtcNow.ToString('o')
  openusdRepository = 'https://github.com/PixarAnimationStudios/OpenUSD'
  openusdCommit = $OpenUSDCommit
  openusdVersion = $Version
  tbbVersion = '2021.12.0'
  platform = 'Windows x64'
  relocation = 'pxrConfig.cmake and lib/pkgconfig/tbb.pc use relative paths'
}
[IO.File]::WriteAllText((Join-Path $stage 'SDK-PROVENANCE.json'),
  ($provenance | ConvertTo-Json -Depth 3), [Text.UTF8Encoding]::new($false))

$parent = Split-Path -Parent $OutputZip
New-Item -ItemType Directory -Path $parent -Force | Out-Null
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($stage, $OutputZip,
  [IO.Compression.CompressionLevel]::Optimal, $false)
$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $OutputZip).Hash.ToLowerInvariant()
Write-Host "Package: $OutputZip"
Write-Host "SHA-256: $hash"
