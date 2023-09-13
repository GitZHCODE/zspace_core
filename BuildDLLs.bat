
call premake\premake5.exe vs2019

start /d "%programfiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin" MSBuild.exe C:\Users\Vlad.Levyant\source\repos\zspace_core\zSpace_core.sln /t:zSpace_core /p:Configuration="Release_DLL" /p:Platform="x64"

Pause