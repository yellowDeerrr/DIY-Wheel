#include "CommandRegistry.h"

// =====================================================
// COMMAND TABLE
// =====================================================
//
// THIS IS THE SINGLE SOURCE OF TRUTH FOR THE CLI.
//
// To add a command:
// 1. Add CommandId to CommandId enum.
// 2. Add one entry here.
// 3. Add handling in WheelConsole.
//
// HELP is generated automatically from this table.
// =====================================================

const CommandDefinition CommandRegistry::_commands[] =
    {
        // =================================================
        // SYSTEM
        // =================================================

        {
            CommandId::PING,
            "SYSTEM",
            "PING",
            "PING",
            "Check whether the wheel is responding."},

        {CommandId::HELP,
         "SYSTEM",
         "HELP",
         "HELP [CATEGORY]",
         "Show available commands."},

        {CommandId::INFO,
         "SYSTEM",
         "INFO",
         "INFO",
         "Show basic wheel information."},

        {CommandId::CENTER,
         "SYSTEM",
         "CENTER",
         "CENTER",
         "Set current steering position as center."},

        // =================================================
        // STEERING
        // =================================================

        {
            CommandId::STEER_GET,
            "STEERING",
            "STEER GET",
            "STEER GET",
            "Show steering settings."},

        {CommandId::STEER_DEG,
         "STEERING",
         "STEER DEG",
         "STEER DEG <degrees>",
         "Set total steering angle."},

        {CommandId::STEER_DEADZONE,
         "STEERING",
         "STEER DEADZONE",
         "STEER DEADZONE <percent>",
         "Set steering deadzone."},

        {CommandId::STEER_OFFSET,
         "STEERING",
         "STEER OFFSET",
         "STEER OFFSET <degrees>",
         "Set steering center offset."},

        {CommandId::STEER_INVERT,
         "STEERING",
         "STEER INVERT",
         "STEER INVERT <0|1>",
         "Invert steering direction."},

        // =================================================
        // PEDALS
        // =================================================

        {
            CommandId::PEDAL_GET,
            "PEDALS",
            "PEDAL GET",
            "PEDAL GET",
            "Show pedal settings."},

        {CommandId::PEDAL_CAL_START,
         "PEDALS",
         "PEDAL CAL START",
         "PEDAL CAL START",
         "Start pedal calibration."},

        {CommandId::PEDAL_CAL_STOP,
         "PEDALS",
         "PEDAL CAL STOP",
         "PEDAL CAL STOP",
         "Finish pedal calibration."},

        {CommandId::PEDAL_MARGIN,
         "PEDALS",
         "PEDAL MARGIN",
         "PEDAL MARGIN <value>",
         "Set pedal calibration margin."},

        // =================================================
        // DEBUG
        // =================================================

        {
            CommandId::DEBUG_GET,
            "DEBUG",
            "DEBUG GET",
            "DEBUG GET",
            "Show debug settings."},

        {CommandId::DEBUG_ON,
         "DEBUG",
         "DEBUG ON",
         "DEBUG ON",
         "Enable periodic debug output."},

        {CommandId::DEBUG_OFF,
         "DEBUG",
         "DEBUG OFF",
         "DEBUG OFF",
         "Disable periodic debug output."},

        {CommandId::DEBUG_INTERVAL,
         "DEBUG",
         "DEBUG INTERVAL",
         "DEBUG INTERVAL <ms>",
         "Set debug output interval."}};

const size_t CommandRegistry::_commandCount =
    sizeof(_commands) / sizeof(_commands[0]);

// =====================================================
// FIND COMMAND
// =====================================================
bool CommandRegistry::find(
    const String &input,
    CommandMatch &match) const
{
    const String normalized =
        _normalize(input);

    if (normalized.isEmpty())
        return false;

    // Try every command.
    //
    // Longest command wins.
    // This allows:
    //
    // DEBUG
    // DEBUG GET
    // DEBUG INTERVAL
    //
    // to coexist safely later.
    //
    const CommandDefinition *bestMatch = nullptr;
    size_t bestLength = 0;

    for (size_t i = 0; i < _commandCount; ++i)
    {
        const CommandDefinition &command =
            _commands[i];

        if (!_matches(
                normalized,
                command.name))
        {
            continue;
        }

        const size_t length =
            _commandLength(command.name);

        if (length > bestLength)
        {
            bestMatch = &command;
            bestLength = length;
        }
    }

    if (bestMatch == nullptr)
        return false;

    match.id = bestMatch->id;

    if (normalized.length() > bestLength)
    {
        String args =
            normalized.substring(bestLength);

        args.trim();

        match.arguments = args;
    }
    else
    {
        match.arguments = "";
    }

    return true;
}

// =====================================================
// HELP
// =====================================================

void CommandRegistry::printHelp(
    Print &output) const
{
    output.println();
    output.println("==============================");
    output.println("      AVAILABLE COMMANDS     ");
    output.println("==============================");

    const char *currentCategory = nullptr;

    for (size_t i = 0; i < _commandCount; ++i)
    {
        const CommandDefinition &command =
            _commands[i];

        if (currentCategory == nullptr ||
            strcmp(currentCategory, command.category) != 0)
        {
            output.println();
            output.print("[");
            output.print(command.category);
            output.println("]");

            currentCategory = command.category;
        }

        output.print("  ");
        output.print(command.usage);

        output.print("  -  ");

        output.println(command.description);
    }

    output.println();
}

// =====================================================
// CATEGORY HELP
// =====================================================

void CommandRegistry::printCategoryHelp(
    Print &output,
    const String &category) const
{
    String wanted = category;
    wanted.trim();
    wanted.toUpperCase();

    bool found = false;

    output.println();

    for (size_t i = 0; i < _commandCount; ++i)
    {
        const CommandDefinition &command =
            _commands[i];

        if (wanted != command.category)
            continue;

        if (!found)
        {
            output.print("=== ");
            output.print(command.category);
            output.println(" COMMANDS ===");

            found = true;
        }

        output.print("  ");
        output.print(command.usage);

        output.print("  -  ");

        output.println(command.description);
    }

    if (!found)
    {
        output.print("ERROR: Unknown category: ");
        output.println(category);
    }

    output.println();
}

// =====================================================
// HELPERS
// =====================================================

String CommandRegistry::_normalize(
    const String &input)
{
    String result = input;

    result.trim();
    result.toUpperCase();

    return result;
}

bool CommandRegistry::_matches(
    const String &input,
    const char *commandName)
{
    const String name(commandName);

    if (input == name)
        return true;

    if (!input.startsWith(name))
        return false;

    // Must be followed by a space.
    //
    // Without this:
    //
    // STEER DEGXYZ
    //
    // could incorrectly match:
    //
    // STEER DEG
    //

    if (input.length() <= name.length())
        return false;

    return input.charAt(name.length()) == ' ';
}

size_t CommandRegistry::_commandLength(
    const char *commandName)
{
    return strlen(commandName);
}