#pragma once

#include <Arduino.h>

/**
 * Manages ESP32 deep sleep cycles.
 *
 * Deep sleep cuts power to CPU, RAM, and most peripherals — only the
 * RTC domain stays alive. Current draw drops from ~240 mA (active WiFi)
 * to ~10 µA during sleep, making it essential for battery-powered devices.
 *
 * Important: deep sleep is a full chip reset. Execution always resumes
 * from setup(), not from where deepSleep() was called.
 *
 * All methods are static — no instantiation needed.
 */
class SleepManager {
public:

    /**
     * Enters deep sleep for the given duration, then resets.
     * This function never returns — execution resumes at setup().
     *
     * Call this as the very last statement in loop(), after all sensors
     * have been read, payload published, and connections closed.
     *
     * @param durationMs Sleep duration in milliseconds.
     */
    static void deepSleep(uint32_t durationMs);

    /**
     * Returns true if this boot was caused by a timer wake-up from deep sleep.
     * Returns false on first power-on or manual reset.
     * Useful for skipping one-time init (e.g. NTP sync) on wake cycles.
     */
    static bool isWakeFromSleep();

    /** Prints wake reason and sleep duration to Serial. */
    static void printStatus();

private:
    // Duration stored in RTC memory — survives deep sleep, cleared on power-on.
    static RTC_DATA_ATTR uint32_t _lastSleepDurationMs;
};
