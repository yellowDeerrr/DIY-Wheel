#include <ESP32Encoder.h>
#include "Arduino.h"
#include <WiFi.h>
#include <esp_now.h>
#include "protocol.h"
#include <ArduinoOTA.h>

const char *ssid = "YOUR_HOME_WIFI_SSID";
const char *password = "YOUR_HOME_WIFI_PASSWORD";
bool otaMode = false;

uint8_t baseMac[] = {0x80, 0xB5, 0x4E, 0xC5, 0xED, 0x84};
WheelPacket packet;

// Battery
#define BATTERY_PIN 4

#define BATTERY_R1 10000.0f
#define BATTERY_R2 10000.0f

#define BATTERY_MEASURE_INTERVAL 500

#define BATTERY_LOW_VOLTAGE 3.30f

float batteryVoltage = 0.0f;
uint16_t batteryVoltage_mV = 0;

unsigned long lastBatteryMeasure = 0;

// Encoder
#define ENC_A 14
#define ENC_B 3

ESP32Encoder encoder;
long lastAckedPosition = 0;

// Timings
const unsigned long MIN_SEND_INTERVAL = 20;
const unsigned long HEARTBEAT_INTERVAL = 100;
const unsigned long DEBOUNCE_TIME = 30;

bool buttonState[BUTTON_COUNT];
bool lastReading[BUTTON_COUNT];
unsigned long lastDebounceTime[BUTTON_COUNT];

uint32_t lastSentButtons = 0;
unsigned long lastSendTime = 0;
uint8_t lastSentPOV = POV_CENTER;

uint16_t getButtonState()
{
    uint16_t state = 0;

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (buttons[i].function != BUTTON_NORMAL)
            continue;
        if (buttonState[i] == LOW)
        {
            state |=
                (1U << buttons[i].bitIndex);
        }
    }

    return state;
}

bool isPOVPressed(ButtonFunction function)
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (buttons[i].function != function)
            continue;

        return buttonState[i] == LOW;
    }

    return false;
}

uint8_t getPOVState()
{
    bool up =
        isPOVPressed(BUTTON_POV_UP);

    bool right =
        isPOVPressed(BUTTON_POV_RIGHT);

    bool down =
        isPOVPressed(BUTTON_POV_DOWN);

    bool left =
        isPOVPressed(BUTTON_POV_LEFT);

    // -------------------------------------------------
    // Opposite directions = CENTER
    // -------------------------------------------------

    if ((up && down) || (left && right))
    {
        return POV_CENTER;
    }

    // -------------------------------------------------
    // Diagonals
    // -------------------------------------------------

    if (up && right)
        return POV_UP_RIGHT;

    if (right && down)
        return POV_DOWN_RIGHT;

    if (down && left)
        return POV_DOWN_LEFT;

    if (left && up)
        return POV_UP_LEFT;

    // -------------------------------------------------
    // Cardinal directions
    // -------------------------------------------------

    if (up)
        return POV_UP;

    if (right)
        return POV_RIGHT;

    if (down)
        return POV_DOWN;

    if (left)
        return POV_LEFT;

    return POV_CENTER;
}
float readBatteryVoltage()
{
    const int samples = 16;

    uint64_t sum_mV = 0;

    for (int i = 0; i < samples; i++)
    {
        sum_mV += analogReadMilliVolts(BATTERY_PIN);
        delayMicroseconds(100);
    }

    float pinVoltage =
        (sum_mV / (float)samples) / 1000.0f;

    float batteryVoltage =
        pinVoltage *
        ((BATTERY_R1 + BATTERY_R2) / BATTERY_R2);

    Serial.print("ADC GPIO4: ");
    Serial.print(pinVoltage, 3);
    Serial.print(" V | Battery: ");
    Serial.print(batteryVoltage, 3);
    Serial.println(" V");

    return batteryVoltage;
}
void checkBatteryAtStartup()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("       BATTERY CHECK");
    Serial.println("================================");

    delay(100);

    batteryVoltage = readBatteryVoltage();
    batteryVoltage_mV = (uint16_t)(batteryVoltage * 1000.0f);

    Serial.print("Battery voltage: ");
    Serial.print(batteryVoltage, 2);
    Serial.println(" V");

    if (batteryVoltage >= 4.15f)
    {
        Serial.println("Battery status: FULL");
    }
    else if (batteryVoltage >= 3.85f)
    {
        Serial.println("Battery status: GOOD");
    }
    else if (batteryVoltage >= 3.50f)
    {
        Serial.println("Battery status: MEDIUM");
    }
    else if (batteryVoltage >= BATTERY_LOW_VOLTAGE)
    {
        Serial.println("Battery status: LOW");
    }
    else
    {
        Serial.println("WARNING: BATTERY VOLTAGE TOO LOW!");
    }

    Serial.println("================================");
    Serial.println();
}
void updateBatteryVoltage(unsigned long now)
{
    if (now - lastBatteryMeasure < BATTERY_MEASURE_INTERVAL)
        return;

    lastBatteryMeasure = now;

    batteryVoltage = readBatteryVoltage();
    batteryVoltage_mV = (uint16_t)(batteryVoltage * 1000.0f);

    Serial.print("Battery: ");
    Serial.print(batteryVoltage, 2);
    Serial.print(" V");

    if (batteryVoltage >= 4.15f)
    {
        Serial.println(" | FULL");
    }
    else if (batteryVoltage >= 3.85f)
    {
        Serial.println(" | GOOD");
    }
    else if (batteryVoltage >= 3.50f)
    {
        Serial.println(" | MEDIUM");
    }
    else if (batteryVoltage >= BATTERY_LOW_VOLTAGE)
    {
        Serial.println(" | LOW");
    }
    else
    {
        Serial.println(" | CRITICAL");
    }
}

bool checkOtaButton()
{
    int encButtonPin = -1;

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (strcmp(buttons[i].name, "ENC_BTN") == 0)
        {
            encButtonPin = buttons[i].pin;
            break;
        }
    }

    if (encButtonPin == -1)
        return false;

    pinMode(encButtonPin, INPUT_PULLUP);
    delay(50);
    if (digitalRead(encButtonPin) == LOW)
    {
        return true;
    }

    return false;
}

void startOtaMode()
{
    Serial.println("\n[OTA MODE ACTIVATED via ENC_BTN]");

    Serial.println("\n[OTA MODE: AP (Access Point) ACTIVATED via ENC_BTN]");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    Serial.println(WiFi.localIP());

    ArduinoOTA.setHostname("SteeringWheel-ESP32");

    ArduinoOTA.onStart([]()
                       { Serial.println("Start updating firmware..."); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nUpdate complete! Rebooting..."); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       { Serial.printf("Error[%u]: ", error); });

    ArduinoOTA.begin();
    Serial.println("Ready for OTA updates. Waiting for upload...");
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    pinMode(BATTERY_PIN, INPUT);

    analogReadResolution(12);
    analogSetPinAttenuation(
        BATTERY_PIN,
        ADC_11db);

    checkBatteryAtStartup();

    if (checkOtaButton())
    {
        otaMode = true;
        startOtaMode();
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.print("Wheel MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW initialization failed!");
        return;
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, baseMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer!");
        return;
    }

    packet.magic = 0xABCD;
    packet.sequence = 0;

    packet.buttons = 0;
    packet.pov = POV_CENTER;
    packet.encoderDelta = 0;

    // Normal buttons
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        pinMode(
            buttons[i].pin,
            INPUT_PULLUP);

        bool state =
            digitalRead(buttons[i].pin);

        buttonState[i] = state;
        lastReading[i] = state;

        lastDebounceTime[i] = 0;
    }

    // Encoder
    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    encoder.attachSingleEdge(ENC_A, ENC_B);
    encoder.setCount(0);
    lastAckedPosition = 0;

    Serial.println("\n================================");
    Serial.println("       WHEEL BUTTON TESTER");
    Serial.println("================================");
    Serial.println("Ready!\n");
}

void buttonsLoop(unsigned long now)
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        bool reading = digitalRead(buttons[i].pin);

        if (reading != lastReading[i])
        {
            lastDebounceTime[i] = now;
        }

        if ((now - lastDebounceTime[i]) > DEBOUNCE_TIME)
        {
            if (reading != buttonState[i])
            {
                buttonState[i] = reading;
                if (buttonState[i] == LOW)
                {
                    Serial.print("BUTTON -> ");
                    Serial.print(buttons[i].name);
                    Serial.println("  PRESSED");
                }
                else
                {
                    Serial.print("BUTTON -> ");
                    Serial.print(buttons[i].name);
                    Serial.println("  RELEASED");
                }
            }
        }
        lastReading[i] = reading;
    }
}

void sendPacket(unsigned long now)
{
    int64_t currentPosition = encoder.getCount();
    int16_t currentDelta = (currentPosition - lastAckedPosition);
    uint32_t currentButtons = getButtonState();

    uint8_t currentPOV = getPOVState();

    bool stateChanged = (currentDelta != 0) || (currentButtons != lastSentButtons || currentPOV != lastSentPOV);
    bool timeToHeartbeat = (now - lastSendTime >= HEARTBEAT_INTERVAL);

    if (stateChanged || timeToHeartbeat)
    {
        if (now - lastSendTime < MIN_SEND_INTERVAL)
            return;

        lastSendTime = now;

        packet.buttons = currentButtons;
        packet.encoderDelta = currentDelta;
        packet.pov = currentPOV;
        packet.sequence++;

        esp_err_t result = esp_now_send(baseMac, (uint8_t *)&packet, sizeof(packet));

        if (result == ESP_OK)
        {
            lastAckedPosition = currentPosition;
            lastSentButtons = currentButtons;
            lastSentPOV = currentPOV;

            if (currentDelta > 0)
                Serial.println("ENC->RIGHT (Sent)");
            else if (currentDelta < 0)
                Serial.println("ENC->LEFT (Sent)");
        }
        else
        {
            Serial.println("Packet failed to send. Retrying next cycle...");
        }
    }
}

void loop()
{
    unsigned long now = millis();

    if (otaMode)
    {
        ArduinoOTA.handle();
        delay(1);
        return;
    }

    updateBatteryVoltage(now);
    buttonsLoop(now);
    sendPacket(now);
    delay(10);
}