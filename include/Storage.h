#pragma once
#include <FS.h>
#include <SD_MMC.h>
#include "utilities.h"


static File file;

namespace Storage
{
    void printSDFile(const char* path);
    void write(Data& data);
    bool Begin();
}