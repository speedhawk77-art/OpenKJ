@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"
title OpenKJ - Single EXE Installer Builder
echo ========================================================
echo OpenKJ - Single File Windows Installer Builder
echo ========================================================
echo.
echo This packages the WORKING portable folder into one
echo OpenKJ-v9-Setup.exe file.
echo.
py -3 "%~dp0MAKE_SINGLE_FILE_INSTALLER.py"
set "RESULT=%ERRORLEVEL%"
echo.
if "%RESULT%"=="0" (
  echo SUCCESS - OpenKJ-Installer-Output is next to this folder.
) else (
  echo BUILD NOT COMPLETED. Read the ERROR above.
)
echo.
pause
exit /b %RESULT%
