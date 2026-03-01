#include "Modem.h"


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


bool Modem::sendAtAndRead(const char* cmd,
                          char* buffer,
                          int buffer_size,
                          unsigned long timeout_ms)
{
    if (!buffer || buffer_size <= 0)
        return false;

    while (Serial1.available())
        Serial1.read();

    memset(buffer, 0, buffer_size);

    Serial1.print(cmd);
    Serial1.print("\r\n");

    int index = 0;
    unsigned long start = millis();
    unsigned long last_rx = millis();

    while ((millis() - start) < timeout_ms)
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            last_rx = millis();

            if (index < buffer_size - 1)
            {
                buffer[index++] = c;
                buffer[index] = '\0';
            }
        }

        if (strstr(buffer, "\r\nOK\r\n") ||
            strstr(buffer, "\r\nERROR\r\n"))
        {
            break;
        }

        yield();
    }

    while ((millis() - last_rx) < 50)
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            last_rx = millis();

            if (index < buffer_size - 1)
            {
                buffer[index++] = c;
                buffer[index] = '\0';
            }
        }
        yield();
    }

    if (strstr(buffer, "\r\nOK\r\n"))
        return true;

    if (strstr(buffer, "\r\nERROR\r\n"))
        return false;

    return false;
}