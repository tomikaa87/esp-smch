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

    _webApi.setCommandHandler([this](const WebApi::Command command, const uint8_t deviceIndex) {
        handleWebApiCommand(command, deviceIndex);
    });
}

void Smch::task()
{
    _coreApplication.task();
    _radio.task();
    _webApi.task();
    // _deviceHub.task();

    if (!_commandQueue.empty()) {
        if (millis() - _lastQueuedExecTimestamp >= 200) {
            const auto p = _commandQueue.front();
            _commandQueue.pop();

            sendRemoteControlCommand(p.first, p.second);

            _lastQueuedExecTimestamp = millis();
        }
    }
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

void Smch::handleWebApiCommand(WebApi::Command command, const uint8_t subDeviceIndex)
{
    _log.debug("handleWebApiCommand: command=%s, subDeviceIndex=%u", toString(command), subDeviceIndex);

    if (subDeviceIndex == 255) {
        if (!_commandQueue.empty()) {
            _log.warning("handleWebApiCommand: command queue is not empty");
            return;
        }

        constexpr auto MaxDeviceIndex = 4;

        switch (command) {
            case WebApi::Command::ShutterUp:
                for (uint8_t i = 0; i <= MaxDeviceIndex; ++i) {
                    _commandQueue.emplace(std::make_pair(i, radio::Command::AllUp));
                }
                break;

            case WebApi::Command::ShutterDown:
                for (uint8_t i = 0; i <= MaxDeviceIndex; ++i) {
                    _commandQueue.emplace(std::make_pair(i, radio::Command::AllDown));
                }
                break;
        }
    } else if (subDeviceIndex < 10 * 2) {
        const auto deviceIndex = subDeviceIndex >> 1;

        switch (command) {
            case WebApi::Command::ShutterUp:
                sendRemoteControlCommand(
                    deviceIndex,
                    subDeviceIndex & 1 ? radio::Command::Shutter2Up : radio::Command::Shutter1Up
                );
                break;

            case WebApi::Command::ShutterDown:
                sendRemoteControlCommand(
                    deviceIndex,
                    subDeviceIndex & 1 ? radio::Command::Shutter2Down : radio::Command::Shutter1Down
                );
                break;
        }
    }
}