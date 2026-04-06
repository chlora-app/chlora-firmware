#include "SleepManager.h"
#include <esp_sleep.h>

// RTC_DATA_ATTR survives deep sleep cycles.
// Stores the last configured sleep duration for printStatus() reporting.
RTC_DATA_ATTR uint32_t SleepManager::_lastSleepDurationMs = 0;

// ──────────────────────────────────────────────────────────────────────────────

void SleepManager::deepSleep(uint32_t durationMs) {
    _lastSleepDurationMs = durationMs;

    // Convert ms → µs for esp_sleep API.
    const uint64_t durationUs = static_cast<uint64_t>(durationMs) * 1000ULL;

    Serial.printf("[SleepManager] Entering deep sleep for %u ms...\n", durationMs);
    Serial.flush(); // Flush Serial buffer before power-down.

    esp_sleep_enable_timer_wakeup(durationUs);
    esp_deep_sleep_start(); // Never returns.
}

// ──────────────────────────────────────────────────────────────────────────────

bool SleepManager::isWakeFromSleep() {
    return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;
}

// ──────────────────────────────────────────────────────────────────────────────

void SleepManager::printStatus() {
    const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.printf("[SleepManager] Wake reason: TIMER | last sleep: %u ms\n",
                          _lastSleepDurationMs);
        break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            Serial.println("[SleepManager] Wake reason: POWER-ON / RESET");
        break;
        default:
            Serial.printf("[SleepManager] Wake reason: OTHER (cause=%d)\n",
                          static_cast<int>(cause));
        break;
    }
}
