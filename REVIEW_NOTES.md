# Review and test notes — Basic 7-inch dashboard

## Target hardware

This firmware is for the **Elecrow CrowPanel Basic 7.0-inch V3.0, 800×480 display, SKU DIS08070H**, using an **ESP32-S3-WROOM-1-N4R8** with **4 MB flash and 8 MB octal PSRAM**. It is not for the Advanced/AI or ESP32-P4 versions, other CrowPanel sizes, or a generic ESP32 board.

The physical board was identified by its `ESP32 DISPLAY 7IN`, `SKU: DIS08070H`, and `ESP32-S3-WROOM-1` markings. Esptool detected an ESP32-S3 revision v0.2 with 4 MB flash at `USB-SERIAL CH340 (COM5)`.

## Corrections made

1. **LVGL time base:** added `lv_tick_inc(now - last_tick_ms)` and timestamp bookkeeping. The original source did not advance LVGL's clock, preventing normal refreshes, animations, and input handling.
2. **Correct board:** replaced the original 5-inch pinout and timing with Elecrow's Basic 7-inch configuration: HENABLE GPIO41, VSYNC GPIO40, HSYNC GPIO39, PCLK GPIO0; 800×480 RGB565; backlight GPIO2.
3. **Working GT911 touch:** uses one Arduino `Wire` bus on SDA19/SCL20, the PCA9557 reset sequence at `0x18`, and GT911 register reads. The controller was found at **`0x5D`** on this board. The earlier conflict between separate I²C controller instances was removed.
4. **Stable display:** upgraded to Arduino-ESP32 **3.3.8**, pioarduino platform **55.3.38**, LVGL **8.3.11**, and LovyanGFX **1.2.25**, with Elecrow's dual-framebuffer/VSYNC driver patch. Both complete framebuffers reside in PSRAM, and LVGL uses full-screen refresh with two buffers.
5. **Artifact fix:** set the RGB pixel clock to **16 MHz**. The physically tested 24 MHz configuration left persistent fragments on the left side. At 16 MHz, the user confirmed that the fragments were gone, the display was stable, and touch still worked. Keep this setting; do not use experimental direct-mode or frame-completion-callback variants from troubleshooting.
6. **Cleanup:** removed the temporary I²C/touch diagnostic overlay and restored the normal `VEHICLE MONITOR` subtitle.
7. **Telemetry convention:** corrected the current comment to **positive = charging**; battery power remains positive = discharge.

The uploaded original `golf-cart-dashboard-project.zip` was never modified. The earlier `golf-cart-dashboard-reviewed.zip` targets the 5-inch board and must not be used on this display.

## Verification performed

- Clean `crowpanel-7` release build succeeded and was uploaded to the physical board on COM5.
- Bootloader, partition table, boot-selection data, and application all passed `Hash of data verified` during upload.
- Esptool subsequently compared each programmed region with the exact final build files; every region reported **`Verification successful (digest matched)`**.
- The combined release image contains the exact final bootloader at `0x0000`, partition table at `0x8000`, and application at `0x10000`. Bootloader and partition signatures are valid.
- Release SHA-256 values were regenerated from the final clean build.
- Final resource use: **92,328 / 327,680 bytes RAM (28.2%)** and **969,431 / 4,194,304 bytes application flash (23.1%)**.
- Physical testing confirmed a stable display without the left-side fragments, working touch, and the corrected three-page dashboard.

> A whole-file `verify-flash 0x0 firmware.factory.bin` comparison can report differences in unused gaps: the generated merged image pads those gaps with zeroes, while esptool leaves skipped flash gaps unchanged. Verifying the four actual programmed regions separately avoids that false mismatch.

## Flashing on this computer

Python, PlatformIO, esptool **5.2.0**, and the CH340 driver are working. No Arduino IDE is needed. The display is **COM5**; COM3/COM4 are unrelated Bluetooth serial links.

1. Connect only the target display using a known-good USB data cable.
2. Use `FLASH_COMPILED_BINARY.cmd COM5`, or build and upload with PlatformIO:

   ```powershell
   $env:PYTHONUTF8 = '1'
   $env:PYTHONIOENCODING = 'utf-8'
   & .\.venv\Scripts\pio.exe run -e crowpanel-7 -t upload --upload-port COM5
   ```

3. If automatic download mode fails, hold **BOOT**, tap **RESET**, release **BOOT**, and retry.
4. Press **RESET** if the dashboard does not restart automatically.
5. Confirm OVERVIEW, SYSTEMS, V2V LAB, SWAP DEMO SOC, and return to OVERVIEW.

Use `release/golf-cart-dashboard-complete.bin` at offset `0x0` for ordinary USB flashing. The app-only image is for an appropriate application-partition update, not a blank board.

The pre-dashboard backup remains in the original work area at `backups/crowpanel-7-before-dashboard.bin` (SHA-256 `bf94cce36c3c6c31a52435f80d0ca523352f861a31d854f4d43c4f3d8cd43ccd`). It is intentionally **not included in the clean release ZIP**. It is a backup of whatever was installed before this work, not a guaranteed Elecrow factory image.

## Important limitation

This is a **simulated dashboard demo**, not live golf-cart instrumentation. It does not read a BMS, motor controller, solar charge controller, speed sensor, or second cart, and it does not control charging, relays, a motor, or V2V conversion hardware. Live telemetry and vehicle power interfaces require separate hardware and software work.
