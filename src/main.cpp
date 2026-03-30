#include <Arduino.h>
#include "Config.h"
#include "SoilSensor.h"
#include "DHTSensor.h"
#include "DeviceIdentity.h"

// ─── Module Instances ─────────────────────────────────────────────────────────
SoilSensor soilSensor(PIN_SOIL_SENSOR, SOIL_DRY_VALUE, SOIL_WET_VALUE);
DHTSensor  dhtSensor(PIN_DHT, DHT_TYPE);

// ─── Lifecycle ────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    DeviceIdentity::begin();
    soilSensor.begin();
    dhtSensor.begin();
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void loop() {
    soilSensor.printStatus();
    dhtSensor.printStatus();

    if (!dhtSensor.isReady()) {
        Serial.println("[main] Waiting for DHT22 to be ready...");
        delay(2000);
        return;
    }

    Serial.printf(
        "{\"device_id\":\"%s\","
        "\"soil_moisture\":%d,"
        "\"temperature\":%.2f,"
        "\"humidity\":%.2f}\n",
        DeviceIdentity::getDeviceId(),
        soilSensor.getMoisturePercent(),
        dhtSensor.getTemperature(),
        dhtSensor.getHumidity()
    );

    delay(READ_INTERVAL_MS);
}
