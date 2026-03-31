#pragma once
#include <Arduino.h>

/**
 * Reads pack voltage from a resistive voltage divider connected to the ESP32 ADC.
 * Designed for multi-cell series configurations where pack voltage exceeds 3.3V.
 *
 * Circuit:  Vbat(+) — R1 — [ADC pin] — R2 — GND
 * Formula:  Vbat = (raw/4095 × Vref) / (R2 / (R1 + R2))
 */
class BatteryMonitor {
public:
    BatteryMonitor(uint8_t pin, float r1Kohm, float r2Kohm, float minVoltage, float maxVoltage);

    void  begin();

    /** Returns the measured pack voltage in volts (e.g., 7.42). For JSON output. */
    float getVoltage()      const;

    /** Returns estimated state-of-charge as a percentage [0.0, 100.0]. */
    float getPercent()      const;

    /** Alias for getVoltage() — maps to the "battery_level" JSON field. */
    float getBatteryLevel() const;

    void  printStatus()     const;

    // Public static: pure math, testable without hardware
    static float calcVoltage(int raw, float dividerRatio, float adcRefVoltage = 3.3f);
    static float calcPercent(float voltage, float minVoltage, float maxVoltage);

private:
    uint8_t _pin;
    float   _dividerRatio;  // R2 / (R1 + R2), pre-calculated in constructor
    float   _minVoltage;
    float   _maxVoltage;

    static const uint8_t NUM_SAMPLES = 10;
    int _averagedRead() const;
};
