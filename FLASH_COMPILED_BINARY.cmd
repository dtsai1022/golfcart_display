@echo off
setlocal
cd /d "%~dp0"
set "PYTHONUTF8=1"
set "PYTHONIOENCODING=utf-8"
chcp 65001 >nul
if not exist "release\golf-cart-dashboard-complete.bin" (
    echo Missing release\golf-cart-dashboard-complete.bin
    echo Restore the release folder from the project ZIP,
    echo or build and upload with UPLOAD_FIRMWARE.cmd instead.
    pause
    exit /b 1
)
if exist ".venv\Scripts\python.exe" (
    set "PYTHON=.venv\Scripts\python.exe"
) else (
    set "PYTHON=python"
)
set "ESPTOOL_EXE=%USERPROFILE%\.platformio\penv\Scripts\esptool.exe"
set "ESPTOOL_PY=%USERPROFILE%\.platformio\packages\tool-esptoolpy\esptool.py"
set "PORT_ARGS="
if not "%~1"=="" set "PORT_ARGS=--port %~1"
echo.
echo This will replace the current CrowPanel firmware with the Golf Cart Dashboard.
echo Connect only the target display using a USB DATA cable.
if not "%~1"=="" echo Using serial port %~1
echo.
pause
if exist "%ESPTOOL_EXE%" (
    "%ESPTOOL_EXE%" --chip esp32s3 --baud 921600 %PORT_ARGS% --before default-reset --after hard-reset write-flash 0x0 "release\golf-cart-dashboard-complete.bin"
) else if exist "%ESPTOOL_PY%" (
    "%PYTHON%" "%ESPTOOL_PY%" --chip esp32s3 --baud 921600 %PORT_ARGS% --before default-reset --after hard-reset write-flash 0x0 "release\golf-cart-dashboard-complete.bin"
) else (
    esptool --chip esp32s3 --baud 921600 %PORT_ARGS% --before default-reset --after hard-reset write-flash 0x0 "release\golf-cart-dashboard-complete.bin"
)
if errorlevel 1 (
    echo.
    echo FLASH FAILED. Try a different USB data cable or enter BOOT mode manually.
    echo You can specify the port, for example: FLASH_COMPILED_BINARY.cmd COM5
) else (
    echo.
    echo FLASH COMPLETE. The dashboard should restart automatically.
)
echo.
pause
