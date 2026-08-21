#include "WheelHID.h"
#include <string.h>

static const uint8_t wheelReportDescriptor[] = {
    0x05, 0x01,
    0x09, 0x04,
    0xA1, 0x01,

    // X
    0x09, 0x30,
    0x16, 0x00, 0x80,
    0x26, 0xFF, 0x7F,
    0x75, 0x10,
    0x95, 0x01,
    0x81, 0x02,

    // Y
    0x09, 0x31,
    0x16, 0x00, 0x80,
    0x26, 0xFF, 0x7F,
    0x75, 0x10,
    0x95, 0x01,
    0x81, 0x02,

    // Z
    0x09, 0x32,
    0x15, 0x00,
    0x26, 0xFF, 0x0F,
    0x75, 0x10,
    0x95, 0x01,
    0x81, 0x02,

    // Buttons
    0x05, 0x09,
    0x19, 0x01,
    0x29, 0x10,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x10,
    0x81, 0x02,

    // POV
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

    // Padding
    0x75, 0x04,
    0x95, 0x01,
    0x81, 0x03,

    0xC0};

WheelHID::WheelHID()
{
    HID.addDevice(
        this,
        sizeof(wheelReportDescriptor));
}

uint16_t WheelHID::_onGetDescriptor(uint8_t *buffer)
{
    memcpy(
        buffer,
        wheelReportDescriptor,
        sizeof(wheelReportDescriptor));

    return sizeof(wheelReportDescriptor);
}

void WheelHID::begin()
{
    HID.begin();
}

bool WheelHID::ready()
{
    return HID.ready();
}

bool WheelHID::send(
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
        uint8_t pov_padding;
    } report;

    static_assert(sizeof(report) == 9);

    report.steering = steering;
    report.pedals = pedals;
    report.mag = mag;
    report.buttons = buttons;
    report.pov_padding = pov & 0x0F;

    return HID.SendReport(
        0,
        &report,
        sizeof(report));
}