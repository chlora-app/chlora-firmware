#include "WiFiConnector.h"
#include "Config.h"

bool WiFiConnector::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
    Serial.printf("[WiFiConnector] Connecting to \"%s\"...", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    const uint32_t deadline = millis() + timeoutMs;
    while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFiConnector] ERROR: Connection timed out.");
        return false;
    }

    Serial.printf("[WiFiConnector] Connected — IP: %s | RSSI: %d dBm\n",
                  WiFi.localIP().toString().c_str(),
                  WiFi.RSSI());
    return true;
}

bool WiFiConnector::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiConnector::disconnect() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFiConnector] Disconnected.");
}

void WiFiConnector::printStatus() {
    if (isConnected()) {
        Serial.printf("[WiFiConnector] SSID: %s | IP: %s | RSSI: %d dBm\n",
                      WiFi.SSID().c_str(),
                      WiFi.localIP().toString().c_str(),
                      WiFi.RSSI());
    } else {
        Serial.println("[WiFiConnector] Not connected.");
    }
}
