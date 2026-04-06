#pragma once

#include <Arduino.h>

/**
 * Manages NTP time synchronization using the ESP32's built-in SNTP client.
 * Provides Unix timestamps in milliseconds for the JSON payload.
 * All methods are static — no instantiation needed.
 */
class TimeManager {
public:
    // Configures SNTP and waits up to 5 s for the first sync.
    // Must be called after Wi-Fi is connected.
    static void begin(const char* ntpServer, long gmtOffsetSec, int dstOffsetSec);

    // Returns true once the system clock has been set by NTP.
    static bool isTimeSynced();

    // Returns Unix timestamp in milliseconds (for JSON "timestamp" field).
    // Returns 0 if not yet synchronized.
    static uint64_t getTimestampMs();

    static void printStatus();
};
