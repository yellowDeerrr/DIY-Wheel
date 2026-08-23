#include "WheelHID.h"
#include <string.h>

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

    // Buttons - 32
    0x05, 0x09, // Usage Page (Button)
    0x19, 0x01, // Usage Minimum (Button 1)
    0x29, 0x20, // Usage Maximum (Button 32)
    0x15, 0x00, // Logical Minimum (0)
    0x25, 0x01, // Logical Maximum (1)
    0x75, 0x01, // Report Size = 1 bit
    0x95, 0x20, // Report Count = 32 buttons
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
    uint32_t buttons,
    uint8_t pov)
{
    struct __attribute__((packed))
    {
        int16_t steering;
        int16_t pedals;
        uint16_t mag;
        uint32_t buttons;
        uint8_t pov_padding;
    } report;

    static_assert(sizeof(report) == 11);

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