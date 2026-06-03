#pragma once

#include <Arduino.h>

/**
 * Manages NTP time synchronization using the ESP32's built-in SNTP client.
 * Provides Unix timestamps in milliseconds for the JSON payload.
 * All methods are static — no instantiation needed.
 *
 * isTimeSynced() menggunakan flag internal _syncedThisSession yang di-reset
 * di awal setiap begin(). Dengan cara ini tidak perlu sntp_stop() yang
 * ternyata mengganggu cycle SNTP berikutnya, dan tidak tertipu oleh
 * time(nullptr) yang tetap valid dari RTC carry-over deep sleep.
 */
class TimeManager {
public:
    // Configures SNTP and waits up to 10s for sync.
    // Must be called after Wi-Fi is connected.
    static void begin(const char* ntpServer, long gmtOffsetSec, int dstOffsetSec);

    // Returns true ONLY if NTP has successfully synced in the current begin() call.
    static bool isTimeSynced();

    // Returns Unix timestamp in milliseconds (for JSON "timestamp" field).
    // Returns 0 if not synced in this session.
    static uint64_t getTimestampMs();

    static void printStatus();

private:
    static bool _syncedThisSession;
};