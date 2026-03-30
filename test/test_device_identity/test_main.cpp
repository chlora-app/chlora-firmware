#include <Arduino.h>
#include <unity.h>
#include <string.h>
#include "DeviceIdentity.h"

// ─── Format: ID must not be null or empty after begin() ──────────────────────
void test_id_is_not_empty() {
    const char* id = DeviceIdentity::getDeviceId();
    TEST_ASSERT_NOT_NULL(id);
    TEST_ASSERT_GREATER_THAN(0u, strlen(id));
}

// ─── Format: ID must start with the "DVC-" prefix ────────────────────────────
void test_id_has_correct_prefix() {
    const char* id = DeviceIdentity::getDeviceId();
    TEST_ASSERT_EQUAL_STRING_LEN("DVC-", id, 4);
}

// ─── Format: total length must be exactly 16 chars ("DVC-" + 12 hex) ─────────
void test_id_has_correct_length() {
    const char* id = DeviceIdentity::getDeviceId();
    TEST_ASSERT_EQUAL(16u, strlen(id));
}

// ─── Format: the 12 characters after "DVC-" must all be valid hex digits ──────
void test_id_hex_portion_is_valid() {
    const char* hexPart = DeviceIdentity::getDeviceId() + 4;

    for (uint8_t i = 0; i < 12; i++) {
        const char c = hexPart[i];
        const bool isHex = (c >= '0' && c <= '9') ||
                           (c >= 'A' && c <= 'F') ||
                           (c >= 'a' && c <= 'f');
        TEST_ASSERT_TRUE_MESSAGE(isHex, "Non-hex character found in ID");
    }
}

// ─── Stability: two consecutive calls must return the same string ─────────────
void test_id_is_stable_across_calls() {
    const char* first  = DeviceIdentity::getDeviceId();
    const char* second = DeviceIdentity::getDeviceId();
    TEST_ASSERT_EQUAL_STRING(first, second);
}

void setup() {
    delay(2000);
    DeviceIdentity::begin();

    UNITY_BEGIN();

    RUN_TEST(test_id_is_not_empty);
    RUN_TEST(test_id_has_correct_prefix);
    RUN_TEST(test_id_has_correct_length);
    RUN_TEST(test_id_hex_portion_is_valid);
    RUN_TEST(test_id_is_stable_across_calls);

    UNITY_END();
}

void loop() {}
