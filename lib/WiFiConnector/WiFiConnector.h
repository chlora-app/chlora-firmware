#pragma once

#include <Arduino.h>
#include <WiFi.h>

/**
 * Thin wrapper around the ESP32 WiFi stack.
 * All methods are static — no instantiation needed.
 */
class WiFiConnector {
public:
    // Attempts to connect. Returns true on success, false on timeout.
    static bool connect(const char* ssid, const char* password, uint32_t timeoutMs);

    static bool       isConnected();
    static void       disconnect();
    static void       printStatus();
};
