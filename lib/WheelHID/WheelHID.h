#pragma once

#include <Arduino.h>
#include "USB.h"
#include "USBHID.h"

class WheelHID : public USBHIDDevice
{
public:
    WheelHID();

    void begin();
    bool ready();

    bool send(
        int16_t steering,
        int16_t pedals,
        uint16_t mag,
        uint32_t buttons,
        uint8_t pov);

protected:
    uint16_t _onGetDescriptor(uint8_t *buffer) override;

private:
    USBHID HID;
};