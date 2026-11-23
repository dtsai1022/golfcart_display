#pragma once

#include <cstdint>
#include "telemetry.h"

void dashboard_ui_init();
void dashboard_ui_update(const TelemetryData &data, uint32_t now_ms);
