#include <Arduino.h>
#include <unity.h>
#include <math.h>
#include "Config.h"
#include "DHTSensor.h"

DHTSensor dht(PIN_DHT, DHT_TYPE);

void test_sensor_returns_valid_readings() {
    TEST_ASSERT_TRUE_MESSAGE(dht.isReady(), "DHT22 returned NaN — check wiring.");
}

void test_temperature_within_spec_range() {
    const float temp = dht.getTemperature();
    TEST_ASSERT_FALSE_MESSAGE(isnan(temp), "Temperature is NaN");
    TEST_ASSERT_TRUE_MESSAGE(temp > -40.0f, "Temperature below -40°C spec limit");
    TEST_ASSERT_TRUE_MESSAGE(temp < 80.0f,  "Temperature above 80°C spec limit");
}

void test_humidity_within_physical_bounds() {
    const float hum = dht.getHumidity();
    TEST_ASSERT_FALSE_MESSAGE(isnan(hum), "Humidity is NaN");
    TEST_ASSERT_TRUE_MESSAGE(hum >= 0.0f,   "Humidity below 0%");
    TEST_ASSERT_TRUE_MESSAGE(hum <= 100.0f, "Humidity above 100%");
}

void test_temperature_reasonable_for_indoors() {
    const float temp = dht.getTemperature();
    TEST_ASSERT_TRUE_MESSAGE(temp > 10.0f, "Temperature unexpectedly below 10°C");
    TEST_ASSERT_TRUE_MESSAGE(temp < 45.0f, "Temperature unexpectedly above 45°C");
}

void test_humidity_reasonable_for_indoors() {
    const float hum = dht.getHumidity();
    TEST_ASSERT_TRUE_MESSAGE(hum > 5.0f,  "Humidity unexpectedly below 5%");
    TEST_ASSERT_TRUE_MESSAGE(hum < 98.0f, "Humidity unexpectedly above 98%");
}

void test_readings_are_stable_over_time() {
    const float temp1 = dht.getTemperature();
    const float hum1  = dht.getHumidity();

    delay(2500);

    const float temp2 = dht.getTemperature();
    const float hum2  = dht.getHumidity();

    const float tempDelta = fabsf(temp2 - temp1);
    const float humDelta  = fabsf(hum2  - hum1);

    TEST_ASSERT_TRUE_MESSAGE(tempDelta < 2.0f,  "Temperature shifted >2°C in 2.5s — sensor unstable");
    TEST_ASSERT_TRUE_MESSAGE(humDelta  < 10.0f, "Humidity shifted >10% in 2.5s — sensor unstable");
}

void setup() {
    delay(2000);
    Serial.begin(115200);
    dht.begin();
    delay(2500);

    UNITY_BEGIN();

    RUN_TEST(test_sensor_returns_valid_readings);
    RUN_TEST(test_temperature_within_spec_range);
    RUN_TEST(test_humidity_within_physical_bounds);
    RUN_TEST(test_temperature_reasonable_for_indoors);
    RUN_TEST(test_humidity_reasonable_for_indoors);
    RUN_TEST(test_readings_are_stable_over_time);

    UNITY_END();
}

void loop() {}
