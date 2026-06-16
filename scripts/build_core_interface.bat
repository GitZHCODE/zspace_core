@echo off
setlocal

set "SCRIPT_DIR=%~dp0"

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%build.ps1" -Configuration Release
set "BUILD_ERROR=%ERRORLEVEL%"

echo.
pause
exit /b %BUILD_ERROR%
