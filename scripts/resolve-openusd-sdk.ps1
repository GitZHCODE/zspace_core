param(
  [string]$OpenUSDRoot,
  [string]$OpenUSDPackage,
  [string]$CacheRoot = "$PSScriptRoot/../sdk/openusd",
  [string]$LockFile = "$PSScriptRoot/openusd-sdk.lock.json"
)

$ErrorActionPreference = 'Stop'
function Get-Sha256([string]$Path) {
  $stream = [IO.File]::OpenRead($Path)
  $hasher = [Security.Cryptography.SHA256]::Create()
  try {
    return [BitConverter]::ToString($hasher.ComputeHash($stream)).Replace('-', '').ToLowerInvariant()
  } finally {
    $hasher.Dispose()
    $stream.Dispose()
  }
}

$requiredFiles = @('pxrConfig.cmake', 'include/pxr/pxr.h', 'lib/usd/plugInfo.json')
if (!$OpenUSDRoot) { $OpenUSDRoot = $env:ZSPACE_OPENUSD_ROOT }
if ($OpenUSDRoot) {
  $root = [IO.Path]::GetFullPath($OpenUSDRoot)
} else {
  $lock = Get-Content -LiteralPath $LockFile -Raw | ConvertFrom-Json
  if ($lock.platform -ne 'windows-x64' -or $lock.sha256 -notmatch '^[0-9a-f]{64}$') {
    throw "Invalid OpenUSD SDK lock file: $LockFile"
  }
  if (!$OpenUSDPackage) { $OpenUSDPackage = $env:ZSPACE_OPENUSD_PACKAGE }
  if (!$OpenUSDPackage) { $OpenUSDPackage = $lock.source }
  $CacheRoot = [IO.Path]::GetFullPath($CacheRoot)
  $archiveDir = Join-Path $CacheRoot 'cache'
  $archive = Join-Path $archiveDir "$($lock.artifact)-$($lock.sha256.Substring(0, 12)).zip"
  $root = Join-Path $CacheRoot "$($lock.artifact)-$($lock.sha256.Substring(0, 12))"
  $stamp = Join-Path $root '.zspace-openusd-sha256'
  $ready = (Test-Path -LiteralPath $stamp) -and
    ((Get-Content -LiteralPath $stamp -Raw).Trim() -eq $lock.sha256)
  foreach ($relative in $requiredFiles) {
    if (!(Test-Path -LiteralPath (Join-Path $root $relative))) { $ready = $false }
  }
  if (!$ready) {
    New-Item -ItemType Directory -Path $archiveDir -Force | Out-Null
    $validArchive = (Test-Path -LiteralPath $archive) -and
      ((Get-Sha256 $archive) -ieq $lock.sha256)
    if (!$validArchive) {
      $download = "$archive.download"
      try {
        if ($OpenUSDPackage -match '^https?://') {
          Invoke-WebRequest -Uri $OpenUSDPackage -OutFile $download -UseBasicParsing
        } else {
          Copy-Item -LiteralPath $OpenUSDPackage -Destination $download -Force
        }
      } catch {
        throw "OpenUSD SDK package unavailable at '$OpenUSDPackage'. Set -OpenUSDPackage or -OpenUSDRoot. $($_.Exception.Message)"
      }
      $actual = Get-Sha256 $download
      if ($actual -ine $lock.sha256) {
        throw "OpenUSD SDK SHA-256 mismatch: expected $($lock.sha256), got $actual"
      }
      Move-Item -LiteralPath $download -Destination $archive -Force
    }
    New-Item -ItemType Directory -Path $root -Force | Out-Null
    Expand-Archive -LiteralPath $archive -DestinationPath $root -Force
  }
}

foreach ($relative in $requiredFiles) {
  if (!(Test-Path -LiteralPath (Join-Path $root $relative))) {
    throw "Incomplete native OpenUSD SDK at '$root': missing $relative"
  }
}
if ($stamp -and !$ready) {
  [IO.File]::WriteAllText($stamp, $lock.sha256, [Text.UTF8Encoding]::new($false))
}
Write-Output $root
