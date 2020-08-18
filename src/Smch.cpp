#include "PrivateConfig.h"
#include "Smch.h"

Smch::Smch(const ApplicationConfig& appConfig)
    : _coreApplication(appConfig)
    , _appConfig(appConfig)
    , _radio(0)
    , _deviceHub(_radio)
{
    Logger::setup(_appConfig, _coreApplication.systemClock());
}

void Smch::task()
{
    _coreApplication.task();
    _radio.task();
    _deviceHub.task();
}