#pragma once

#ifdef IOT_ENABLE_BLYNK

#include "Command.h"

#include <Logger.h>

#include <cstdint>
#include <functional>

class IBlynkHandler;

class Blynk
{
public:
    explicit Blynk(IBlynkHandler& blynkHandler);

    using CommandHandler = std::function<void (uint8_t deviceIndex, radio::Command command)>;

    void setCommandHandler(CommandHandler&& handler);

private:
    Logger _log{ "Blynk" };
    IBlynkHandler& _blynkHandler;
    CommandHandler _commandHandler;

    void setupHandlers();
};

#endif