#pragma once
#include <Arduino.h>
#include "utilities.h"

namespace Lora
{
    void setupLora();
    void sendBatch();
    void sendSingle(Data data);
    //char* parseData() ???
}