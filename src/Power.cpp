#include "Power.h"


void Power::turnOnMinimalHW() 
{
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA_PMU, I2C_SCL_PMU))
    {
        esp_sleep_enable_timer_wakeup(60 * 1000000ULL);
        esp_deep_sleep_start();
    }

    PMU.disableTSPinMeasure();
}


bool Power::isColdBoot() 
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    return (cause == ESP_SLEEP_WAKEUP_UNDEFINED);
}


int Power::getBattPercent() 
{
    return PMU.getBatteryPercent();
}


void Power::enableModemPower() 
{
    PMU.setDC3Voltage(3000);
    PMU.enableDC3();

    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();

    delay(100);
}


void Power::disableModemPower() 
{
    PMU.disableBLDO2();
    PMU.disableDC3();
}


void Power::enableSDCardPower() 
{
    PMU.setALDO3Voltage(3300);
    PMU.enableALDO3();

    delay(10);
}


void Power::disableSDCardPower() 
{
    PMU.disableALDO3();
}


void Power::enableDC5() 
{
    PMU.setDC5Voltage(3300);
    PMU.enableDC5();
    delay(10);
}


void Power::disableDC5() {
    PMU.disableDC5();
}


void Power::enterDeepSleep() 
{
    disableModemPower();
    disableSDCardPower();
    disableDC5();

    gpio_reset_pin((gpio_num_t)LORA_DIO0);
    gpio_reset_pin((gpio_num_t)SDMMC_CMD);
    gpio_reset_pin((gpio_num_t)SDMMC_CLK);
    gpio_reset_pin((gpio_num_t)SDMMC_DATA);

    esp_sleep_enable_timer_wakeup(10 * 1000000ULL);
    esp_deep_sleep_start();
}