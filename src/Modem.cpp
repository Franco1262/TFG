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


bool Modem::sendAtAndRead(const char* cmd, char* buffer, int buffer_size, unsigned long timeout_ms) 
{
    while (Serial1.available()) Serial1.read();
    Serial1.print(cmd);
    Serial1.print("\r\n");

    char dummy_buffer[32]; 
    char* target_buf = (buffer != nullptr) ? buffer : dummy_buffer;
    int target_size = (buffer != nullptr) ? buffer_size : sizeof(dummy_buffer);
    
    int index = 0;
    unsigned long t = millis();
    while (millis() - t < timeout_ms) 
    {
        while(Serial1.available())
        {
            char c = Serial1.read();

            if(index < (target_size - 1))
            {
                target_buf[index++] = c;
                target_buf[index] = '\0';
            }

            else if (buffer == nullptr) 
            {
                for(int i = 0; i < target_size - 2; i++) target_buf[i] = target_buf[i+1];
                target_buf[target_size - 2] = c;
                target_buf[target_size - 1] = '\0';
            }
        }

        if (strstr(target_buf, "OK") || strstr(target_buf, "ERROR")) 
            return true;

        yield();
    }

    return false;
}