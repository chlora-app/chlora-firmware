#include "MQTTManager.h"
#include "WiFiConnector.h"
#include "Config.h"

// ── Static member definitions ──────────────────────────────────────────────────
WiFiClient   MQTTManager::_wifiClient;
PubSubClient MQTTManager::_mqttClient(MQTTManager::_wifiClient);
const char*  MQTTManager::_broker   = nullptr;
uint16_t     MQTTManager::_port     = 1883;
const char*  MQTTManager::_clientId = nullptr;

// ──────────────────────────────────────────────────────────────────────────────

constexpr int MQTT_PUBLISH_RETRIES = 3;
constexpr int MQTT_PUBLISH_RETRY_DELAY_MS = 500;

void MQTTManager::begin(const char* broker, uint16_t port, const char* clientId) {
    _broker   = broker;
    _port     = port;
    _clientId = clientId;

    _mqttClient.setServer(_broker, _port);

    // TX/RX buffer: default 256 B. Payload JSON kita ~145 B + topic + MQTT
    // framing; 512 B memberi headroom aman jika payload bertambah besar.
    _mqttClient.setBufferSize(512);

    // Keep-alive 60 s: broker tidak drop koneksi silent selama minimal 90 s,
    // cukup untuk melewati satu siklus READ_INTERVAL.
    _mqttClient.setKeepAlive(60);

    // Socket read timeout (detik). Mencegah blocking tanpa batas jika broker
    // lambat atau tidak merespons.
    _mqttClient.setSocketTimeout(5);

    Serial.printf("[MQTTManager] Configured — broker: %s:%u | clientId: %s\n",
                  _broker, _port, _clientId);
}


bool MQTTManager::connectToBroker(const char* username, const char* password, uint32_t timeoutMs) {
    if (_broker == nullptr || _clientId == nullptr) {
        Serial.println("[MQTTManager] ERROR: Call begin() before connect().");
        return false;
    }

    if (!WiFiConnector::isConnected()) {
        Serial.println("[MQTTManager] ERROR: WiFi not connected — cannot reach broker.");
        return false;
    }

    Serial.printf("[MQTTManager] Connecting to %s:%u as \"%s\"...", _broker, _port, _clientId);

    const uint32_t deadline = millis() + timeoutMs;
    bool connected = false;

    while (millis() < deadline) {
        // Pilih overload PubSubClient yang sesuai berdasarkan kebutuhan auth.
        connected = (username != nullptr)
            ? _mqttClient.connect(_clientId, username, password)
            : _mqttClient.connect(_clientId);

        if (connected) break;

        Serial.print(".");
        delay(500);
    }

    Serial.println();

    if (connected) {
        Serial.printf("[MQTTManager] Connected — broker: %s:%u\n",
                      _broker, _port);
    } else {
        // PubSubClient rc codes:
        //  -4 MQTT_CONNECTION_TIMEOUT     -3 MQTT_CONNECTION_LOST
        //  -2 MQTT_CONNECT_FAILED         -1 MQTT_DISCONNECTED
        //   1 MQTT_CONNECT_BAD_PROTOCOL    2 MQTT_CONNECT_BAD_CLIENT_ID
        //   3 MQTT_CONNECT_UNAVAILABLE     4 MQTT_CONNECT_BAD_CREDENTIALS
        //   5 MQTT_CONNECT_UNAUTHORIZED
        Serial.printf("[MQTTManager] ERROR: Connection failed — rc=%d\n", _mqttClient.state());
    }

    return connected;
}

bool MQTTManager::isConnected() {
    return _mqttClient.connected();
}

bool MQTTManager::publish(const char* topic, const char* payload, bool retain) {
    if (!isConnected()) {
        Serial.println("[MQTTManager] ERROR: Not connected — cannot publish.");
        return false;
    }

    bool ok = false;

    for (uint8_t attempt = 1; attempt <= MQTT_PUBLISH_RETRIES; attempt++) {
        ok = _mqttClient.publish(topic, payload, retain);

        if (ok) {
            Serial.printf("[MQTTManager] Published → topic: \"%s\" | %u bytes"
                          " (attempt %u/%u)\n",
                          topic,
                          strlen(payload),
                          attempt,
                          MQTT_PUBLISH_RETRIES);
            return true;
        }

        Serial.printf("[MQTTManager] WARN: Publish failed — attempt %u/%u, "
                      "retrying in %u ms...\n",
                      attempt, MQTT_PUBLISH_RETRIES, MQTT_PUBLISH_RETRY_DELAY_MS);

        delay(MQTT_PUBLISH_RETRY_DELAY_MS);

        // Pompa state machine agar koneksi tetap hidup antar retry.
        _mqttClient.loop();
    }

    Serial.printf("[MQTTManager] ERROR: Publish failed after %u attempts — "
                  "topic: \"%s\"\n", MQTT_PUBLISH_RETRIES, topic);
    return false;
}


void MQTTManager::disconnect() {
    if (isConnected()) {
        _mqttClient.disconnect();
        Serial.println("[MQTTManager] Disconnected.");
    }
}

void MQTTManager::loop() {
    _mqttClient.loop();
}

void MQTTManager::printStatus() {
    if (isConnected()) {
        Serial.printf("[MQTTManager] Status: CONNECTED | broker: %s:%u | clientId: %s\n",
                      _broker, _port, _clientId);
    } else {
        Serial.printf("[MQTTManager] Status: DISCONNECTED | rc=%d\n",
                      _mqttClient.state());
    }
}
