// #include "WheelConsole.h"

// WheelConsole::WheelConsole(
//     SteeringSensor &steering,
//     PedalsInput &pedals)
//     : _steering(steering),
//       _pedals(pedals)
// {
// }

// void WheelConsole::begin()
// {
//     _serial.begin();

//     delay(500);

//     _serial.println();
//     _serial.println("====================================");
//     _serial.println("         WHEEL CONSOLE READY         ");
//     _serial.println("====================================");
//     _serial.println("Type HELP for commands.");
//     _serial.println();
// }

// void WheelConsole::update()
// {
//     if (_serial.available())
//     {
//         String command =
//             _serial.readStringUntil('\n');

//         command.trim();

//         if (!command.isEmpty())
//         {
//             _processCommand(command);
//         }
//     }

//     if (_debugEnabled &&
//         millis() - _lastDebug >= _debugInterval)
//     {
//         _lastDebug = millis();

//         _printDebug();
//     }
// }

// void WheelConsole::print(
//     const String &text)
// {
//     _serial.print(text);
// }

// void WheelConsole::println(
//     const String &text)
// {
//     _serial.println(text);
// }

// // =====================================================
// // COMMAND PROCESSOR
// // =====================================================

// void WheelConsole::_processCommand(
//     const String &command)
// {
//     String cmd = command;
//     cmd.trim();
//     cmd.toUpperCase();

//     // -----------------------------------------------
//     // BASIC
//     // -----------------------------------------------

//     if (cmd == "PING")
//     {
//         _serial.println("PONG");
//         return;
//     }

//     if (cmd == "HELP")
//     {
//         _printHelp();
//         return;
//     }

//     if (cmd == "CENTER")
//     {
//         _steering.center();

//         _serial.println(
//             "OK: Steering centered");

//         return;
//     }

//     if (cmd == "INFO")
//     {
//         _serial.println(
//             "=== WHEEL INFO ===");

//         _serial.print("Steering angle: ");
//         _serial.print(
//             _steering.getSettings().steeringDegrees);
//         _serial.println(" deg");

//         _serial.print("Steering position: ");
//         _serial.println(
//             _steering.getPosition());

//         _serial.print("Steering raw: ");
//         _serial.println(
//             _steering.getRawAngle());

//         _serial.print("Steering HID: ");
//         _serial.println(
//             _steering.getValue());

//         return;
//     }

//     // -----------------------------------------------
//     // STEERING
//     // -----------------------------------------------

//     if (cmd == "STEER GET")
//     {
//         _printSteeringSettings();
//         return;
//     }

//     if (cmd.startsWith("STEER "))
//     {
//         _handleSteeringCommand(cmd);
//         return;
//     }

//     // -----------------------------------------------
//     // PEDALS
//     // -----------------------------------------------

//     if (cmd == "PEDAL GET")
//     {
//         _printPedalSettings();
//         return;
//     }

//     if (cmd.startsWith("PEDAL "))
//     {
//         _handlePedalCommand(cmd);
//         return;
//     }

//     // -----------------------------------------------
//     // DEBUG
//     // -----------------------------------------------

//     if (cmd == "DEBUG GET")
//     {
//         _serial.print("DEBUG: ");
//         _serial.println(
//             _debugEnabled ? "ON" : "OFF");

//         _serial.print("INTERVAL: ");
//         _serial.println(_debugInterval);

//         return;
//     }

//     if (cmd.startsWith("DEBUG "))
//     {
//         _handleDebugCommand(cmd);
//         return;
//     }

//     _serial.print("ERROR: Unknown command: ");
//     _serial.println(command);
// }

// // =====================================================
// // HELP
// // =====================================================

// void WheelConsole::_printHelp()
// {
//     _serial.println();
//     _serial.println("=== COMMANDS ===");

//     _serial.println();

//     _serial.println("GENERAL:");
//     _serial.println("  PING");
//     _serial.println("  INFO");
//     _serial.println("  CENTER");
//     _serial.println("  HELP");

//     _serial.println();

//     _serial.println("STEERING:");
//     _serial.println("  STEER GET");
//     _serial.println("  STEER DEG <value>");
//     _serial.println("  STEER DEADZONE <percent>");
//     _serial.println("  STEER OFFSET <degrees>");
//     _serial.println("  STEER INVERT <0|1>");

//     _serial.println();

//     _serial.println("PEDALS:");
//     _serial.println("  PEDAL GET");
//     _serial.println("  PEDAL CAL START");
//     _serial.println("  PEDAL CAL STOP");
//     _serial.println("  PEDAL MARGIN <value>");

//     _serial.println();

//     _serial.println("DEBUG:");
//     _serial.println("  DEBUG ON");
//     _serial.println("  DEBUG OFF");
//     _serial.println("  DEBUG INTERVAL <ms>");
//     _serial.println("  DEBUG GET");

//     _serial.println();
// }

// // =====================================================
// // STEERING
// // =====================================================

// void WheelConsole::_handleSteeringCommand(
//     const String &command)
// {
//     if (command.startsWith("STEER DEG "))
//     {
//         float value =
//             command
//                 .substring(
//                     String("STEER DEG ").length())
//                 .toFloat();

//         _steering.setSteeringDegrees(value);

//         _serial.print(
//             "OK: Steering degrees = ");

//         _serial.println(
//             _steering
//                 .getSettings()
//                 .steeringDegrees);

//         return;
//     }

//     if (command.startsWith("STEER DEADZONE "))
//     {
//         float value =
//             command
//                 .substring(
//                     String("STEER DEADZONE ").length())
//                 .toFloat();

//         _steering.setDeadzone(value);

//         _serial.print(
//             "OK: Steering deadzone = ");

//         _serial.println(
//             _steering
//                 .getSettings()
//                 .deadzonePercent);

//         return;
//     }

//     if (command.startsWith("STEER OFFSET "))
//     {
//         float value =
//             command
//                 .substring(
//                     String("STEER OFFSET ").length())
//                 .toFloat();

//         _steering.setCenterOffsetDegrees(value);

//         _serial.print(
//             "OK: Steering offset = ");

//         _serial.println(
//             _steering
//                 .getSettings()
//                 .centerOffsetDegrees);

//         return;
//     }

//     if (command.startsWith("STEER INVERT "))
//     {
//         int value =
//             command
//                 .substring(
//                     String("STEER INVERT ").length())
//                 .toInt();

//         _steering.setInvert(value != 0);

//         _serial.print(
//             "OK: Steering invert = ");

//         _serial.println(
//             _steering
//                     .getSettings()
//                     .invert
//                 ? "ON"
//                 : "OFF");

//         return;
//     }

//     _serial.print(
//         "ERROR: Unknown steering command: ");

//     _serial.println(command);
// }

// // =====================================================
// // STEERING SETTINGS
// // =====================================================

// void WheelConsole::_printSteeringSettings()
// {
//     const SteeringSettings &settings =
//         _steering.getSettings();

//     _serial.println();
//     _serial.println("=== STEERING SETTINGS ===");

//     _serial.print("DEG: ");
//     _serial.println(
//         settings.steeringDegrees);

//     _serial.print("DEADZONE: ");
//     _serial.println(
//         settings.deadzonePercent);

//     _serial.print("OFFSET: ");
//     _serial.println(
//         settings.centerOffsetDegrees);

//     _serial.print("INVERT: ");
//     _serial.println(
//         settings.invert ? "ON" : "OFF");

//     _serial.println();
// }

// // =====================================================
// // PEDALS
// // =====================================================

// void WheelConsole::_handlePedalCommand(
//     const String &command)
// {
//     if (command == "PEDAL CAL START")
//     {
//         _pedals.startCalibration();

//         _serial.println(
//             "OK: Pedal calibration started");

//         return;
//     }

//     if (command == "PEDAL CAL STOP")
//     {
//         _pedals.finishCalibration();

//         _serial.println(
//             "OK: Pedal calibration finished");

//         return;
//     }

//     if (command.startsWith("PEDAL MARGIN "))
//     {
//         int value =
//             command
//                 .substring(
//                     String("PEDAL MARGIN ").length())
//                 .toInt();

//         _pedals.setCalibrationMargin(value);

//         _serial.print(
//             "OK: Pedal calibration margin = ");

//         _serial.println(
//             _pedals.getCalibrationMargin());

//         return;
//     }

//     _serial.print(
//         "ERROR: Unknown pedal command: ");

//     _serial.println(command);
// }

// // =====================================================
// // PEDAL SETTINGS
// // =====================================================

// void WheelConsole::_printPedalSettings()
// {
//     _serial.println();
//     _serial.println("=== PEDAL SETTINGS ===");

//     _serial.print("MIN: ");
//     _serial.println(
//         _pedals.getRawMinValue());

//     _serial.print("CENTER: ");
//     _serial.println(
//         _pedals.getRawCenterValue());

//     _serial.print("MAX: ");
//     _serial.println(
//         _pedals.getRawMaxValue());

//     _serial.print("THRESHOLD: ");
//     _serial.println(
//         _pedals.getThreshold());

//     _serial.print("CALIBRATION MARGIN: ");
//     _serial.println(
//         _pedals.getCalibrationMargin());

//     _serial.println();
// }

// // =====================================================
// // DEBUG
// // =====================================================

// void WheelConsole::_handleDebugCommand(
//     const String &command)
// {
//     if (command == "DEBUG ON")
//     {
//         _debugEnabled = true;

//         _serial.println(
//             "OK: Debug ON");

//         return;
//     }

//     if (command == "DEBUG OFF")
//     {
//         _debugEnabled = false;

//         _serial.println(
//             "OK: Debug OFF");

//         return;
//     }

//     if (command.startsWith("DEBUG INTERVAL "))
//     {
//         uint32_t value =
//             command
//                 .substring(
//                     String("DEBUG INTERVAL ").length())
//                 .toInt();

//         _debugInterval =
//             constrain(value, 10UL, 10000UL);

//         _serial.print(
//             "OK: Debug interval = ");

//         _serial.print(
//             _debugInterval);

//         _serial.println(" ms");

//         return;
//     }

//     _serial.print(
//         "ERROR: Unknown debug command: ");

//     _serial.println(command);
// }

// // =====================================================
// // DEBUG OUTPUT
// // =====================================================

// void WheelConsole::_printDebug()
// {
//     _serial.print("ANGLE: ");
//     _serial.print(
//         _steering.getRawAngle());

//     _serial.print(
//         " | POSITION: ");

//     _serial.print(
//         _steering.getPosition());

//     _serial.print(
//         " | STEERING: ");

//     _serial.print(
//         _steering.getValue());

//     _serial.print(
//         " | PEDAL RAW: ");

//     _serial.print(
//         _pedals.getRawValue());

//     _serial.print(
//         " | PEDAL: ");

//     _serial.print(
//         _pedals.getValue());

//     _serial.print(
//         " | MAG: ");

//     _serial.println(
//         _steering.getMagnitude());
// }
#include "WheelConsole.h"

WheelConsole::WheelConsole(
    USBCDC &serial,
    SteeringSensor &steering,
    PedalsInput &pedals)
    : _serial(serial),
      _steering(steering),
      _pedals(pedals)
{
}

// =====================================================
// BEGIN
// =====================================================

void WheelConsole::begin()
{
    _serial.println();
    _serial.println("====================================");
    _serial.println("         WHEEL CONSOLE READY        ");
    _serial.println("====================================");
    _serial.println("Type HELP for commands.");
    _serial.println();
}

// =====================================================
// UPDATE
// =====================================================

void WheelConsole::update()
{
    if (_serial.available())
    {
        String command =
            _serial.readStringUntil('\n');

        command.trim();

        if (!command.isEmpty())
        {
            _processCommand(command);
        }
    }

    if (_debugEnabled && _serial &&
        millis() - _lastDebug >= _debugInterval)
    {
        _lastDebug = millis();

        _printDebug();
    }
}

// =====================================================
// BASIC OUTPUT
// =====================================================

void WheelConsole::print(
    const String &text)
{
    _serial.print(text);
}

void WheelConsole::println(
    const String &text)
{
    _serial.println(text);
}

// =====================================================
// PROCESS COMMAND
// =====================================================

void WheelConsole::_processCommand(
    const String &input)
{
    CommandMatch match;

    if (!_commands.find(input, match))
    {
        _serial.print("ERROR: Unknown command: ");
        _serial.println(input);

        _serial.println(
            "Type HELP for available commands.");

        return;
    }

    _executeCommand(match);
}

// =====================================================
// EXECUTE COMMAND
// =====================================================

void WheelConsole::_executeCommand(
    const CommandMatch &command)
{
    switch (command.id)
    {
        // =================================================
        // SYSTEM
        // =================================================

    case CommandId::PING:

        _serial.println("PONG");

        break;

    case CommandId::HELP:

        if (command.arguments.isEmpty())
        {
            _commands.printHelp(_serial);
        }
        else
        {
            _commands.printCategoryHelp(
                _serial,
                command.arguments);
        }

        break;

    case CommandId::INFO:

        _printInfo();

        break;

    case CommandId::CENTER:

        _steering.center();

        _serial.println(
            "OK: Steering centered");

        break;

        // =================================================
        // STEERING
        // =================================================

    case CommandId::STEER_GET:
    case CommandId::STEER_DEG:
    case CommandId::STEER_DEADZONE:
    case CommandId::STEER_OFFSET:
    case CommandId::STEER_INVERT:

        _handleSteeringCommand(
            command.id,
            command.arguments);

        break;

        // =================================================
        // PEDALS
        // =================================================

    case CommandId::PEDAL_GET:
    case CommandId::PEDAL_CAL_START:
    case CommandId::PEDAL_CAL_STOP:
    case CommandId::PEDAL_MARGIN:

        _handlePedalCommand(
            command.id,
            command.arguments);

        break;

        // =================================================
        // DEBUG
        // =================================================

    case CommandId::DEBUG_GET:
    case CommandId::DEBUG_ON:
    case CommandId::DEBUG_OFF:
    case CommandId::DEBUG_INTERVAL:

        _handleDebugCommand(
            command.id,
            command.arguments);

        break;
    }
}

// =====================================================
// INFO
// =====================================================

void WheelConsole::_printInfo()
{
    _serial.println();
    _serial.println("=== WHEEL INFO ===");

    _serial.print("Steering angle: ");
    _serial.print(
        _steering
            .getSettings()
            .steeringDegrees);

    _serial.println(" deg");

    _serial.print("Steering position: ");
    _serial.println(
        _steering.getPosition());

    _serial.print("Steering raw: ");
    _serial.println(
        _steering.getRawAngle());

    _serial.print("Steering HID: ");
    _serial.println(
        _steering.getValue());

    _serial.print("Steering magnitude: ");
    _serial.println(
        _steering.getMagnitude());

    _serial.println();
}

// =====================================================
// STEERING COMMANDS
// =====================================================

void WheelConsole::_handleSteeringCommand(
    CommandId id,
    const String &args)
{
    switch (id)
    {
    case CommandId::STEER_GET:

        _printSteeringSettings();

        break;

    case CommandId::STEER_DEG:
    {
        if (args.isEmpty())
        {
            _serial.println(
                "ERROR: Missing degrees.");

            return;
        }

        const float value =
            args.toFloat();

        _steering.setSteeringDegrees(value);

        _serial.print(
            "OK: Steering degrees = ");

        _serial.println(
            _steering
                .getSettings()
                .steeringDegrees);

        break;
    }

    case CommandId::STEER_DEADZONE:
    {
        if (args.isEmpty())
        {
            _serial.println(
                "ERROR: Missing percentage.");

            return;
        }

        const float value =
            args.toFloat();

        _steering.setDeadzone(value);

        _serial.print(
            "OK: Steering deadzone = ");

        _serial.println(
            _steering
                .getSettings()
                .deadzonePercent);

        break;
    }

    case CommandId::STEER_OFFSET:
    {
        if (args.isEmpty())
        {
            _serial.println(
                "ERROR: Missing offset.");

            return;
        }

        const float value =
            args.toFloat();

        _steering.setCenterOffsetDegrees(
            value);

        _serial.print(
            "OK: Steering offset = ");

        _serial.println(
            _steering
                .getSettings()
                .centerOffsetDegrees);

        break;
    }

    case CommandId::STEER_INVERT:
    {
        if (args.isEmpty())
        {
            _serial.println(
                "ERROR: Missing value.");

            return;
        }

        const int value =
            args.toInt();

        _steering.setInvert(
            value != 0);

        _serial.print(
            "OK: Steering invert = ");

        _serial.println(
            _steering
                    .getSettings()
                    .invert
                ? "ON"
                : "OFF");

        break;
    }

    default:
        break;
    }
}

// =====================================================
// STEERING SETTINGS
// =====================================================

void WheelConsole::_printSteeringSettings()
{
    const SteeringSettings &settings =
        _steering.getSettings();

    _serial.println();
    _serial.println(
        "=== STEERING SETTINGS ===");

    _serial.print("DEG: ");
    _serial.println(
        settings.steeringDegrees);

    _serial.print("DEADZONE: ");
    _serial.println(
        settings.deadzonePercent);

    _serial.print("OFFSET: ");
    _serial.println(
        settings.centerOffsetDegrees);

    _serial.print("INVERT: ");
    _serial.println(
        settings.invert ? "ON" : "OFF");

    _serial.println();
}

// =====================================================
// PEDAL COMMANDS
// =====================================================

void WheelConsole::_handlePedalCommand(
    CommandId id,
    const String &args)
{
    switch (id)
    {
    case CommandId::PEDAL_GET:

        _printPedalSettings();

        break;

    case CommandId::PEDAL_CAL_START:

        _pedals.startCalibration();

        _serial.println(
            "OK: Pedal calibration started");

        break;

    case CommandId::PEDAL_CAL_STOP:

        _pedals.finishCalibration();

        _serial.println(
            "OK: Pedal calibration finished");

        break;

    case CommandId::PEDAL_MARGIN:
    {
        if (args.isEmpty())
        {
            _serial.println(
                "ERROR: Missing margin.");

            return;
        }

        const int value =
            args.toInt();

        _pedals.setCalibrationMargin(
            value);

        _serial.print(
            "OK: Pedal calibration margin = ");

        _serial.println(
            _pedals.getCalibrationMargin());

        break;
    }

    default:
        break;
    }
}

// =====================================================
// PEDAL SETTINGS
// =====================================================

void WheelConsole::_printPedalSettings()
{
    _serial.println();
    _serial.println(
        "=== PEDAL SETTINGS ===");

    _serial.print("MIN: ");
    _serial.println(
        _pedals.getRawMinValue());

    _serial.print("CENTER: ");
    _serial.println(
        _pedals.getRawCenterValue());

    _serial.print("MAX: ");
    _serial.println(
        _pedals.getRawMaxValue());

    _serial.print("THRESHOLD: ");
    _serial.println(
        _pedals.getThreshold());

    _serial.print("CALIBRATION MARGIN: ");
    _serial.println(
        _pedals.getCalibrationMargin());

    _serial.println();
}

// =====================================================
// DEBUG COMMANDS
// =====================================================

void WheelConsole::_handleDebugCommand(
    CommandId id,
    const String &args)
{
    switch (id)
    {
    case CommandId::DEBUG_GET:

        _printDebugSettings();

        break;

    case CommandId::DEBUG_ON:

        _debugEnabled = true;

        _serial.println(
            "OK: Debug ON");

        break;

    case CommandId::DEBUG_OFF:

        _debugEnabled = false;

        _serial.println(
            "OK: Debug OFF");

        break;

    case CommandId::DEBUG_INTERVAL:
    {
        if (args.isEmpty())
        {
            _serial.println(
                "ERROR: Missing interval.");

            return;
        }

        const uint32_t value =
            args.toInt();

        _debugInterval =
            constrain(
                value,
                10UL,
                10000UL);

        _serial.print(
            "OK: Debug interval = ");

        _serial.print(
            _debugInterval);

        _serial.println(" ms");

        break;
    }

    default:
        break;
    }
}

// =====================================================
// DEBUG SETTINGS
// =====================================================

void WheelConsole::_printDebugSettings()
{
    _serial.println();
    _serial.println(
        "=== DEBUG SETTINGS ===");

    _serial.print("ENABLED: ");
    _serial.println(
        _debugEnabled ? "ON" : "OFF");

    _serial.print("INTERVAL: ");
    _serial.print(
        _debugInterval);

    _serial.println(" ms");

    _serial.println();
}

// =====================================================
// DEBUG OUTPUT
// =====================================================

void WheelConsole::_printDebug()
{
    _serial.print("ANGLE: ");
    _serial.print(
        _steering.getRawAngle());

    _serial.print(" | POSITION: ");
    _serial.print(
        _steering.getPosition());

    _serial.print(" | STEERING: ");
    _serial.print(
        _steering.getValue());

    _serial.print(" | PEDAL RAW: ");
    _serial.print(
        _pedals.getRawValue());

    _serial.print(" | PEDAL: ");
    _serial.print(
        _pedals.getValue());

    _serial.print(" | MAG: ");
    _serial.println(
        _steering.getMagnitude());
}