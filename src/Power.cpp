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
    char buffer[256];

    // --- Alimentación PMU ---
    PMU.setDC3Voltage(3000);
    PMU.enableDC3();

    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();

    delay(500);

    // --- Encender módem ---
    pinMode(MODEM_PWR, OUTPUT);
    digitalWrite(MODEM_PWR, LOW);
    delay(100);
    digitalWrite(MODEM_PWR, HIGH);
    delay(1000);
    digitalWrite(MODEM_PWR, LOW);

    delay(5000);

    int retry = 0;
    while (!Modem::sendAtAndRead("AT", buffer, sizeof(buffer), 1000))
    {
        if (retry++ > 10)
        {
            digitalWrite(MODEM_PWR, HIGH);
            delay(1000);
            digitalWrite(MODEM_PWR, LOW);
            retry = 0;
        }
    }

    int retries = 3;

    auto sendWithRetry = [&](const char* cmd, unsigned long timeout_ms, int delay_ms = 200) -> bool 
    {
        for(int i = 0; i < retries; i++) 
        {
            if(Modem::sendAtAndRead(cmd, buffer, sizeof(buffer), timeout_ms))
                return true;
            delay(delay_ms);
        }
        return false;
    };

    sendWithRetry("AT+CFUN=1", 2000);
    delay(1000);

    sendWithRetry("AT+CGNSMOD=15", 1000);
    delay(500);

    sendWithRetry("AT+CGANT=0", 1000);
    delay(500);

    sendWithRetry("AT+CGNSPWR=1", 2000);
    delay(1000);
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