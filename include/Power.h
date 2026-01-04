#pragma once
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"
#include "Modem.h"

static XPowersPMU   PMU;

namespace Power
{
    void Begin();
    void enterDeepSleep();
    int getBattPercent();
    bool isFirstBoot();
    void enableModemAndGPS();
    void disableModemAndGPS();
}