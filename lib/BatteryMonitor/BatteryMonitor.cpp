#include "BatteryMonitor.h"

constexpr float BATTERY_ADC_REF_VOLTAGE = 3.3f;

// ─── Constructor ──────────────────────────────────────────────────────────────
BatteryMonitor::BatteryMonitor(uint8_t pin, float r1Kohm, float r2Kohm, float minVoltage, float maxVoltage, float calFactor)
    : _pin(pin),
      _dividerRatio(r2Kohm / (r1Kohm + r2Kohm)),
      _minVoltage(minVoltage),
      _maxVoltage(maxVoltage),
      _calFactor(calFactor) {}

// ─── Lifecycle ────────────────────────────────────────────────────────────────
void BatteryMonitor::begin() {
    pinMode(_pin, INPUT);
    Serial.printf("[BatteryMonitor] Initialized — pin: %d | divider: %.4f | range: %.1f–%.1fV\n",
                  _pin, _dividerRatio, _minVoltage, _maxVoltage);
}

// ─── Public Readings ──────────────────────────────────────────────────────────
float BatteryMonitor::getVoltage() const {
    float v = calcVoltage(_filteredRead(), _dividerRatio, BATTERY_ADC_REF_VOLTAGE);
    return constrain(v * _calFactor, _minVoltage, _maxVoltage);
}

float BatteryMonitor::getPercent() const {
    return calcPercent(getVoltage(), _minVoltage, _maxVoltage);
}

float BatteryMonitor::getBatteryLevel() const {
    return getVoltage();
}

// ─── Diagnostics ──────────────────────────────────────────────────────────────
void BatteryMonitor::printStatus() const {
    const float voltage = getVoltage();
    const float percent = calcPercent(voltage, _minVoltage, _maxVoltage);
    const char* label   = (percent < 20.0f) ? "LOW" : (percent < 80.0f) ? "OK" : "FULL";
    Serial.printf("[BatteryMonitor] voltage: %.2fV | charge: %.1f%% | %s\n", voltage, percent, label);
}

// ─── Static Utility ───────────────────────────────────────────────────────────
float BatteryMonitor::calcVoltage(int raw, float dividerRatio, float adcRefVoltage) {
    const float adcVoltage = (static_cast<float>(raw) / 4095.0f) * adcRefVoltage;
    return adcVoltage / dividerRatio;
}

float BatteryMonitor::calcPercent(float voltage, float minVoltage, float maxVoltage) {
    const float pct = (voltage - minVoltage) / (maxVoltage - minVoltage) * 100.0f;
    return constrain(pct, 0.0f, 100.0f);
}

// ─── Private Helpers ──────────────────────────────────────────────────────────
/**
 * Reads the ADC pin NUM_SAMPLES times and returns the MEDIAN value.
 * Median rejects ESP32 ADC outlier spikes (occasional 0 or 4095) that would
 * skew a simple mean. See SoilSensor::_filteredRead() for full rationale.
 */
int BatteryMonitor::_filteredRead() const {
    int samples[NUM_SAMPLES];

    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        samples[i] = analogRead(_pin);
        delayMicroseconds(200);
    }

    // Insertion sort (N=11, O(N²) is fine and avoids pulling in <algorithm>).
    for (uint8_t i = 1; i < NUM_SAMPLES; i++) {
        const int key = samples[i];
        int j = i - 1;
        while (j >= 0 && samples[j] > key) {
            samples[j + 1] = samples[j];
            j--;
        }
        samples[j + 1] = key;
    }

    return samples[NUM_SAMPLES / 2];
}