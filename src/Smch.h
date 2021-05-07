#pragma once

#include "Blynk.h"
#include "Command.h"
#include "DeviceHub.h"
#include "Radio.h"
#include "WebApi.h"

#include <CoreApplication.h>
#include <Logger.h>

#include <cstdint>

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

    void sendRemoteControlCommand(uint8_t deviceIndex, radio::Command command);
    void handleWebApiCommand(WebApi::Command command, uint8_t subDeviceIndex);
};