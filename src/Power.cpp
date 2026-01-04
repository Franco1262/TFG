#include "Power.h"
#define TIME_TO_SLEEP_S 300

void Power::Begin()
{
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL))
    {
        esp_sleep_enable_timer_wakeup(60 * 1000000ULL); // Retry again in 1 minute
        esp_deep_sleep_start();
    }

    PMU.setDC5Voltage(3300); // RTC main power channel 2700~ 3400V
    PMU.enableDC5();

    PMU.setALDO3Voltage(3300);   // SD Card VDD 3300
    PMU.enableALDO3();

    pinMode(RS485_DIR1, OUTPUT);
}


bool Power::isFirstBoot()
{
    bool firstBoot = false;
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if(cause == ESP_SLEEP_WAKEUP_UNDEFINED)
        firstBoot = true;

    return firstBoot;
}


void Power::enterDeepSleep()
{
    disableModemAndGPS();
    PMU.disableALDO3();
    digitalWrite(RS485_DIR1, LOW);

    uint64_t sleepTimeSeconds = TIME_TO_SLEEP_S;
    esp_sleep_enable_timer_wakeup(sleepTimeSeconds * 1000000ULL);
    Serial.flush();
    esp_deep_sleep_start();
}


int Power::getBattPercent()
{
    return PMU.getBatteryPercent();
}


void Power::enableModemAndGPS()
{
    PMU.setDC3Voltage(3000);    // SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();

    pinMode(MODEM_PWR, OUTPUT);

    int retry = 0;
    while (!Modem::sendATandRead("AT", 1000)) 
    {
        if (retry++ > 15) 
        {
            digitalWrite(MODEM_PWR, LOW);
            delay(100);
            digitalWrite(MODEM_PWR, HIGH);
            delay(1000);
            digitalWrite(MODEM_PWR, LOW);
            retry = 0;
        }
    }
    
    Modem::sendATandRead("AT+CGNSPWR=1", 1000);
}


void Power::disableModemAndGPS()
{
    Modem::sendATandRead("AT+CGNSPWR=0", 1000); // Apagar motor GNSS
    PMU.disableBLDO2();
    PMU.disableDC3();
}