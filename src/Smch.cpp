#include "PrivateConfig.h"
#include "Smch.h"
#include "Utils.h"

Smch::Smch(const ApplicationConfig& appConfig)
    : _coreApplication(appConfig)
    , _appConfig(appConfig)
    , _radio(0)
    // , _deviceHub(_radio)
    , _blynk(_coreApplication.blynkHandler())
{
    Logger::setup(_appConfig, _coreApplication.systemClock());

    _blynk.setCommandHandler([this](const uint8_t deviceIndex, const radio::Command command) {
        sendRemoteControlCommand(deviceIndex, command);
    });
}

void Smch::task()
{
    _coreApplication.task();
    _radio.task();
    // _deviceHub.task();
}

void Smch::sendRemoteControlCommand(const uint8_t deviceIndex, const radio::Command command)
{
    const auto request = radio::createCommandRequest(
        command,
        radio::deviceIndexToAddress(deviceIndex)
    );

    auto message = radio::Message{ request };

    if (!_radio.sendMessage(std::move(message))) {
        _log.error("failed to send remote control message");
    }
}