#pragma once
#include <FS.h>
#include <SD_MMC.h>
#include "utilities.h"
#include "Clock.h"


static File file;

/**
 * @brief Persistent storage management for SD_MMC.
 */
namespace Storage
{
    /**
     * @brief Initializes SD_MMC hardware using 1-bit mode.
     * @return true if mounted successfully, false otherwise.
     */
    bool Begin();

    /**
     * @brief Dumps file content to Serial line by line.
     * @param path Full path to the file (e.g., "/data.txt").
     */
    void printSDFile(const char* path);

    /**
     * @brief Appends data to a daily file formatted as JSONL.
     * @details Filename is generated automatically (e.g., "/2026-01-04.jsonl").
     * @param data Reference to sensor data structure.
     */
    void writeToDailyLog(Data& data);

    /**
     * @brief Appends raw sensor data to a binary file for batch transmission.
     * @param data Reference to sensor data structure.
     */
    void writeToPending(Data& data);

    /**
     * @brief Deletes the pending binary file from the SD card.
     */
    void clearPending();
}