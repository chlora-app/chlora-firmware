#include "TimeManager.h"
#include "Config.h"
#include <esp_sntp.h>

// ─── Debug logging macros (mirrors main.cpp) ──────────────────────────────────
#ifndef DEBUG_MODE
  #define DEBUG_MODE 0
#endif

#if DEBUG_MODE
  #define LOG_I(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
  #define LOG_W(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
  #define LOG_I(fmt, ...) do {} while(0)
  #define LOG_W(fmt, ...) do {} while(0)
#endif

// ─── Static member definition ─────────────────────────────────────────────────
bool TimeManager::_syncedThisSession = false;

void TimeManager::begin(const char* ntpServer, long gmtOffsetSec, int dstOffsetSec) {
    _syncedThisSession = false;

    configTime(gmtOffsetSec, dstOffsetSec, ntpServer, "time.google.com", "time.cloudflare.com");

    LOG_I("[TimeManager] NTP servers: %s, time.google.com, time.cloudflare.com | UTC offset: %+.1f h\n",
          ntpServer, gmtOffsetSec / 3600.0f);
    LOG_I("[TimeManager] Waiting for sync...");

    const uint32_t deadline = millis() + 10000;
    while (millis() < deadline) {
        delay(200);
        LOG_I(".");

        if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED
                && time(nullptr) > 1700000000UL) {
            _syncedThisSession = true;
            break;
        }
    }
    LOG_I("\n");

    if (_syncedThisSession) {
        char buf[32];
        const time_t now = time(nullptr);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        LOG_I("[TimeManager] Synced — local time: %s\n", buf);
    } else {
        LOG_W("[TimeManager] WARNING: NTP sync timed out — timestamp will use RTC fallback.\n");
    }
}

bool TimeManager::isTimeSynced() {
    return _syncedThisSession;
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
        LOG_W("[TimeManager] Clock not synchronized in this session.\n");
        return;
    }
    char buf[32];
    const time_t now = time(nullptr);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    LOG_I("[TimeManager] %s | %llu ms\n", buf, getTimestampMs());
}