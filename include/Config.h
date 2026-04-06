#pragma once

// ─── Pin Definitions ──────────────────────────────────────────────────────────
#define PIN_BATTERY      35
#define PIN_SOIL_SENSOR  34
#define PIN_DHT          4

// ─── Soil Sensor Calibration ─────────────────────────────────────────────────
#define SOIL_DRY_VALUE   2300
#define SOIL_WET_VALUE   450

// ─── DHT Sensor ───────────────────────────────────────────────────────────────
#define DHT_TYPE         DHT22

// ─── Battery Monitor ──────────────────────────────────────────────────────────
#define BATTERY_R1_KOHM             220.0f
#define BATTERY_R2_KOHM             100.0f
#define BATTERY_MIN_VOLTAGE         6.0f
#define BATTERY_MAX_VOLTAGE         8.4f
#define BATTERY_ADC_REF_VOLTAGE     3.3f
#define BATTERY_CAL_FACTOR          1.0364f

// ─── Network ──────────────────────────────────────────────────────────────────
#define WIFI_TIMEOUT_MS     10000

// ─── NTP ──────────────────────────────────────────────────────────────────────
#define NTP_SERVER          "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC  25200   // UTC+7 (WIB — Jakarta)
#define NTP_DST_OFFSET_SEC  0
#define NTP_RESYNC_INTERVAL 60

// ── MQTT ──────────────────────────────────────────────────────────────────────
#define MQTT_BROKER_HOST        "103.150.92.246"
#define MQTT_BROKER_PORT        1883
#define MQTT_TOPIC_TEMPLATE     "chlora/%s/sensors"   // %s → device_id
#define MQTT_CONNECT_TIMEOUT_MS 5000

// ─── MQTT Publish Retry ────────────────────────────────────────────────────────
#define MQTT_PUBLISH_RETRIES         3
#define MQTT_PUBLISH_RETRY_DELAY_MS  500

// ─── Timing ───────────────────────────────────────────────────────────────────
#define READ_INTERVAL_MS 60000
