#include "DHTSensor.h"

DHTSensor::DHTSensor(uint8_t pin, uint8_t type)
    : _dht(pin, type), _pin(pin), _type(type) {}

void DHTSensor::begin() {
    _dht.begin();
    Serial.printf("[DHTSensor] Initialized — pin: %d | type: DHT%d\n", _pin, _type == DHT22 ? 22 : 11);
    delay(2000);
}

float DHTSensor::getTemperature() {
    return _dht.readTemperature(); // °C
}

float DHTSensor::getHumidity() {
    return _dht.readHumidity();    // %RH
}

bool DHTSensor::isReady() {
    return !isnan(getTemperature()) && !isnan(getHumidity());
}

void DHTSensor::printStatus() {
    const float temp = getTemperature();
    const float hum  = getHumidity();

    if (isnan(temp) || isnan(hum)) {
        Serial.println("[DHTSensor] ERROR: Failed to read sensor — check wiring.");
        return;
    }

    Serial.printf("[DHTSensor] temp: %.2f°C | humidity: %.2f%%\n", temp, hum);
}
