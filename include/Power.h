#pragma once
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"
#include "Modem.h"

static XPowersPMU   PMU;


/**
 * @brief Power management utilities for ESP32 and SIM7080 modem.
 * * This namespace handles the initialization of the PMU (AXP2101), 
 * battery monitoring, and the complex power-up/down sequences 
 * required for the modem and GPS hardware.
 */
namespace Power
{
    /**
     * @brief Initializes the Power Management Unit (PMU) and hardware pins.
     * * Configures the AXP2101 via I2C and sets the initial voltages for the 
     * system peripherals. If the PMU is not detected, it triggers a safety 
     * sleep to prevent hardware instability.
     */
    void Begin();

    /**
     * @brief Powers down peripherals and puts the ESP32 into Deep Sleep mode.
     * * Disables the modem, GPS, and RS485 circuitry before entering sleep.
     * The wake-up trigger is set according to the TIME_TO_SLEEP_S constant.
     * This function is terminal; the CPU will reset upon wake-up.
     */
    void enterDeepSleep();

    /**
     * @brief Retrieves the current battery charge level.
     * * @return int Battery percentage (0-100) provided by the PMU fuel gauge.
     */
    int getBattPercent();

    /**
     * @brief Checks if the current boot is a fresh start or a wake-up from sleep.
     * * Useful for running initialization code that should only execute once 
     * per power cycle, not after every deep sleep wake-up.
     * * @return true  If the system just performed a cold boot (Power On).
     * @return false If the system woke up from a deep sleep state.
     */
    bool isFirstBoot();

    /**
     * @brief Executes the power-up sequence for the SIM7080 modem and GNSS.
     * * Enables DC3 and BLDO2 voltage rails, performs the hardware Power Key 
     * pulse if the modem is unresponsive, and activates the GNSS engine via AT commands.
     */
    void enableModemAndGps();

    /**
     * @brief Executes a graceful shutdown of the modem and cuts power.
     * * Sends the Power Down command (AT+CPOWD) to the modem to ensure 
     * flash memory integrity and unregistration from the network before 
     * physically cutting the voltage rails.
     */
    void disableModemAndGps();
}