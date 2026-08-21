#include "Arduino.h"
#include <Wire.h>
#include "USB.h"
#include "USBHID.h"
#include "USBCDC.h"
#include <WiFi.h>
#include <esp_now.h>
#include "protocol.h"

// own libs
#include <SteeringSensor.h>
#include <PedalsInput.h>

constexpr uint8_t STEERING_SDA_PIN = 8;
constexpr uint8_t STEERING_SCL_PIN = 9;

SteeringSensor steering(720);

// PEDAL & STEERING CALIBRATION
constexpr uint8_t PEDAL_PIN = 4;

PedalsInput pedals(PEDAL_PIN);

USBCDC USBSerial;

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

WheelHIDDevice wheel;

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
    USBSerial.begin();
    delay(1000);

    if (!steering.begin(8, 9))
    {
        USBSerial.println("AS5600 not found!");
    }

    analogReadResolution(12);
    // pinMode(PEDAL_PIN, INPUT);

    pedals.begin();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK)
    {
        Serial0.println("ESP-NOW Init Failed!");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    // USB
    wheel.begin();
    USB.begin();

    USBSerial.println("====================================");
    USBSerial.println("   WHEEL RECEIVER READY (HID + CDC) ");
    USBSerial.println("====================================");
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
            steering.center();
            USBSerial.println("OK: Centered");
        }
        else if (pcCommand == "PEDALS_CALIBRATION:START")
        {
            pedals.startCalibration();
            USBSerial.println("OK: Calibration started");
        }
        else if (pcCommand == "PEDALS_CALIBRATION:STOP")
        {
            pedals.finishCalibration();
            USBSerial.println("OK: Calibration finished");
        }
        else if (pcCommand.startsWith("SET_MARGIN_CALIBRATION "))
        {
            int margin = pcCommand
                             .substring(String("SET_MARGIN_CALIBRATION ").length())
                             .toInt();

            pedals.setCalibrationMargin(margin);

            USBSerial.print("OK: Calibration margin = ");
            USBSerial.println(pedals.getCalibrationMargin());
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
        steering.update();

        int16_t steeringValue = steering.getValue();
        uint16_t angle = steering.getRawAngle();
        uint16_t mag = steering.getMagnitude();

        // PEDALS

        pedals.update();
        int16_t pedalsValue = pedals.getValue();

        // SEND DATA
        static uint32_t lastHID = 0;
        if (millis() - lastHID >= 20)
        {
            lastHID = millis();
            if (wheel.ready())
            {
                wheel.send(steeringValue, pedalsValue, mag, finalHIDButtons, currentPOV);
                encoderDelta = 0;
            }
        }

        // Debug
        static uint32_t lastDebug = 0;

        if (USBSerial && millis() - lastDebug >= 200)
        {
            lastDebug = millis();

            if (pedals.isCalibrating())
            {
                USBSerial.print("CALIBRATION IS GOING");

                USBSerial.print(" | Max Value: ");
                USBSerial.print(pedals.getMaxRawCalibrationValue());

                USBSerial.print(" | Min Value: ");
                USBSerial.print(pedals.getMinRawCalibrationValue());

                USBSerial.print(" | Pedal RAW: ");
                USBSerial.print(pedals.getRawValue());

                USBSerial.print(" | Pedal: ");
                USBSerial.print(pedalsValue);

                USBSerial.print(" | Threshold: ");
                USBSerial.print(pedals.getThreshold());

                USBSerial.print(" | Calibration Margin: ");
                USBSerial.print(pedals.getCalibrationMargin());

                USBSerial.println();
            }
            else
            {
                USBSerial.print("Angle: ");
                USBSerial.print(angle);

                USBSerial.print(" | Position: ");
                USBSerial.print(steering.getPosition());

                USBSerial.print(" | Steering: ");
                USBSerial.print(steeringValue);

                USBSerial.print(" | Pedal RAW: ");
                USBSerial.print(pedals.getRawValue());

                USBSerial.print(" | Pedal: ");
                USBSerial.print(pedalsValue);

                USBSerial.print(" | MAG: ");
                USBSerial.print(mag);

                USBSerial.print(" | Buttons: 0x");
                USBSerial.print(finalHIDButtons, HEX);

                USBSerial.print(" | Encoder: ");
                USBSerial.println(currentEncoderDelta);
            }
        }
    }