#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>

#include "board.h"
#include "dashboard_ui.h"
#include "telemetry.h"

namespace {
LGFX tft;
SimulatedTelemetry telemetry_source;
TelemetryData telemetry;
bool app_ready = false;

lv_disp_draw_buf_t draw_buf;
lv_disp_drv_t disp_drv;

void display_flush(lv_disp_drv_t *disp, const lv_area_t *, lv_color_t *colors) {
    if (!tft.rgb_bus().presentFrameBuffer(reinterpret_cast<uint8_t *>(colors))) {
        Serial.println("WARNING: RGB VSYNC frame switch failed");
    }
    lv_disp_flush_ready(disp);
}

uint8_t gt911_address = 0;
int16_t touch_x = 0;
int16_t touch_y = 0;

bool gt911_read_at(uint8_t address, uint16_t reg, uint8_t *buffer, size_t size) {
    Wire.beginTransmission(address);
    Wire.write(highByte(reg));
    Wire.write(lowByte(reg));
    if (Wire.endTransmission(true) != 0) {
        return false;
    }
    const size_t received = Wire.requestFrom(address, static_cast<uint8_t>(size));
    if (received != size) {
        return false;
    }
    for (size_t index = 0; index < size; ++index) {
        buffer[index] = Wire.read();
    }
    return true;
}

bool gt911_read(uint16_t reg, uint8_t *buffer, size_t size) {
    return gt911_address != 0 && gt911_read_at(gt911_address, reg, buffer, size);
}

void gt911_clear_status() {
    Wire.beginTransmission(gt911_address);
    Wire.write(0x81);
    Wire.write(0x4E);
    Wire.write(0);
    Wire.endTransmission();
}

bool initialize_touch_controller() {
    uint8_t product_id[4];
    for (const uint8_t address : {0x5D, 0x14}) {
        if (gt911_read_at(address, 0x8140, product_id, sizeof(product_id))) {
            gt911_address = address;
            Serial.printf("GT911 found at 0x%02X, ID %.4s\n", address, product_id);
            return true;
        }
    }
    Serial.println("WARNING: GT911 touch controller not found");
    return false;
}

void touch_read(lv_indev_drv_t *, lv_indev_data_t *data) {
    bool pressed = false;

    uint8_t touch_status = 0;
    if (gt911_read(0x814E, &touch_status, 1) && (touch_status & 0x80)) {
        const uint8_t points = touch_status & 0x0F;
        uint8_t point[7];
        pressed = points > 0 && points <= 5 &&
                  gt911_read(0x814F, point, sizeof(point));
        gt911_clear_status();
        if (pressed) {
            const uint16_t raw_x = point[1] | (point[2] << 8);
            const uint16_t raw_y = point[3] | (point[4] << 8);
            touch_x = raw_x > LCD_WIDTH - 1 ? LCD_WIDTH - 1 : raw_x;
            touch_y = raw_y > LCD_HEIGHT - 1 ? LCD_HEIGHT - 1 : raw_y;
        }
    }

    data->state = pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    data->point.x = touch_x;
    data->point.y = touch_y;
}

bool pca9557_write(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(0x18);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

void initialize_v3_touch_reset() {
    // Current V3 boards: the PCA9557 sequences GT911 reset/address selection.
    // The I2C error on V1/V2 is harmless; GT911 still starts normally there.
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    pca9557_write(0x03, 0x00);  // Configuration: all outputs.
    pca9557_write(0x01, 0xFC);  // IO0 = low, IO1 = low.
    delay(20);
    pca9557_write(0x01, 0xFD);  // IO0 = high, IO1 = low.
    delay(100);
    pca9557_write(0x03, 0xFD);  // IO1 = input; IO0 remains output.
}

bool initialize_display() {
    initialize_v3_touch_reset();
    for (const int pin : {38, 17, 18, 42}) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
    if (!tft.begin()) {
        return false;
    }
    tft.setBrightness(200);
    tft.fillScreen(TFT_BLACK);
    delay(100);
    return true;
}

bool initialize_lvgl() {
    lv_init();
    auto framebuffer_0 = reinterpret_cast<lv_color_t *>(tft.rgb_bus().getFrameBuffer(0));
    auto framebuffer_1 = reinterpret_cast<lv_color_t *>(tft.rgb_bus().getFrameBuffer(1));
    if (!framebuffer_0 || !framebuffer_1) {
        Serial.println("ERROR: RGB double-frame-buffer allocation failed");
        return false;
    }
    lv_disp_draw_buf_init(&draw_buf, framebuffer_0, framebuffer_1, LCD_WIDTH * LCD_HEIGHT);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = display_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read;
    lv_indev_drv_register(&indev_drv);
    return true;
}
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\nGolf Cart Dashboard starting...");

    if (!initialize_display()) {
        Serial.println("ERROR: RGB display initialization failed");
        return;
    }
    initialize_touch_controller();
    if (!initialize_lvgl()) {
        return;
    }
    dashboard_ui_init();
    telemetry_source.update(millis(), telemetry);
    dashboard_ui_update(telemetry, millis());
    app_ready = true;
    Serial.println("Dashboard ready - simulated telemetry active");
}

void loop() {
    if (!app_ready) {
        delay(100);
        return;
    }
    static uint32_t last_telemetry_ms = 0;
    static uint32_t last_tick_ms = 0;
    const uint32_t now = millis();
    lv_tick_inc(now - last_tick_ms);
    last_tick_ms = now;
    if (now - last_telemetry_ms >= 100) {
        last_telemetry_ms = now;
        telemetry_source.update(now, telemetry);
        dashboard_ui_update(telemetry, now);
    }
    lv_timer_handler();
    delay(4);
}
