#include "Sensor.h"


void Sensor::turnOn()
{
    pinMode(RS485_DIR1, OUTPUT);
    digitalWrite(RS485_DIR1, LOW);

    Serial2.begin(9600, SERIAL_8N1, RS485_RXD, RS485_TXD);
    delay(100);
}


void Sensor::turnOff()
{
    Serial2.end();
    digitalWrite(RS485_DIR1, LOW);
}


bool Sensor::read(Data& data)
{
    uint8_t request[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
    uint8_t response[19];
    int k = 0;

    while(Serial2.available()) Serial2.read();

    digitalWrite(RS485_DIR1, HIGH);
    Serial2.write(request, 8);
    Serial2.flush();
    digitalWrite(RS485_DIR1, LOW);

    unsigned long start = millis();
    while((k < 19) && ((millis() - start) < SENSOR_READ_TIMEOUT)) 
    {
        if(Serial2.available())
            response[k++] = Serial2.read();
        yield(); 
    }

    if(k < 19)
        return false;

    if (response[0] != 0x01 || response[1] != 0x03) 
        return false;

    decodeData(response, data);
    return true;
}