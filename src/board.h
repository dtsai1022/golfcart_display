#pragma once

#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>

constexpr int LCD_WIDTH = 800;
constexpr int LCD_HEIGHT = 480;
constexpr int TOUCH_SDA = 19;
constexpr int TOUCH_SCL = 20;

// Elecrow CrowPanel Basic 7.0" DIS08070H (ESP32-S3-WROOM-1-N4R8).
class LGFX final : public lgfx::LGFX_Device {
    lgfx::Bus_RGB _bus;
    lgfx::Panel_RGB _panel;
    lgfx::Light_PWM _light;

public:
    lgfx::Bus_RGB &rgb_bus() { return _bus; }

    LGFX() {
        {
            auto cfg = _panel.config();
            cfg.memory_width = LCD_WIDTH;
            cfg.memory_height = LCD_HEIGHT;
            cfg.panel_width = LCD_WIDTH;
            cfg.panel_height = LCD_HEIGHT;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            _panel.config(cfg);
        }
        {
            auto cfg = _bus.config();
            cfg.panel = &_panel;
            cfg.pin_d0 = GPIO_NUM_15;
            cfg.pin_d1 = GPIO_NUM_7;
            cfg.pin_d2 = GPIO_NUM_6;
            cfg.pin_d3 = GPIO_NUM_5;
            cfg.pin_d4 = GPIO_NUM_4;
            cfg.pin_d5 = GPIO_NUM_9;
            cfg.pin_d6 = GPIO_NUM_46;
            cfg.pin_d7 = GPIO_NUM_3;
            cfg.pin_d8 = GPIO_NUM_8;
            cfg.pin_d9 = GPIO_NUM_16;
            cfg.pin_d10 = GPIO_NUM_1;
            cfg.pin_d11 = GPIO_NUM_14;
            cfg.pin_d12 = GPIO_NUM_21;
            cfg.pin_d13 = GPIO_NUM_47;
            cfg.pin_d14 = GPIO_NUM_48;
            cfg.pin_d15 = GPIO_NUM_45;
            cfg.pin_henable = GPIO_NUM_41;
            cfg.pin_vsync = GPIO_NUM_40;
            cfg.pin_hsync = GPIO_NUM_39;
            cfg.pin_pclk = GPIO_NUM_0;
            cfg.freq_write = 16000000;
            cfg.hsync_polarity = 0;
            cfg.hsync_front_porch = 40;
            cfg.hsync_pulse_width = 48;
            cfg.hsync_back_porch = 40;
            cfg.vsync_polarity = 0;
            cfg.vsync_front_porch = 1;
            cfg.vsync_pulse_width = 31;
            cfg.vsync_back_porch = 13;
            cfg.pclk_active_neg = 1;
            cfg.de_idle_high = 0;
            cfg.pclk_idle_high = 0;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {
            auto cfg = _light.config();
            cfg.pin_bl = GPIO_NUM_2;
            _light.config(cfg);
            _panel.light(&_light);
        }
        setPanel(&_panel);
    }
};
