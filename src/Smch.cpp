#include "Smch.h"
#include "Utils.h"

#include <sstream>

Smch::Smch(const ApplicationConfig& appConfig)
    : _coreApplication(appConfig)
    , _appConfig(appConfig)
    , _radio(0)
    // , _deviceHub(_radio)
#ifdef IOT_ENABLE_BLYNK
    , _blynk(_coreApplication.blynkHandler())
#endif
    , _mqtt(_coreApplication.mqttClient())
{
    Logger::setup(_appConfig, _coreApplication.systemClock());

#ifdef IOT_ENABLE_BLYNK
    _blynk.setCommandHandler([this](const uint8_t deviceIndex, const radio::Command command) {
        sendRemoteControlCommand(deviceIndex, command);
    });
#endif

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
        _log.error_P(PSTR("failed to send remote control message"));
    }
}

void Smch::handleWebApiCommand(WebApi::Command command, const uint8_t subDeviceIndex)
{
    _log.debug_P(PSTR("handleWebApiCommand: command=%s, subDeviceIndex=%u"), toString(command), subDeviceIndex);

    if (subDeviceIndex == 255) {
        if (!_commandQueue.empty()) {
            _log.warning_P(PSTR("handleWebApiCommand: command queue is not empty"));
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
        static constexpr std::array<std::pair<int, radio::Command>, 7> OpenCommands{
            std::pair<int, radio::Command>{ 0, radio::Command::Shutter1Up },      // Living Room Left Door
            std::pair<int, radio::Command>{ 2, radio::Command::Shutter1Up },      // Living Room Left Window
            std::pair<int, radio::Command>{ 1, radio::Command::Shutter2Up },      // Living Room Right Window
            std::pair<int, radio::Command>{ 1, radio::Command::Shutter1Up },      // Living Room Right Door
            std::pair<int, radio::Command>{ 4, radio::Command::Shutter1Up },      // Kitchen Door
            std::pair<int, radio::Command>{ 4, radio::Command::Shutter2Up },      // Kitchen Left Window
            std::pair<int, radio::Command>{ 3, radio::Command::Shutter1Up },      // Kitchen Right Window
        };

        static constexpr std::array<std::pair<int, radio::Command>, 7> CloseCommands{
            std::pair<int, radio::Command>{ 0, radio::Command::Shutter1Down },    // Living Room Left Door
            std::pair<int, radio::Command>{ 2, radio::Command::Shutter1Down },    // Living Room Left Window
            std::pair<int, radio::Command>{ 1, radio::Command::Shutter2Down },    // Living Room Right Window
            std::pair<int, radio::Command>{ 1, radio::Command::Shutter1Down },    // Living Room Right Door
            std::pair<int, radio::Command>{ 4, radio::Command::Shutter1Down },    // Kitchen Door
            std::pair<int, radio::Command>{ 4, radio::Command::Shutter2Down },    // Kitchen Left Window
            std::pair<int, radio::Command>{ 3, radio::Command::Shutter1Down },    // Kitchen Right Window
        };

        _mqtt.shutters[i].setChangedHandler([this, i](const int v) {
            const auto& command = v > 0 ? OpenCommands[i] : CloseCommands[i];
            _commandQueue.emplace(command);
        });
    }

    const auto makeButtonConfig = [](
        const bool open,
        PGM_P name,
        PGM_P buttonId,
        PGM_P commandTopic
    ) {
        std::stringstream config;

        config
            << '{'
            << Utils::pgmToStdString(PSTR(R"("icon":)"))
            << (open
                ? Utils::pgmToStdString(PSTR(R"("mdi:window-shutter-open")"))
                : Utils::pgmToStdString(PSTR(R"("mdi:window-shutter")"))
            )
            << Utils::pgmToStdString(PSTR(R"(,"name":")"))
            << Utils::pgmToStdString(name)
            << (open
                ? Utils::pgmToStdString(PSTR(" Open"))
                : Utils::pgmToStdString(PSTR(" Close"))
            ) 
            << '"'
            << Utils::pgmToStdString(PSTR(R"(,"object_id":"smch_)"))
            << Utils::pgmToStdString(buttonId)
            << (open
                ? Utils::pgmToStdString(PSTR("_open"))
                : Utils::pgmToStdString(PSTR("_close"))
            )
            << '"'
            << Utils::pgmToStdString(PSTR(R"(,"unique_id":"smch_)"))
            << Utils::pgmToStdString(buttonId)
            << (open
                ? Utils::pgmToStdString(PSTR("_open"))
                : Utils::pgmToStdString(PSTR("_close"))
            )
            << '"'
            << Utils::pgmToStdString(PSTR(R"(,"command_topic":")"))
            << Utils::pgmToStdString(commandTopic)
            << '"'
            << Utils::pgmToStdString(PSTR(R"(,"payload_press":")"))
            << (open
                ? Utils::pgmToStdString(PSTR("1"))
                : Utils::pgmToStdString(PSTR("0"))
            )
            << '"'
            << '}';

        return config.str();
    };

    static const std::array<PGM_P, 7> OpenButtonConfigTopics{
        PSTR("homeassistant/button/smch_livingroom_leftdoor_open/config"),
        PSTR("homeassistant/button/smch_livingroom_leftwindow_open/config"),
        PSTR("homeassistant/button/smch_livingroom_rightwindow_open/config"),
        PSTR("homeassistant/button/smch_livingroom_rightdoor_open/config"),
        PSTR("homeassistant/button/smch_kitchen_door_open/config"),
        PSTR("homeassistant/button/smch_kitchen_leftwindow_open/config"),
        PSTR("homeassistant/button/smch_kitchen_rightwindow_open/config")
    };

    static const std::array<PGM_P, 7> CloseButtonConfigTopics{
        PSTR("homeassistant/button/smch_livingroom_leftdoor_close/config"),
        PSTR("homeassistant/button/smch_livingroom_leftwindow_close/config"),
        PSTR("homeassistant/button/smch_livingroom_rightwindow_close/config"),
        PSTR("homeassistant/button/smch_livingroom_rightdoor_close/config"),
        PSTR("homeassistant/button/smch_kitchen_door_close/config"),
        PSTR("homeassistant/button/smch_kitchen_leftwindow_close/config"),
        PSTR("homeassistant/button/smch_kitchen_rightwindow_close/config")
    };

    static const std::array<std::tuple<PGM_P, PGM_P, PGM_P>, 7> CommandButtons{
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Living Room Left Door"), PSTR("livingroom_leftdoor"), PSTR("home/shutters/livingroom/leftdoor/state/set") },
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Living Room Left Window"), PSTR("livingroom_leftwindow"), PSTR("home/shutters/livingroom/leftwindow/state/set") },
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Living Room Right Window"), PSTR("livingroom_rightwindow"), PSTR("home/shutters/livingroom/rightwindow/state/set") },
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Living Room Right Door"), PSTR("livingroom_rightdoor"), PSTR("home/shutters/livingroom/rightdoor/state/set") },
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Kitchen Door"), PSTR("kitchen_door"), PSTR("home/shutters/kitchen/door/state/set") },
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Kitchen Left Window"), PSTR("kitchen_leftwindow"), PSTR("home/shutters/kitchen/leftwindow/state/set") },
        std::tuple<PGM_P, PGM_P, PGM_P>{ PSTR("Kitchen Right Window"), PSTR("kitchen_rightwindow"), PSTR("home/shutters/kitchen/rightwindow/state/set") }
    };

    for (auto i = 0u; i < CommandButtons.size(); ++i) {
        _coreApplication.mqttClient().publish(
            OpenButtonConfigTopics[i],
            makeButtonConfig(
                true,
                std::get<0>(CommandButtons[i]),
                std::get<1>(CommandButtons[i]),
                std::get<2>(CommandButtons[i])
            ),
            false
        );

        _coreApplication.mqttClient().publish(
            CloseButtonConfigTopics[i],
            makeButtonConfig(
                false,
                std::get<0>(CommandButtons[i]),
                std::get<1>(CommandButtons[i]),
                std::get<2>(CommandButtons[i])
            ),
            false
        );
    }

    updateMqtt();
}

void Smch::updateMqtt()
{
}