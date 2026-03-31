#pragma once

// ─── Pin Definitions ──────────────────────────────────────────────────────────
#define PIN_BATTERY      35
#define PIN_SOIL_SENSOR  27
#define PIN_DHT          4

// ─── Soil Sensor Calibration ─────────────────────────────────────────────────
#define SOIL_DRY_VALUE   2300
#define SOIL_WET_VALUE   450

// ─── DHT Sensor ───────────────────────────────────────────────────────────────
#define DHT_TYPE         DHT22

// ─── Battery Monitor ──────────────────────────────────────────────────────────
#define BATTERY_R1_KOHM     220.0f
#define BATTERY_R2_KOHM     100.0f
#define BATTERY_MIN_VOLTAGE   6.0f
#define BATTERY_MAX_VOLTAGE   8.4f

// ─── Network ──────────────────────────────────────────────────────────────────
#define WIFI_TIMEOUT_MS     10000

// ─── Timing ───────────────────────────────────────────────────────────────────
#define READ_INTERVAL_MS 5000
