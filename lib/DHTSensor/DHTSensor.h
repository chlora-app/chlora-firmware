#pragma once

#include <Arduino.h>
#include <DHT.h>

/**
 * Thin wrapper around the Adafruit DHT library that exposes
 * a clean, consistent interface for reading temperature and humidity.
 *
 * Note: Adafruit DHT methods are not const-correct, so none of the
 * public methods in this class are marked const.
 */
class DHTSensor {
public:
    DHTSensor(uint8_t pin, uint8_t type = DHT22);

    // Must be called once in setup(). Includes a 2-second warm-up delay.
    void begin();

    // Returns temperature in Celsius. May return NaN on read failure.
    float getTemperature();

    // Returns relative humidity in %. May return NaN on read failure.
    float getHumidity();

    // Returns true if both readings are valid (non-NaN).
    bool  isReady();

    // Prints current readings to Serial. Logs an error if sensor is not ready.
    void  printStatus();

private:
    DHT     _dht;
    uint8_t _pin;
    uint8_t _type;
};
