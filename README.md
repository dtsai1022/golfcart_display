# Golf Cart Dashboard — Elecrow CrowPanel 7.0"

A three-page, touch-enabled LVGL dashboard for the **Elecrow CrowPanel Basic 7.0-inch V3.0, 800×480 ESP32-S3 HMI display (SKU DIS08070H)**. It has been physically tested on that board: both framebuffers in PSRAM, a 16 MHz RGB clock, and direct GT911 touch reads provide a stable display and working touch.

The project uses **Arduino-ESP32 3.3.8**, **LVGL 8.3.11**, and **LovyanGFX 1.2.25** with Elecrow's dual-framebuffer driver patch. It is designed around the style of the supplied references, with fewer dense diagnostics and larger driving information.

## What works now

- **Overview:** animated speed, range, battery SOC, pack power, solar, motor power, regenerative power, and moving energy-flow indicators.
- **Systems:** simplified battery, motor, solar, cell delta, torque, temperature, and health readouts.
- **V2V Lab:** clearly labelled **WORK IN PROGRESS**, animated donor-to-receiver direction, and a button to swap simulated SOC values. Direction is selected from the higher-SOC pack.
- Values update ten times per second from a built-in simulator. **No live BMS or charger is required to try the UI.**
- Touch navigation uses large buttons along the bottom.

> The numbers are simulated. This firmware does not measure a battery and does not control a motor, charger, contactor, relay, or V2V conversion hardware.

## Easiest upload (Windows)

1. Install [Visual Studio Code](https://code.visualstudio.com/) and the **PlatformIO IDE** extension. Alternatively, install Python and run `python -m pip install --user platformio`.
2. Connect the display using a known-good **USB data cable**. Some charging-only cables will not work.
3. Open this folder in VS Code and let PlatformIO finish installing its build tools. From a terminal in this folder, run:

   ```powershell
   pio run -e crowpanel-7 -t upload
   ```

   To select a port explicitly, for example the CH340 port on the test PC:

   ```powershell
   pio run -e crowpanel-7 -t upload --upload-port COM5
   ```

   You can instead double-click **`UPLOAD_FIRMWARE.cmd`**, or run **`UPLOAD_FIRMWARE.cmd COM5`** to select a specific port. It uses the project's local PlatformIO environment when present, otherwise the `pio` command on your PATH. No Arduino IDE is required. The CH340 COM port must appear in Windows Device Manager before uploading; COM3/COM4 Bluetooth links are not the display.
4. If the board does not enter download mode automatically, hold **BOOT**, tap **RESET**, release **BOOT**, and retry. V2 and V3 boards normally support automatic download.
5. Press **RESET** after a successful upload if the screen does not restart automatically.

To compile without uploading:

```powershell
pio run -e crowpanel-7
```

or double-click **`BUILD_FIRMWARE.cmd`**.

Output firmware: `.pio/build/crowpanel-7/firmware.bin` (application only) and `.pio/build/crowpanel-7/firmware.factory.bin` (combined USB-flashing image).

## Flash the compiled image without rebuilding

`release/golf-cart-dashboard-complete.bin` is a combined ESP32-S3 USB-flashing image for the board's 4 MB flash (bootloader, partitions, boot selection, and application). Double-click **`FLASH_COMPILED_BINARY.cmd`** on the build PC, or run it with a specific port such as `FLASH_COMPILED_BINARY.cmd COM5`.

On another PC with Python, install Espressif's uploader and run:

```powershell
python -m pip install esptool
esptool --chip esp32s3 --baud 921600 --before default-reset --after hard-reset write-flash 0x0 release/golf-cart-dashboard-complete.bin
```

`release/golf-cart-dashboard-app.bin` is only the application partition for advanced/OTA use; normal USB flashing should use the **complete** image above.

## Firmware environments

`platformio.ini` defines two environments that share the same board configuration:

| Environment | Source | Purpose |
|---|---|---|
| `crowpanel-7` | `src/main.cpp` + dashboard files | The dashboard (default). `pio run -e crowpanel-7 -t upload` |
| `sensor-test` | `src/sensor_test_main.cpp` | Standalone WCS1800 Hall-sensor test: shows the module's Dout state on IO38 (GPIO_D connector). `pio run -e sensor-test -t upload` |

Always pass `-e <env>`. A bare `pio run -t upload` builds **both** environments and flashes them in sequence, so the sensor test would overwrite the dashboard.

### Current-sensor hardware notes (WCS1800 module)

- The WCS1800 breakout is **analog-only** (Aout proportional to current, plus a comparator Dout with a trim-pot threshold). It has no I²C interface and no address to set.
- The CrowPanel 7.0" exposes **no free ADC-capable GPIO**: GPIO38 (GPIO_D) is digital-only, and GPIO19/20 are the touch I²C bus. To read Aout, add an **ADS1115** I²C ADC on the I²C port (address `0x48`, which does not conflict with the GT911 at `0x5D` or the PCA9557 at `0x18`).
- Power the module from **3.3 V**, or from 5 V **with a resistor divider on Dout** — ESP32-S3 pins are not 5 V-tolerant.
- The module measures roughly **±35 A**. A golf-cart traction circuit can exceed that, so size the sensor for the circuit you actually measure.

## Controls

- Tap **OVERVIEW**, **SYSTEMS**, or **V2V LAB** to change pages.
- On V2V Lab, tap **SWAP DEMO SOC** to reverse which pack is fuller and watch the proposed energy direction change.
- A small SOC gap (under 5 percentage points) displays a **hold / no transfer** recommendation. This is only a UI rule for the demo, not a validated charging algorithm.

## Hardware target and assumptions

This project uses the pinout published in Elecrow's official CrowPanel 7.0" repository:

- ESP32-S3-WROOM-1-N4R8
- 800×480 RGB panel, 4 MB flash, 8 MB octal PSRAM
- Two complete 800×480 RGB565 framebuffers, 16 MHz pixel clock, VSYNC buffer switching
- GT911 capacitive touch at I²C SDA 19 / SCL 20, using controller address `0x5D` on the tested V3 board
- Backlight GPIO 2
- V3 touch timing/reset sequence through PCA9557 at I²C address `0x18`

Check the hardware version printed on your board. This project is intended for the **Basic** 7.0" CrowPanel, **not** the newer Advanced/AI or ESP32-P4 versions, which use different hardware.

## Replace demo values with live telemetry

The UI consumes one `TelemetryData` structure, declared in [`src/telemetry.h`](src/telemetry.h). The screen code does not read sensors directly.

The present data source is `SimulatedTelemetry::update()` in [`src/telemetry.cpp`](src/telemetry.cpp). For live data:

1. Create a `TelemetrySource` implementation that updates all fields you support.
2. Set `data_valid = false` when data is stale or invalid, rather than presenting old data as live.
3. Replace `SimulatedTelemetry telemetry_source;` in [`src/main.cpp`](src/main.cpp) with your live source.
4. Add protocol parsing (CAN/TWAI, UART/RS-485, ESP-NOW, etc.) independently of `dashboard_ui.cpp`.

For a real vehicle, the next design inputs are:

- BMS brand/model and its protocol or documentation
- Pack chemistry, series-cell count, nominal voltage, and capacity
- Motor controller brand/model and available speed/RPM/regen signals
- Solar charge controller model and MPPT telemetry interface
- Which cart/module supplies 5 V to the display and how grounds/isolation are handled

**Never connect pack voltage or a CAN/RS-485 transceiver signal directly to ESP32 GPIO.** Use the correct isolated/protected interface and a properly fused DC/DC supply. A safe hardware interface should be reviewed before connection to the cart.

## V2V boundary

V2V Lab is a **visual concept only**. Real cart-to-cart transfer needs, at minimum, compatible BMS permissions, voltage/SoC limits, precharge, contactor control, fault handling, emergency shutdown, appropriate connectors/cabling, and a suitably rated isolated bidirectional DC/DC conversion stage. Do not directly parallel two traction packs.

## Main files

| File | Purpose |
|---|---|
| `src/main.cpp` | Display/touch setup, PSRAM buffer, application loop |
| `src/board.h` | Elecrow RGB/GT911 pin configuration |
| `src/dashboard_ui.cpp` | All three screens, animations, touch events |
| `src/telemetry.h` | Shared live-data structure and provider interface |
| `src/telemetry.cpp` | Animated simulation; replace/extend for real data |
| `src/sensor_test_main.cpp` | Standalone WCS1800 Dout test (env `sensor-test`) |
| `include/lv_conf.h` | LVGL colors, widgets, and font configuration |
| `platformio.ini` | Reproducible board, framework, and dependency versions |

## Editing the UI — where to look

**Almost everything you see on screen is drawn in one file: [`src/dashboard_ui.cpp`](src/dashboard_ui.cpp)** (~480 lines, LVGL 8.3). The other files support it:

| File | Role | Edit it when… |
|---|---|---|
| [`src/dashboard_ui.cpp`](src/dashboard_ui.cpp) | Builds and updates all three pages | you want to change layout, colors, text, widgets, or animations |
| [`include/lv_conf.h`](include/lv_conf.h) | LVGL config: which fonts/widgets are enabled | you need a different font size or an LVGL widget that's currently off |
| [`src/telemetry.h`](src/telemetry.h) / [`src/telemetry.cpp`](src/telemetry.cpp) | The data shown (currently simulated) | you want different values or live sensor data — **not** the visuals |
| [`src/main.cpp`](src/main.cpp) | Display/touch hardware bring-up and the loop | rarely, for UI work — only for display/touch/buffer behavior |

### Inside `dashboard_ui.cpp`

| What you want to change | Where to look |
|---|---|
| **Colors** (background, panels, accents) | The palette constants at the top: `BG`, `PANEL`, `BORDER`, `TEXT`, `MUTED`, `GREEN`, `CYAN`, `YELLOW`, `ORANGE` (lines ~13–22) |
| **Page geometry** (where the content/nav sit) | `PAGE_Y`, `PAGE_H`, `NAV_Y` constants (lines ~9–11) |
| **Overview page** (speed, range, battery, energy-flow dots) | `build_overview()` (~line 209) |
| **Systems page** (SOC arc, motor, solar, detail cards) | `build_systems()` (~line 265) |
| **V2V Lab page** (cart A/B, direction, swap button) | `build_v2v()` (~line 330) |
| **Header / bottom navigation buttons** | `build_header()` (~175), `build_nav()` (~189) |
| **Reusable widgets** (cards, labels, bars, lines, dots) | helpers `make_obj`, `label`, `bar`, `line`, `flow_dot`, `make_metric_card` (~72–148) |
| **The live numbers shown** (how each value is formatted/written) | `dashboard_ui_update()` (~line 416) |
| **Page switching / touch buttons** | `show_page()` (~150), `nav_event()` (~167), `swap_v2v_event()` (~171) |
| **Moving/energy-flow animation** | `update_flow()` (~line 368) |

Line numbers are approximate and shift as you edit — search for the function name to jump straight there.

### Common edits, step by step

- **Recolor the theme:** change the hex values in the palette constants at the top of `dashboard_ui.cpp`. They're used everywhere, so one change re-themes the whole UI.
- **Move or resize a card:** in the relevant `build_*()` function, cards are placed with `make_obj(parent, x, y, w, h, …)` and `make_metric_card(page, y, …)` — edit the pixel coordinates (screen is 800×480).
- **Change a label's wording or font:** find the `label(parent, "TEXT", x, y, &lv_font_montserrat_XX, color)` call. To use a font size that isn't enabled yet, turn it on in `include/lv_conf.h` (e.g. `LV_FONT_MONTSERRAT_28`).
- **Show a different number:** the value comes from `TelemetryData` (see `telemetry.h`); `dashboard_ui_update()` writes it into a label with `text_value()` / `int_value()` / `lv_label_set_text()`.
- **Add a fourth page:** add a `pages[]` slot, write a `build_<page>()`, call it in `dashboard_ui_init()`, and add a name to the `names[]` array in `build_nav()`.

After any change: `pio run -e crowpanel-7 -t upload` (or `UPLOAD_FIRMWARE.cmd`) to see it on the display.

## References

- [Elecrow CrowPanel 7.0" official hardware repository](https://github.com/Elecrow-RD/CrowPanel-7.0-HMI-ESP32-Display-800x480)
- [Elecrow PlatformIO/LVGL tutorial](https://www.elecrow.com/wiki/CrowPanel_ESP32_7.0-inch_with_PlatformIO.html)
- [LVGL 8.3 documentation](https://docs.lvgl.io/8.3/)
- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)
