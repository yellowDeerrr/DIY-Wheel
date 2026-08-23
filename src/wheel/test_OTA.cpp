#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"

// =====================================================
// Wi-Fi
// =====================================================

const char *ssid = "TP-Link_67C6";
const char *password = "tarik34379387";

// =====================================================
// Timing
// =====================================================

unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 2000;

// =====================================================
// OTA progress tracking
// =====================================================

unsigned int lastProgress = 0;

// =====================================================
// Setup
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("      ESP32-S3 SIMPLE OTA TEST");
    Serial.println("========================================");

    // -------------------------------------------------
    // Chip information
    // -------------------------------------------------

    Serial.println();
    Serial.println("[SYSTEM] Chip information:");

    Serial.printf("[SYSTEM] Chip model: %s\n", ESP.getChipModel());
    Serial.printf("[SYSTEM] Chip revision: %d\n", ESP.getChipRevision());
    Serial.printf("[SYSTEM] CPU frequency: %u MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("[SYSTEM] Flash size: %u bytes\n", ESP.getFlashChipSize());
    Serial.printf("[SYSTEM] Free heap: %u bytes\n", ESP.getFreeHeap());

    // -------------------------------------------------
    // Current OTA partition
    // -------------------------------------------------

    Serial.println();
    Serial.println("[OTA] Current partition information:");

    const esp_partition_t *running =
        esp_ota_get_running_partition();

    if (running != nullptr)
    {
        Serial.printf("[OTA] Running partition: %s\n", running->label);
        Serial.printf("[OTA] Running type: %d\n", running->type);
        Serial.printf("[OTA] Running subtype: %d\n", running->subtype);
        Serial.printf("[OTA] Running address: 0x%08X\n", running->address);
        Serial.printf("[OTA] Running size: 0x%08X (%u bytes)\n",
                      running->size,
                      running->size);
    }
    else
    {
        Serial.println("[OTA] ERROR: Could not get running partition!");
    }

    // -------------------------------------------------
    // Next OTA partition
    // -------------------------------------------------

    const esp_partition_t *next =
        esp_ota_get_next_update_partition(nullptr);

    if (next != nullptr)
    {
        Serial.printf("[OTA] Next OTA partition: %s\n", next->label);
        Serial.printf("[OTA] Next type: %d\n", next->type);
        Serial.printf("[OTA] Next subtype: %d\n", next->subtype);
        Serial.printf("[OTA] Next address: 0x%08X\n", next->address);
        Serial.printf("[OTA] Next size: 0x%08X (%u bytes)\n",
                      next->size,
                      next->size);
    }
    else
    {
        Serial.println("[OTA] ERROR: Could not get next OTA partition!");
    }

    // -------------------------------------------------
    // Wi-Fi
    // -------------------------------------------------

    Serial.println();
    Serial.println("[WIFI] Connecting...");
    Serial.printf("[WIFI] SSID: %s\n", ssid);

    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    unsigned long wifiStart = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);

        Serial.print(".");

        if (millis() - wifiStart > 30000)
        {
            Serial.println();
            Serial.println("[WIFI] ERROR: Connection timeout!");
            Serial.printf("[WIFI] Status: %d\n", WiFi.status());

            return;
        }
    }

    Serial.println();
    Serial.println("[WIFI] Connected!");

    Serial.printf("[WIFI] IP address: %s\n",
                  WiFi.localIP().toString().c_str());

    Serial.printf("[WIFI] Gateway: %s\n",
                  WiFi.gatewayIP().toString().c_str());

    Serial.printf("[WIFI] Subnet: %s\n",
                  WiFi.subnetMask().toString().c_str());

    Serial.printf("[WIFI] RSSI: %d dBm\n",
                  WiFi.RSSI());

    Serial.printf("[WIFI] MAC: %s\n",
                  WiFi.macAddress().c_str());

    Serial.printf("[WIFI] Channel: %d\n",
                  WiFi.channel());

    Serial.println();

    // -------------------------------------------------
    // OTA configuration
    // -------------------------------------------------

    Serial.println("[OTA] Configuring ArduinoOTA...");

    ArduinoOTA.setHostname("ESP32-S3-OTA-TEST");

    // Optional password:
    // ArduinoOTA.setPassword("123456");

    ArduinoOTA.onStart([]()
                       {
        Serial.println();
        Serial.println("========================================");
        Serial.println("[OTA] UPDATE STARTED");
        Serial.println("========================================");

        Serial.printf("[OTA] Free heap BEFORE update: %u bytes\n",
                      ESP.getFreeHeap());

        const esp_partition_t *running =
            esp_ota_get_running_partition();

        const esp_partition_t *next =
            esp_ota_get_next_update_partition(nullptr);

        if (running)
        {
            Serial.printf("[OTA] Running: %s @ 0x%08X\n",
                          running->label,
                          running->address);
        }

        if (next)
        {
            Serial.printf("[OTA] Target:  %s @ 0x%08X\n",
                          next->label,
                          next->address);

            Serial.printf("[OTA] Target size: %u bytes\n",
                          next->size);
        }

        lastProgress = 0; });

    ArduinoOTA.onProgress([](unsigned int progress,
                             unsigned int total)
                          {
        unsigned int percent = 0;

        if (total > 0)
            percent = (progress * 100) / total;

        // Print every 5%
        if (percent >= lastProgress + 5 || percent == 100)
        {
            lastProgress = percent;

            Serial.printf(
                "[OTA] Progress: %u%%  (%u / %u bytes)\n",
                percent,
                progress,
                total
            );
        } });

    ArduinoOTA.onEnd([]()
                     {
        Serial.println();
        Serial.println("========================================");
        Serial.println("[OTA] UPDATE FINISHED");
        Serial.println("========================================");

        Serial.printf("[OTA] Free heap AFTER update: %u bytes\n",
                      ESP.getFreeHeap());

        Serial.println("[OTA] Device should reboot..."); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.println();
        Serial.println("========================================");
        Serial.println("[OTA] UPDATE ERROR");
        Serial.println("========================================");

        Serial.printf("[OTA] Error code: %u\n", error);

        switch (error)
        {
            case OTA_AUTH_ERROR:
                Serial.println("[OTA] OTA_AUTH_ERROR");
                break;

            case OTA_BEGIN_ERROR:
                Serial.println("[OTA] OTA_BEGIN_ERROR");
                break;

            case OTA_CONNECT_ERROR:
                Serial.println("[OTA] OTA_CONNECT_ERROR");
                break;

            case OTA_RECEIVE_ERROR:
                Serial.println("[OTA] OTA_RECEIVE_ERROR");
                break;

            case OTA_END_ERROR:
                Serial.println("[OTA] OTA_END_ERROR");
                break;

            default:
                Serial.println("[OTA] Unknown OTA error");
                break;
        }

        Serial.printf("[OTA] Free heap: %u bytes\n",
                      ESP.getFreeHeap()); });

    // -------------------------------------------------
    // Start OTA
    // -------------------------------------------------

    Serial.println("[OTA] Calling ArduinoOTA.begin()...");

    ArduinoOTA.begin();

    Serial.println("[OTA] ArduinoOTA.begin() completed!");

    Serial.println();
    Serial.println("========================================");
    Serial.println("       READY FOR OTA UPDATE");
    Serial.println("========================================");

    Serial.printf("[OTA] Hostname: ESP32-S3-OTA-TEST\n");
    Serial.printf("[OTA] IP: %s\n",
                  WiFi.localIP().toString().c_str());

    Serial.println();
}

// =====================================================
// Loop
// =====================================================

void loop()
{
    // IMPORTANT:
    // ArduinoOTA.handle() must be called continuously.

    ArduinoOTA.handle();

    // -------------------------------------------------
    // Heartbeat
    // -------------------------------------------------

    unsigned long now = millis();

    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL)
    {
        lastHeartbeat = now;

        Serial.printf(
            "[RUNNING] uptime=%lu s | heap=%u | RSSI=%d dBm\n",
            millis() / 1000,
            ESP.getFreeHeap(),
            WiFi.RSSI());
    }

    // Very short delay is OK.
    delay(1);
}