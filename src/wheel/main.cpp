#include <ESP32Encoder.h>
#include "Arduino.h"
#include <WiFi.h>
#include <esp_now.h>
#include "protocol.h"

uint8_t baseMac[] = {0x80, 0xB5, 0x4E, 0xC5, 0xED, 0x84};

WheelPacket packet;

// =========================
// Encoder
// =========================
#define ENC_A 14
#define ENC_B 3

ESP32Encoder encoder;
long lastAckedPosition = 0; // Остання успішно підтверджена базами позиція

// =========================
// Timings
// =========================
const unsigned long MIN_SEND_INTERVAL = 20;   // Анти-спам (мс)
const unsigned long HEARTBEAT_INTERVAL = 100; // Keep-Alive (мс)
const unsigned long DEBOUNCE_TIME = 30;

bool buttonState[BUTTON_COUNT];
bool lastReading[BUTTON_COUNT];
unsigned long lastDebounceTime[BUTTON_COUNT];

// Для оптимізації відправки
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

// =====================================================
// CHECK POV DIRECTION
// =====================================================

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

// =====================================================
// GET POV
// =====================================================

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

void setup()
{
    Serial.begin(115200);
    delay(1500);

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
    // 1. Поточний стан з апаратного лічильника
    int64_t currentPosition = encoder.getCount();
    int16_t currentDelta = (currentPosition - lastAckedPosition);
    uint32_t currentButtons = getButtonState();

    uint8_t currentPOV = getPOVState();

    // 2. Умови для відправки
    bool stateChanged = (currentDelta != 0) || (currentButtons != lastSentButtons || currentPOV != lastSentPOV);
    bool timeToHeartbeat = (now - lastSendTime >= HEARTBEAT_INTERVAL);

    if (stateChanged || timeToHeartbeat)
    {
        // Анти-спам інтервал
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
            // Оновлюємо точки відліку ТІЛЬКИ при успішній доставці
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
            // Якщо пакет не дійшов, lastAckedPosition не змінюється,
            // і дельта автоматично підсумується в наступному пакеті!
        }
    }
}

void loop()
{
    unsigned long now = millis();

    buttonsLoop(now);
    sendPacket(now);
}