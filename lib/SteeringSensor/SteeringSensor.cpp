// #include "SteeringSensor.h"
// #include "Wire.h"

// SteeringSensor::SteeringSensor(float steeringDegrees)
// {
//     setSteeringDegrees(steeringDegrees);
// }

// void SteeringSensor::setSteeringDegrees(float steeringDegrees)
// {
//     _minPosition = -round(
//         4096.0f * steeringDegrees / 360.0f / 2.0f);

//     _maxPosition = round(
//         4096.0f * steeringDegrees / 360.0f / 2.0f);
// }

// bool SteeringSensor::begin(uint8_t sdaPin, uint8_t sclPin)
// {
//     Wire.begin(sdaPin, sclPin);
//     Wire.beginTransmission(AS5600_ADDR);

//     if (Wire.endTransmission() != 0)
//         return false;

//     _currentAngle = _readRegister(REG_RAW_ANGLE);
//     _magnitude = _readRegister(REG_MAGNITUDE);

//     _lastAngle = _currentAngle;
//     _position = 0;

//     _initialized = true;

//     return true;
// }

// void SteeringSensor::update()
// {
//     if (!_initialized)
//         return;

//     _currentAngle = _readRegister(REG_RAW_ANGLE);
//     _magnitude = _readRegister(REG_MAGNITUDE);

//     int32_t delta =
//         static_cast<int32_t>(_currentAngle) -
//         static_cast<int32_t>(_lastAngle);

//     if (delta > 2048)
//         delta -= 4096;
//     else if (delta < -2048)
//         delta += 4096;

//     _position += delta;

//     // _position = constrain(
//     //     _position,
//     //     _minPosition,
//     //     _maxPosition);

//     const int32_t range = _maxPosition - _minPosition + 1;

//     if (_position > _maxPosition)
//     {
//         _position = _minPosition +
//                     ((_position - _minPosition) % range);
//     }
//     else if (_position < _minPosition)
//     {
//         _position = _maxPosition -
//                     ((_minPosition - _position - 1) % range);
//     }

//     _lastAngle = _currentAngle;
// }

// void SteeringSensor::center()
// {
//     _position = 0;
// }

// uint16_t SteeringSensor::getRawAngle() const
// {
//     return _currentAngle;
// }

// uint16_t SteeringSensor::getMagnitude() const
// {
//     return _magnitude;
// }

// int32_t SteeringSensor::getPosition() const
// {
//     return _position;
// }

// int16_t SteeringSensor::getValue() const
// {
//     return map(
//         _position,
//         _minPosition,
//         _maxPosition,
//         -32768,
//         32767);
// }

// uint16_t SteeringSensor::_readRegister(uint8_t reg)
// {
//     Wire.beginTransmission(AS5600_ADDR);
//     Wire.write(reg);

//     if (Wire.endTransmission(false) != 0)
//         return 0;

//     if (Wire.requestFrom(AS5600_ADDR, (uint8_t)2) != 2)
//         return 0;

//     uint8_t high = Wire.read();
//     uint8_t low = Wire.read();

//     return ((uint16_t)high << 8) | low;
// }

#include "SteeringSensor.h"

#include <Wire.h>
#include <math.h>

SteeringSensor::SteeringSensor(float steeringDegrees)
{
    _settings.steeringDegrees = steeringDegrees;

    _updateLimits();
    _updateCenterOffset();
}

// =====================================================
// SETTINGS
// =====================================================

void SteeringSensor::setSteeringDegrees(float degrees)
{
    // Reasonable limits for a steering wheel.
    degrees = constrain(degrees, 1.0f, 2160.0f);

    _settings.steeringDegrees = degrees;

    _updateLimits();

    // Keep current position inside new limits.
    _position = constrain(
        _position,
        _minPosition,
        _maxPosition);
}

void SteeringSensor::setDeadzone(float percent)
{
    _settings.deadzonePercent =
        constrain(percent, 0.0f, 50.0f);
}

void SteeringSensor::setCenterOffsetDegrees(float degrees)
{
    degrees = constrain(
        degrees,
        -_settings.steeringDegrees / 2.0f,
        _settings.steeringDegrees / 2.0f);

    _settings.centerOffsetDegrees = degrees;

    _updateCenterOffset();
}

void SteeringSensor::setInvert(bool invert)
{
    _settings.invert = invert;
}

void SteeringSensor::setSettings(
    const SteeringSettings &settings)
{
    _settings = settings;

    _settings.steeringDegrees =
        constrain(
            _settings.steeringDegrees,
            1.0f,
            2160.0f);

    _settings.deadzonePercent =
        constrain(
            _settings.deadzonePercent,
            0.0f,
            50.0f);

    _settings.centerOffsetDegrees =
        constrain(
            _settings.centerOffsetDegrees,
            -_settings.steeringDegrees / 2.0f,
            _settings.steeringDegrees / 2.0f);

    _updateLimits();
    _updateCenterOffset();

    _position = constrain(
        _position,
        _minPosition,
        _maxPosition);
}

const SteeringSettings &SteeringSensor::getSettings() const
{
    return _settings;
}

// =====================================================
// INTERNAL SETTINGS CALCULATION
// =====================================================

void SteeringSensor::_updateLimits()
{
    const float halfDegrees =
        _settings.steeringDegrees / 2.0f;

    const float halfPosition =
        (static_cast<float>(AS5600_RESOLUTION) *
         halfDegrees) /
        360.0f;

    const int32_t half =
        static_cast<int32_t>(roundf(halfPosition));

    _minPosition = -half;
    _maxPosition = half;
}

void SteeringSensor::_updateCenterOffset()
{
    const float position =
        (static_cast<float>(AS5600_RESOLUTION) *
         _settings.centerOffsetDegrees) /
        360.0f;

    _centerOffsetPosition =
        static_cast<int32_t>(roundf(position));
}

// =====================================================
// INITIALIZATION
// =====================================================

bool SteeringSensor::begin(
    uint8_t sdaPin,
    uint8_t sclPin)
{
    Wire.begin(sdaPin, sclPin);

    Wire.beginTransmission(AS5600_ADDR);

    if (Wire.endTransmission() != 0)
        return false;

    uint16_t rawAngle = 0;
    uint16_t magnitude = 0;

    if (!_readRegister(REG_RAW_ANGLE, rawAngle))
        return false;

    if (!_readRegister(REG_MAGNITUDE, magnitude))
        return false;

    _currentAngle = rawAngle;
    _lastAngle = rawAngle;

    _magnitude = magnitude;

    _position = 0;

    _initialized = true;

    return true;
}

// =====================================================
// UPDATE
// =====================================================

void SteeringSensor::update()
{
    if (!_initialized)
        return;

    uint16_t rawAngle = 0;
    uint16_t magnitude = 0;

    // If I2C read fails, don't corrupt the previous state.
    if (!_readRegister(REG_RAW_ANGLE, rawAngle))
        return;

    if (!_readRegister(REG_MAGNITUDE, magnitude))
        return;

    _currentAngle = rawAngle;
    _magnitude = magnitude;

    int32_t delta =
        static_cast<int32_t>(_currentAngle) -
        static_cast<int32_t>(_lastAngle);

    // Handle AS5600 0 -> 4095 rollover.
    if (delta > AS5600_HALF)
    {
        delta -= AS5600_RESOLUTION;
    }
    else if (delta < -AS5600_HALF)
    {
        delta += AS5600_RESOLUTION;
    }

    _position += delta;

    // _position = constrain(
    //     _position,
    //     _minPosition,
    //     _maxPosition);

    // _lastAngle = _currentAngle;
    const int32_t range = _maxPosition - _minPosition + 1;

    if (_position > _maxPosition)
    {
        _position = _minPosition +
                    ((_position - _minPosition) % range);
    }
    else if (_position < _minPosition)
    {
        _position = _maxPosition -
                    ((_minPosition - _position - 1) % range);
    }

    _lastAngle = _currentAngle;
}

// =====================================================
// CENTER
// =====================================================

void SteeringSensor::center()
{
    _position = 0;
}

// =====================================================
// GETTERS
// =====================================================

uint16_t SteeringSensor::getRawAngle() const
{
    return _currentAngle;
}

uint16_t SteeringSensor::getMagnitude() const
{
    return _magnitude;
}

int32_t SteeringSensor::getPosition() const
{
    return _position;
}

// =====================================================
// HID VALUE
// =====================================================

int16_t SteeringSensor::getValue() const
{
    return _convertToHID(_position);
}

int16_t SteeringSensor::_convertToHID(
    int32_t position) const
{
    // Apply logical center offset.
    int32_t logicalPosition =
        position - _centerOffsetPosition;

    // Use the configured steering half-range.
    const float halfRange =
        static_cast<float>(_maxPosition);

    if (halfRange <= 0.0f)
        return 0;

    // Normalize to -1 ... +1.
    float normalized =
        static_cast<float>(logicalPosition) /
        halfRange;

    normalized =
        constrain(normalized, -1.0f, 1.0f);

    // -------------------------------------------------
    // Deadzone
    // -------------------------------------------------

    const float deadzone =
        _settings.deadzonePercent / 100.0f;

    if (fabsf(normalized) <= deadzone)
    {
        normalized = 0.0f;
    }
    else if (deadzone > 0.0f)
    {
        const float sign =
            normalized < 0.0f ? -1.0f : 1.0f;

        const float magnitude =
            (fabsf(normalized) - deadzone) /
            (1.0f - deadzone);

        normalized =
            sign * constrain(
                       magnitude,
                       0.0f,
                       1.0f);
    }

    // -------------------------------------------------
    // Invert
    // -------------------------------------------------

    if (_settings.invert)
    {
        normalized = -normalized;
    }

    // -------------------------------------------------
    // Convert to HID int16
    // -------------------------------------------------

    if (normalized <= -1.0f)
        return -32768;

    if (normalized >= 1.0f)
        return 32767;

    return static_cast<int16_t>(
        roundf(normalized * 32767.0f));
}

// =====================================================
// AS5600 REGISTER
// =====================================================

bool SteeringSensor::_readRegister(
    uint8_t reg,
    uint16_t &value)
{
    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(reg);

    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(
            AS5600_ADDR,
            static_cast<uint8_t>(2)) != 2)
    {
        return false;
    }

    const uint8_t high = Wire.read();
    const uint8_t low = Wire.read();

    value =
        ((static_cast<uint16_t>(high) << 8) |
         static_cast<uint16_t>(low)) &
        0x0FFF;

    return true;
}