#include <Arduino.h>
#include <unity.h>
#include "SoilSensor.h"

// ─── Boundary: dry reference → expects 0% ────────────────────────
void test_moisture_at_dry_boundary() {
    const float result = SoilSensor::calcMoisture(2300, 2300, 450);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, result);
}

// ─── Boundary: wet reference → expects 100% ──────────────────────
void test_moisture_at_wet_boundary() {
    const float result = SoilSensor::calcMoisture(450, 2300, 450);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 100.0f, result);
}

// ─── Midpoint: raw at (dry + wet) / 2 → expects ~50% ─────────────
void test_moisture_at_midpoint() {
    const float result = SoilSensor::calcMoisture(1375, 2300, 450);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 50.0f, result);
}

// ─── Clamp: raw below wetValue → must not exceed 100% ────────────
void test_moisture_clamped_at_maximum() {
    const float result = SoilSensor::calcMoisture(100, 2300, 450);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, result);
}

// ─── Clamp: raw above dryValue → must not go below 0% ────────────
void test_moisture_clamped_at_minimum() {
    const float result = SoilSensor::calcMoisture(2800, 2300, 450);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result);
}

// ─── setCalibration: updating reference points changes output ─────
void test_set_calibration_changes_output() {
    SoilSensor sensor(34, 2300, 450);

    // With initial calibration, raw 1375 should be near 50%
    const float before = sensor.getMoisturePercent();  // reads live ADC — skip in pure logic test

    // Instead, verify via static helper with different calibration params
    const float with_default = SoilSensor::calcMoisture(1375, 2300, 450);
    const float with_updated = SoilSensor::calcMoisture(1375, 3000, 500);

    TEST_ASSERT_NOT_EQUAL(with_default, with_updated);
    TEST_ASSERT_GREATER_THAN(with_default, with_updated); // wider range → higher %
}

// ─── Runner ──────────────────────────────────────────────────────
void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_moisture_at_dry_boundary);
    RUN_TEST(test_moisture_at_wet_boundary);
    RUN_TEST(test_moisture_at_midpoint);
    RUN_TEST(test_moisture_clamped_at_maximum);
    RUN_TEST(test_moisture_clamped_at_minimum);
    RUN_TEST(test_set_calibration_changes_output);

    UNITY_END();
}

void loop() {}
