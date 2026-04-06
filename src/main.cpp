#include <Arduino.h>
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
// #include "soc/soc.h"
// #include "soc/rtc_cntl_reg.h"

SoilSensor     soilSensor(PIN_SOIL_SENSOR, SOIL_DRY_VALUE, SOIL_WET_VALUE);
DHTSensor      dhtSensor(PIN_DHT, DHT_TYPE);
BatteryMonitor batteryMonitor(
    PIN_BATTERY,
    BATTERY_R1_KOHM,
    BATTERY_R2_KOHM,
    BATTERY_MIN_VOLTAGE,
    BATTERY_MAX_VOLTAGE,
    BATTERY_CAL_FACTOR   // ← tambah ini
);
RTC_DATA_ATTR static uint16_t cycleCount    = 0;
RTC_DATA_ATTR static bool     ntpSyncFailed = false;

void setup() {
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    SleepManager::printStatus();

    DeviceIdentity::begin();
    soilSensor.begin();
    dhtSensor.begin();
    batteryMonitor.begin();

    // ── NTP sync ───────────────────────────────────────────────────────────────
    cycleCount++;
    const bool needsNtp = !SleepManager::isWakeFromSleep() || cycleCount >= NTP_RESYNC_INTERVAL || ntpSyncFailed;

    if (needsNtp) {
        cycleCount = 0;
        if (WiFiConnector::connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS)) {
            TimeManager::begin(NTP_SERVER, NTP_GMT_OFFSET_SEC, NTP_DST_OFFSET_SEC);
            WiFiConnector::disconnect();
        }

        ntpSyncFailed = !TimeManager::isTimeSynced();

        if (ntpSyncFailed) {
            Serial.println("[main] WARN: NTP sync failed — will retry next cycle.");
        }
    }

    // ── MQTT config — di luar needsNtp ────────────────────────────────────────
    MQTTManager::begin(MQTT_BROKER_HOST, MQTT_BROKER_PORT, DeviceIdentity::getDeviceId());

    // ── Sensor status ──────────────────────────────────────────────────────────
    soilSensor.printStatus();
    dhtSensor.printStatus();
    batteryMonitor.printStatus();
    TimeManager::printStatus();

    // ── Guard DHT22 ───────────────────────────────────────────────────────────
    if (!dhtSensor.isReady()) {
        Serial.println("[main] DHT22 not ready — skipping cycle.");
        SleepManager::deepSleep(READ_INTERVAL_MS);
        return;
    }

    // ── Build JSON payload ─────────────────────────────────────────────────────
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{"
             "\"device_id\":\"%s\","
             "\"timestamp\":%llu,"
             "\"soil_moisture\":%d,"
             "\"battery_level\":%.2f,"
             "\"temperature\":%.2f,"
             "\"humidity\":%.2f"
             "}",
             DeviceIdentity::getDeviceId(),
             TimeManager::getTimestampMs(),
             soilSensor.getMoisturePercent(),
             batteryMonitor.getPercent(),
             dhtSensor.getTemperature(),
             dhtSensor.getHumidity());

    // ── Build topic ───────────────────────────────────────────────────────────
    char topic[64];
    snprintf(topic, sizeof(topic), MQTT_TOPIC_TEMPLATE, DeviceIdentity::getDeviceId());

    // ── Burst-connect: WiFi on → MQTT → publish → keduanya off ───────────────
    if (WiFiConnector::connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS)) {
        if (MQTTManager::connectToBroker(MQTT_USERNAME, MQTT_PASSWORD, MQTT_CONNECT_TIMEOUT_MS)) {
            MQTTManager::publish(topic, payload);
            Serial.printf("[main] Published data: %s\n", payload);
            MQTTManager::disconnect();
        }
        WiFiConnector::disconnect();
    }

    SleepManager::deepSleep(READ_INTERVAL_MS);
}

void loop() {}
