#pragma once

#include <Arduino.h>

class SteeringSensor
{
public:
    // SteeringSensor(
    //     int32_t minPosition,
    //     int32_t maxPosition);

    SteeringSensor(float degrees);

    bool begin(
        uint8_t sdaPin,
        uint8_t sclPin);

    void update();
    void center();

    uint16_t getRawAngle() const;
    uint16_t getMagnitude() const;

    int32_t getPosition() const;
    int16_t getValue() const;

private:
    static constexpr uint8_t AS5600_ADDR = 0x36;
    static constexpr uint8_t REG_RAW_ANGLE = 0x0C;
    static constexpr uint8_t REG_MAGNITUDE = 0x1B;

    int32_t _minPosition;
    int32_t _maxPosition;

    uint16_t _currentAngle = 0;
    uint16_t _lastAngle = 0;
    uint16_t _magnitude = 0;

    int32_t _position = 0;

    bool _initialized = false;

    uint16_t _readRegister(uint8_t reg);
};