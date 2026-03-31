#include "BatteryMonitor.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

BatteryMonitor::BatteryMonitor(uint8_t pin, float r1Kohm, float r2Kohm, float minVoltage, float maxVoltage)
    : _pin(pin), _dividerRatio(r2Kohm / (r1Kohm + r2Kohm)), _minVoltage(minVoltage), _maxVoltage(maxVoltage) {}

// ─── Lifecycle ────────────────────────────────────────────────────────────────
void BatteryMonitor::begin() {
    pinMode(_pin, INPUT);
    Serial.printf("[BatteryMonitor] Initialized — pin: %d | divider: %.4f | range: %.1f–%.1fV\n",
                  _pin, _dividerRatio, _minVoltage, _maxVoltage);
}

// ─── Public Readings ──────────────────────────────────────────────────────────
float BatteryMonitor::getVoltage() const {
    return calcVoltage(_averagedRead(), _dividerRatio);
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
int BatteryMonitor::_averagedRead() const {
    long sum = 0;
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        sum += analogRead(_pin);
        delay(5);
    }
    return static_cast<int>(sum / NUM_SAMPLES);
}
