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
#include <WheelHID.h>

WheelHID wheel;

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
    // =====================================================
    // CDC COMMANDS
    // =====================================================

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
    }

    // =====================================================
    // ESP-NOW
    // =====================================================

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

    // =====================================================
    // STEERING
    // =====================================================

    steering.update();

    int16_t steeringValue = steering.getValue();

    uint16_t angle = steering.getRawAngle();

    uint16_t mag = steering.getMagnitude();

    // =====================================================
    // PEDALS
    // =====================================================

    pedals.update();

    int16_t pedalsValue = pedals.getValue();

    // =====================================================
    // HID
    // =====================================================

    uint16_t finalHIDButtons = getFinalButtons();

    static uint32_t lastHID = 0;

    if (millis() - lastHID >= 20)
    {
        lastHID = millis();

        if (wheel.ready())
        {
            bool result = wheel.send(
                steeringValue,
                pedalsValue,
                mag,
                finalHIDButtons,
                currentPOV);

            if (result)
            {
                encoderDelta = 0;
            }
        }
    }

    currentEncoderDelta = 0;

    // =====================================================
    // DEBUG
    // =====================================================

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