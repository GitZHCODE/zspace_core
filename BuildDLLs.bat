@echo off
set arg0=%~0

if "%arg0:~0,2%" == "C:" GOTO zcore
if "%arg0:~0,2%" == "De" GOTO other

:zcore
call premake\premake5.exe vs2019

echo Building zSpace_Core Release_DLL...

set start=%time%
start /d "%programfiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin" /wait MSBuild.exe %cd%\zSpace_core.sln /t:zSpace_Core /p:Configuration="Release_DLL" /p:Platform="x64"
set end=%time%
set /a start = (1%start:~0,2%-100)*3600 + (1%start:~3,2%-100)*60 + (1%start:~6,2%-100)
set /a end = (1%end:~0,2%-100)*3600 + (1%end:~3,2%-100)*60 + (1%end:~6,2%-100)
set /a delta=%end%-%start%
echo Done (%delta%s).


echo Building zSpace_Interface Release_DLL

set start=%time%
start /d "%programfiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin" /wait MSBuild.exe %cd%\zSpace_core.sln /t:zSpace_Interface /p:Configuration="Release_DLL" /p:Platform="x64"
set end=%time%
set /a start = (1%start:~0,2%-100)*3600 + (1%start:~3,2%-100)*60 + (1%start:~6,2%-100)
set /a end = (1%end:~0,2%-100)*3600 + (1%end:~3,2%-100)*60 + (1%end:~6,2%-100)
set /a delta=%end%-%start%
echo Done (%delta%s).

pause
GOTO end

:other
call premake\premake5.exe --file=buildDeps.lua vs2019

echo Building zSpace_Core Release_DLL...

set start=%time%
start /d "%programfiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin" /wait MSBuild.exe %cd%\Dependencies\ZSPACE\zSpace_core.sln /t:zSpace_Core /p:Configuration="Release_DLL" /p:Platform="x64"
set end=%time%
set /a start = (1%start:~0,2%-100)*3600 + (1%start:~3,2%-100)*60 + (1%start:~6,2%-100)
set /a end = (1%end:~0,2%-100)*3600 + (1%end:~3,2%-100)*60 + (1%end:~6,2%-100)
set /a delta=%end%-%start%
echo Done (%delta%s).

echo Building zSpace_Interface Release_DLL

set start=%time%
start /d "%programfiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin" /wait MSBuild.exe %cd%\Dependencies\ZSPACE\zSpace_core.sln /t:zSpace_Interface /p:Configuration="Release_DLL" /p:Platform="x64"
set end=%time%
set /a start = (1%start:~0,2%-100)*3600 + (1%start:~3,2%-100)*60 + (1%start:~6,2%-100)
set /a end = (1%end:~0,2%-100)*3600 + (1%end:~3,2%-100)*60 + (1%end:~6,2%-100)
set /a delta=%end%-%start%
echo Done (%delta%s).

goto :end

:end