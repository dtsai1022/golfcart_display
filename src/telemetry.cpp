#include "telemetry.h"

#include <Arduino.h>

namespace {
float wave(float seconds, float period, float phase = 0.0f) {
    return sinf((seconds / period + phase) * TWO_PI);
}
}

void SimulatedTelemetry::update(uint32_t now_ms, TelemetryData &d) {
    const float t = now_ms * 0.001f;

    d.speed_kmh = 24.5f + 1.4f * wave(t, 11.0f);
    d.battery_soc_pct = 78.0f + 0.8f * wave(t, 47.0f);
    d.battery_capacity_kwh = 10.0f;
    d.estimated_range_km = d.battery_soc_pct * 0.538f;
    d.pack_voltage_v = 51.8f + 0.35f * wave(t, 13.0f, 0.15f);
    d.battery_power_kw = 2.4f + 0.35f * wave(t, 7.0f, 0.2f);
    d.pack_current_a = -1000.0f * d.battery_power_kw / d.pack_voltage_v;
    d.battery_temp_c = 32.6f + 0.7f * wave(t, 31.0f);
    d.battery_soh_pct = 96.0f;
    d.cell_min_v = 3.235f + 0.003f * wave(t, 17.0f);
    d.cell_max_v = d.cell_min_v + 0.027f + 0.002f * wave(t, 19.0f);

    d.motor_power_kw = 2.8f + 0.45f * wave(t, 8.0f, 0.3f);
    d.motor_rpm = 3480.0f + 160.0f * wave(t, 8.0f, 0.3f);
    d.motor_torque_nm = 27.5f + 2.2f * wave(t, 8.0f, 0.3f);
    d.motor_temp_c = 44.8f + 1.1f * wave(t, 29.0f);
    d.regen_power_kw = fmaxf(0.0f, 0.4f + 0.3f * wave(t, 12.0f));

    d.solar_power_kw = fmaxf(0.0f, 1.2f + 0.18f * wave(t, 15.0f, 0.4f));
    d.solar_voltage_v = 36.8f + 0.7f * wave(t, 15.0f, 0.4f);
    d.solar_current_a = 1000.0f * d.solar_power_kw / d.solar_voltage_v;
    d.solar_temp_c = 41.3f + 0.6f * wave(t, 27.0f);
    d.other_cart_soc_pct = 34.0f + 1.5f * wave(t, 53.0f);
    d.uptime_s = now_ms / 1000U;
    d.data_valid = true;
}
