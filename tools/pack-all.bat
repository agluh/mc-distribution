@echo off
Setlocal EnableDelayedExpansion
Setlocal EnableExtensions

set "WORK_DIR=%~dp0"

for %%A in ("%~dp0\..") do set "WORK_DIR=%%~fA"

set "SRC_DIR=%WORK_DIR%\Release"
set "OUT_DIR=%SRC_DIR%\Packed"

IF not exist "%OUT_DIR%" (mkdir "%OUT_DIR%")

for %%f in ("%SRC_DIR%\*.exe") do (
    set "FILE_NAME=%%~nf%%~xf"
    echo Pack file '!FILE_NAME!'
	upx.exe -9 -f --ultra-brute -k --lzma --all-methods --all-filters -o "%OUT_DIR%\!FILE_NAME!" "%%f"
)
