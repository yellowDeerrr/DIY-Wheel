// #include <Wire.h>
// #include <AS5600.h>

// // Initialize the AS5600 object
// AS5600 as5600;

// // Define standard I2C pins for ESP32-S3
// #define SDA_PIN 8
// #define SCL_PIN 9

// void setup() {
//   // Start the serial monitor
//   Serial.begin(115200);
//   delay(1000); 

//   // Initialize I2C with the specific ESP32-S3 pins
//   Wire.begin(SDA_PIN, SCL_PIN);
  
//   // Initialize the sensor
//   as5600.begin(4);  // 4 is the default DIR pin in the library, though we grounded it physically
//   as5600.setDirection(AS5600_CLOCK_WISE);
  
//   // Check if the sensor is connected on the I2C bus
//   if (!as5600.isConnected()) {
//     Serial.println("Error: AS5600 not found. Check your wiring!");
//     while (1); // Halt execution
//   }
  
//   Serial.println("AS5600 Found!");

//   // Wait until the sensor actually detects the magnet
//   while (!as5600.detectMagnet()) {
//     Serial.println("Waiting for magnet... Place it directly above the chip.");
//     delay(1000);
//   }
  
//   Serial.println("Magnet detected. Starting readings...");
// }

// void loop() {
//   // Read the raw angle (0 to 4095)
//   int rawAngle = as5600.readAngle();
  
//   // Convert the 12-bit value into standard degrees (0 to 359.9)
//   float degrees = rawAngle * (360.0 / 4096.0);
  
//   Serial.print("Raw Value: ");
//   Serial.print(rawAngle);
//   Serial.print(" | Degrees: ");
//   Serial.println(degrees, 1);
  
//   delay(100); // Read 10 times a second
// }
// Define the pin connected to the AS5600 OUT pin
// GPIO 4 is an ADC1 pin on the ESP32-S3, which is safe to use.
// #define ANALOG_PIN 4 
// #include "Arduino.h"

// void setup() {
//   // Start the serial monitor
//   Serial.begin(115200);
//   delay(1000);
  
//   // Set the pin as an input
//   pinMode(ANALOG_PIN, INPUT);
  
//   Serial.println("Starting Analog AS5600 Readings...");
// }

// void loop() {
//   // Read the analog voltage (Returns 0 to 4095)
//   int potValue = analogRead(ANALOG_PIN);
  
//   // Convert to standard degrees (0 to 359.9)
//   float degrees = potValue * (360.0 / 4095.0);
  
//   // Print the results
//   Serial.print("Simulated Pot Value: ");
//   Serial.print(potValue);
//   Serial.print(" | Angle: ");
//   Serial.println(degrees, 1);
  
//   delay(50); // Small delay to make the serial monitor readable
// }

// #include <Wire.h>
// #include <AS5600.h>

// AS5600 as5600;

// #define SDA_PIN 8
// #define SCL_PIN 9

// void setup() {
//   Serial.begin(115200);
//   delay(1000); 

//   Wire.begin(SDA_PIN, SCL_PIN);
//   as5600.begin(4);
  
//   if (!as5600.isConnected()) {
//     Serial.println("AS5600 not found. Check I2C wiring!");
//     while (1); 
//   }

//   Serial.println("AS5600 Connected. Resetting limits in RAM...");

//   // 1. Reset the Start Position (ZPOS) to 0
//   as5600.setZPosition(0);
  
//   // 2. Reset the End Position (MPOS) to 0
//   as5600.setMPosition(0);
  
//   // 3. Reset the Maximum Angle to full range (4095 is max 12-bit value)
//   as5600.setMaxAngle(4095);

//   Serial.println("Done! Turn the magnet and check the values below.");
//   Serial.println("If it sweeps 0-360 properly now, the RAM is fixed.");
//   Serial.println("NOTE: If it reverts to 90 degrees after you unplug it again, ");
//   Serial.println("you will need to permanently burn these settings.");
// }

// void loop() {
//   Serial.print("Raw Angle: ");
//   Serial.println(as5600.readAngle());
//   delay(200);
// }
// #include "Arduino.h"
// #include <Arduino.h>
// #include <Wire.h>
// #include "USB.h"
// #include "USBHID.h"

// #define SDA_PIN 8
// #define SCL_PIN 9
// #define AS5600_ADDR 0x36

// // =====================================================
// // HID REPORT DESCRIPTOR
// // =====================================================

// static const uint8_t wheelReportDescriptor[] = {
//     0x05, 0x01,              // Usage Page (Generic Desktop)
//     0x09, 0x04,              // Usage (Joystick)
//     0xA1, 0x01,              // Collection (Application)

//     0x09, 0x30,              // Usage (X)

//     0x16, 0x00, 0x80,        // Logical Minimum (-32768)
//     0x26, 0xFF, 0x7F,        // Logical Maximum (32767)

//     0x75, 0x10,              // Report Size (16 bits)
//     0x95, 0x01,              // Report Count (1)

//     0x81, 0x02,              // Input (Data, Variable, Absolute)

//     0xC0                     // End Collection
// };


// // =====================================================
// // CUSTOM HID DEVICE
// // =====================================================

// class WheelHIDDevice : public USBHIDDevice {

// public:

//     WheelHIDDevice() {
//         static bool initialized = false;

//         if (!initialized) {
//             initialized = true;

//             HID.addDevice(
//                 this,
//                 sizeof(wheelReportDescriptor)
//             );
//         }
//     }

//     uint16_t _onGetDescriptor(uint8_t *buffer) override {
//         memcpy(
//             buffer,
//             wheelReportDescriptor,
//             sizeof(wheelReportDescriptor)
//         );

//         return sizeof(wheelReportDescriptor);
//     }

//     bool send(int16_t steering) {
//         return HID.SendReport(
//             0,
//             &steering,
//             sizeof(steering)
//         );
//     }

//     void begin() {
//         HID.begin();
//     }

// private:
//     USBHID HID;
// };

// WheelHIDDevice wheel;


// // =====================================================
// // AS5600
// // =====================================================

// uint16_t readAS5600() {

//     Wire.beginTransmission(AS5600_ADDR);
//     Wire.write(0x0C);

//     if (Wire.endTransmission(false) != 0) {
//         return 0;
//     }

//     Wire.requestFrom(AS5600_ADDR, 2);

//     if (Wire.available() < 2) {
//         return 0;
//     }

//     uint8_t high = Wire.read();
//     uint8_t low  = Wire.read();

//     return ((high << 8) | low) & 0x0FFF;
// }


// // =====================================================
// // SETUP
// // =====================================================

// void setup() {

//     Serial.begin(115200);

//     Wire.begin(SDA_PIN, SCL_PIN);

//     wheel.begin();

//     USB.begin();

//     delay(1000);

//     Serial.println("Wheel HID started");
// }


// // =====================================================
// // LOOP
// // =====================================================

// void loop() {

//     uint16_t angle = readAS5600();

//     int16_t steering = map(
//         angle,
//         0,
//         4095,
//         -2048,
//         2047
//     );

//     bool sent = wheel.send(steering);

//     Serial.print("Angle: ");
//     Serial.print(angle);

//     Serial.print(" | Steering: ");
//     Serial.print(steering);

//     Serial.print(" | HID: ");
//     Serial.println(sent ? "OK" : "NOT READY");

//     delay(5);
// }

// #include "Arduino.h"
// #include <Arduino.h>

// #define PEDAL_PIN 4

// void setup() {
//     Serial.begin(115200);

//     // ESP32-S3 ADC
//     analogReadResolution(12);   // 0...4095

//     Serial.println("Pedal test started");
// }

// void loop() {

//     int raw = analogRead(PEDAL_PIN);

//     float voltage = raw * 3.3 / 4095.0;

//     Serial.print("RAW: ");
//     Serial.print(raw);

//     Serial.print("   Voltage: ");
//     Serial.print(voltage, 3);

//     Serial.println(" V");

//     delay(50);
// }

// #include "Arduino.h"
// #include <Arduino.h>

// #define PEDAL_PIN 4
// #define BOOT_PIN 0

// #define CALIBRATION_SAMPLES 100

// uint16_t center = 0;


// void setup() {
//     Serial.begin(115200);

//     pinMode(BOOT_PIN, INPUT_PULLUP);

//     analogReadResolution(12);  // 0...4095

//     delay(1000);

//     // =========================
//     // Calculate center
//     // =========================

//     uint32_t sum = 0;

//     Serial.println("Calibrating center...");
//     Serial.println("Do not touch pedals!");

//     for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
//         uint16_t value = analogRead(PEDAL_PIN);

//         sum += value;

//         delay(10);
//     }

//     center = sum / CALIBRATION_SAMPLES;

//     Serial.print("Center = ");
//     Serial.println(center);

//     Serial.println("Now move pedals.");
//     Serial.println("Press BOOT to print MIN / MAX / CENTER.");
// }
// uint16_t minValue = center;
// uint16_t maxValue = center;
// void loop() {

//     // Read pedal
//     uint16_t value = analogRead(PEDAL_PIN);

//     // Find minimum
//     if (value < minValue) {
//         minValue = value;
//     }

//     // Find maximum
//     if (value > maxValue) {
//         maxValue = value;
//     }

//     // BOOT button pressed
//     if (digitalRead(BOOT_PIN) == LOW) {

//         Serial.println();
//         Serial.println("========== PEDAL CALIBRATION ==========");

//         Serial.print("MIN    = ");
//         Serial.println(minValue);

//         Serial.print("CENTER = ");
//         Serial.println(center);

//         Serial.print("MAX    = ");
//         Serial.println(maxValue);

//         Serial.println("=======================================");
//         Serial.println();

//         // Wait until BOOT is released
//         while (digitalRead(BOOT_PIN) == LOW) {
//             delay(10);
//         }

//         delay(200);
//     }

//     delay(10);
// }
// #include <Arduino.h>
// #include <Wire.h>

// #define SDA_PIN 8
// #define SCL_PIN 9

// #define AS5600_ADDR 0x36

// // =====================================================
// // READ 8-BIT REGISTER
// // =====================================================

// uint8_t read8(uint8_t reg)
// {
//     Wire.beginTransmission(AS5600_ADDR);
//     Wire.write(reg);

//     if (Wire.endTransmission(false) != 0)
//         return 0xFF;

//     if (Wire.requestFrom(AS5600_ADDR, (uint8_t)1) != 1)
//         return 0xFF;

//     return Wire.read();
// }


// // =====================================================
// // READ 16-BIT REGISTER
// // =====================================================

// uint16_t read16(uint8_t reg)
// {
//     Wire.beginTransmission(AS5600_ADDR);
//     Wire.write(reg);

//     if (Wire.endTransmission(false) != 0)
//         return 0xFFFF;

//     if (Wire.requestFrom(AS5600_ADDR, (uint8_t)2) != 2)
//         return 0xFFFF;

//     uint8_t high = Wire.read();
//     uint8_t low  = Wire.read();

//     return ((uint16_t)high << 8) | low;
// }


// // =====================================================
// // SETUP
// // =====================================================

// void setup()
// {
//     Serial.begin(115200);

//     // ESP32-S3 I2C
//     Wire.begin(SDA_PIN, SCL_PIN);

//     // Optional, but good for AS5600
//     Wire.setClock(400000);

//     delay(500);

//     Serial.println();
//     Serial.println("=== AS5600 ESP32-S3 DIAGNOSTIC ===");
//     Serial.println("SDA = GPIO 8");
//     Serial.println("SCL = GPIO 9");
//     Serial.println();
// }


// // =====================================================
// // LOOP
// // =====================================================

// void loop()
// {
//     // RAW ANGLE
//     uint16_t raw = read16(0x0C) & 0x0FFF;

//     // STATUS
//     uint8_t status = read8(0x0B);

//     // AGC
//     uint8_t agc = read8(0x1A);

//     // MAGNITUDE
//     uint16_t magnitude = read16(0x1B) & 0x0FFF;


//     // Convert RAW angle to degrees
//     float degrees = raw * 360.0 / 4096.0;


//     // Status bits
//     bool MD = status & 0x20;   // Magnet detected
//     bool ML = status & 0x10;   // Magnet too weak
//     bool MH = status & 0x08;   // Magnet too strong


//     // =================================================
//     // SERIAL OUTPUT
//     // =================================================

//     Serial.print("RAW=");
//     Serial.print(raw);

//     Serial.print("  DEG=");
//     Serial.print(degrees, 1);

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