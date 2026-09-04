@echo off
setlocal
cd /d "%~dp0"
set "PATH=C:\Users\Lenovo\anaconda3\Library\mingw-w64\bin;C:\Users\Lenovo\anaconda3\Library\bin;%PATH%"
echo =========================================================
echo   Lost & Found Management System - C++ Crow Backend
echo   Listening on http://127.0.0.1:18080
echo =========================================================
build_native\LostFoundBackend.exe
pause
