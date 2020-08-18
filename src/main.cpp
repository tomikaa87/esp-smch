#include "Smch.h"
#include "PrivateConfig.h"

#include <Arduino.h>

#include <memory>

static std::unique_ptr<Smch> smch;

void setup()
{
    static ApplicationConfig appConfig;

    appConfig.firmwareVersion = VersionNumber{ 1, 0, 0 };

    appConfig.wifi.password = Config::WiFi::Password;
    appConfig.wifi.ssid = Config::WiFi::SSID;

    smch.reset(new Smch(appConfig));
}

void loop()
{
    smch->task();
}