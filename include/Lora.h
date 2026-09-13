#pragma once
#include <Arduino.h>
#include "types.h"
#include "Power.h"
#include "config.h"
#include "CircularBuffer.h"
#include <SPI.h>
#include <RadioLib.h>
#include "debug.h"
#include <Preferences.h>

namespace Lora
{
    /**
     * @brief Powers on the LoRa module and initializes SPI and radio.
     * 
     * Enables the LoRa power rail via Power namespace, initializes the SPI bus
     * with configured pins, and initializes the radio module at 868.1 MHz.
     * Does not perform network join. Call turnOn() before joinOTAA().
     * 
     * @return true  Radio initialized successfully.
     * @return false Radio initialization failed.
     */
    bool turnOn();

    /**
     * @brief Performs OTAA (Over-The-Air Activation) join with LoRaWAN network.
     * 
     * Initializes OTAA configuration and activates the LoRaWAN session using
     * credentials from config.h. Call after turnOn() on cold boot.
     * 
     * @return true  OTAA join successful and session activated.
     * @return false OTAA join failed.
     */
    bool joinOTAA();

    /**
     * @brief Powers off the LoRa module.
     * 
     * Disables the LoRa power rail via Power namespace to conserve power.
     * Called when LoRa communication is complete or during sleep cycles.
     */
    void turnOff();

    /**
     * @brief Restores a saved session if available, otherwise joins OTAA and stores the session.
     *
     * Must be called after turnOn(). This centralizes the session bootstrap logic so
     * the application can safely query the current payload size only after a session exists.
     *
     * @return true  A LoRaWAN session is ready for transmission.
     * @return false Session restoration/join failed.
     */
    bool ensureSessionReady();

    /**
     * @brief Returns the maximum number of Data records that fit in a single LoRaWAN uplink.
     *
     * Calculates the maximum number of Data structures that can be transmitted in a
     * single LoRaWAN uplink frame based on the current maximum application payload
     * size allowed by the active data rate (which may change when ADR is enabled).
     *
     * This value can be used by the application to determine when enough buffered
     * measurements are available to efficiently transmit one packet.
     *
     * @return uint8_t Maximum number of Data records that fit in one LoRaWAN uplink.
     *                 Returns 0 until a session is active and the payload size is known.
    */
    uint8_t getMaxItemsPerPacket();

    /**
     * @brief Saves the current LoRaWAN session to RTC memory for persistence across sleep.
     * 
     * Stores session keys, device address, frame counters, and current SF.
     * Must be called after join or significant network changes.
     */
    void saveLoRaSessionToRTC();

    /**
     * @brief Restores the LoRaWAN session from RTC memory.
     * 
     * Applies saved session keys, device address, and frame counters to the node instance.
     * Call after warm boot before sending data.
     */
    bool restoreLoRaSessionFromRTC();

    /**
     * @brief Sends queued Data packets in batches respecting LoRa payload limits.
     * @details Transmits packets that fit within the current LoRa payload size.
     * May not send entire buffer if limited by payload size - caller should retry remaining items.
     * 
     * @param buffer Reference to CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE> containing packets to send.
     * @return Number of items actually sent (0 if send failed, > 0 if successful).
     */
    uint8_t sendBatch(CircularBuffer<Data, MAX_ABSOLUTE_BATCH_SIZE>& buffer);

    /**
     * @brief Saves the current LoRaWAN Nonces buffer to Non-Volatile Storage (NVS).
     * 
     * @details Extracts only the Nonces buffer (which tracks the DevNonce and JoinNonce) 
     * from the RadioLib node and stores it in the ESP32's flash memory. This is crucial 
     * to call after an OTAA Join attempt—even if the attempt fails (e.g., due to bad coverage). 
     * It ensures that the consumed DevNonce is persistently recorded so it is not reused 
     * in the next Join attempt, preventing the network server from permanently blocking 
     * the device due to a replay attack error.
     */
    static void saveNoncesToNVS();

    /**
     * @brief Restores only the LoRaWAN Nonces buffer from Non-Volatile Storage (NVS).
     * 
     * @details Reads the previously saved Nonces buffer from flash memory and injects it 
     * into the RadioLib node. This function must be called right before initiating a 
     * new OTAA Join-Request (when a full session restore is not possible or desired). 
     * By pre-loading the historical nonces, it forces the node to increment the DevNonce 
     * from its last known value rather than starting from zero, ensuring the network server 
     * (like ChirpStack) accepts the new Join request.
     */
    static void restoreNoncesFromNVS();
}