#include "Power.h"
#define TIME_TO_SLEEP_S 20


void Power::Begin()
{
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL))
    {
        esp_sleep_enable_timer_wakeup(60 * 1000000ULL);
        esp_deep_sleep_start();
    }

    PMU.setDC5Voltage(3300);
    PMU.enableDC5();

    PMU.setALDO3Voltage(3300);
    PMU.enableALDO3();

    pinMode(RS485_DIR1, OUTPUT);
}


bool Power::isFirstBoot()
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    return (cause == ESP_SLEEP_WAKEUP_UNDEFINED);
}


int Power::getBattPercent()
{
    return PMU.getBatteryPercent();
}


void Power::enableModemAndGps()
{
    PMU.setDC3Voltage(3000);
    PMU.enableDC3();

    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();

    pinMode(MODEM_PWR, OUTPUT);
    delay(100);

    int retry = 0;
    while (!Modem::sendAtAndRead("AT", NULL, 0, 1000)) 
    {
        if (retry++ > 15) 
        {
            digitalWrite(MODEM_PWR, LOW);
            delay(100);
            digitalWrite(MODEM_PWR, HIGH);
            delay(100);
            digitalWrite(MODEM_PWR, LOW);
            retry = 0;
        }
    }
    
    Modem::sendAtAndRead("AT+CGNSPWR=1", NULL, 0, 1000);
    delay(100);
}


void Power::disableModemAndGps()
{
    Modem::sendAtAndRead("AT+CGNSPWR=0", NULL, 0, 1000);
    Modem::sendAtAndRead("AT+CPOWD=1", NULL, 0, 1000);


    PMU.disableBLDO2();
    PMU.disableDC3();
}


void Power::enterDeepSleep()
{
    disableModemAndGps();
    PMU.disableALDO3();
    digitalWrite(RS485_DIR1, LOW);

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_S * 1000000ULL);
    
    Serial.flush();
    esp_deep_sleep_start();
}