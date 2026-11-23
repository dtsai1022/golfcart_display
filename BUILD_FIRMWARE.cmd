@echo off
setlocal
cd /d "%~dp0"
set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
chcp 65001 >nul
if exist ".venv\Scripts\pio.exe" (
    set "PIO=.venv\Scripts\pio.exe"
) else (
    set "PIO=pio"
)
echo Building Golf Cart Dashboard...
%PIO% run -e crowpanel-7
if errorlevel 1 (
    echo.
    echo BUILD FAILED. Review the messages above.
) else (
    echo.
    echo BUILD SUCCESSFUL.
    echo Firmware: .pio\build\crowpanel-7\firmware.bin
)
echo.
pause
