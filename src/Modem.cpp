#include "Modem.h"
#include "config.h"


bool Modem::turnOn()
{
    char buffer[256];

    // --- Initialize Serial1 for modem communication ---
    Serial1.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(1000);

    // --- Power on modem hardware ---
    pinMode(MODEM_PWR, OUTPUT);
    digitalWrite(MODEM_PWR, LOW);
    delay(100);
    digitalWrite(MODEM_PWR, HIGH);
    delay(1000);
    digitalWrite(MODEM_PWR, LOW);

    // --- Wait for modem to respond to AT commands ---
    int retry = 0;
    int total_attempts = 0;
    while (!sendAtAndRead("AT", buffer, sizeof(buffer), 1000) && total_attempts < 10)
    {
        total_attempts++;
        if (retry++ > 10)
        {
            digitalWrite(MODEM_PWR, HIGH);
            delay(1000);
            digitalWrite(MODEM_PWR, LOW);
            retry = 0;
        }
    }

    // --- Initialize modem in GSM mode ---
    int retries = 3;
    auto sendWithRetry = [&](const char* cmd, unsigned long timeout_ms, int delay_ms = 200) -> bool 
    {
        for(int i = 0; i < retries; i++) 
        {
            if(sendAtAndRead(cmd, buffer, sizeof(buffer), timeout_ms))
                return true;
            delay(delay_ms);
        }
        return false;
    };

    sendWithRetry("AT+CFUN=1", 2000);
    delay(1000);

    if (total_attempts >= 10)
        return false;

    return true;
}

void Modem::turnOff()
{
    char buffer[256];
    sendAtAndRead("AT+CPOWD=1", buffer, sizeof(buffer), 1000);
    Serial1.end();
}

bool Modem::enableGps()
{
    char buffer[256];

    sendAtAndRead("AT+CGNSPWR=0", buffer, sizeof(buffer), 1000);
    delay(500);

    if (sendAtAndRead("AT+CGNSPWR=1", buffer, sizeof(buffer), 2000)) {
        return true;
    }

    return false;
}

void Modem::disableGps()
{
    char buffer[256];
    sendAtAndRead("AT+CGNSPWR=0", buffer, sizeof(buffer), 1000);
}


int Modem::getGnssField(const char* response, int n_field, char* result, int result_size)
{
    const char* prefix = "+CGNSINF: ";
    const char* data = strstr(response, prefix);
    if(data) data += strlen(prefix);
    else return 0;

    for(int i = 0; i < n_field; i++)
    {
        data = strchr(data, ',');
        if(!data) return 0;
        data++;
    }

    const char* end = strchr(data, ',');
    if(!end) end = strchr(data, '\r');

    if(end)
    {
        uint8_t len = end - data;
        uint8_t copy_size = (len >= result_size) ? (result_size - 1) : len;
        strncpy(result, data, copy_size);
        result[copy_size] = '\0';

        return (int)copy_size;
    }

    return 0;
}


bool Modem::sendAtAndRead(const char* cmd, char* buffer, int buffer_size, unsigned long timeout_ms)
{
    if (!buffer || buffer_size <= 0) return false;

    while (Serial1.available()) Serial1.read();
    memset(buffer, 0, buffer_size);

    Serial1.print(cmd);
    Serial1.print("\r\n");

    int index = 0;
    unsigned long start = millis();

    while ((millis() - start) < timeout_ms)
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            if (index < buffer_size - 1)
            {
                buffer[index++] = c;
                buffer[index] = '\0';
            }
        }

        if (strstr(buffer, "OK\r\n") || strstr(buffer, "ERROR\r\n"))
        {
            break; 
        }

        yield();
    }

    return (strstr(buffer, "OK\r\n") != NULL);
}