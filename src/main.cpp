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

SoilSensor     soilSensor(PIN_SOIL_SENSOR, SOIL_DRY_VALUE, SOIL_WET_VALUE);
DHTSensor      dhtSensor(PIN_DHT, DHT_TYPE);
BatteryMonitor batteryMonitor(PIN_BATTERY, BATTERY_R1_KOHM, BATTERY_R2_KOHM, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE);

void setup() {
    Serial.begin(115200);

    DeviceIdentity::begin();

    soilSensor.begin();
    dhtSensor.begin();
    batteryMonitor.begin();

    if (WiFiConnector::connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS)) {
        TimeManager::begin(NTP_SERVER, NTP_GMT_OFFSET_SEC, NTP_DST_OFFSET_SEC);
        WiFiConnector::disconnect();
    }

    MQTTManager::begin(MQTT_BROKER_HOST, MQTT_BROKER_PORT, DeviceIdentity::getDeviceId());
}

void loop() {
    soilSensor.printStatus();
    dhtSensor.printStatus();
    batteryMonitor.printStatus();
    TimeManager::printStatus();

    if (!dhtSensor.isReady()) {
        Serial.println("[main] Waiting for DHT22...");
        delay(2000);
        return;
    }

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

    Serial.printf("[main] Publishing: %s\n", payload);

    char topic[64];
    snprintf(topic, sizeof(topic), MQTT_TOPIC_TEMPLATE, DeviceIdentity::getDeviceId());

    if (WiFiConnector::connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS)) {
        if (MQTTManager::connectToBroker(MQTT_USERNAME, MQTT_PASSWORD, MQTT_CONNECT_TIMEOUT_MS)) {
            MQTTManager::publish(topic, payload);
            MQTTManager::disconnect();
        }

        WiFiConnector::disconnect();
    }

    MQTTManager::begin(MQTT_BROKER_HOST, MQTT_BROKER_PORT, DeviceIdentity::getDeviceId());

    delay(READ_INTERVAL_MS);
}
