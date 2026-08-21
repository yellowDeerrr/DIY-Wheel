#include "PedalsInput.h"

PedalsInput::PedalsInput(uint8_t pin) : _pin(pin)
{
}

bool PedalsInput::begin()
{
    pinMode(_pin, INPUT);

    _rawValue = analogRead(_pin);
    _value = _convertRawToValue(_rawValue);

    _initialized = true;

    return true;
}

void PedalsInput::update()
{
    if (!_initialized)
        return;

    const int raw = analogRead(_pin);

    if (_calibration)
    {
        if (raw < _minRawCalibrationValue)
            _minRawCalibrationValue = raw;

        if (raw > _maxRawCalibrationValue)
            _maxRawCalibrationValue = raw;
    }

    if (abs(raw - _rawValue) > _threshold)
    {
        _rawValue = raw;
        _value = _convertRawToValue(raw);
    }
}

int PedalsInput::getRawValue() const
{
    return _rawValue;
}

int16_t PedalsInput::getValue() const
{
    return _value;
}

int PedalsInput::getRawMinValue() const
{
    return _minRawValue;
}
int PedalsInput::getRawCenterValue() const
{
    return _centerRawValue;
}
int PedalsInput::getRawMaxValue() const
{
    return _maxRawValue;
}

int PedalsInput::getMaxRawCalibrationValue() const { return _maxRawCalibrationValue; }
int PedalsInput::getMinRawCalibrationValue() const { return _minRawCalibrationValue; }

int PedalsInput::getThreshold() const { return _threshold; }
int PedalsInput::getCalibrationMargin() const { return _calibrationMargin; }

void PedalsInput::setCurrentValueAsCenter()
{
    _centerRawValue = _rawValue;
    _value = _convertRawToValue(_rawValue);
}

void PedalsInput::setRawCenter(int raw)
{
    _centerRawValue = constrain(
        raw,
        _minRawValue,
        _maxRawValue);

    _value = _convertRawToValue(_rawValue);
}

void PedalsInput::startCalibration()
{
    _calibration = true;

    _maxRawCalibrationValue = ADC_MIN;
    _minRawCalibrationValue = ADC_MAX;
}

void PedalsInput::finishCalibration()
{
    if (!_calibration)
        return;

    if (_minRawCalibrationValue >= _maxRawCalibrationValue)
    {
        _calibration = false;
        return;
    }

    _maxRawValue = constrain(_maxRawCalibrationValue - _calibrationMargin, ADC_MIN, ADC_MAX);
    _minRawValue = constrain(_minRawCalibrationValue + _calibrationMargin, ADC_MIN, ADC_MAX);

    _calibration = false;
    _value = _convertRawToValue(_rawValue);
}

bool PedalsInput::isCalibrating() const
{
    return _calibration;
}

void PedalsInput::resetCalibration()
{
    _minRawValue = ADC_MIN;
    _centerRawValue = DEFAULT_CENTER;
    _maxRawValue = ADC_MAX;

    _value = _convertRawToValue(_rawValue);
}

void PedalsInput::setRawMin(int raw)
{
    _minRawValue = constrain(raw, ADC_MIN, ADC_MAX);
    _value = _convertRawToValue(_rawValue);
}
void PedalsInput::setRawMax(int raw)
{
    _maxRawValue = constrain(raw, ADC_MIN, ADC_MAX);
    _value = _convertRawToValue(_rawValue);
}
void PedalsInput::setThreshold(int newThreshold)
{
    _threshold = constrain(newThreshold, 0, 4095);
}

void PedalsInput::setCalibrationMargin(int margin)
{
    _calibrationMargin = constrain(margin, 0, 2048);
}

int16_t PedalsInput::_convertRawToValue(int raw) const
{
    raw = constrain(
        raw,
        _minRawValue,
        _maxRawValue);

    if (raw < _centerRawValue)
    {
        return map(
            raw,
            _minRawValue,
            _centerRawValue,
            -32768,
            0);
    }

    return map(
        raw,
        _centerRawValue,
        _maxRawValue,
        0,
        32767);
}