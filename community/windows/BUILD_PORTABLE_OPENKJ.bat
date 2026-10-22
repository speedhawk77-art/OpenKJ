@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title Build Portable OpenKJ (Windows)
echo ===========================================================
echo    OpenKJ Portable Build / Copy-to-Another-PC Package
 echo ===========================================================
echo.
if not exist "src\main.cpp" (
  echo ERROR: Put BOTH files beside CMakeLists.txt in OpenKJ-Source.
  goto failed
)
if not exist "build\Release\openkj.exe" (
  echo ERROR: Existing Release build was not found.
  goto failed
)
where py >nul 2>nul
if errorlevel 1 (
  echo ERROR: Python launcher ^(py -3^) not found.
  goto failed
)
py -3 PREPARE_OPENKJ_PORTABLE.py --prepare
if errorlevel 1 goto failed

set "DEVTOOLS=%ProgramFiles%\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
if not exist "%DEVTOOLS%" (
  echo ERROR: VS 2026 Developer Tools not found at:
  echo %DEVTOOLS%
  echo Run this from an x64 Native Tools Prompt, or edit DEVTOOLS here.
  goto failed
)
call "%DEVTOOLS%" -arch=x64 -host_arch=x64
if errorlevel 1 goto failed

echo.
echo Building the OpenKJ Release executable ...
cmake --build build --config Release --parallel 2
if errorlevel 1 goto failed

echo.
echo Packaging Qt and GStreamer into a new separate folder ...
py -3 PREPARE_OPENKJ_PORTABLE.py --package
if errorlevel 1 goto failed

echo.
echo DONE. Find the NEW OpenKJ-Portable-... folder in C:
 echo    OpenKJ-Development. Test it on a separate computer.
echo.
pause
exit /b 0
:failed
echo.
echo Packaging was not completed. Read the error above.
echo Original installed OpenKJ has not been touched.
pause
exit /b 1
