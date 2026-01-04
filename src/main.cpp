#include "Power.h"
#include "Clock.h"
#include "Sensor.h"
#include "Storage.h"
#include "utilities.h"

#define N_DATA_TO_SEND 10
RTC_DATA_ATTR static uint8_t writes_counter = 0;


void printSensorData(const Data& data) 
{
    Serial.println("\n======= REPORT DE DATOS =======");
    
    // Timestamp (Unix)
    Serial.printf("Fecha/Hora (Unix): %u\n", data.timestamp);
    
    // Floats con 2 decimales
    Serial.printf("Temperatura:       %.2f °C\n", data.temperature);
    Serial.printf("Humedad:           %.2f %%\n", data.humidity);
    
    // Integros de 16 bits
    Serial.printf("Conductividad:     %d uS/cm\n", data.conductivity);
    
    // El pH suele venir de Modbus multiplicado por 10 (ej: 70 = 7.0)
    // Si tu sensor ya lo devuelve decodificado, quita el "/ 10.0"
    Serial.printf("pH:                %.1f\n", data.ph / 10.0);
    
    // Nutrientes NPK
    Serial.println("--- Nutrientes (NPK) ---");
    Serial.printf("  Nitrógeno (N):   %d mg/kg\n", data.nitrogen);
    Serial.printf("  Fósforo (P):     %d mg/kg\n", data.phosphorus);
    Serial.printf("  Potasio (K):     %d mg/kg\n", data.potassium);
    
    // Batería
    Serial.printf("===============================\n");
    Serial.printf("Batería:           %u %%\n", data.battery);
    Serial.println("===============================\n");
}


void setup() 
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    Serial2.begin(4800, SERIAL_8N1, RS485_RXD, RS485_TXD);

    Power::Begin();
    bool sdOk = Storage::Begin();
    
    if(Power::isFirstBoot() || Clock::needsDailySync())
        Clock::syncWithGPS();
    
    Data data;
    data.timestamp = Clock::getTime();
    data.battery = Power::getBattPercent();
    
    if(Sensor::read(data))
    {
        if(sdOk)
        {
            Storage::writeToDailyLog(data);
            Storage::writeToPending(data);
            writes_counter++;

            if(writes_counter >= N_DATA_TO_SEND)
            {
                writes_counter = 0;
                //Lora::sendBatch();
                Storage::clearPending();
            }
        }
        else
            ;
            //Lora::sendSingle(data);
    }

    Power::enterDeepSleep();
}

void loop()
{

}
