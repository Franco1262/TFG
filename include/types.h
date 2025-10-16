#pragma once
#include <Arduino.h>

struct __attribute__((packed)) Data {
    uint32_t timestamp;
    int16_t temperature;
    uint16_t humidity;
    uint16_t conductivity;
    uint16_t ph;
    uint16_t nitrogen;
    uint16_t phosphorus;
    uint16_t potassium;
    uint8_t battery;
};

struct LoRaSessionState {
    bool is_joined;
    uint32_t netid;
    uint32_t devaddr;
    uint8_t nwkKey[16];
    uint8_t artKey[16];
    uint32_t seqnoUp;
    uint32_t seqnoDn;
};