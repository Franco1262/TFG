#pragma once
#include <Arduino.h>
#include "types.h"
#include "config.h"
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
     * @brief Powers on the RS485 UART interface and sensor hardware.
     * * Initializes Serial2 with Modbus parameters (9600 baud, 8N1).
     * Allows time for the sensor to boot if needed.
     */
    void turnOn();

    /**
     * @brief Powers down the RS485 UART interface.
     * * Ends Serial2 connection to free GPIO pins for isolation during deep sleep.
     */
    void turnOff();

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
}