#pragma once
#include <FS.h>
#include <SD_MMC.h>
#include "Clock.h"
#include "Power.h"
#include "config.h"
#include "types.h"
#include "CircularBuffer.h"

static File file;

/**
 * @brief Persistent storage management for SD_MMC.
 */
namespace SD
{
    /**
     * @brief Powers on the SD card hardware.
     *
     * This enables the SD power rail only. It does not mount the filesystem.
     */
    void powerOn();

    /**
     * @brief Mounts and initializes the SD filesystem.
     * @return true if mounted successfully, false otherwise.
     */
    bool mount();

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
     * @brief Writes a batch of sensor readings to the SD card from a circular buffer.
     * @details Iterates over all items in the circular buffer and writes them to a daily log file as JSONL.
     * @param buffer Reference to CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE> containing readings to log.
     */
    void logBatch(const CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>& buffer);

    /**
     * @brief Appends raw sensor data to a binary file for batch transmission.
     * @param data Reference to sensor data structure.
     */
    void writeToPending(Data& data);

    /**
     * @brief Unmounts the filesystem and powers down the SD card.
     *
     * This releases file resources, ends the SD_MMC filesystem, and disables
     * the SD power rail.
     */
    void shutdown();

    /**
     * @brief Deletes the pending binary file from the SD card.
     */
    void clearPending();
}