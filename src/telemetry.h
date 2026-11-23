#pragma once

#include <cstdint>

// One UI-ready sample. Replace SimulatedTelemetry::update() with your BMS,
// CAN, UART, or ESP-NOW parser later; no screen-layout code needs to change.
struct TelemetryData {
    float speed_kmh = 0.0f;
    float estimated_range_km = 0.0f;
    float battery_soc_pct = 0.0f;
    float battery_capacity_kwh = 10.0f;
    float pack_voltage_v = 0.0f;
    float pack_current_a = 0.0f;       // Positive = charging.
    float battery_power_kw = 0.0f;     // Positive = discharge.
    float battery_temp_c = 0.0f;
    float battery_soh_pct = 0.0f;
    float cell_min_v = 0.0f;
    float cell_max_v = 0.0f;
    float motor_power_kw = 0.0f;
    float motor_rpm = 0.0f;
    float motor_torque_nm = 0.0f;
    float motor_temp_c = 0.0f;
    float regen_power_kw = 0.0f;
    float solar_power_kw = 0.0f;
    float solar_voltage_v = 0.0f;
    float solar_current_a = 0.0f;
    float solar_temp_c = 0.0f;
    float other_cart_soc_pct = 0.0f;
    uint32_t uptime_s = 0;
    bool data_valid = true;
};

class TelemetrySource {
public:
    virtual ~TelemetrySource() = default;
    virtual void update(uint32_t now_ms, TelemetryData &out) = 0;
};

class SimulatedTelemetry final : public TelemetrySource {
public:
    void update(uint32_t now_ms, TelemetryData &out) override;
};
