#pragma once
#include <ctime>
#include <Arduino.h>
#include "Modem.h"
#include "Power.h"

RTC_DATA_ATTR static time_t lastSyncTimestamp = 0;

namespace Clock
{
    String getGPSTime();
    time_t fromGPSToUnix(String FechaGPS);
    void syncWithGPS();
    bool needsDailySync();
    time_t getTime();
}