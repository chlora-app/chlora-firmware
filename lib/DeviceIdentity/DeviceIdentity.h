#pragma once
#include <Arduino.h>

/**
 * Provides a stable, hardware-derived device identity based on the ESP32's
 * factory-burned eFuse MAC address. The identifier is unique per chip,
 * persists across reflashes, and requires no provisioning.
 */
class DeviceIdentity {
public:
    // Must be called once in setup() before getDeviceId() is used.
    static void       begin();

    // Returns the device ID string in the format "DVC-AABBCCDDEEFF".
    // Returns an empty string if begin() has not been called.
    static const char* getDeviceId();

private:
    static char _deviceId[20]; // "DVC-" + 12 hex chars + null terminator
};
