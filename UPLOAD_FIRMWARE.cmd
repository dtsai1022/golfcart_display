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
set "PORT_ARGS="
if not "%~1"=="" set "PORT_ARGS=--upload-port %~1"
echo.
echo Golf Cart Dashboard upload
echo Connect the CrowPanel to this PC with a USB DATA cable.
echo Disconnect other ESP32 boards so PlatformIO picks the correct port.
echo.
pause
%PIO% run -e crowpanel-7 -t upload %PORT_ARGS%
if errorlevel 1 (
    echo.
    echo UPLOAD FAILED OR WAS CANCELLED.
    echo If download mode did not start: hold BOOT, tap RESET, release BOOT,
    echo then run this script again. Try another USB data cable if no port appears.
) else (
    echo.
    echo UPLOAD COMPLETE. Press RESET if the dashboard does not start itself.
)
echo.
pause
