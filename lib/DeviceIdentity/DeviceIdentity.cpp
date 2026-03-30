#include "DeviceIdentity.h"
#include "esp_efuse.h"
#include "esp_mac.h"

char DeviceIdentity::_deviceId[20] = {};

/**
 * Reads the 6-byte MAC address from the ESP32 eFuse (factory-programmed,
 * read-only) and formats it as a device identifier string.
 *
 * Format: "DVC-AABBCCDDEEFF"
 * Must be called once in setup() before getDeviceId() is used.
 */
void DeviceIdentity::begin() {
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);

    snprintf(_deviceId, sizeof(_deviceId), "DVC-%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.printf("[DeviceIdentity] ID: %s\n", _deviceId);
}

const char* DeviceIdentity::getDeviceId() {
    return _deviceId;
}
