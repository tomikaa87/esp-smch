#pragma once

#include "DeviceHub.h"
#include "Logger.h"
#include "NtpClient.h"
#include "OtaUpdater.h"
#include "Radio.h"
#include "SystemClock.h"

#include <cstdint>

class Smch
{
public:
    Smch();

    void task();

private:
    const Logger _log{ "Smch" };
    SystemClock _systemClock;
    NtpClient _ntpClient;
    OtaUpdater _otaUpdater;
    radio::Radio _radio;
    DeviceHub _deviceHub;

    static constexpr auto SlowLoopRunIntervalMs = 500;
    uint32_t _lastSlowLoopRun = 0;

    bool _updateChecked = false;
    uint32_t _updateCheckTimer = 0;

    void setupWiFi();
};