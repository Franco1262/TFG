#pragma once
#include "config.h"
#include "types.h"

inline void printSensorData(const Data& data) 
{
    Serial.println("\n======= REPORT DE DATOS =======");
    
    // Timestamp (Unix)
    Serial.printf("Fecha/Hora (Unix): %u\n", data.timestamp);
    
    // Values are stored as integers: temperature/humidity x100.
    Serial.printf("Temperatura:       %.2f °C\n", data.temperature / 100.0);
    Serial.printf("Humedad:           %.2f %%\n", data.humidity / 100.0);
    
    // Integros de 16 bits
    Serial.printf("Conductividad:     %d uS/cm\n", data.conductivity);
    
    // pH is stored multiplied by 10 (for example, 68 means 6.8).
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

inline void decodeData(const uint8_t (&response)[19], Data& data)
{
    // The sensor reports temperature and humidity in tenths. Store them in
    // hundredths so all Data fields remain integer-scaled.
    const int16_t raw_temperature = static_cast<int16_t>((response[4] << 8) | response[5]);
    const uint16_t raw_humidity = static_cast<uint16_t>((response[6] << 8) | response[7]);
    data.temperature  = raw_temperature * 10; // C x100
    data.humidity     = raw_humidity * 10; // % x100
    data.conductivity = ((uint16_t)response[8]  << 8) | (uint16_t)response[9]; //1µS/cm
    data.ph           = ((uint16_t)response[10] << 8) | (uint16_t)response[11]; // pH x10
    data.nitrogen     = ((uint16_t)response[12] << 8) | (uint16_t)response[13]; //1mg/kg(mg/L)
    data.phosphorus   = ((uint16_t)response[14] << 8) | (uint16_t)response[15]; //1mg/kg(mg/L)
    data.potassium    = ((uint16_t)response[16] << 8) | (uint16_t)response[17]; //1mg/kg(mg/L)
}

inline bool fakeData(Data& data)
{
    // Generate fake sensor data for testing
    data.temperature  = 2450;  // 24.50°C (stored as int16_t, divided by 100 in real use)
    data.humidity     = 6500;  // 65.00% (stored as int16_t, divided by 100)
    data.conductivity = 1200;  // 1200 µS/cm
    data.ph           = 68;    // 6.8 pH (stored as int16_t, divided by 10)
    data.nitrogen     = 150;   // 150 mg/kg
    data.phosphorus   = 80;    // 80 mg/kg
    data.potassium    = 200;   // 200 mg/kg
    // battery and timestamp are already set by the caller
    
    return true;  // Fake sensor always succeeds
}

