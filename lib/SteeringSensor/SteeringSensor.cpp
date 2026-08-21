#include "SteeringSensor.h"
#include "Wire.h"

SteeringSensor::SteeringSensor(float steeringDegrees)
{
    _minPosition = -round(
        4096.0f * steeringDegrees / 360.0f / 2.0f);

    _maxPosition = round(
        4096.0f * steeringDegrees / 360.0f / 2.0f);
}

bool SteeringSensor::begin(uint8_t sdaPin, uint8_t sclPin)
{
    Wire.begin(sdaPin, sclPin);
    Wire.beginTransmission(AS5600_ADDR);

    if (Wire.endTransmission() != 0)
        return false;

    _currentAngle = _readRegister(REG_RAW_ANGLE);
    _magnitude = _readRegister(REG_MAGNITUDE);

    _lastAngle = _currentAngle;
    _position = 0;

    _initialized = true;

    return true;
}

void SteeringSensor::update()
{
    if (!_initialized)
        return;

    _currentAngle = _readRegister(REG_RAW_ANGLE);
    _magnitude = _readRegister(REG_MAGNITUDE);

    int32_t delta =
        static_cast<int32_t>(_currentAngle) -
        static_cast<int32_t>(_lastAngle);

    if (delta > 2048)
        delta -= 4096;
    else if (delta < -2048)
        delta += 4096;

    _position += delta;

    _position = constrain(
        _position,
        _minPosition,
        _maxPosition);

    _lastAngle = _currentAngle;
}

void SteeringSensor::center()
{
    _position = 0;
}

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

int16_t SteeringSensor::getValue() const
{
    return map(
        _position,
        _minPosition,
        _maxPosition,
        -32768,
        32767);
}

uint16_t SteeringSensor::_readRegister(uint8_t reg)
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