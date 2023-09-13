@echo off
set arg0=%~0

if "%arg0:~0,2%" == "C:" GOTO zcore
if "%arg0:~0,2%" == "De" GOTO other

:zcore
call premake\premake5.exe vs2019

start /d "%programfiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin" MSBuild.exe %cd%\zSpace_core.sln /t:zSpace_core /p:Configuration="Release_DLL" /p:Platform="x64"
pause
GOTO end

:other

call premake\premake5.exe --file=buildDeps.lua vs2019

start /d "%programfiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin" MSBuild.exe %cd%\Dependencies\ZSPACE\zSpace_core.sln /t:zSpace_core /p:Configuration="Release_DLL" /p:Platform="x64"
goto :end

:end