
#include "Arduino.h"
#include <Wire.h>
#include "USB.h"
#include "USBHID.h"
#include <WiFi.h>
#include <esp_now.h>
#include "protocol.h"

#define SDA_PIN 8
#define SCL_PIN 9
#define AS5600_ADDR 0x36

// =====================================================
// PEDAL CALIBRATION
// =====================================================
#define PEDAL_PIN 4

#define PEDAL_MIN 0
#define PEDAL_CENTER 1949
#define PEDAL_MAX 4096
// =====================================================
// STEERING
// =====================================================
// AS5600 = 4096 positions per revolution
// 4096 = 360 degrees
//
// Desired range:
// -360° ... 0° ... +360°
//
// Therefore:
// -4096 ... 0 ... +4096
#define STEERING_MIN -4096
#define STEERING_MAX 4096
// =====================================================
// CENTER
// =====================================================
uint16_t centerAngle = 0;

// =====================================================
// ESP-NOW
// =====================================================
volatile bool packetReceived = false;

WheelPacket receivedPacket;

// Стан кнопок з керма
uint16_t currentButtonState = 0;

// Поточна дельта енкодера
int16_t encoderDelta = 0;
int16_t currentEncoderDelta = 0;
uint8_t currentPOV = POV_CENTER;

// Sequence
uint8_t lastSequence = 0;
bool firstPacket = true;

// Callback функція прийому даних від керма
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    if (len == sizeof(WheelPacket))
    {
        memcpy(&receivedPacket, incomingData, sizeof(WheelPacket));
        packetReceived = true;
    }
}

// =====================================================
// HID REPORT DESCRIPTOR
//
// X = steering
// Y = pedals
// Z = MAG
// =====================================================
static const uint8_t wheelReportDescriptor[] = {
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x04, // Usage (Joystick)
    0xA1, 0x01, // Collection (Application)
    // =================================================
    // X AXIS - STEERING
    // =================================================
    0x09, 0x30,       // Usage (X)
    0x16, 0x00, 0x80, // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F, // Logical Maximum (32767)
    0x75, 0x10,       // Report Size = 16 bits
    0x95, 0x01,       // Report Count = 1
    0x81, 0x02,       // Input (Data, Variable, Absolute)
    // =================================================
    // Y AXIS - PEDALS
    // =================================================
    0x09, 0x31,       // Usage (Y)
    0x16, 0x00, 0x80, // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F, // Logical Maximum (32767)
    0x75, 0x10,       // Report Size = 16 bits
    0x95, 0x01,       // Report Count = 1
    0x81, 0x02,       // Input (Data, Variable, Absolute)
    // =================================================
    // Z AXIS - MAG
    // =================================================
    0x09, 0x32,       // Usage (Z)
    0x15, 0x00,       // Logical Minimum (0)
    0x26, 0xFF, 0x0F, // Logical Maximum (4095)
    0x75, 0x10,       // Report Size = 16 bits
    0x95, 0x01,       // Report Count = 1
    0x81, 0x02,       // Input (Data, Variable, Absolute)
    // =================================================
    // BUTTONS - 16
    // =================================================
    0x05, 0x09, // Usage Page (Button)

    0x19, 0x01, // Usage Minimum (Button 1)
    0x29, 0x10, // Usage Maximum (Button 16)

    0x15, 0x00, // Logical Minimum = 0
    0x25, 0x01, // Logical Maximum = 1

    0x75, 0x01, // Report Size = 1 bit
    0x95, 0x10, // Report Count = 32

    0x81, 0x02, // Input (Data, Variable, Absolute)
    // =================================================
    // POV HAT
    // =================================================

    0x05, 0x01,

    0x09, 0x39,

    0x15, 0x00,
    0x25, 0x07,

    0x35, 0x00,
    0x46, 0x3B, 0x01,

    0x65, 0x14,

    0x75, 0x04,
    0x95, 0x01,

    0x81, 0x42,
    // =================================================
    // 4 BIT PADDING
    // =================================================

    0x75, 0x04,
    0x95, 0x01,

    0x81, 0x03,

    // =================================================
    0xC0 // End Collection
};
// =====================================================
// HID DEVICE
// =====================================================
class WheelHIDDevice : public USBHIDDevice
{
public:
    WheelHIDDevice()
    {
        static bool initialized = false;
        if (!initialized)
        {
            initialized = true;
            HID.addDevice(
                this,
                sizeof(wheelReportDescriptor));
        }
    }
    uint16_t _onGetDescriptor(uint8_t *buffer) override
    {
        memcpy(
            buffer,
            wheelReportDescriptor,
            sizeof(wheelReportDescriptor));
        return sizeof(wheelReportDescriptor);
    }
    bool ready()
    {
        return HID.ready();
    }
    // =================================================
    // SEND X + Y + Z
    // =================================================
    bool send(
        int16_t steering,
        int16_t pedals,
        uint16_t mag,
        uint16_t buttons,
        uint8_t pov)
    {
        struct __attribute__((packed))
        {
            int16_t steering;
            int16_t pedals;
            uint16_t mag;
            uint16_t buttons;
            uint8_t pov;
        } report;

        report.steering = steering;
        report.pedals = pedals;
        report.mag = mag;
        report.buttons = buttons;
        report.pov = pov;
        static_assert(
            sizeof(report) == 9,
            "HID report size must be 10 bytes");
        return HID.SendReport(
            0,
            &report,
            sizeof(report));
    }
    void begin()
    {
        HID.begin();
    }

private:
    USBHID HID;
};
WheelHIDDevice wheel;
// =====================================================
// AS5600 READ 16-BIT
// =====================================================
uint16_t readAS5600(uint8_t reg)
{
    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return 0;
    if (Wire.requestFrom(AS5600_ADDR, (uint8_t)2) != 2)
        return 0;
    uint8_t high = Wire.read();
    uint8_t low = Wire.read();
    return ((uint16_t)high << 8) | low;
}
// =====================================================
// AS5600 RAW ANGLE
// =====================================================
uint16_t readAngle()
{
    return readAS5600(0x0C) & 0x0FFF;
}
// =====================================================
// AS5600 MAG
// Register 0x1B / 0x1C
// =====================================================
uint16_t readMagnitude()
{
    return readAS5600(0x1B) & 0x0FFF;
}
// =====================================================
// PEDAL
// =====================================================
int16_t readPedals(int raw)
{
    raw = constrain(
        raw,
        PEDAL_MIN,
        PEDAL_MAX);
    if (raw < PEDAL_CENTER)
    {
        return map(raw, PEDAL_MIN, PEDAL_CENTER, -32768, 0);
    }
    else
    {
        return map(raw, PEDAL_CENTER, PEDAL_MAX, 0, 32767);
    }
}
// =====================================================
// ENCODER VIRTUAL BUTTON BITS
// =====================================================
//
// Normal buttons are automatically numbered
// from protocol.h.
//
// Encoder direction buttons occupy the next bits.
//
// =====================================================

uint16_t getFinalButtons()
{
    uint16_t result = currentButtonState;

    if (encoderDelta > 0)
        result |= (1U << NORMAL_BUTTON_COUNT);
    else if (encoderDelta < 0)
        result |= (1U << (NORMAL_BUTTON_COUNT + 1));

    return result;
}
// =====================================================
// SETUP
// =====================================================
void setup()
{
    Serial0.begin(115200);
    delay(1000);

    Wire.begin(SDA_PIN, SCL_PIN);
    centerAngle = readAngle();

    analogReadResolution(12);
    pinMode(PEDAL_PIN, INPUT);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial0.print("Receiver MAC: ");
    Serial0.println(
        WiFi.macAddress());

    if (esp_now_init() != ESP_OK)
    {
        Serial0.println("ESP-NOW Init Failed!");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    Serial0.println(
        "ESP-NOW initialized.");

    // =================================================
    // USB HID
    // =================================================

    wheel.begin();
    USB.begin();

    Serial0.println("USB HID initialized.");
    Serial0.println();
    Serial0.println("====================================");
    Serial0.println("      WHEEL RECEIVER READY");
    Serial0.println("====================================");
}

void loop()
{
    // =================================================
    // 1. ESP-NOW
    // =================================================

    if (packetReceived)
    {
        packetReceived = false;

        // ---------------------------------------------
        // CHECK MAGIC
        // ---------------------------------------------

        if (receivedPacket.magic != 0xABCD)
        {
            Serial0.println(
                "Invalid packet!");
        }
        else
        {
            // -----------------------------------------
            // SAVE BUTTON STATE
            // -----------------------------------------

            currentButtonState =
                receivedPacket.buttons;

            // -----------------------------------------
            // SAVE ENCODER DELTA
            // -----------------------------------------

            currentEncoderDelta = receivedPacket.encoderDelta;

            encoderDelta += currentEncoderDelta;

            currentPOV = receivedPacket.pov;

            // -----------------------------------------
            // SEQUENCE DEBUG
            // -----------------------------------------

            if (firstPacket)
            {
                lastSequence =
                    receivedPacket.sequence;

                firstPacket = false;
            }
            else
            {
                lastSequence =
                    receivedPacket.sequence;
            }
        }
    }
    // else
    // {
    //     // Encoder is an impulse.
    //     //
    //     // Buttons remain stored,
    //     // encoder delta should not.

    //     currentEncoderDelta = 0;
    // }

    // =================================================
    // 2. FINAL HID BUTTON MASK
    // =================================================

    uint16_t finalHIDButtons = getFinalButtons();

    currentEncoderDelta = 0;

    // -------------------------------------------------
    // ENCODER RIGHT
    // HID BUTTON 18
    // bit 17
    // -------------------------------------------------

    // if (currentEncoderDelta > 0)
    // {
    //     finalHIDButtons |=
    //         (1UL << 17);
    // }

    // // -------------------------------------------------
    // // ENCODER LEFT
    // // HID BUTTON 19
    // // bit 18
    // // -------------------------------------------------

    // else if (currentEncoderDelta < 0)
    // {
    //     finalHIDButtons |=
    //         (1UL << 18);
    // }

    // =================================================
    // 3. STEERING
    // =================================================

    static uint16_t lastAngle = 0;

    static int32_t accumulatedAngle = 0;

    static bool firstAngle = true;

    uint16_t angle =
        readAngle();

    if (firstAngle)
    {
        lastAngle = angle;
        firstAngle = false;
    }

    // -------------------------------------------------
    // ANGLE DELTA
    // -------------------------------------------------

    int32_t delta =
        (int32_t)angle -
        (int32_t)lastAngle;

    // -------------------------------------------------
    // HANDLE 0° / 360° WRAP
    // -------------------------------------------------

    if (delta > 2048)
    {
        delta -= 4096;
    }
    else if (delta < -2048)
    {
        delta += 4096;
    }

    // -------------------------------------------------
    // ACCUMULATE
    // -------------------------------------------------

    accumulatedAngle += delta;

    lastAngle = angle;

    // -------------------------------------------------
    // LIMIT WHEEL
    // -------------------------------------------------

    accumulatedAngle =
        constrain(
            accumulatedAngle,
            STEERING_MIN,
            STEERING_MAX);

    // -------------------------------------------------
    // CONVERT TO HID
    // -------------------------------------------------

    int16_t steering =
        map(
            accumulatedAngle,
            STEERING_MIN,
            STEERING_MAX,
            -32768,
            32767);

    // =================================================
    // 4. PEDALS
    // =================================================

    static int oldPedalsRawValue = 0;

    static int16_t oldPedalsValue = 0;

    int pedalRaw =
        analogRead(PEDAL_PIN);

    int16_t pedals;

    // -------------------------------------------------
    // SIMPLE FILTER
    // -------------------------------------------------

    if (
        abs(
            pedalRaw -
            oldPedalsRawValue) > 100)
    {
        pedals =
            readPedals(
                pedalRaw);

        oldPedalsRawValue =
            pedalRaw;

        oldPedalsValue =
            pedals;
    }
    else
    {
        pedals =
            oldPedalsValue;
    }

    // =================================================
    // 5. MAG
    // =================================================

    uint16_t mag =
        readMagnitude();

    // =================================================
    // 6. SEND HID
    // =================================================

    static uint32_t lastHID = 0;

    static uint32_t lastNotReadyMessage = 0;

    bool sent = false;

    if (millis() - lastHID >= 20)
    {
        lastHID = millis();

        // ---------------------------------------------
        // IMPORTANT:
        // DO NOT call SendReport if HID is not ready.
        // ---------------------------------------------

        if (wheel.ready())
        {
            sent =
                wheel.send(
                    steering,
                    pedals,
                    mag,
                    finalHIDButtons,
                    currentPOV);
            encoderDelta = 0;
        }
        else
        {
            // Print only once per second.
            // This avoids Serial0 spam.

            if (
                millis() -
                    lastNotReadyMessage >=
                1000)
            {
                lastNotReadyMessage =
                    millis();

                Serial0.println(
                    "USB HID: not ready");
            }
        }
    }

    // =================================================
    // 7. DEBUG
    // =================================================

    static uint32_t lastDebug = 0;

    if (millis() - lastDebug >= 200)
    {
        lastDebug = millis();

        Serial0.print("Angle: ");
        Serial0.print(angle);

        Serial0.print(" | Accum: ");
        Serial0.print(accumulatedAngle);

        Serial0.print(" | Steering: ");
        Serial0.print(steering);

        Serial0.print(" | Pedal: ");
        Serial0.print(pedals);

        Serial0.print(" | MAG: ");
        Serial0.print(mag);

        Serial0.print(" | Buttons: 0x");
        Serial0.print(
            finalHIDButtons,
            HEX);

        Serial0.print(" | Encoder: ");
        Serial0.println(
            currentEncoderDelta);

        // Serial0.print(" | HID: ");

        // if (!wheel.ready())
        // {
        //     Serial0.println(
        //         "NOT READY");
        // }
        // else if (sent)
        // {
        //     Serial0.println(
        //         "OK");
        // }
        // else
        // {
        //     Serial0.println(
        //         "SEND FAILED");
        // }
    }
}
// =====================================================
// LOOP
// =====================================================
// void loop()
// {
//     static uint16_t lastAngle = 0;
//     static int32_t accumulatedAngle = 0;
//     // =================================================
//     // PEDAL FILTER
//     // =================================================
//     static int oldPedalsRawValue = 0;
//     static int16_t oldPedalsValue = 0;
//     // =================================================
//     // FIRST ANGLE
//     // =================================================
//     uint16_t angle = readAngle();
//     // On first loop
//     static bool firstLoop = true;
//     if (firstLoop)
//     {
//         lastAngle = angle;
//         firstLoop = false;
//     }
//     // =================================================
//     // CALCULATE ANGLE DIFFERENCE
//     // =================================================
//     int32_t delta =
//         (int32_t)angle -
//         (int32_t)lastAngle;
//     // =================================================
//     // HANDLE 0° / 360° WRAP
//     // =================================================
//     if (delta > 2048)
//     {
//         delta -= 4096;
//     }
//     else if (delta < -2048)
//     {
//         delta += 4096;
//     }
//     // =================================================
//     // ACCUMULATE
//     // =================================================
//     accumulatedAngle += delta;
//     lastAngle = angle;
//     // =================================================
//     // LIMIT TO ±360°
//     //
//     // 4096 = 360°
//     // =================================================
//     // accumulatedAngle = constrain(
//     //     accumulatedAngle,
//     //     STEERING_MIN,
//     //     STEERING_MAX
//     // );
//     // =================================================
//     // CONVERT STEERING TO HID
//     //
//     // -4096 ... 0 ... +4096
//     //
//     // becomes
//     //
//     // -32768 ... 0 ... +32767
//     // =================================================
//     int16_t steering = map(
//         accumulatedAngle,
//         STEERING_MIN,
//         STEERING_MAX,
//         -32768,
//         32767);
//     // =================================================
//     // PEDALS
//     // =================================================
//     int pedalRaw = analogRead(
//         PEDAL_PIN);
//     int16_t pedals;
//     if (abs(
//             pedalRaw -
//             oldPedalsRawValue) > 100)
//     {
//         pedals = readPedals(
//             pedalRaw);
//         oldPedalsRawValue = pedalRaw;
//         oldPedalsValue = pedals;
//     }
//     else
//     {
//         pedals = oldPedalsValue;
//     }
//     // =================================================
//     // MAG
//     // =================================================
//     uint16_t mag = readMagnitude();
//     // =====================================================
//     // SEND HID
//     // =====================================================
//     static uint32_t lastHID = 0;
//     bool sent = false;
//     if (millis() - lastHID >= 20)
//     {
//         lastHID = millis();
//         sent = wheel.send(steering, pedals, mag);
//     }
//     // =====================================================
//     // DEBUG
//     // =====================================================
//     static uint32_t lastDebug = 0;
//     if (millis() - lastDebug >= 100)
//     {
//         lastDebug = millis();
//         Serial.print("RAW: ");
//         Serial.print(angle);
//         Serial.print(" | Delta: ");
//         Serial.print(delta);
//         Serial.print(" | Accum: ");
//         Serial.print(accumulatedAngle);
//         Serial.print(" | X Steering: ");
//         Serial.print(steering);
//         Serial.print(" | Pedal RAW: ");
//         Serial.print(pedalRaw);
//         Serial.print(" | Y Pedals: ");
//         Serial.print(pedals);
//         Serial.print(" | MAG: ");
//         Serial.print(mag);
//         Serial.print(" | HID: ");
//         Serial.println(
//             sent ? "OK" : "NOT READY");
//     }
// }