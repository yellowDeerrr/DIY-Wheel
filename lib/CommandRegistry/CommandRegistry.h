#pragma once

#include <Arduino.h>

enum class CommandId : uint8_t
{
    // -------------------------
    // SYSTEM
    // -------------------------
    PING,
    HELP,
    INFO,
    CENTER,

    // -------------------------
    // STEERING
    // -------------------------
    STEER_GET,
    STEER_DEG,
    STEER_DEADZONE,
    STEER_OFFSET,
    STEER_INVERT,

    // -------------------------
    // PEDALS
    // -------------------------
    PEDAL_GET,
    PEDAL_CAL_START,
    PEDAL_CAL_STOP,
    PEDAL_MARGIN,

    // -------------------------
    // DEBUG
    // -------------------------
    DEBUG_GET,
    DEBUG_ON,
    DEBUG_OFF,
    DEBUG_INTERVAL
};

struct CommandDefinition
{
    CommandId id;

    const char *category;
    const char *name;
    const char *usage;
    const char *description;
};

struct CommandMatch
{
    CommandId id;
    String arguments;
};

class CommandRegistry
{
public:
    bool find(
        const String &input,
        CommandMatch &match) const;

    void printHelp(Print &output) const;

    void printCategoryHelp(
        Print &output,
        const String &category) const;

private:
    static const CommandDefinition _commands[];
    static const size_t _commandCount;

    static String _normalize(
        const String &input);

    static bool _matches(
        const String &input,
        const char *commandName);

    static size_t _commandLength(
        const char *commandName);
};