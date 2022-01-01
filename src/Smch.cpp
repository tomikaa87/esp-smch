#include "Smch.h"
#include "Utils.h"

Smch::Smch(const ApplicationConfig& appConfig)
    : _coreApplication(appConfig)
    , _appConfig(appConfig)
    , _radio(0)
    // , _deviceHub(_radio)
    , _blynk(_coreApplication.blynkHandler())
    , _mqtt(_coreApplication.mqttClient())
{
    Logger::setup(_appConfig, _coreApplication.systemClock());

    _blynk.setCommandHandler([this](const uint8_t deviceIndex, const radio::Command command) {
        sendRemoteControlCommand(deviceIndex, command);
    });

    _webApi.setCommandHandler([this](const WebApi::Command command, const uint8_t deviceIndex) {
        handleWebApiCommand(command, deviceIndex);
    });

    setupMqtt();
    _coreApplication.setMqttUpdateHandler([this]{ updateMqtt(); });
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


void Smch::setupMqtt()
{
    for (unsigned i = 0; i < _mqtt.shutters.size(); ++i) {
        _mqtt.shutters[i] = 0;

        // Order of these must match the order of the MQTT variables
        static const std::vector<std::pair<int, radio::Command>> OpenCommands{
            { 0, radio::Command::Shutter1Up },      // Living Room Left Door
            { 2, radio::Command::Shutter1Up },      // Living Room Left Window
            { 1, radio::Command::Shutter2Up },      // Living Room Right Window
            { 1, radio::Command::Shutter1Up },      // Living Room Right Door
            { 4, radio::Command::Shutter1Up },      // Kitchen Door
            { 4, radio::Command::Shutter2Up },      // Kitchen Left Window
            { 3, radio::Command::Shutter1Up },      // Kitchen Right Window
        };

        static const std::vector<std::pair<int, radio::Command>> CloseCommands{
            { 0, radio::Command::Shutter1Down },    // Living Room Left Door
            { 2, radio::Command::Shutter1Down },    // Living Room Left Window
            { 1, radio::Command::Shutter2Down },    // Living Room Right Window
            { 1, radio::Command::Shutter1Down },    // Living Room Right Door
            { 4, radio::Command::Shutter1Down },    // Kitchen Door
            { 4, radio::Command::Shutter2Down },    // Kitchen Left Window
            { 3, radio::Command::Shutter1Down },    // Kitchen Right Window
        };

        _mqtt.shutters[i].setChangedHandler([this, i](const int v) {
            const auto& command = v > 0 ? OpenCommands[i] : CloseCommands[i];
            _commandQueue.emplace(command);
        });
    }

    updateMqtt();
}

void Smch::updateMqtt()
{
}