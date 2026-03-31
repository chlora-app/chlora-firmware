#include "TimeManager.h"

void TimeManager::begin(const char* ntpServer, long gmtOffsetSec, int dstOffsetSec) {
    configTime(gmtOffsetSec, dstOffsetSec, ntpServer);
    Serial.printf("[TimeManager] NTP server: %s | UTC offset: %+.1f h\n", ntpServer, gmtOffsetSec / 3600.0f);

    Serial.print("[TimeManager] Waiting for sync...");
    const uint32_t deadline = millis() + 5000;
    while (!isTimeSynced() && millis() < deadline) {
        delay(200);
        Serial.print(".");
    }
    Serial.println();

    if (isTimeSynced()) {
        char buf[32];
        const time_t now = time(nullptr);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        Serial.printf("[TimeManager] Synced — local time: %s\n", buf);
    } else {
        Serial.println("[TimeManager] WARNING: Initial sync timed out — timestamp will be 0.");
    }
}

bool TimeManager::isTimeSynced() {
    // Any epoch value past year 2001 is considered valid
    return time(nullptr) > 1000000000UL;
}

uint64_t TimeManager::getTimestampMs() {
    if (!isTimeSynced()) return 0ULL;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint64_t)tv.tv_sec * 1000ULL
         + (uint64_t)tv.tv_usec / 1000ULL;
}

void TimeManager::printStatus() {
    if (!isTimeSynced()) {
        Serial.println("[TimeManager] Clock not synchronized.");
        return;
    }
    char buf[32];
    const time_t now = time(nullptr);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    Serial.printf("[TimeManager] %s | %llu ms\n", buf, getTimestampMs());
}
