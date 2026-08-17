// #include <Wire.h>
// #include "Arduino.h"
// #define AS5600_ADDR 0x36

// uint8_t read8(uint8_t reg)
// {
//     Wire.beginTransmission(AS5600_ADDR);
//     Wire.write(reg);
//     Wire.endTransmission(false);

//     if (Wire.requestFrom(AS5600_ADDR, (uint8_t)1) != 1)
//         return 0xFF;

//     return Wire.read();
// }

// uint16_t read16(uint8_t reg)
// {
//     Wire.beginTransmission(AS5600_ADDR);
//     Wire.write(reg);
//     Wire.endTransmission(false);

//     if (Wire.requestFrom(AS5600_ADDR, (uint8_t)2) != 2)
//         return 0xFFFF;

//     return ((uint16_t)Wire.read() << 8) | Wire.read();
// }

// void setup()
// {
//     Serial.begin(115200);
//     Wire.begin(8, 9);

//     Serial.println("=== AS5600 DIAGNOSTIC ===");
// }

// void loop()
// {
//     uint16_t raw = read16(0x0C) & 0x0FFF;
//     uint8_t status = read8(0x0B);
//     uint8_t agc = read8(0x1A);
//     uint16_t magnitude = read16(0x1B) & 0x0FFF;

//     // int adc = analogRead(A0);

//     float degrees = raw * 360.0 / 4096.0;
//     // float voltage = adc * 5.0 / 1023.0;

//     bool MD = status & 0x20;
//     bool ML = status & 0x10;
//     bool MH = status & 0x08;

//     Serial.print("RAW=");
//     Serial.print(raw);

//     Serial.print("  DEG=");
//     Serial.print(degrees, 1);

//     // Serial.print("  ADC=");
//     // Serial.print(adc);

//     // Serial.print("  V=");
//     // Serial.print(voltage, 3);

//     Serial.print("  AGC=");
//     Serial.print(agc);

//     Serial.print("  MAG=");
//     Serial.print(magnitude);

//     Serial.print("  MD=");
//     Serial.print(MD);

//     Serial.print("  ML=");
//     Serial.print(ML);

//     Serial.print("  MH=");
//     Serial.println(MH);

//     delay(100);
// }
