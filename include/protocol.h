// #pragma once
// #include <stdint.h>

// struct WheelPacket
// {
//     uint16_t magic;
//     uint8_t sequence;
//     uint32_t buttons;
//     int16_t encoderDelta;
// };

// struct ButtonDesc
// {
//     const char *name;
//     uint8_t pin;
//     uint8_t bitIndex;
// };

// // Єдиний масив для всієї системи
// // const ButtonDesc buttons[] = {
// //     {"AL", 5, 0},
// //     {"AR", 6, 1},
// //     {"AD", 7, 2},
// //     {"AU", 15, 3},
// //     {"5", 42, 4},
// //     {"7", 41, 5},
// //     {"11", 40, 6},
// //     {"12", 39, 7},
// //     {"9", 38, 8},  // Замість старого ризикового 37
// //     {"10", 48, 9}, // Замість старого ризикового 36
// //     {"8", 47, 10}, // Замість старого ризикового 35
// //     {"6", 21, 11}, // Замість старого ризикового 45
// //     {"1", 10, 12},
// //     {"2", 13, 13},
// //     {"3", 11, 14},
// //     {"4", 12, 15},
// //     {"ENC_BTN", 8, 16}};
// const ButtonDesc buttons[] = {
//     {"AL", 5, 13},
//     {"AR", 6, 14},
//     {"AD", 7, 15},
//     {"AU", 15, 16},
//     {"5", 42, 5},
//     {"7", 41, 7},
//     {"11", 40, 11},
//     {"12", 39, 12},
//     {"9", 38, 9},   // Замість старого ризикового 37
//     {"10", 48, 10}, // Замість старого ризикового 36
//     {"8", 47, 8},   // Замість старого ризикового 35
//     {"6", 21, 6},   // Замість старого ризикового 45
//     {"1", 10, 1},
//     {"2", 13, 2},
//     {"3", 11, 3},
//     {"4", 12, 4},
//     {"ENC_BTN", 8, 17}};

// const int BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

// #define AL_BIT 13
// #define AR_BIT 14
// #define AD_BIT 15
// #define AU_BIT 16
#pragma once

#include <stdint.h>

// =====================================================
// POV
// =====================================================

enum POVDirection : uint8_t
{
    POV_UP = 0,
    POV_UP_RIGHT = 1,
    POV_RIGHT = 2,
    POV_DOWN_RIGHT = 3,
    POV_DOWN = 4,
    POV_DOWN_LEFT = 5,
    POV_LEFT = 6,
    POV_UP_LEFT = 7,

    POV_CENTER = 15
};

// =====================================================
// BUTTON FUNCTION
// =====================================================

enum ButtonFunction : uint8_t
{
    BUTTON_NORMAL,

    BUTTON_POV_UP,
    BUTTON_POV_RIGHT,
    BUTTON_POV_DOWN,
    BUTTON_POV_LEFT
};

// =====================================================
// ESP-NOW PACKET
// =====================================================

struct WheelPacket
{
    uint16_t magic;
    uint8_t sequence;

    uint8_t pov;
    uint16_t buttons;

    int16_t encoderDelta;
};

static_assert(
    sizeof(WheelPacket) == 8,
    "WheelPacket must be 8 bytes");

// =====================================================
// BUTTON DESCRIPTION
// =====================================================

struct ButtonDesc
{
    const char *name;
    uint8_t pin;

    // Використовується тільки для BUTTON_NORMAL
    uint8_t bitIndex;

    ButtonFunction function;
};

// =====================================================
// ALL WHEEL BUTTONS
// =====================================================
//
// Normal buttons:
// bits 0..12
//
// POV:
// AL / AR / AD / AU
//
// =====================================================

const ButtonDesc buttons[] =
    {
        // =========================
        // POV
        // =========================

        {"AL", 5, 0, BUTTON_POV_LEFT},
        {"AR", 6, 0, BUTTON_POV_RIGHT},
        {"AD", 7, 0, BUTTON_POV_DOWN},
        {"AU", 15, 0, BUTTON_POV_UP},

        // =========================
        // NORMAL BUTTONS
        // =========================

        {"1", 10, 0, BUTTON_NORMAL},
        {"2", 13, 1, BUTTON_NORMAL},
        {"3", 11, 2, BUTTON_NORMAL},
        {"4", 12, 3, BUTTON_NORMAL},
        {"5", 42, 4, BUTTON_NORMAL},
        {"6", 21, 5, BUTTON_NORMAL},
        {"7", 41, 6, BUTTON_NORMAL},
        {"8", 47, 7, BUTTON_NORMAL},
        {"9", 38, 8, BUTTON_NORMAL},
        {"10", 48, 9, BUTTON_NORMAL},
        {"11", 40, 10, BUTTON_NORMAL},
        {"12", 39, 11, BUTTON_NORMAL},

        // Encoder push button
        {"ENC_BTN", 8, 12, BUTTON_NORMAL}};

const int BUTTON_COUNT =
    sizeof(buttons) / sizeof(buttons[0]);

// =====================================================
// NORMAL BUTTON COUNT
// =====================================================

const uint8_t NORMAL_BUTTON_COUNT = []()
{
    uint8_t count = 0;
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (buttons[i].function == BUTTON_NORMAL)
            count++;
    }
    return count;
}();