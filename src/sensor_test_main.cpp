// WCS1800 Hall current sensor — Dout signal test for CrowPanel 7.0" (ESP32-S3).
// Wiring: module VCC -> 3.3V (or 5V with divider on Dout), GND -> GND,
//         module Dout -> IO38 on the GPIO_D connector.
// Build/flash: pio run -e sensor-test -t upload --upload-port COM5
#include <Arduino.h>
#include "board.h"

static LGFX lcd;

constexpr int DOUT_PIN = 38;          // GPIO_D connector "IO38"
constexpr uint32_t SAMPLE_MS = 10;    // polling interval
constexpr uint32_t HEARTBEAT_MS = 5000;

static bool stable_state = false;
static bool candidate = false;
static uint32_t candidate_since = 0;
static uint32_t transitions = 0;
static uint32_t last_change_ms = 0;
static uint32_t last_heartbeat_ms = 0;
static uint32_t boot_ms = 0;

static void drawStatic() {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextDatum(top_left);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setTextSize(3);
    lcd.drawString("WCS1800 SIGNAL TEST", 24, 20);
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    lcd.drawString("Dout -> IO38 (GPIO_D connector)", 24, 60);

    lcd.drawRect(20, 100, 760, 160, TFT_DARKGREY);

    lcd.setTextSize(2);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.drawString("Transitions:", 24, 290);
    lcd.drawString("Last change:", 24, 320);
    lcd.drawString("Uptime:", 24, 350);

    lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    lcd.drawString("1. Pass a current-carrying wire through the sensor hole.", 24, 395);
    lcd.drawString("2. Turn the W103 pot slowly until the state flips.", 24, 420);
    lcd.drawString("Module LED should change at the same moment.", 24, 445);
}

static void drawState(bool high) {
    lcd.startWrite();
    lcd.fillRect(22, 102, 756, 156, high ? TFT_DARKGREEN : TFT_NAVY);
    lcd.setTextDatum(middle_center);
    lcd.setTextSize(6);
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString(high ? "HIGH" : "LOW", 400, 155);
    lcd.setTextSize(2);
    lcd.drawString(high ? "comparator tripped" : "below threshold", 400, 220);
    lcd.setTextDatum(top_left);
    lcd.endWrite();
}

static void drawCounters(uint32_t now) {
    lcd.startWrite();
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    char buf[48];
    snprintf(buf, sizeof(buf), "%lu   ", (unsigned long)transitions);
    lcd.drawString(buf, 220, 290);
    if (transitions == 0) {
        snprintf(buf, sizeof(buf), "none yet   ");
    } else {
        snprintf(buf, sizeof(buf), "%lus ago   ",
                 (unsigned long)((now - last_change_ms) / 1000));
    }
    lcd.drawString(buf, 220, 320);
    snprintf(buf, sizeof(buf), "%lus   ", (unsigned long)((now - boot_ms) / 1000));
    lcd.drawString(buf, 220, 350);
    lcd.endWrite();
}

void setup() {
    Serial.begin(115200);
    delay(1200);
    Serial.println();
    Serial.println("=== WCS1800 SENSOR TEST v1 (Dout on IO38) ===");

    pinMode(DOUT_PIN, INPUT_PULLUP);

    lcd.init();
    lcd.setBrightness(220);
    drawStatic();

    boot_ms = millis();
    stable_state = digitalRead(DOUT_PIN) == HIGH;
    candidate = stable_state;
    candidate_since = boot_ms;
    drawState(stable_state);
    drawCounters(boot_ms);
    Serial.printf("Initial Dout state: %s\n", stable_state ? "HIGH" : "LOW");
}

void loop() {
    const uint32_t now = millis();

    // Debounce: a new level must persist 30 ms before we accept it.
    const bool raw = digitalRead(DOUT_PIN) == HIGH;
    if (raw != candidate) {
        candidate = raw;
        candidate_since = now;
    } else if (candidate != stable_state && now - candidate_since >= 30) {
        stable_state = candidate;
        ++transitions;
        last_change_ms = now;
        drawState(stable_state);
        Serial.printf("[%8lu ms] Dout -> %s (transition #%lu)\n",
                      (unsigned long)now, stable_state ? "HIGH" : "LOW",
                      (unsigned long)transitions);
    }

    static uint32_t last_counter_draw = 0;
    if (now - last_counter_draw >= 1000) {
        last_counter_draw = now;
        drawCounters(now);
    }

    if (now - last_heartbeat_ms >= HEARTBEAT_MS) {
        last_heartbeat_ms = now;
        Serial.printf("[%8lu ms] heartbeat: state=%s transitions=%lu\n",
                      (unsigned long)now, stable_state ? "HIGH" : "LOW",
                      (unsigned long)transitions);
    }

    delay(SAMPLE_MS);
}
