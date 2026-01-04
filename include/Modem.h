#pragma once
#include <Arduino.h>


/**
 * @brief Communication interface for the SIM7080 modem.
 * * Provides low-level utilities to send AT commands and parse specialized 
 * GNSS responses. Designed to be memory-efficient by avoiding String objects.
 */
namespace Modem
{
    /**
     * @brief Parses and extracts a specific comma-separated field from a GNSS response.
     * * This function locates the "+CGNSINF: " prefix and iterates through the data 
     * to find the requested field index. It performs safety checks to ensure the 
     * destination buffer is not overrun and ensures the result is null-terminated.
     * * @param response     Pointer to the character array containing the raw modem response.
     * @param n_field      The zero-based index of the field to extract (e.g., 2 for UTC time).
     * @param buffer       Pointer to the destination buffer where the field will be stored.
     * @param buffer_size  The total capacity of the destination buffer.
     * @return int         The length of the extracted field, or 0 if the field was not found.
     */
    int getGnssField(const char* response, int n_field, char* buffer, int buffer_size);

    /**
     * @brief Transmits an AT command and captures the response until a status is received.
     * * Clears the Serial1 RX buffer before transmission. It then reads incoming data into 
     * the provided buffer until "OK" or "ERROR" is detected, or the timeout period expires.
     * Includes a yield() call to maintain ESP32 system stability during the wait loop.
     * * @param cmd          The null-terminated AT command string to send.
     * @param buffer       Pointer to the buffer where the modem's response will be captured.
     * @param buffer_size  The maximum capacity of the capture buffer.
     * @param timeout_ms   The maximum time to wait for a terminal response in milliseconds.
     * @return true        If a terminal response ("OK" or "ERROR") was successfully received.
     * @return false       If the timeout was reached without receiving a terminal response.
     */
    bool sendAtAndRead(const char* cmd, char* buffer, int buffer_size, unsigned long timeout_ms);
}