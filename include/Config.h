#pragma once

// ─── Debug mode ───────────────────────────────────────────────────────────────
// Set via platformio.ini build_flags:
//   [env:production]  build_flags = -D DEBUG_MODE=0
//   [env:debug]       build_flags = -D DEBUG_MODE=1
// Saat DEBUG_MODE=0: Serial tidak diinisialisasi, hemat ~1–2mA sepanjang wake.
#ifndef DEBUG_MODE
  #define DEBUG_MODE 0
#endif

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
#define BATTERY_R1_KOHM             20.0f
#define BATTERY_R2_KOHM             10.0f
#define BATTERY_MIN_VOLTAGE         6.0f
#define BATTERY_MAX_VOLTAGE         8.4f
#define BATTERY_ADC_REF_VOLTAGE     3.3f
#define BATTERY_CAL_FACTOR          1.0364f

// ─── Network ──────────────────────────────────────────────────────────────────
#define WIFI_TIMEOUT_MS     10000

// WiFi TX power — mengurangi lonjakan arus saat WiFi aktif.
// WIFI_POWER_8_5dBm  : ~100mA peak, cocok untuk jarak dekat (< 10m ke AP).
// WIFI_POWER_13dBm   : ~150mA peak, jika sinyal < -70 dBm.
// WIFI_POWER_19_5dBm : default ESP32, ~300–400mA peak (penyebab voltage sag).
#define WIFI_TX_POWER       WIFI_POWER_8_5dBm

// ─── NTP ──────────────────────────────────────────────────────────────────────
#define NTP_SERVER          "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC  25200   // UTC+7 (WIB — Jakarta)
#define NTP_DST_OFFSET_SEC  0
// NTP di-sync setiap cycle — tidak perlu interval counter.
#define NTP_MAX_RETRIES      3      // Jumlah percobaan NTP per sesi WiFi
#define NTP_RETRY_DELAY_MS   1500   // Jeda antar percobaan NTP (ms)

// ── MQTT ──────────────────────────────────────────────────────────────────────
#define MQTT_BROKER_HOST        "103.150.92.246"
#define MQTT_BROKER_PORT        1884
#define MQTT_TOPIC_TEMPLATE     "chlora/%s/sensors"   // %s → device_id
#define MQTT_CONNECT_TIMEOUT_MS 5000

// ─── MQTT Publish Retry ────────────────────────────────────────────────────────
#define MQTT_PUBLISH_RETRIES         3
#define MQTT_PUBLISH_RETRY_DELAY_MS  500

// ─── Timing ───────────────────────────────────────────────────────────────────
#define READ_INTERVAL_MS 60000

// ─── Watchdog ─────────────────────────────────────────────────────────────────
// Harus lebih besar dari total worst-case awake time:
// WIFI_TIMEOUT(10s) + NTP retries(~20s) + MQTT(5s) + margin(10s) = 45s.
#define WDT_TIMEOUT_SEC 45