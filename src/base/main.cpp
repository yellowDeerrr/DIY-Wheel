#include "Arduino.h"
#include <Wire.h>
#include "USB.h"
#include "USBHID.h"
#include "USBCDC.h"
#include <WiFi.h>
#include <esp_now.h>
#include "protocol.h"
#include "soc/rtc_cntl_reg.h"

#define SDA_PIN 8
#define SCL_PIN 9
#define AS5600_ADDR 0x36

// PEDAL & STEERING CALIBRATION
#define PEDAL_PIN 4
#define PEDAL_MIN 0
#define PEDAL_CENTER 1949
#define PEDAL_MAX 4096

#define STEERING_MIN -4096
#define STEERING_MAX 4096

USBCDC USBSerial;
WheelHIDDevice wheel;

uint16_t centerAngle = 0;

// ESP-NOW
volatile bool packetReceived = false;
WheelPacket receivedPacket;

uint16_t currentButtonState = 0;
int16_t encoderDelta = 0;
int16_t currentEncoderDelta = 0;
uint8_t currentPOV = POV_CENTER;

uint8_t lastSequence = 0;
bool firstPacket = true;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    if (len == sizeof(WheelPacket))
    {
        memcpy(&receivedPacket, incomingData, sizeof(WheelPacket));
        packetReceived = true;
    }
}

// 16-BIT HID REPORT DESCRIPTOR
static const uint8_t wheelReportDescriptor[] = {
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x04, // Usage (Joystick)
    0xA1, 0x01, // Collection (Application)

    // X AXIS - STEERING (16 bits)
    0x09, 0x30,       // Usage (X)
    0x16, 0x00, 0x80, // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F, // Logical Maximum (32767)
    0x75, 0x10,       // Report Size = 16 bits
    0x95, 0x01,       // Report Count = 1
    0x81, 0x02,       // Input (Data, Variable, Absolute)

    // Y AXIS - PEDALS (16 bits)
    0x09, 0x31,       // Usage (Y)
    0x16, 0x00, 0x80, // Logical Minimum (-32768)
    0x26, 0xFF, 0x7F, // Logical Maximum (32767)
    0x75, 0x10,       // Report Size = 16 bits
    0x95, 0x01,       // Report Count = 1
    0x81, 0x02,       // Input (Data, Variable, Absolute)

    // Z AXIS - MAG (16 bits)
    0x09, 0x32,       // Usage (Z)
    0x15, 0x00,       // Logical Minimum (0)
    0x26, 0xFF, 0x0F, // Logical Maximum (4095)
    0x75, 0x10,       // Report Size = 16 bits
    0x95, 0x01,       // Report Count = 1
    0x81, 0x02,       // Input (Data, Variable, Absolute)

    // BUTTONS - 16
    0x05, 0x09, // Usage Page (Button)
    0x19, 0x01, // Usage Minimum (Button 1)
    0x29, 0x10, // Usage Maximum (Button 16)
    0x15, 0x00, // Logical Minimum = 0
    0x25, 0x01, // Logical Maximum = 1
    0x75, 0x01, // Report Size = 1 bit
    0x95, 0x10, // Report Count = 16
    0x81, 0x02, // Input (Data, Variable, Absolute)

    // POV HAT
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x39,       // Usage (Hat switch)
    0x15, 0x00,       // Logical Minimum (0)
    0x25, 0x07,       // Logical Maximum (7)
    0x35, 0x00,       // Physical Minimum (0)
    0x46, 0x3B, 0x01, // Physical Maximum (315)
    0x65, 0x14,       // Unit (Eng Rot:Angular Pos)
    0x75, 0x04,       // Report Size = 4 bits
    0x95, 0x01,       // Report Count = 1
    0x81, 0x42,       // Input (Data, Variable, Absolute, Null State)

    // 4 BIT PADDING
    0x75, 0x04, // Report Size = 4 bits
    0x95, 0x01, // Report Count = 1
    0x81, 0x03, // Input (Constant, Variable, Absolute)

    0xC0 // End Collection
};

// CUSTOM HID DEVICE CLASS
class WheelHIDDevice : public USBHIDDevice
{
private:
    USBHID HID;

public:
    WheelHIDDevice()
    {
        static bool initialized = false;
        if (!initialized)
        {
            initialized = true;
            HID.addDevice(this, sizeof(wheelReportDescriptor));
        }
    }

    uint16_t _onGetDescriptor(uint8_t *buffer) override
    {
        memcpy(buffer, wheelReportDescriptor, sizeof(wheelReportDescriptor));
        return sizeof(wheelReportDescriptor);
    }

    bool ready()
    {
        return HID.ready();
    }

    void begin()
    {
        HID.begin();
    }

    bool send(int16_t steering, int16_t pedals, uint16_t mag, uint16_t buttons, uint8_t pov)
    {
        struct __attribute__((packed))
        {
            int16_t steering;
            int16_t pedals;
            uint16_t mag;
            uint16_t buttons;
            uint8_t pov_padding; // POV (4 bits) + Padding (4 bits)
        } report;

        report.steering = steering;
        report.pedals = pedals;
        report.mag = mag;
        report.buttons = buttons;
        report.pov_padding = pov & 0x0F;

        return HID.SendReport(0, &report, sizeof(report));
    }
};

uint16_t readAS5600(uint8_t reg)
{
    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return 0;

    if (Wire.requestFrom((uint8_t)AS5600_ADDR, (uint8_t)2) != 2)
        return 0;

    uint8_t high = Wire.read();
    uint8_t low = Wire.read();
    return ((uint16_t)high << 8) | low;
}

uint16_t readAngle()
{
    return readAS5600(0x0C) & 0x0FFF;
}

uint16_t readMagnitude()
{
    return readAS5600(0x1B) & 0x0FFF;
}

int16_t readPedals(int raw)
{
    raw = constrain(raw, PEDAL_MIN, PEDAL_MAX);
    if (raw < PEDAL_CENTER)
    {
        return map(raw, PEDAL_MIN, PEDAL_CENTER, -32768, 0);
    }
    else
    {
        return map(raw, PEDAL_CENTER, PEDAL_MAX, 0, 32767);
    }
}

uint16_t getFinalButtons()
{
    uint16_t result = currentButtonState;
    if (encoderDelta > 0)
        result |= (1U << NORMAL_BUTTON_COUNT);
    else if (encoderDelta < 0)
        result |= (1U << (NORMAL_BUTTON_COUNT + 1));
    return result;
}

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

    if (esp_now_init() != ESP_OK)
    {
        Serial0.println("ESP-NOW Init Failed!");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    // USB
    USBSerial.begin();
    wheel.begin();
    USB.begin();

    USBSerial.println("====================================");
    USBSerial.println("   WHEEL RECEIVER READY (HID + CDC) ");
    USBSerial.println("====================================");

    Serial0.println("====================================");
    Serial0.println("      WHEEL RECEIVER READY");
    Serial0.println("====================================");
}

void loop()
{
    if (USBSerial.available())
    {
        String pcCommand = USBSerial.readStringUntil('\n');
        pcCommand.trim();

        if (pcCommand == "PING")
        {
            USBSerial.println("PONG: Wheel is online");
        }
        else if (pcCommand == "CENTER")
        {
            centerAngle = readAngle();
            USBSerial.println("OK: Centered");
        }
    }

    if (packetReceived)
    {
        packetReceived = false;
        if (receivedPacket.magic == 0xABCD)
        {
            currentButtonState = receivedPacket.buttons;
            currentEncoderDelta = receivedPacket.encoderDelta;
            encoderDelta += currentEncoderDelta;
            currentPOV = receivedPacket.pov;
        }
    }

    uint16_t finalHIDButtons = getFinalButtons();
    currentEncoderDelta = 0;

    // STEERING
    static uint16_t lastAngle = 0;
    static int32_t accumulatedAngle = 0;
    static bool firstAngle = true;

    uint16_t angle = readAngle();
    if (firstAngle)
    {
        lastAngle = angle;
        firstAngle = false;
    }

    int32_t delta = (int32_t)angle - (int32_t)lastAngle;
    if (delta > 2048)
        delta -= 4096;
    else if (delta < -2048)
        delta += 4096;

    accumulatedAngle += delta;
    lastAngle = angle;
    accumulatedAngle = constrain(accumulatedAngle, STEERING_MIN, STEERING_MAX);
    int16_t steering = map(accumulatedAngle, STEERING_MIN, STEERING_MAX, -32768, 32767);

    // PEDALS
    static int oldPedalsRawValue = 0;
    static int16_t oldPedalsValue = 0;
    int pedalRaw = analogRead(PEDAL_PIN);
    int16_t pedals;

    if (abs(pedalRaw - oldPedalsRawValue) > 20)
    {
        pedals = readPedals(pedalRaw);
        oldPedalsRawValue = pedalRaw;
        oldPedalsValue = pedals;
    }
    else
    {
        pedals = oldPedalsValue;
    }

    // MAG
    uint16_t mag = readMagnitude();

    // SEND DATA
    static uint32_t lastHID = 0;
    if (millis() - lastHID >= 20)
    {
        lastHID = millis();
        if (wheel.ready())
        {
            wheel.send(steering, pedals, mag, finalHIDButtons, currentPOV);
            encoderDelta = 0;
        }
    }

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
    }
}