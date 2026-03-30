#include "SoilSensor.h"

SoilSensor::SoilSensor(uint8_t pin, int dryValue, int wetValue)
    : _pin(pin), _dryValue(dryValue), _wetValue(wetValue) {}

void SoilSensor::begin() const {
    pinMode(_pin, INPUT);
    Serial.printf("[SoilSensor] Initialized — pin: %d | dry: %d | wet: %d\n", _pin, _dryValue, _wetValue);
}

/**
 * Returns soil moisture as an integer percentage [0, 100].
 * Intended for JSON payload output.
 */
int SoilSensor::getMoisturePercent() const {
    return static_cast<int>(roundf(
        calcMoisture(_averagedRead(), _dryValue, _wetValue)
    ));
}

/**
 * Returns the raw ADC reading, averaged over NUM_SAMPLES.
 * Use this during sensor calibration to determine dry/wet reference values.
 */
int SoilSensor::readRaw() const {
    return _averagedRead();
}

/**
 * Returns soil moisture as a floating-point percentage.
 * Useful for high-precision logging or debugging.
 */
float SoilSensor::readMoistureFloat() const {
    return calcMoisture(_averagedRead(), _dryValue, _wetValue);
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
    const int   raw     = _averagedRead();
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
 * Reads the ADC pin NUM_SAMPLES times with a short delay between each sample
 * to reduce noise, then returns the integer average.
 */
int SoilSensor::_averagedRead() const {
    long sum = 0;
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        sum += analogRead(_pin);
        delay(5);
    }
    return static_cast<int>(sum / NUM_SAMPLES);
}
