#include "FirmwareVersion.h"
#include "PrivateConfig.h"
#include "Smch.h"

#include <ESP8266WiFi.h>
#include <ESP8266WiFiSTA.h>

Smch::Smch()
    : _ntpClient(_systemClock)
    , _otaUpdater(PrivateConfig::OtaUpdateBaseUrl, _systemClock)
    , _radio(0)
    , _updateCheckTimer(millis())
{
    _log.info("initializing, firmware version: %d.%d.%d", FW_VER_MAJOR, FW_VER_MINOR, FW_VER_PATCH);

    setupWiFi();
}

void Smch::task()
{
    _systemClock.task();
    _ntpClient.task();
    _otaUpdater.task();
    _radio.task();

    // Slow loop
    if (_lastSlowLoopRun == 0 || millis() - _lastSlowLoopRun >= SlowLoopRunIntervalMs) {
        _lastSlowLoopRun = millis();

        if (!_updateChecked && WiFi.isConnected() && millis() - _updateCheckTimer >= 5000) {
            _updateChecked = true;
            _otaUpdater.forceUpdate();
        }
    }
}

void Smch::setupWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
    WiFi.setOutputPower(20.5);
    WiFi.begin(PrivateConfig::WiFiSSID, PrivateConfig::WiFiPassword);
}