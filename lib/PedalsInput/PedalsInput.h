#pragma once

#include <Arduino.h>

class PedalsInput
{
public:
    explicit PedalsInput(uint8_t pin);

    bool begin();

    void update();

    int getRawValue() const;
    int16_t getValue() const;

    int getRawMinValue() const;
    int getRawCenterValue() const;
    int getRawMaxValue() const;

    int getMaxRawCalibrationValue() const;
    int getMinRawCalibrationValue() const;

    int getThreshold() const;
    int getCalibrationMargin() const;

    void setCurrentValueAsCenter();

    void setRawMin(int raw);
    void setRawCenter(int raw);
    void setRawMax(int raw);

    void setThreshold(int newThreshold);

    void setCalibrationMargin(int margin);

    void startCalibration();
    void finishCalibration();
    bool isCalibrating() const;
    void resetCalibration();

private:
    static constexpr int ADC_MIN = 0;
    static constexpr int ADC_MAX = 4095;

    static constexpr int DEFAULT_CENTER = 1949;
    static constexpr int DEFAULT_CHANGE_THRESHOLD = 45;

    static constexpr int DEFAULT_CALIBRATION_MARGIN = 150;

    uint8_t _pin;

    int _minRawValue = ADC_MIN;
    int _centerRawValue = DEFAULT_CENTER;
    int _maxRawValue = ADC_MAX;
    int _threshold = DEFAULT_CHANGE_THRESHOLD;
    int _calibrationMargin = DEFAULT_CALIBRATION_MARGIN;

    int _rawValue = 0;
    int16_t _value = 0;

    bool _initialized = false;
    bool _calibration = false;

    int _maxRawCalibrationValue = 0;
    int _minRawCalibrationValue = 0;

    int16_t _convertRawToValue(int raw) const;
};