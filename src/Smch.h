#pragma once

#include "Blynk.h"
#include "Command.h"
#include "DeviceHub.h"
#include "Radio.h"
#include "WebApi.h"

#include <CoreApplication.h>
#include <Logger.h>

#include <network/MQTT/MqttVariable.h>

#include <cstdint>
#include <queue>
#include <utility>

class Smch
{
public:
    Smch(const ApplicationConfig& appConfig);

    void task();

private:
    CoreApplication _coreApplication;
    const ApplicationConfig& _appConfig;
    const Logger _log{ "Smch" };
    radio::Radio _radio;
    // DeviceHub _deviceHub;
#ifdef IOT_ENABLE_BLYNK
    Blynk _blynk;
#endif
    WebApi _webApi;
    std::queue<std::pair<uint8_t, radio::Command>> _commandQueue;
    uint32_t _lastQueuedExecTimestamp = 0;

    struct Mqtt {
        Mqtt(MqttClient& mqttClient)
            : shutters{
                MqttVariable<int>{ PSTR("home/shutters/livingroom/leftdoor/state/get"), PSTR("home/shutters/livingroom/leftdoor/state/set"), mqttClient },
                MqttVariable<int>{ PSTR("home/shutters/livingroom/leftwindow/state/get"), PSTR("home/shutters/livingroom/leftwindow/state/set"), mqttClient },
                MqttVariable<int>{ PSTR("home/shutters/livingroom/rightwindow/state/get"), PSTR("home/shutters/livingroom/rightwindow/state/set"), mqttClient },
                MqttVariable<int>{ PSTR("home/shutters/livingroom/rightdoor/state/get"), PSTR("home/shutters/livingroom/rightdoor/state/set"), mqttClient },
                MqttVariable<int>{ PSTR("home/shutters/kitchen/door/state/get"), PSTR("home/shutters/kitchen/door/state/set"), mqttClient },
                MqttVariable<int>{ PSTR("home/shutters/kitchen/leftwindow/state/get"), PSTR("home/shutters/kitchen/leftwindow/state/set"), mqttClient },
                MqttVariable<int>{ PSTR("home/shutters/kitchen/rightwindow/state/get"), PSTR("home/shutters/kitchen/rightwindow/state/set"), mqttClient }
            }
        {}

        std::array<MqttVariable<int>, 7> shutters;
    } _mqtt;

    void setupMqtt();
    void updateMqtt();

    void sendRemoteControlCommand(uint8_t deviceIndex, radio::Command command);
    void handleWebApiCommand(WebApi::Command command, uint8_t subDeviceIndex);
};