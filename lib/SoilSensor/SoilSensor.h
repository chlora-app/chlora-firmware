#pragma once
#include <Arduino.h>

class SoilSensor {
public:
    SoilSensor(uint8_t pin, int dryValue, int wetValue);

    void  begin() const;

    /** Returns moisture as integer [0, 100]. Use for JSON output. */
    int getMoisturePercent() const;

    /** Returns averaged raw ADC value. Use during calibration. */
    int readRaw() const;

    float readMoistureFloat() const;

    void  setCalibration(int dryValue, int wetValue);

    void  printStatus() const;

    static float calcMoisture(int raw, int dryValue, int wetValue);

private:
    uint8_t _pin;
    int     _dryValue;
    int     _wetValue;

    static const uint8_t NUM_SAMPLES = 10;
    int _averagedRead() const;
};
