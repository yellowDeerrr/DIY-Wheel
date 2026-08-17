// #include <Arduino.h>
// #include <WiFi.h>
// #include <esp_now.h>

// #include "protocol.h"

// uint8_t baseMac[] = {
//     0x80,
//     0xB5,
//     0x4E,
//     0xC5,
//     0xED,
//     0x84};

// WheelPacket packet;

// void setup()
// {
//     Serial.begin(115200);
//     delay(1000);

//     WiFi.mode(WIFI_STA);

//     Serial.print("Wheel MAC: ");
//     Serial.println(WiFi.macAddress());

//     if (esp_now_init() != ESP_OK)
//     {
//         Serial.println("ESP-NOW initialization failed!");
//         return;
//     }

//     esp_now_peer_info_t peerInfo = {};

//     memcpy(
//         peerInfo.peer_addr,
//         baseMac,
//         6);

//     peerInfo.channel = 0;
//     peerInfo.encrypt = false;

//     if (esp_now_add_peer(&peerInfo) != ESP_OK)
//     {
//         Serial.println("Failed to add peer!");
//         return;
//     }

//     packet.magic = 0xABCD;
//     packet.sequence = 0;

//     Serial.println("Sender ready!");
// }

// void loop()
// {
//     packet.buttons = 1;
//     packet.sequence++;

//     esp_err_t result = esp_now_send(
//         baseMac,
//         (uint8_t *)&packet,
//         sizeof(packet));

//     Serial.print("Sending packet #");
//     Serial.print(packet.sequence);
//     Serial.print(" | result: ");

//     if (result == ESP_OK)
//         Serial.println("OK");
//     else
//         Serial.println("FAILED");

//     delay(19);
// }