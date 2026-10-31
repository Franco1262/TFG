#pragma once
#include <FS.h>
#include <SD_MMC.h>
#include "Clock.h"
#include "Power.h"
#include "config.h"
#include "types.h"
#include "CircularBuffer.h"

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
     * @brief Writes the first count sensor readings to the daily JSONL file.
     * @param buffer Reference to the readings that are about to be sent.
     * @param count Number of readings to log.
     * @return true when all requested readings were written.
     */
    bool logBatch(const CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>& buffer, uint8_t count);

    /**
     * @brief Unmounts the filesystem and powers down the SD card.
     *
     * This releases file resources, ends the SD_MMC filesystem, and disables
     * the SD power rail.
     */
    void shutdown();

}