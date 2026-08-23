// #pragma once

// #include <Arduino.h>

// class SteeringSensor
// {
// public:
//     // SteeringSensor(
//     //     int32_t minPosition,
//     //     int32_t maxPosition);

//     SteeringSensor(float degrees);

//     bool begin(
//         uint8_t sdaPin,
//         uint8_t sclPin);

//     void update();
//     void center();

//     uint16_t getRawAngle() const;
//     uint16_t getMagnitude() const;

//     int32_t getPosition() const;
//     int16_t getValue() const;

//     void setSteeringDegrees(float steeringDegrees);

// private:
//     static constexpr uint8_t AS5600_ADDR = 0x36;
//     static constexpr uint8_t REG_RAW_ANGLE = 0x0C;
//     static constexpr uint8_t REG_MAGNITUDE = 0x1B;

//     int32_t _minPosition;
//     int32_t _maxPosition;

//     uint16_t _currentAngle = 0;
//     uint16_t _lastAngle = 0;
//     uint16_t _magnitude = 0;

//     int32_t _position = 0;

//     bool _initialized = false;

//     uint16_t _readRegister(uint8_t reg);
// };

#pragma once

#include <Arduino.h>

struct SteeringSettings
{
    float steeringDegrees = 540.0f;

    // 0.0 ... 50.0 %
    float deadzonePercent = 0.0f;

    // Physical offset of the logical center, in degrees.
    // +10° means logical center is shifted by +10°.
    float centerOffsetDegrees = 0.0f;

    bool invert = false;
};

class SteeringSensor
{
public:
    explicit SteeringSensor(float steeringDegrees = 540.0f);

    bool begin(uint8_t sdaPin, uint8_t sclPin);
    void update();

    // -------------------------------------------------
    // Calibration / center
    // -------------------------------------------------

    void center();

    // -------------------------------------------------
    // Settings
    // -------------------------------------------------

    void setSteeringDegrees(float degrees);
    void setDeadzone(float percent);
    void setCenterOffsetDegrees(float degrees);
    void setInvert(bool invert);

    void setSettings(const SteeringSettings &settings);

    const SteeringSettings &getSettings() const;

    // -------------------------------------------------
    // Values
    // -------------------------------------------------

    uint16_t getRawAngle() const;
    uint16_t getMagnitude() const;

    // Position in AS5600 counts relative to current center.
    int32_t getPosition() const;

    // Final HID-compatible value: -32768 ... 32767
    int16_t getValue() const;

private:
    static constexpr uint8_t AS5600_ADDR = 0x36;

    static constexpr uint8_t REG_RAW_ANGLE = 0x0C;
    static constexpr uint8_t REG_MAGNITUDE = 0x1B;

    static constexpr int32_t AS5600_RESOLUTION = 4096;
    static constexpr int32_t AS5600_HALF = 2048;

    SteeringSettings _settings;

    int32_t _minPosition = -3072;
    int32_t _maxPosition = 3072;
    int32_t _centerOffsetPosition = 0;

    uint16_t _currentAngle = 0;
    uint16_t _lastAngle = 0;
    uint16_t _magnitude = 0;

    int32_t _position = 0;

    bool _initialized = false;

    void _updateLimits();
    void _updateCenterOffset();

    int16_t _convertToHID(int32_t position) const;

    bool _readRegister(
        uint8_t reg,
        uint16_t &value);
};