// #pragma once

// #include <Arduino.h>
// #include <USBCDC.h>

// #include <SteeringSensor.h>
// #include <PedalsInput.h>

// #include <CommandRegistry.h>

// class WheelConsole
// {
// public:
//     WheelConsole(
//         SteeringSensor &steering,
//         PedalsInput &pedals);

//     void begin();
//     void update();

//     void print(const String &text);
//     void println(const String &text);

// private:
//     SteeringSensor &_steering;
//     PedalsInput &_pedals;

//     USBCDC _serial;

//     CommandRegistry _commands;

//     bool _debugEnabled = true;
//     uint32_t _debugInterval = 200;
//     uint32_t _lastDebug = 0;

//     void _processCommand(const String &command);

//     void _printHelp();

//     void _printSteeringSettings();
//     void _printPedalSettings();

//     void _printDebug();

//     void _handleSteeringCommand(
//         const String &command);

//     void _handlePedalCommand(
//         const String &command);

//     void _handleDebugCommand(
//         const String &command);
// };
#pragma once

#include <Arduino.h>
#include <USBCDC.h>

#include <SteeringSensor.h>
#include <PedalsInput.h>

#include <CommandRegistry.h>

class WheelConsole
{
public:
    WheelConsole(
        USBCDC &serial,
        SteeringSensor &steering,
        PedalsInput &pedals);

    void begin();
    void update();

    void print(const String &text);
    void println(const String &text);

private:
    USBCDC &_serial;

    SteeringSensor &_steering;
    PedalsInput &_pedals;

    CommandRegistry _commands;

    bool _debugEnabled = true;

    uint32_t _debugInterval = 200;
    uint32_t _lastDebug = 0;

    void _processCommand(
        const String &input);

    void _executeCommand(
        const CommandMatch &command);

    void _printInfo();
    void _printSteeringSettings();
    void _printPedalSettings();
    void _printDebugSettings();

    void _printDebug();

    void _handleSteeringCommand(
        CommandId id,
        const String &args);

    void _handlePedalCommand(
        CommandId id,
        const String &args);

    void _handleDebugCommand(
        CommandId id,
        const String &args);
};