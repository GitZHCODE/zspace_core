if not exist "%2" mkdir %2
Xcopy %1\*.lib %2 /S /Y
Xcopy %1\*.dll %2 /S /Y

If errorlevel 1 @exit 0