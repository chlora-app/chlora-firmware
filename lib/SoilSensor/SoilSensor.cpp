#include "SoilSensor.h"

SoilSensor::SoilSensor(uint8_t pin, int dryValue, int wetValue) : _pin(pin), _dryValue(dryValue), _wetValue(wetValue) {}

void SoilSensor::begin() const {
    pinMode(_pin, INPUT);
    Serial.printf("[SoilSensor] Initialized — pin: %d | dry: %d | wet: %d\n", _pin, _dryValue, _wetValue);
}

/**
 * Returns soil moisture as an integer percentage [0, 100], or INVALID_READING (-1)
 * if the underlying ADC read is outside the valid sensor range.
 *
 * IMPORTANT: callers MUST check the return value against INVALID_READING before
 * including it in published payloads. Forwarding -1 to downstream pipelines will
 * corrupt time-series data and confuse anomaly-detection models.
 */
int SoilSensor::getMoisturePercent() const {
    const int raw = _filteredRead();
    if (!isReadingValid(raw)) {
        Serial.printf("[SoilSensor] WARN: Invalid raw reading %d — outside [%d, %d]. "
                      "Possible causes: sensor disconnected, power-up race, ADC fault.\n",
                      raw, MIN_VALID_RAW, MAX_VALID_RAW);
        return INVALID_READING;
    }
    return static_cast<int>(roundf(calcMoisture(raw, _dryValue, _wetValue)));
}

/**
 * Returns the filtered raw ADC reading (median of NUM_SAMPLES).
 * Use this during sensor calibration to determine dry/wet reference values.
 */
int SoilSensor::readRaw() const {
    return _filteredRead();
}

/**
 * Returns true if a raw ADC reading falls within the physically plausible range.
 * Capacitive soil sensors should never read 0 (sensor disconnected/fault) or
 * 4095 (ADC saturated/short to VCC).
 */
bool SoilSensor::isReadingValid(int raw) const {
    return raw >= MIN_VALID_RAW && raw <= MAX_VALID_RAW;
}

/**
 * Updates calibration reference points at runtime.
 * Allows recalibration without reflashing the firmware.
 */
void SoilSensor::setCalibration(int dryValue, int wetValue) {
    _dryValue = dryValue;
    _wetValue = wetValue;
    Serial.printf("[SoilSensor] Calibration updated — dry: %d | wet: %d\n",
                  dryValue, wetValue);
}

void SoilSensor::printStatus() const {
    const int raw = _filteredRead();

    if (!isReadingValid(raw)) {
        Serial.printf("[SoilSensor] FAULT — raw: %d (outside [%d, %d])\n",
                      raw, MIN_VALID_RAW, MAX_VALID_RAW);
        return;
    }

    const int   percent = static_cast<int>(roundf(calcMoisture(raw, _dryValue, _wetValue)));
    const char* label   = (percent < 30) ? "DRY"
                        : (percent > 70) ? "WET"
                        :                  "NORMAL";
    Serial.printf("[SoilSensor] raw: %4d | moisture: %3d%% | %s\n",
                  raw, percent, label);
}

/**
 * Converts a raw ADC value to a moisture percentage using linear interpolation
 * between the dry and wet calibration reference points.
 * Result is clamped to [0.0, 100.0].
 *
 * @param raw       ADC reading to convert
 * @param dryValue  ADC value when sensor is in air (0% moisture)
 * @param wetValue  ADC value when sensor is fully submerged (100% moisture)
 * @return          Moisture percentage in range [0.0, 100.0]
 */
float SoilSensor::calcMoisture(int raw, int dryValue, int wetValue) {
    const float pct = static_cast<float>(dryValue - raw)
                    / static_cast<float>(dryValue - wetValue)
                    * 100.0f;
    return constrain(pct, 0.0f, 100.0f);
}

/**
 * Reads the ADC pin NUM_SAMPLES times and returns the MEDIAN value.
 *
 * Why median instead of mean:
 *   ESP32 ADC1 has a documented quirk where occasional samples spike to 0 or
 *   4095, especially when WiFi/BT is active (RF coupling) or right after wake
 *   from deep sleep (power rail not fully settled). A single 0 in 10 samples
 *   skews a mean by ~10%; the median is immune to up to (N-1)/2 such outliers.
 *
 * NUM_SAMPLES is odd (11) so the median is a single unambiguous element.
 */
int SoilSensor::_filteredRead() const {
    int samples[NUM_SAMPLES];

    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        samples[i] = analogRead(_pin);
        delayMicroseconds(200);  // brief settle between samples; ~25× faster than delay(5)
    }

    // Insertion sort — N is small (11), so O(N²) is faster than pulling in <algorithm>.
    for (uint8_t i = 1; i < NUM_SAMPLES; i++) {
        const int key = samples[i];
        int j = i - 1;
        while (j >= 0 && samples[j] > key) {
            samples[j + 1] = samples[j];
            j--;
        }
        samples[j + 1] = key;
    }

    return samples[NUM_SAMPLES / 2];  // middle element of sorted array
}