#include "Blynk.h"

#include <IBlynkHandler.h>

#include <array>

namespace VirtualPins
{
    static constexpr auto Shutter1 = 73;
    static constexpr auto Shutter2 = 72;
    static constexpr auto Shutter3 = 71;
    static constexpr auto Shutter4 = 70;
    static constexpr auto Shutter5 = 69;
    static constexpr auto Shutter6 = 68;
    static constexpr auto Shutter7 = 67;
}

Blynk::Blynk(IBlynkHandler& blynkHandler)
    : _blynkHandler(blynkHandler)
{
    setupHandlers();
}

void Blynk::setCommandHandler(CommandHandler&& handler)
{
    _commandHandler = std::move(handler);
}

void Blynk::Blynk::setupHandlers()
{
    static const std::array<int, 7> DeviceIndexLut = {
        0,      // Living room left
        1, 1,   // Living room right & right-middle
        2,      // Living room left-middle
        4, 4,   // Kitchen left & middle
        3       // Kitchen right
    };

    using rc = radio::Command;

    static const std::array<radio::Command, 7> UpCommandLut = {
        rc::Shutter1Up,     // 0
        rc::Shutter2Up,     // 1
        rc::Shutter1Up,     // 1
        rc::Shutter1Up,     // 2
        rc::Shutter1Up,     // 4
        rc::Shutter2Up,     // 4
        rc::Shutter1Up      // 3
    };

    static const std::array<radio::Command, 7> DownCommandLut = {
        rc::Shutter1Down,   // 0
        rc::Shutter2Down,   // 1
        rc::Shutter1Down,   // 1
        rc::Shutter1Down,   // 2
        rc::Shutter1Down,   // 4
        rc::Shutter2Down,   // 4
        rc::Shutter1Down    // 3
    };

    //
    // Write handlers
    //

    _blynkHandler.setPinWrittenHandler([this](const int pin, const Variant& value) {
        if (pin > VirtualPins::Shutter1 || pin < VirtualPins::Shutter7) {
            return;
        }

        // WARNING this only works with sequential pin numbers
        const auto lutIndex = DeviceIndexLut.size() - 1 - (pin - VirtualPins::Shutter7);
        const auto deviceIndex = DeviceIndexLut[lutIndex];
        const auto direction = static_cast<int>(value);

        _log.debug(
            "pinWrittenHandler: pin=%d, lutIndex=%d, deviceIndex=%d, direction=%d",
            pin, lutIndex, deviceIndex, direction
        );

        if (_commandHandler) {
            if (direction > 0) {
                _commandHandler(deviceIndex, UpCommandLut[lutIndex]);
            } else if (direction < 0) {
                _commandHandler(deviceIndex, DownCommandLut[lutIndex]);
            }
        }
    });
}
