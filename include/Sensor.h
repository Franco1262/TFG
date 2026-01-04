#pragma once
#include <Arduino.h>
#include "utilities.h"
#include <cstdlib>

/**
 * @brief Interface for industrial sensor communication via Modbus RTU.
 * * This namespace encapsulates all logic required to poll sensors over 
 * a half-duplex RS485 bus, including direction control, raw data 
 * acquisition, and protocol-specific decoding.
 */
namespace Sensor
{
    /**
     * @brief Performs a complete Modbus RTU read cycle.
     * * Manages the RS485 transceiver state (TX/RX), sends a 0x03 (Read Holding Registers) 
     * command, and waits for a 19-byte response. Validates the response based on 
     * Slave ID and Function Code
     * * @param data Reference to a Data structure where the results will be stored.
     * @return true  If a valid 19-byte frame was received and passed all checks.
     * @return false If a timeout occurred or the response was malformed.
     */
    bool read(Data& data);

    /**
     * @brief Decodes a raw Modbus RTU frame into structured data.
     * * Takes the 19-byte raw payload, extracts the register values
     * and maps them to the appropriate fields in the Data structure.
     * * @param response Reference to the fixed-size 19-byte array containing the raw frame.
     * @param data     Reference to the Data structure to be populated.
     */
    void decodeData(const uint8_t (&response)[19], Data& data); 
}