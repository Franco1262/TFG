#pragma once
#include <ctime>
#include <Arduino.h>
#include "Modem.h"
#include "Power.h"
#include "debug.h"


/**
 * @brief Time management and GPS synchronization utilities.
 * * This namespace provides functions to synchronize the internal ESP32 RTC 
 * with the high-precision time provided by the GNSS satellites.
 */
namespace Clock
{
    /**
     * @brief Orchestrates the hardware to obtain a valid Unix timestamp from GPS.
     * * This function enables the GPS hardware, polls for a valid signal until 
     * a timeout occurs, and parses the response into a time_t value. 
     * It automatically handles hardware power-down after completion.
     * * @return time_t The UTC Unix timestamp if successful, or 0 if a timeout occurred.
     */
    time_t getGpsTime();

    /**
     * @brief Parses a raw GPS date-time string into a Unix timestamp.
     * * Expects a string in the format "YYYYMMDDHHMMSS.sss". This function 
     * uses sscanf for memory-efficient parsing without temporary string objects.
     * * @param gps_string Pointer to the null-terminated string containing the GPS timestamp.
     * @return time_t The resulting Unix timestamp (UTC), or 0 if the format is invalid.
     */
    time_t fromGpsToUnix(const char* gps_string);

    /**
     * @brief Higher-level function to synchronize the ESP32 system clock with GPS.
     * * It calls getGpsTime() and, if a valid time is obtained, updates the 
     * internal system RTC using settimeofday(). It also updates the last 
     * synchronization timestamp for scheduling.
    * * @return true if the system clock was successfully updated with valid GPS data.
     * @return false if GPS fix was unavailable or system update failed.
     */
    bool syncWithGPS();

    /**
     * @brief Determines if the system clock needs a new synchronization.
     * * Compares the current system time with the last successful synchronization 
     * timestamp. Returns true if more than 24 hours (SECONDS_DAY) have passed.
     * * @return true  If a synchronization is required.
     * @return false If the current system time is still considered accurate.
     */
    bool needsDailySync();

    /**
     * @brief Retrieves the current system time from the internal RTC.
     * * This is a fast wrapper for the standard time(NULL) function. 
     * It should be used after a successful syncWithGPS() call.
     * * @return time_t Current system Unix timestamp (UTC).
     */
    time_t getTime();
    
    /**
     * @brief Returns the current date (YYYY-MM-DD) in user-provided buffer.
     * @param buffer Pointer to buffer where date string will be written.
     * @param len Size of buffer (must be at least 11 bytes for "YYYY-MM-DD\0").
     * @return const char* Pointer to buffer, or "1970-01-01" if gmtime_r fails.
     */
    const char* getDate(char* buffer, size_t len);
}