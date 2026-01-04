#include "Sensor.h"

#define SENSOR_READ_TIMEOUT 2000

bool Sensor::read(Data& data)
{
    uint8_t request[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
    uint8_t response[19];
    int k = 0;

    while(Serial2.available()) Serial2.read();

    digitalWrite(RS485_DIR1, HIGH); // Transmitir
    Serial2.write(request, 8);
    Serial2.flush();
    digitalWrite(RS485_DIR1, LOW); // Recibir

    unsigned long start = millis();
    while((k < 19) && ((millis() - start) < SENSOR_READ_TIMEOUT)) 
    {
        if(Serial2.available())
            response[k++] = Serial2.read();      
    }

    if(k < 19)
        return false;

    if (response[0] != 0x01 || response[1] != 0x03) 
        return false;

    decodeData(response, data);
    return true;
}


void Sensor::decodeData(const uint8_t (&response)[19], Data& data)
{
    data.temperature  = (((uint16_t)response[4] << 8) | (uint16_t)response[5]) / 10.0; //C
    data.humidity     = (((uint16_t)response[6] << 8) | (uint16_t)response[7]) / 10.0; //%
    data.conductivity = ((uint16_t)response[8]  << 8) | (uint16_t)response[9]; //1µS/cm
    data.ph           = (((uint16_t)response[10] << 8) | (uint16_t)response[11]) / 10; //
    data.nitrogen     = ((uint16_t)response[12] << 8) | (uint16_t)response[13]; //1mg/kg(mg/L)
    data.phosphorus   = ((uint16_t)response[14] << 8) | (uint16_t)response[15]; //1mg/kg(mg/L)
    data.potassium    = ((uint16_t)response[16] << 8) | (uint16_t)response[17]; //1mg/kg(mg/L)
}