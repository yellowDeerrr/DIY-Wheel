#pragma once

#include <stdint.h>

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

enum ButtonFunction : uint8_t
{
    BUTTON_NORMAL,

    BUTTON_POV_UP,
    BUTTON_POV_RIGHT,
    BUTTON_POV_DOWN,
    BUTTON_POV_LEFT
};

// ESP-NOW PACKET
struct WheelPacket
{
    uint16_t magic;
    uint8_t sequence;

    uint8_t pov;
    uint32_t buttons;

    int16_t encoderDelta;
};

static_assert(
    sizeof(WheelPacket) == 12,
    "WheelPacket must be 12 bytes");

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

        // shifters
        {"LEFT_SHF", 1, 12, BUTTON_NORMAL},
        {"RIGHT_SHF", 2, 13, BUTTON_NORMAL},

        // Encoder push button
        {"ENC_BTN", 8, 14, BUTTON_NORMAL}};

const int BUTTON_COUNT =
    sizeof(buttons) / sizeof(buttons[0]);

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