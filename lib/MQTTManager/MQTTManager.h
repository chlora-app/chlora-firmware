#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

/**
 * Thin wrapper around knolleary/PubSubClient untuk ESP32.
 *
 * Dirancang untuk pola burst-connect (hemat baterai):
 *   WiFi on → begin() → connect() → publish() → disconnect() → WiFi off → deep sleep
 *
 * Untuk persistent connection (always-on), panggil loop() secara rutin
 * agar keep-alive ping tetap berjalan.
 *
 * All methods are static — no instantiation needed.
 */
class MQTTManager {
public:

    /**
     * Menyimpan konfigurasi broker dan menginisialisasi PubSubClient.
     * Tidak memerlukan WiFi aktif — aman dipanggil di setup() sebelum WiFi connect.
     * Wajib dipanggil sekali sebelum connect().
     *
     * @param broker    Hostname atau IP broker MQTT.
     * @param port      Port TCP — 1883 plain, 8883 TLS.
     * @param clientId  Client ID unik; gunakan DeviceIdentity::getDeviceId().
     */
    static void begin(const char* broker, uint16_t port, const char* clientId);

    /**
     * Konek ke broker. WiFi harus sudah terhubung sebelum memanggil ini.
     * Retry setiap 500 ms sampai sukses atau timeoutMs habis.
     * Mengembalikan true jika berhasil.
     *
     * @param username  MQTT username. Gunakan nullptr untuk akses anonymous.
     * @param password  MQTT password. Gunakan nullptr untuk akses anonymous.
     * @param timeoutMs Waktu maksimal (ms) menunggu koneksi berhasil.
     */
    static bool connectToBroker(const char* username  = nullptr, const char* password  = nullptr, uint32_t    timeoutMs = 5000);

    /** Mengembalikan true jika client sedang terhubung ke broker. */
    static bool isConnected();

    /**
     * Publish payload ke topic yang diberikan (QoS 0).
     * Mengembalikan true jika PubSubClient menerima pesan.
     * Log error dan return false jika tidak terkoneksi atau payload melebihi buffer.
     *
     * @param topic    Path topic lengkap, misal "sensors/DVC-AABBCCDDEEFF/data".
     * @param payload  Null-terminated string (JSON, plain text, dsb).
     * @param retain   Jika true, broker menyimpan pesan terakhir di topic ini.
     */
    static bool publish(const char* topic,
                        const char* payload,
                        bool        retain = false);

    /**
     * Mengirim MQTT DISCONNECT packet dan menutup koneksi TCP.
     * Aman dipanggil meski sudah disconnect.
     */
    static void disconnect();

    /**
     * Menjalankan state machine PubSubClient: dispatch pesan masuk dan
     * keep-alive ping. Panggil minimal sekali per detik untuk persistent
     * connection. Tidak wajib untuk pola burst-connect.
     */
    static void loop();

    /** Print status koneksi, broker, port, dan clientId ke Serial. */
    static void printStatus();

private:
    static WiFiClient   _wifiClient;
    static PubSubClient _mqttClient;
    static const char*  _broker;
    static uint16_t     _port;
    static const char*  _clientId;
};
