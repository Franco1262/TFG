#pragma once
#include <Arduino.h>
#include "utilities.h"
#include <cstdlib>

namespace Sensor
{
    bool read(Data& data);
    void decodeData(const uint8_t (&response)[19], Data& data); 
}