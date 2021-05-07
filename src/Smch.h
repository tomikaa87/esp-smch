#pragma once

#include "Blynk.h"
#include "Command.h"
#include "DeviceHub.h"
#include "Radio.h"
#include "WebApi.h"

#include <CoreApplication.h>
#include <Logger.h>

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
    Blynk _blynk;
    WebApi _webApi;
    std::queue<std::pair<uint8_t, radio::Command>> _commandQueue;
    uint32_t _lastQueuedExecTimestamp = 0;

    void sendRemoteControlCommand(uint8_t deviceIndex, radio::Command command);
    void handleWebApiCommand(WebApi::Command command, uint8_t subDeviceIndex);
};