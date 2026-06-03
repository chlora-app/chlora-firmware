#pragma once
#include <Arduino.h>

class SoilSensor {
public:
    /** Sentinel returned when sensor reading is invalid (out-of-range, fault). */
    static constexpr int INVALID_READING = -1;

    /** ADC sanity bounds — readings outside this range indicate sensor fault. */
    static constexpr int MIN_VALID_RAW = 100;
    static constexpr int MAX_VALID_RAW = 4000;

    SoilSensor(uint8_t pin, int dryValue, int wetValue);

    void  begin() const;

    /**
     * Returns moisture as integer [0, 100], or INVALID_READING (-1) on fault.
     * Caller MUST check for INVALID_READING before publishing to avoid corrupting
     * downstream time-series with sentinel values.
     */
    int getMoisturePercent() const;

    /** Returns filtered raw ADC value (median of samples). Use during calibration. */
    int readRaw() const;

    /** Returns true if last reading is within valid sensor range. */
    bool isReadingValid(int raw) const;

    void  setCalibration(int dryValue, int wetValue);

    void  printStatus() const;

    static float calcMoisture(int raw, int dryValue, int wetValue);

private:
    uint8_t _pin;
    int     _dryValue;
    int     _wetValue;

    static const uint8_t NUM_SAMPLES = 11;  // odd count → unambiguous median
    int _filteredRead() const;
};