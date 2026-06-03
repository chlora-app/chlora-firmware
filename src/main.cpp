#include <Arduino.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#include "secrets.h"
#include "Config.h"
#include "DeviceIdentity.h"
#include "SoilSensor.h"
#include "DHTSensor.h"
#include "BatteryMonitor.h"
#include "WiFiConnector.h"
#include "TimeManager.h"
#include "MQTTManager.h"
#include "SleepManager.h"

SoilSensor     soilSensor(PIN_SOIL_SENSOR, SOIL_DRY_VALUE, SOIL_WET_VALUE);
DHTSensor      dhtSensor(PIN_DHT, DHT_TYPE);
BatteryMonitor batteryMonitor(
    PIN_BATTERY,
    BATTERY_R1_KOHM,
    BATTERY_R2_KOHM,
    BATTERY_MIN_VOLTAGE,
    BATTERY_MAX_VOLTAGE,
    BATTERY_CAL_FACTOR
);

// ─── RTC timestamp fallback ───────────────────────────────────────────────────
RTC_DATA_ATTR static uint64_t lastKnownTimestampMs = 0;
RTC_DATA_ATTR static uint32_t lastSleepDurationMs  = 0;

static uint32_t wakeStartMs = 0;

// ─── Diagnostic counters ─────────────────────────────────────────────────────
RTC_DATA_ATTR static uint16_t totalWakeCount   = 0;
RTC_DATA_ATTR static uint16_t brownoutCount    = 0;
RTC_DATA_ATTR static uint16_t panicCount       = 0;
RTC_DATA_ATTR static uint16_t watchdogCount    = 0;
RTC_DATA_ATTR static uint16_t publishOkCount   = 0;
RTC_DATA_ATTR static uint16_t publishFailCount = 0;

static constexpr uint8_t RESET_HISTORY_SIZE = 32;
RTC_DATA_ATTR static uint8_t  resetHistory[RESET_HISTORY_SIZE] = {0};
RTC_DATA_ATTR static uint8_t  resetHistoryIdx = 0;

// ─── Debug logging macros ─────────────────────────────────────────────────────
// Set DEBUG_MODE=1 di platformio.ini build_flags untuk enable Serial output.
// Produksi: DEBUG_MODE=0 → Serial tidak diinisialisasi, hemat ~1–2mA.
#ifndef DEBUG_MODE
  #define DEBUG_MODE 0
#endif

#if DEBUG_MODE
  #define LOG_I(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
  #define LOG_W(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
  #define LOG_E(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
  #define LOG_I(fmt, ...) do {} while(0)
  #define LOG_W(fmt, ...) do {} while(0)
  #define LOG_E(fmt, ...) do {} while(0)
#endif

static void recordResetReason(esp_reset_reason_t reason) {
    resetHistory[resetHistoryIdx] = static_cast<uint8_t>(reason);
    resetHistoryIdx = (resetHistoryIdx + 1) % RESET_HISTORY_SIZE;

    switch (reason) {
        case ESP_RST_BROWNOUT: brownoutCount++; break;
        case ESP_RST_PANIC:    panicCount++;    break;
        case ESP_RST_WDT:
        case ESP_RST_TASK_WDT:
        case ESP_RST_INT_WDT:  watchdogCount++; break;
        default: break;
    }
}

static void resetDiagnostics() {
    totalWakeCount   = 0;
    brownoutCount    = 0;
    panicCount       = 0;
    watchdogCount    = 0;
    publishOkCount   = 0;
    publishFailCount = 0;
    memset(resetHistory, 0, sizeof(resetHistory));
    resetHistoryIdx  = 0;
}

static void dumpDiagnostics() {
#if DEBUG_MODE
    Serial.println("\n========== DIAGNOSTIC DUMP ==========");
    Serial.printf("Total wakes:    %u\n", totalWakeCount);
    Serial.printf("Brownouts:      %u (%.1f%%)\n",
                  brownoutCount,
                  totalWakeCount ? 100.0f * brownoutCount / totalWakeCount : 0.0f);
    Serial.printf("Panics:         %u\n", panicCount);
    Serial.printf("Watchdogs:      %u\n", watchdogCount);
    Serial.printf("Publish OK:     %u\n", publishOkCount);
    Serial.printf("Publish FAIL:   %u\n", publishFailCount);

    Serial.print("Reset history (oldest → newest): ");
    for (uint8_t i = 0; i < RESET_HISTORY_SIZE; i++) {
        const uint8_t idx = (resetHistoryIdx + i) % RESET_HISTORY_SIZE;
        const uint8_t r   = resetHistory[idx];
        if (r == 0) continue;
        Serial.print(resetReasonChar(r));
    }
    Serial.println();
    Serial.println("Legend: D=DeepSleep O=PowerOn B=Brownout P=Panic W=Watchdog S=SwReset");
    Serial.println("=====================================\n");
#endif
}

// ─── NTP sync dengan retry loop ───────────────────────────────────────────────
static bool syncNtpWithRetry() {
    for (uint8_t attempt = 1; attempt <= NTP_MAX_RETRIES; attempt++) {
        LOG_I("[main] NTP sync attempt %u/%u...\n", attempt, NTP_MAX_RETRIES);
        TimeManager::begin(NTP_SERVER, NTP_GMT_OFFSET_SEC, NTP_DST_OFFSET_SEC);
        if (TimeManager::isTimeSynced()) {
            LOG_I("[main] NTP synced on attempt %u.\n", attempt);
            return true;
        }
        if (attempt < NTP_MAX_RETRIES) {
            LOG_W("[main] NTP attempt %u failed — waiting %u ms before retry.\n",
                  attempt, NTP_RETRY_DELAY_MS);
            delay(NTP_RETRY_DELAY_MS);
        }
    }
    LOG_E("[main] ERROR: All NTP attempts failed.\n");
    return false;
}

// ─── Resolve timestamp: NTP → RTC fallback → abort ───────────────────────────
static uint64_t resolveTimestamp(bool& isEstimated) {
    isEstimated = false;

    const uint64_t ntpTs = TimeManager::getTimestampMs();
    if (ntpTs > 0) {
        lastKnownTimestampMs = ntpTs;
        lastSleepDurationMs = READ_INTERVAL_MS + millis() - wakeStartMs;
        return ntpTs;
    }

    if (lastKnownTimestampMs > 0) {
        isEstimated = true;
        const uint64_t estimated = lastKnownTimestampMs + lastSleepDurationMs;

        LOG_W("[main] WARN: NTP unavailable — using RTC estimate: %llu ms.\n", estimated);
        lastKnownTimestampMs = estimated;
        lastSleepDurationMs  = READ_INTERVAL_MS + millis() - wakeStartMs;
        return estimated;
    }

    return 0ULL;
}

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
#if DEBUG_MODE
    Serial.begin(115200);
    delay(100);
#endif

    wakeStartMs = millis();

    // ── [IMPROVEMENT #6] Watchdog timer ──────────────────────────────────────
    esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
    esp_task_wdt_add(nullptr);

    // ── Capture reset reason ─────────────────────────────────────────────────
    const esp_reset_reason_t resetReason = esp_reset_reason();

    if (resetReason == ESP_RST_POWERON) {
        resetDiagnostics();
        lastKnownTimestampMs = 0;
        lastSleepDurationMs  = 0;
        LOG_I("[main] Counters reset (power-on detected).\n");
    }

    totalWakeCount++;
    recordResetReason(resetReason);

    LOG_I("[main] Reset reason: %d (%s)\n", resetReason, resetReasonName(resetReason));
    dumpDiagnostics();
    SleepManager::printStatus();

    DeviceIdentity::begin();

    // ── [IMPROVEMENT #5] ADC attenuation ──────────────────────────────────────
    analogSetAttenuation(ADC_11db);

    // ── [IMPROVEMENT #4] Baca sensor SEBELUM WiFi ────────────────────────────
    soilSensor.begin();
    dhtSensor.begin();
    batteryMonitor.begin();

    soilSensor.printStatus();
    dhtSensor.printStatus();
    batteryMonitor.printStatus();

    if (!dhtSensor.isReady()) {
        LOG_W("[main] DHT22 not ready — skipping cycle (WiFi never activated).\n");
        esp_task_wdt_delete(nullptr);
        SleepManager::deepSleep(READ_INTERVAL_MS);
        return;
    }

    const int soilMoisture = soilSensor.getMoisturePercent();
    if (soilMoisture == SoilSensor::INVALID_READING) {
        LOG_W("[main] Soil sensor fault — skipping cycle (WiFi never activated).\n");
        esp_task_wdt_delete(nullptr);
        SleepManager::deepSleep(READ_INTERVAL_MS);
        return;
    }

    const float batteryPct  = batteryMonitor.getPercent();
    const float temperature = dhtSensor.getTemperature();
    const float humidity    = dhtSensor.getHumidity();

    // ── [IMPROVEMENT #2] Turunkan TX power WiFi ───────────────────────────────
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    // ── [IMPROVEMENT #1] Single WiFi session untuk NTP + MQTT ────────────────
    const bool isHealthyBoot = (resetReason == ESP_RST_POWERON ||
                                resetReason == ESP_RST_DEEPSLEEP);

    MQTTManager::begin(MQTT_BROKER_HOST, MQTT_BROKER_PORT, DeviceIdentity::getDeviceId());
    TimeManager::printStatus();

    bool published = false;

    if (WiFiConnector::connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS)) {

        if (isHealthyBoot) {
            const bool synced = syncNtpWithRetry();
            if (!synced) {
                LOG_W("[main] WARN: NTP sync failed — will use RTC fallback.\n");
            }
        } else {
            LOG_I("[main] Skipping NTP — non-healthy boot.\n");
        }

        bool isEstimatedTimestamp = false;
        const uint64_t timestamp  = resolveTimestamp(isEstimatedTimestamp);

        if (timestamp == 0) {
            publishFailCount++;
            LOG_E("[main] ERROR: Timestamp unavailable — skipping publish.\n");
        } else {
            char topic[64];
            snprintf(topic, sizeof(topic), MQTT_TOPIC_TEMPLATE, DeviceIdentity::getDeviceId());

            char payload[300];
            snprintf(payload, sizeof(payload),
                     "{"
                     "\"device_id\":\"%s\","
                     "\"timestamp\":%llu,"
                     "\"soil_moisture\":%d,"
                     "\"temperature\":%.2f,"
                     "\"humidity\":%.2f,"
                     "\"battery_level\":%.2f"
                     "}",
                     DeviceIdentity::getDeviceId(),
                     timestamp,
                     soilMoisture,
                     temperature,
                     humidity,
                     batteryPct
            );

            if (MQTTManager::connectToBroker(MQTT_USERNAME, MQTT_PASSWORD, MQTT_CONNECT_TIMEOUT_MS)) {
                published = MQTTManager::publish(topic, payload);
                if (published) {
                    LOG_I("[main] Published: %s\n", payload);
                }
                MQTTManager::disconnect();
            }
        }

        WiFiConnector::disconnect();
    }

    if (published) {
        publishOkCount++;
    } else {
        publishFailCount++;
        LOG_W("[main] WARN: Publish failed this cycle.\n");
    }

    esp_task_wdt_delete(nullptr);

    SleepManager::deepSleep(READ_INTERVAL_MS);
}

void loop() {}