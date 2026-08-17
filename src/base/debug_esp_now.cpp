// #include <Arduino.h>
// #include <WiFi.h>
// #include <esp_now.h>
// #include "protocol.h" // Підтягує загальний масив buttons

// volatile bool packetReceived = false;
// WheelPacket receivedPacket;

// void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
// {
//     if (len != sizeof(WheelPacket))
//         return;

//     memcpy(&receivedPacket, incomingData, sizeof(WheelPacket));
//     packetReceived = true;
// }

// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     WiFi.mode(WIFI_STA);
//     WiFi.disconnect();

//     Serial.print("Receiver MAC: ");
//     Serial.println(WiFi.macAddress());

//     if (esp_now_init() != ESP_OK)
//     {
//         Serial.println("ESP-NOW initialization failed!");
//         return;
//     }

//     esp_now_register_recv_cb(OnDataRecv);

//     Serial.println("Receiver ready!");
// }

// void loop()
// {
//     if (packetReceived)
//     {
//         packetReceived = false;

//         if (receivedPacket.magic != 0xABCD)
//         {
//             Serial.println("Invalid packet!");
//             return;
//         }

//         Serial.print("Seq: ");
//         Serial.print(receivedPacket.sequence);
//         Serial.print(" | Pressed: ");

//         bool anyPressed = false;

//         // Перевіряємо звичайні кнопки через спільний масив з protocol.h
//         for (int i = 0; i < BUTTON_COUNT; i++)
//         {
//             if (receivedPacket.buttons & (1UL << buttons[i].bitIndex))
//             {
//                 Serial.print(buttons[i].name);
//                 Serial.print(" ");
//                 anyPressed = true;
//             }
//         }

//         // // Перевіряємо кнопку енкодера
//         // if (receivedPacket.buttons & (1UL << ENC_BTN_BIT))
//         // {
//         //     Serial.print("ENC_BTN ");
//         //     anyPressed = true;
//         // }

//         if (!anyPressed)
//         {
//             Serial.print("None ");
//         }
//         Serial.print(" | Delta: ");
//         Serial.print((int)receivedPacket.encoderDelta);
//         Serial.println();
//     }
// }