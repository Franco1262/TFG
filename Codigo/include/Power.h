#pragma once
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "config.h"
#include "driver/rtc_io.h"

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
    void turnOnMinimalHW();

    /**
     * @brief Powers down peripherals and puts the ESP32 into Deep Sleep mode.
     * * Disables the modem and SD circuitry before entering sleep.
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
    bool isColdBoot();

    /**
     * @brief Enables power rails for the SIM7080 modem (DC3 and BLDO2).
     * * Prerequisite for calling Modem::turnOn(). Does NOT communicate with the modem.
     */
    void enableModemPower();

    /**
     * @brief Disables power rails for the SIM7080 modem (DC3 and BLDO2).
     * * Should be called after Modem::turnOff() to physically cut modem power.
     */
    void disableModemPower();

    /**
     * @brief Enables power rail for the SD card.
     * * Activates the appropriate power rail (ALDO3) to supply
     * voltage to the SD card module, enabling read/write operations.
     */
    void enableSDCardPower();

    /**
     * @brief Disables power rail for the SD card.
     * * Cuts the power supply to the SD card module to reduce power consumption.
     * Should be called when SD card operations are no longer needed.
     */
    void disableSDCardPower();

    /**
     * @brief Enables DC5 power rail (3.3V) for the LoRa module.
     * * Activates the DC5 power rail to supply voltage to the SX1276 LoRa radio module,
     * enabling RF communication. Must be called before any LoRa operations.
     */
    void enableDC5();

    /**
     * @brief Disables DC5 power rail.
     * * Cuts the power supply to the LoRa module to reduce power consumption.
     * Should be called after LoRa transmission is complete or during sleep cycles.
     */
    void disableDC5();
}