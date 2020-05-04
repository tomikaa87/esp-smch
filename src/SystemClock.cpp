/*
    This file is part of esp-thermostat.

    esp-thermostat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    esp-thermostat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with esp-thermostat.  If not, see <http://www.gnu.org/licenses/>.

    Author: Tamas Karpati
    Created on 2016-12-30
*/

#include "SystemClock.h"

SystemClock::SystemClock()
{
    _log.info("initializing");
}

void SystemClock::task()
{
    if (_lastRtcSync > 0 && _epoch - _lastRtcSync > RtcSyncIntervalSec) {
    }
}

void ICACHE_RAM_ATTR SystemClock::timerIsr()
{
    if (++_isrCounter == 3125) {
        _isrCounter = 0;
        ++_epoch;
    }
}

std::time_t SystemClock::localTime() const
{
    auto localTime = _epoch + _localTimeOffsetMinutes * 60;
    if (isDst(localTime)) {
        localTime += _localTimeDstOffsetMinutes * 60;
    }

    return localTime;
}

std::time_t SystemClock::utcTime() const
{
    return _epoch;
}

void SystemClock::setUtcTime(const std::time_t t)
{
    _log.info("setting UTC time: %ld", t);

    _epoch = t;
}

bool SystemClock::isDst(const std::time_t t)
{
    const auto tm = gmtime(&t);

    // Before March or after October
    if (tm->tm_mon < 2 || tm->tm_mon > 9)
        return false;

    // After March and before October
    if (tm->tm_mon > 2 && tm->tm_mon < 9)
        return true;

    const auto previousSunday = tm->tm_mday - tm->tm_wday;

    // Sunday after March 25th
    if (tm->tm_mon == 2)
        return previousSunday >= 25;

    // Sunday before October 25th
    if (tm->tm_mon == 9)
        return previousSunday < 25;

    // This should never happen
    return false;
}