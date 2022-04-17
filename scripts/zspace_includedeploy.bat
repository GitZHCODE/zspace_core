if not exist "%2" mkdir %2
Xcopy %1 %2 /S /Y

If errorlevel 1 @exit 0