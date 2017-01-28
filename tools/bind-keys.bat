@echo off
Setlocal EnableDelayedExpansion
Setlocal EnableExtensions

set "WORK_DIR=%~dp0"

for %%A in ("%~dp0\..") do set "WORK_DIR=%%~fA"

mklink /D "%WORK_DIR%\mc_create\resources\keys" ..\..\keys
mklink /D "%WORK_DIR%\mc_keygen\resources\keys" ..\..\keys
mklink /D "%WORK_DIR%\mc_view\resources\keys" ..\..\keys